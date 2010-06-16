using System;
using System.Windows.Forms;
using System.IO;
using System.IO.Ports;
using System.Diagnostics;


namespace RobobuilderLib
{
    public partial class Basic_frm : Form
    {
        Basic       compiler;
        SerialPort  s;

        Process p;

        bool readyDownload = false;

        string rx = ".";

        bool bm = false;

        public Basic_frm()
        {
            InitializeComponent();
            readyDownload = false;

            compiler = new Basic();
            s = new SerialPort(comPort.Text, 115200);
            s.DataReceived += new SerialDataReceivedEventHandler(s_DataReceived);

            output.KeyPress += new KeyPressEventHandler(output_KeyPress);
        }

        void output_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (bm) return;

            rx = e.KeyChar.ToString();
            if (s.IsOpen)
            {
                s.Write(rx);
            }

            if (p != null)
            {
            //    sw.Write(rx);
            } 
        }


        private void processCompletedOrCanceled(object sender, EventArgs e)
        {
            button1.BackColor = System.Drawing.Color.Red;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (comPort.Text == "!")
            {
                if (button1.BackColor == System.Drawing.Color.Blue)
                {
                    button1.BackColor = System.Drawing.Color.Red;
                    p.Kill();
                    p.Close();
                    return;
                }

                // connect to basic.exe
                // and simulate connection
                p = new Process();
                p.StartInfo.FileName = "basic.exe";
                //p.StartInfo.RedirectStandardInput = true;
                p.StartInfo.RedirectStandardOutput = true;
                p.StartInfo.UseShellExecute = false;
                button1.BackColor = System.Drawing.Color.Blue;
                p.Start();

                //p.StandardInput.Write("test");
                string t = p.StandardOutput.ReadToEnd() ;
                //Console.WriteLine(t);

                return;
            }

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
            MessageBox.Show("Basic IDE (v 0.1)\r\n$Revsion$\r\nby l3v3rz","About ... ",MessageBoxButtons.OK);
        }

        private void compileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (compiler.Compile(input.Text))
            {
                output.Text = "Complete - ready to download\r\n";
                output.Text += compiler.Dump();
                readyDownload = true;
            }
            else
            {
                output.Text = "error on line " + compiler.lineno + " : " + compiler.error_msgs[compiler.errno] + "\r\n";
                output.Text += compiler.Dump();
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

            try
            {
                binxfer btf = new binxfer(s);
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

        }

    }
}
