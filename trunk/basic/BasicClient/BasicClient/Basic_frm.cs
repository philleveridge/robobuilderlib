using System;
using System.Windows.Forms;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace RobobuilderLib
{
    public partial class Basic_frm : Form
    {
        Basic       compiler;
        SerialPort  s;
        binxfer btf;
        ServoSim sim = null;

        bool readyDownload = false;

        string version = "$Revision$";  //$Revision$

        string rx = ".";

        bool bm = false;

        public Basic_frm()
        {
            InitializeComponent();
            readyDownload = false;

            if (!File.Exists("Basic.exe"))
            {
                startBasiclocalToolStripMenuItem.Visible = false;
                simulatorToolStripMenuItem.Visible = false;
            }

            compiler = new Basic();
            s = new SerialPort(comPort.Text, 115200);
            s.DataReceived += new SerialDataReceivedEventHandler(s_DataReceived);

            output.KeyPress += new KeyPressEventHandler(output_KeyPress);

            if (version.StartsWith("$Revision: "))
                version = version.Substring(11, 4);

            this.Text += version;

            syntaxcheck(); //!
        }

        void output_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (bm) return;

            rx = e.KeyChar.ToString();
            if (s.IsOpen)
            {
                s.Write(rx);
            }
        }

        private void processCompletedOrCanceled(object sender, EventArgs e)
        {
            button1.BackColor = System.Drawing.Color.Red;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                if (s.IsOpen)
                {
                    s.Close();
                    button1.BackColor = System.Drawing.Color.Red;
                }
                else
                {
                    s.PortName = comPort.Text;
                    s.Open();
                    button1.BackColor = System.Drawing.Color.Green;
                }
            }
            catch
            {
                MessageBox.Show("Error - can't connect to serial port - " + comPort.Text, "Error", MessageBoxButtons.OK);
            }
        }

        void p_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            rx = e.Data;
            this.Invoke(new EventHandler(DisplayText));
        }

        private void DisplayText(object sender, EventArgs e)
        {
            if (rx != null) output.AppendText(rx);
        }

        void s_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            //throw new NotImplementedException();
            if (bm) return;

            switch (e.EventType)
            {
                case SerialData.Chars:
                    rx = s.ReadExisting();
                    this.Invoke(new EventHandler(DisplayText));
                    break;
                case SerialData.Eof:
                    rx = "@@";
                    this.Invoke(new EventHandler(DisplayText));
                    break;
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog s = new SaveFileDialog();
            if (s.ShowDialog() != DialogResult.OK)
                return;
            try
            {
                File.WriteAllText(s.FileName, input.Text);
                fname.Text = s.FileName;
            }
            catch (Exception e1)
            {
                MessageBox.Show("can't save file - " + e1.Message);
                fname.Text = "";
            }
        }

        private void loadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog s = new OpenFileDialog();
            if (s.ShowDialog() != DialogResult.OK)
                return;
            try
            {
                input.Text = File.ReadAllText(s.FileName);
                output.Text = "";
                //download_btn.Enabled = false;
                fname.Text = s.FileName;

                syntaxcheck(); //!
            }
            catch (Exception e1)
            {
                MessageBox.Show("can't open file - " + e1.Message);
                output.Text = "";
                //download_btn.Enabled = false;
                fname.Text = "";
            }
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutBox1 n = new AboutBox1(version);
            n.Show();
        }

        private void syntaxcheck()
        {
            int n;
            input.SelectAll();
            input.SelectionColor = Color.Black;

            foreach (string s in Basic.tokens)
            {
                n = -1;
                while ((n = input.Find(s, n + 1, RichTextBoxFinds.WholeWord)) > 0)
                    input.SelectionColor = Color.Red;
            }
            n = -1;
            while ((n = input.Find("TO", n + 1, RichTextBoxFinds.WholeWord)) > 0)
                input.SelectionColor = Color.Red;

            foreach (string s in Basic.specials)
            {
                n = -1;
                while ((n = input.Find("$" + s, n + 1, RichTextBoxFinds.WholeWord)) > 0)
                    input.SelectionColor = Color.Blue;
            }
            int sc = -1; int cc = -1;
            for (int c = 0; c < input.Text.Length; c++)
            {
                char ch = input.Text[c];
                if (ch == '\"' && sc < 0)
                {
                    sc = c;
                    continue;
                }
                if (ch == '\'' && cc < 0)
                {
                    cc = c;
                    continue;
                }

                if (ch == '\"' && sc >= 0)
                {
                    input.SelectionStart = sc;
                    input.SelectionLength = c - sc + 1;
                    input.SelectionColor = Color.Green;
                    sc = -1;
                }

                if (ch == 10 && cc >= 0)
                {
                    input.SelectionStart = cc;
                    input.SelectionLength = c - cc;
                    input.SelectionColor = Color.Gray;
                    cc = -1;
                }
            }
        }

        private void compileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            syntaxcheck();
            if (compiler.Compile(input.Text))
            {
                MessageBox.Show(String.Format("Complete - ready to download [{0} Bytes]",compiler.Download().Length/2), "Compiler");
                output.Text += compiler.Dump();
                readyDownload = true;
            }
            else
            {
                output.Text = "Error on line " + compiler.lineno + " : " + compiler.error_msgs[compiler.errno] + "\r\n";
                output.Text = "Error ::      " + compiler.curline;              

                readyDownload = false;
            }
        }

        private void downloadToolStripMenuItem_Click(object sender, EventArgs e)
        {

            if (!readyDownload) // if not already built
            {
                compileToolStripMenuItem_Click(sender, e);
                if (!readyDownload)
                {
                    return; // if failed return
                }
            }

            string c = compiler.Download();
            bm = true;

            if (!s.IsOpen)
            {
                Console.WriteLine("com port not open ..."); 
                File.WriteAllText("bindata.txt", c);
                return;
            }

            progressBar1.Visible = true;
            progressBar1.Value = 0;
            timer1.Enabled = true;

            try
            {
                btf = new binxfer(s);
                btf.dbg = true;
                btf.send_msg_raw('l', c);

                if (btf.recv_packet())
                {
                    bm = false;
                    MessageBox.Show("Download ok");
                }
                else
                {
                    Console.WriteLine("download failed");
                    bm = false;
                }

            }
            catch (Exception err)
            {
                MessageBox.Show("Download failed - connection problem" + err.Message);
            }

            timer1.Enabled = false;
            progressBar1.Visible = false;

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (btf != null && progressBar1.Visible)
            {
            
                progressBar1.Increment((int)(100.0*btf.progress));
            }
            this.Update();
        }

        private void simulatorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (sim == null) sim = new ServoSim();
            //sim.ShowDialog();
            sim.Show();
        }

        private void startBasiclocalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (File.Exists("Basic.exe"))
            {
                Process.Start("Basic.exe");
            }
        }


    }
}
