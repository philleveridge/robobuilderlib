using System;
using System.Windows.Forms;
using System.IO;
using System.IO.Ports;

namespace RobobuilderLib
{
    public partial class Basic_frm : Form
    {
        Basic       compiler;
        SerialPort  s;

        string rx = ".";

        bool bm = false;

        public Basic_frm()
        {
            InitializeComponent();
            download_btn.Enabled = false;

            compiler = new Basic();
            s = new SerialPort(comPort.Text, 115200);
            s.DataReceived += new SerialDataReceivedEventHandler(s_DataReceived);

            output.KeyPress += new KeyPressEventHandler(output_KeyPress);
        }

        void output_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (bm) return;
            //throw new NotImplementedException();
            rx = e.KeyChar.ToString();
            if (s.IsOpen)
            {
                s.Write(rx);
            }
            //this.Invoke(new EventHandler(DisplayText));     //echo      
        }

        private void close_btn_Click(object sender, EventArgs e)
        {
            this.Hide();
        }

        private void compile_btn_Click(object sender, EventArgs e)
        {
            if (compiler.Compile(input.Text))
            {
                output.Text = "Complete - ready to download\r\n";
                output.Text += compiler.Dump();
                download_btn.Enabled = true;
            }
            else
            {
                output.Text = "error on line " + compiler.lineno + " : " + compiler.error_msgs[compiler.errno] + "\r\n";
                output.Text += compiler.Dump();
                download_btn.Enabled = false;
            }
        }

        private void download_btn_Click(object sender, EventArgs e)
        {
            string c= compiler.Download();
            bm = true;

            try
            {

                binxfer btf = new binxfer(s);
                btf.dbg = true;
                btf.send_msg_raw('l', c);

                if (btf.recv_packet())
                {
                    MessageBox.Show("Download ok");
                    output.Text = "Complete - Ready to run";
                }
                else
                {
                    Console.WriteLine("download failed");
                }

            }
            catch (Exception err)
            {
                MessageBox.Show("Download failed - connection problem" + err.Message);
            }
            bm = false;
        }

        private void load_btn_Click(object sender, EventArgs e)
        {
            OpenFileDialog s = new OpenFileDialog();
            if (s.ShowDialog() != DialogResult.OK)
                return;
            try
            {
                input.Text = File.ReadAllText(s.FileName);
                output.Text = "";
                download_btn.Enabled = false;
                fname.Text = s.FileName;
            }
            catch (Exception e1)
            {
                MessageBox.Show("can't open file - " + e1.Message);
                output.Text = "";
                download_btn.Enabled = false;
                fname.Text = "";
            }
        }

        private void save_btn_Click(object sender, EventArgs e)
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

        private void button1_Click(object sender, EventArgs e)
        {
            if (s.IsOpen)
            {
                s.Close();
                button1.BackColor = System.Drawing.Color.Red;
            }
            else
            {
                s.Open();
                button1.BackColor = System.Drawing.Color.Green;
            }
        }

        private void DisplayText(object sender, EventArgs e)
        {
            output.AppendText(rx);
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

    }
}
