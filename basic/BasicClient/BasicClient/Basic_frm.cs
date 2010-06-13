using System;
using System.Windows.Forms;
using System.IO;
using System.IO.Ports;

namespace RobobuilderLib
{
    public partial class Basic_frm : Form
    {
        Basic       compiler;
        PCremote    pcr;
        SerialPort  s;

        string rx = ".";

        public Basic_frm()
        {
            InitializeComponent();
            download_btn.Enabled = false;

            s = new SerialPort(comPort.Text, 115200);
            s.DataReceived += new SerialDataReceivedEventHandler(s_DataReceived);

            output.KeyPress += new KeyPressEventHandler(output_KeyPress);
        }

        void output_KeyPress(object sender, KeyPressEventArgs e)
        {
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

            try
            {
                pcr = new PCremote(s);

                if (pcr == null || pcr.download_basic(c) == false)
                {
                    MessageBox.Show("Download failed");
                }
                else
                {
                    MessageBox.Show("Download ok");
                    output.Text = "Complete - Ready to run";
                }

                pcr.close();
            }
            catch (Exception err)
            {
                MessageBox.Show("Download failed - connection problem" + err.Message);
            }
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
            compiler = new Basic();
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
