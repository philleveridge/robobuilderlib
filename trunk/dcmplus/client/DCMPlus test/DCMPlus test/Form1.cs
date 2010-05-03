using System;
using System.IO.Ports;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        SerialPort p = new SerialPort("COM5", 115200);

        public Form1()
        {
            InitializeComponent();

            p.ReadTimeout  = 500;
            p.WriteTimeout = 500;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int b1, b2;
            if (!p.IsOpen) p.Open();

            int sn = Convert.ToInt32(servoid.Text);

            sn = (5 << 5 | (sn % 31));

            byte d = (byte)Convert.ToInt32(data1.Text);

            byte cs = (byte)((sn ^ d) & 0x7f);

            p.Write(new byte[] { 0xFF, (byte)sn, d, cs }, 0, 4);

            textBox1.AppendText(String.Format("Send: {0:X2} {1:X2} {2:X2} {3:X2}\r\n", 0xFF, sn, d, cs));

            try
            {
                b1 = p.ReadByte();
                textBox1.AppendText(String.Format("Recvd: {0:X2}\r\n", b1));

                b2 = p.ReadByte();
                textBox1.AppendText(String.Format("Recvd : {0:X2}\r\n", b2));

            }
            catch
            {
                textBox1.AppendText("Fail\r\n");

            }
        }
    }
}
