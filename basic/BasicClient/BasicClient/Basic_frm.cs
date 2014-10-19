using System;
using System.Windows.Forms;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Diagnostics;
using System.Text.RegularExpressions;
using System.Net;
using System.Text;
using System.Runtime.InteropServices;

using System.Threading;

using CPI.Plot3D;

// Reference path for the following assemblies --> C:\Program Files\Microsoft Expression\Encoder 4\SDK\
using Microsoft.Expression.Encoder.Devices;
using Microsoft.Expression.Encoder.Live;
using Microsoft.Expression.Encoder;

namespace RobobuilderLib
{
    public partial class Basic_frm : Form
    {

        [DllImport(@"basic_lib.dll", EntryPoint = "basic_api", CallingConvention = CallingConvention.Cdecl)]
        public static extern void basic_api(); 

        [DllImport(@"basic_lib.dll", EntryPoint = "read_mem", CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_mem(int a);

        [DllImport(@"basic_lib.dll", EntryPoint = "set_mem", CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_mem(int a, int b); 

        [DllImport(@"basic_lib.dll", EntryPoint = "read_servo", CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_servo(int a);

        [DllImport(@"basic_lib.dll", EntryPoint = "set_servo", CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_servo(int a, int b);

        [DllImport(@"basic_lib.dll", EntryPoint = "basic_setibuf", CallingConvention = CallingConvention.Cdecl)]
        public static extern void basic_setibuf(string s);

        [DllImport(@"basic_lib.dll", EntryPoint = "basic_getobuf", CallingConvention = CallingConvention.Cdecl)]
        public static extern void basic_getobuf(StringBuilder s, int len);

        [DllImport(@"basic_lib.dll", EntryPoint = "basic_stop", CallingConvention = CallingConvention.Cdecl)]
        public static extern void basic_stop();

        bool rlbasic = false;

        servoUC[] sv;
        int svp;

        Basic       compiler;
        SerialPort  s;
        binxfer btf;
        ServoSim sim = null;

        RobotModel rm;

        bool readyDownload = false;
        string bfn = "basic.exe";
        string bdn = "bindata.txt";
        string cprt = "";

        string version = "$Revision$";  

        string rx = ".";

        bool bm = false;

        bool fmono = false;

        bool editfl = false;

        Graphics gr;

        public static bool IsLinux
        {
            get
            {
                int p = (int)Environment.OSVersion.Platform;
                return (p == 4) || (p == 6) || (p == 128);
            }
        }

        public Basic_frm()
        {
            string fn = "";
            InitializeComponent();
            readyDownload = false;

            this.Width = (groupBox1.Visible) ? 1028 : 868;

            if (File.Exists("BC.ini"))
            {
                string[] r = File.ReadAllLines("BC.ini");
                foreach (string l in r)
                {
                    if (l.StartsWith("BASIC=") && l.Substring(6)!="" )  bfn     = l.Substring(6);
                    if (l.StartsWith("BIN=") && l.Substring(4)!="")     bdn     = l.Substring(4);
                    if (l.StartsWith("COM="))                           cprt    = l.Substring(4);
                    if (l.StartsWith("FILE="))                          fn      = l.Substring(5);
                    if (l.StartsWith("MONO="))                          fmono   = (l.Substring(5, 1).ToUpper() == "Y") ? true : false;
                }
            }

            if (IsLinux && fmono == false) fmono = true; //overide user setting only if actually on linux

            //if (!File.Exists(bfn))
            //{
            simulatorToolStripMenuItem.Visible = false;
            //}

            if (File.Exists(fn))
            {
                input.Text = File.ReadAllText(fn);
                output.Text = "";
                fname.Text = fn;
            }

            comselect.Items.Clear();
            foreach (string s2 in System.IO.Ports.SerialPort.GetPortNames())
            {
                    string s1 = s2;
                    if (!Char.IsDigit(s1[s1.Length - 1]))
                    {
                        Debug.Print("? [" + s1[s1.Length - 1]+"]");
                         s1 = s1.Substring(0, s1.Length-1);
                    }
                    Debug.Print("com port = [" + s1 + "]");
                    comselect.Items.Add(s1);
            }

            if (comselect.Items.Count <= 0)
            {
                MessageBox.Show("No serial ports detected");
            }

            if (comselect.Items.Count == 1)
            {
                comselect.SelectedIndex = 0;
                comPort.Text = (string)comselect.Items[0];
            }

            if (comselect.Items.Count > 1)
            {
                comPort.Text = cprt;
            }

            if (cprt!="") s = new SerialPort(comPort.Text, 115200);

            compiler = new Basic();


            term.KeyPress += new KeyPressEventHandler(output_KeyPress);

            if (version.StartsWith("$Revision: "))
                version = version.Substring(11, 4);

            this.Text += version;
            if (fmono) this.Text += "[Mono]";
            input.Select();

            syntaxcheck(); //!
            editfl = false;

            sv = new servoUC[32];
            for (int i = 0; i < 32; i++) 
            {
                sv[i] = new servoUC();
                sv[i].id = i; 
                sv[i].val = 127;
                sv[i].pm = false;
            }
            svp = 0;
            copyfn();

            clear();

            rm = new RobotModel(pictureBox2.CreateGraphics());
            rm.UpdateDisplay();
        }

        void writeini()
        {
            string s = "#Autogenerated\n";
            s += "FILE=" + fname.Text + "\n";
            s += "COM=" + comPort.Text + "\n";
            s += "BIN=" + bdn + "\n";
            s += "BASIC=" + bfn + "\n";
            s += "MONO=" + (fmono?"Y":"N") + "\n";

            File.WriteAllText("BC.ini", s);
        }

        void output_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(e.KeyChar.ToString());
            }
        }

        private void runBtn_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write("r");
            }
        }

        private void listBtn_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write("l");
            }

        }

        private void stopBtn_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(Char.ToString((char)27));
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
                if (s!=null && s.IsOpen)
                {
                    s.Close();
                    button1.BackColor = System.Drawing.Color.Red;
                    s.DataReceived -= new SerialDataReceivedEventHandler(s_DataReceived);

                    runBtn.Visible = false;
                    listBtn.Visible = false;
                    stopBtn.Visible = false;
                    groupBox1.Visible = false;
                }
                else
                {
                    s.PortName = comPort.Text;
                    s.ReadTimeout = 250;
                    s.Open();
                    s.Write("V");
                    s.ReadTo("v=");
                    string v = s.ReadLine();

                    if (!v.StartsWith("$Rev")) 
                        throw new Exception("Not BASIC firmware?");

                    label1.Text = "FIRMWARE " + v;
                    label1.Visible = true;

                    if (Convert.ToInt32(v.Substring(11, 3)) < Basic.REQ_FIRMWARE) 
                        throw new Exception("BASIC firmware v" + Basic.REQ_FIRMWARE + " or better needed?");

                    if (!fmono) 
                        s.DataReceived += new SerialDataReceivedEventHandler(s_DataReceived);

                    button1.BackColor = System.Drawing.Color.Green;
                    term.Text = "";
                    term.Select();

                    runBtn.Visible = true;
                    listBtn.Visible = true;
                    stopBtn.Visible = true; 
                    groupBox1.Visible = true;

                    if (fmono)
                    {
                        readData();

                        if (s.IsOpen)
                        {
                            s.Close();
                        }
                        button1.BackColor = System.Drawing.Color.Red;
                        runBtn.Visible = false;
                        listBtn.Visible = false;
                        stopBtn.Visible = false;
                        return;
                    }
                }
            }
            catch (Exception em)
            {
                MessageBox.Show("Error - can't connect to serial port - " + comPort.Text + "\r\nExeception=" + em.Message, "Error", MessageBoxButtons.OK);
            }
        }

        void readData()
        {
            // mono read routine - doesn't handle serial events
            s.ReadTimeout = 5;
            int b;
            rx = "";

            while (s.IsOpen)
            {
                if (!bm)
                {
                    try
                    {
                        if ((b = s.ReadByte()) > 0)
                        {
                            while (b > 0 && b != 255)
                            {
                                //Console.WriteLine("{0} - {1}", b, rx);
                                rx += Char.ToString((char)b);
                                b = s.ReadByte();
                            }

                        }
                    }
                    catch
                    {
                    }
                    DisplayText(null, null);
                    rx = "";
                }
                Application.DoEvents();
            }


        }

        void p_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            rx = e.Data;
            this.Invoke(new EventHandler(DisplayText));
        }

        private void DisplayText(object sender, EventArgs e)
        {
            if (rx != null && rx != "")
            {
                if (rx == "\b")
                {
                    term.Text = term.Text.Substring(0, term.Text.Length - 1);
                    term.SelectionStart = term.Text.Length;
                }
                else
                    term.AppendText(rx);

                rx = term.Text;

                if (rx.IndexOf(Char.ToString((char)27) + "[2J") >= 0)
                {
                    rx = rx.Substring(rx.LastIndexOf(Char.ToString((char)27) + "[2J") + 4);
                    term.Text = rx;
                }
            }
            rx = "";
        }

        void s_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
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

        private void loadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog s = new OpenFileDialog();
            s.AddExtension = true;
            s.Filter = "Basic (*.rbas)|*.rbas";
            if (s.ShowDialog() != DialogResult.OK)
                return;
            try
            {
                input.Text = File.ReadAllText(s.FileName);
                output.Text = "";
                //download_btn.Enabled = false;
                fname.Text = s.FileName;

                syntaxcheck(); //!

                editfl = false;
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

            foreach (string s in Basic.specials)
            {
                n = -1;
                while ((n = input.Find("$" + s, n + 1, RichTextBoxFinds.WholeWord)) > 0)
                {
                    if (input.SelectionColor == Color.Black)
                    {
                        input.SelectionColor = Color.Blue;
                        input.SelectedText = input.SelectedText.ToUpper();
                    }
                }
            }

            foreach (string s in Basic.tokens)
            {
                n = -1;
                while ((n = input.Find(s, n + 1, RichTextBoxFinds.WholeWord)) >= 0)
                {
                    if (input.SelectionColor == Color.Black)
                    {
                        input.SelectionColor = Color.Red;
                        input.SelectedText = input.SelectedText.ToUpper();
                    }
                }
            }
            n = -1;
            while ((n = input.Find("TO", n + 1, RichTextBoxFinds.WholeWord)) > 0)
            {
                if (input.SelectionColor == Color.Black)
                {
                    input.SelectionColor = Color.Red;
                    input.SelectedText = input.SelectedText.ToUpper();
                }
            }

        }

        private void compileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            syntaxcheck();
            if (compiler.Compile(input.Text))
            {
                MessageBox.Show(String.Format("Complete - ready to download [{0} Bytes = {1}% Used]", compiler.Download().Length / 2, (50*compiler.Download().Length / compiler.MAX_PROG_SPACE)), "Compiler");
                output.Text = compiler.precomp + "\r\n";
                output.Text += compiler.Dump();
                readyDownload = true;
                File.WriteAllText(bdn, compiler.Download());
            }
            else
            {
                MessageBox.Show("Failed to compile - see output for error");
                output.Text = "Error on line " + compiler.lineno + " : " + compiler.error_msgs[compiler.errno] + "\r\n";
                output.Text += "Line ::      " + compiler.curline;              

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
                MessageBox.Show("Please connect first");
                return;
            }

            progressBar1.Visible = true;
            progressBar1.Value = 0;
            timer1.Enabled = true;

            try
            {
                s.Write(".z"); // Auto set up download mode

                btf = new binxfer(s);
                btf.dbg = false;
                btf.send_msg_raw('l', c);

                if (btf.recv_packet())
                {
                    bm = false;
                    MessageBox.Show("Download ok");
                }
                else
                {
                    output.Text += "download failed";
                    bm = false;
                }

            }
            catch (Exception err)
            {
                MessageBox.Show("Download failed - connection problem" + err.Message);
                output.Text += "Download failed - connection problem" + err.Message;
            }

            timer1.Enabled = false;
            progressBar1.Visible = false;

            s.ReadTimeout = 50;
            s.WriteTimeout = 50;

        }

        private void simulatorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (sim == null)
            {
                sim = new ServoSim();
                sim.bfn = bfn;
                sim.Disposed += new EventHandler(sim_Disposed);
            }
            sim.Show();
        }

        void sim_Disposed(object sender, EventArgs e)
        {
            sim = null;
        }


        void Basic_frm_SizeChanged(object sender, System.EventArgs e)
        {
            tabControl1.Width = this.Width - 18;
            tabControl1.Height = this.Height - 18;

            input.Width = tabControl1.Width-16;
            output.Width = tabControl1.Width-16;
            term.Width = tabControl1.Width-16;

            input.Height = tabControl1.Height - 100;
            output.Height = tabControl1.Height - 80;
            term.Height = tabControl1.Height - 100;

            fname.Width = input.Width;
            fname.Location = new Point (0, input.Height + 5);

            progressBar1.Location = new Point(progressBar1.Location.X, term.Height + 5);
            comPort.Location = new Point(comPort.Location.X, term.Height + 5);
            label1.Location = new Point(label1.Location.X, term.Height + 5);
            comselect.Location = new Point(comselect.Location.X, term.Height + 5);
            button1.Location = new Point(button1.Location.X, term.Height + 5);

            runBtn.Location = new Point(runBtn.Location.X, term.Height + 5);
            stopBtn.Location = new Point(stopBtn.Location.X, term.Height + 5);
            listBtn.Location = new Point(listBtn.Location.X, term.Height + 5);
        
        }

        
        void input_MouseHover(object sender, System.EventArgs e)
        {
            if (input.SelectedText != "")
            {
                string h = "";
                if (compiler.help.Contains(input.SelectedText.Trim()))
                {
                    h=(string)compiler.help[input.SelectedText.Trim()];
                    helptext.Text = h;
                    helptext.Visible = true;
                }
            }
            else
            {
                helptext.Visible = false;
            }
        }

        void input_MouseLeave(object sender, System.EventArgs e)
        {
            helptext.Visible = false;
        }

        void input_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (!helptext.Visible) 
                helptext.Location = new Point(e.X ,e.Y);
        }

        void input_SelectionChanged(object sender, System.EventArgs e)
        {
            helptext.Visible = false;
        }


        void process_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            //term.AppendText(e.Data);
            Console.WriteLine(e.Data);
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            input.Clear();
            fname.Text = "";
            editfl = false;
        }

        private void comselect_SelectedIndexChanged(object sender, EventArgs e)
        {
            comPort.Text = (string)comselect.Items[comselect.SelectedIndex];
            if (comPort.Text != "") s = new SerialPort(comPort.Text, 115200);
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                File.WriteAllText(fname.Text, input.Text);
                editfl = false;
            }
            catch (Exception e1)
            {
                MessageBox.Show("can't save file - " + e1.Message);
                fname.Text = "";
            }
        }
        
        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog s = new SaveFileDialog();
            if (s.ShowDialog() != DialogResult.OK)
                return;
            try
            {
                File.WriteAllText(s.FileName, input.Text);
                fname.Text = s.FileName;
                editfl = false;
            }
            catch (Exception e1)
            {
                MessageBox.Show("can't save file - " + e1.Message);
                fname.Text = "";
            }
        }

        private void extendToolStripMenuItem_Click(object sender, EventArgs e)
        {
            extendToolStripMenuItem.Checked = !extendToolStripMenuItem.Checked;
            if (extendToolStripMenuItem.Checked)
            {
                compiler.MAX_PROG_SPACE=3072;
            }
            else
            {
                compiler.MAX_PROG_SPACE = 64000;
            }
        }

        private void loadBinaryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // tbd
            OpenFileDialog s = new OpenFileDialog();
            s.Filter = "Bindata.txt|bindata.txt|*.bin";
            if (s.ShowDialog() != DialogResult.OK)
                return;
            try
            {
                string bf = File.ReadAllText(s.FileName);
                input.Text = compiler.LoadBin(bf);
                output.Text = "Binary download";
                fname.Text = "";
                //download_btn.Enabled = true;
                readyDownload = true;
            }
            catch (Exception e1)
            {
                MessageBox.Show("can't open file - " + e1.Message);
                output.Text = "";
                //download_btn.Enabled = false;
                fname.Text = "";
            }
        }

        Bitmap n = null;
        int minR, minG, minB;
        int maxR, maxG, maxB;

        bool star_push = false;
        bool hash_push = false;

        void clear()
        {
            maxR = 0; maxG = 0; maxB = 0;
            minR = 255; minG = 255; minB = 255;
            show();
        }

        void show()
        {
            label4.Text = minB.ToString();
            label5.Text = minG.ToString();
            label6.Text = minR.ToString();

            label7.Text = maxB.ToString();
            label8.Text = maxG.ToString();
            label9.Text = maxR.ToString();

            textBox3.Text = String.Format("!IMAGE FILT '({6} {0} {1} {2} {3} {4} {5})",
                minR, maxR, minG, maxG, minB, maxB, textBox2.Text);
        }

        void loadimage()
        {
            if (textBox1.Text.StartsWith("https://"))
            {
                Stream stream = File.OpenRead("temp.jpg");
                n = new Bitmap(stream);
                stream.Close();
            }
            else
            {
                n = new Bitmap(textBox1.Text);
            }
            pictureBox1.Image = n;
            pictureBox1.Update();
            //clear();
        }

        void filter()
        {
            if (n == null) return;

            for (int i = 0; i < n.Height; i++)
            {
                for (int j = 0; j < n.Width; j++)
                {
                    Color c = n.GetPixel(j, i);
                    if (c.R >= minR && c.R <= maxR && c.B >= minB && c.B <= maxB && c.G >= minG && c.G <= maxG)
                    {
                        n.SetPixel(j, i, Color.White);
                    }
                    else
                    {
                        n.SetPixel(j, i, Color.Black);
                    }
                }

                pictureBox1.Image = n;
                pictureBox1.Update();
            }
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            Console.Out.WriteLine("E=" + e.ToString() + sender.ToString());
            MouseEventArgs m = (MouseEventArgs)e;
            Console.Out.WriteLine("X=" + m.X + " Y=" + m.Y);
            double fx = (double)pictureBox1.Image.Width * ((double)m.X / (double)pictureBox1.Size.Width);
            double fy = (double)pictureBox1.Image.Height * ((double)m.Y / (double)pictureBox1.Size.Height);
            Console.Out.WriteLine("X=" + (int)fx + " Y=" + (int)fy);
            int x = (int)fx;
            int y = (int)fy;
            if (n != null)
            {
                if (minB > n.GetPixel(x, y).B) minB = n.GetPixel(x, y).B;
                if (minG > n.GetPixel(x, y).G) minG = n.GetPixel(x, y).G;
                if (minR > n.GetPixel(x, y).R) minR = n.GetPixel(x, y).R;

                if (maxB < n.GetPixel(x, y).B) maxB = n.GetPixel(x, y).B;
                if (maxG < n.GetPixel(x, y).G) maxG = n.GetPixel(x, y).G;
                if (maxR < n.GetPixel(x, y).R) maxR = n.GetPixel(x, y).R;

                show();
            }
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (n == null) return;
            if (filtercb.Checked)
                filter();
            else
            {
                loadimage();
            }
        }

        private void ldFile_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                textBox1.Text = openFileDialog1.FileName;
                loadimage();
                clear();
            }
        }

        private void ldWeb_Click(object sender, EventArgs e)
        {
            try
            {
                WebClient webClient = new WebClient();
                webClient.DownloadFile(textBox1.Text, "temp.jpg");
                loadimage();
                clear();
            }
            catch
            {
            }
        }

        private void imageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            imageToolStripMenuItem.Checked = !imageToolStripMenuItem.Checked;
        }

        private void checkBox1_CheckedChanged_1(object sender, EventArgs e)
        {
            if (!checkBox1.Checked)
            {
                loadimage();
                return;
            }

            try
            {
                long sz = Convert.ToInt64(textBox2.Text);
                if (sz < 2)  { sz = 2; textBox2.Text = sz.ToString(); }
                if (sz >100) { sz = 100; textBox2.Text = sz.ToString(); }

                long[,] m = new long[sz, sz];

                for (int i = 0; i < n.Height; i++)
                {
                    for (int j = 0; j < n.Width; j++)
                    {
                        long v = (n.GetPixel(j, i).R + n.GetPixel(j, i).G + n.GetPixel(j, i).B) / 3;
                        m[sz*j/n.Width,sz*i/n.Height] += v;
                    }
                }

                long mv = (n.Height * n.Width) / (sz * sz);

                for (int i = 0; i < n.Height; i++)
                {
                    for (int j = 0; j < n.Width; j++)
                    {
                        long v=(m[sz*j/n.Width,sz*i/n.Height]/mv)%256;
                        n.SetPixel(j, i, Color.FromArgb((int)v,(int)v,(int)v));
                    }

                    pictureBox1.Image = n;
                    pictureBox1.Update();

                }

            }
            catch (Exception e1)
            {
                Console.WriteLine("error - " + e1.Message);
            }
        }

        void normalise()
        {
            if (n == null) return;

            for (int i = 0; i < n.Height; i++)
            {
                for (int j = 0; j < n.Width; j++)
                {
                    long v = (n.GetPixel(j, i).R + n.GetPixel(j, i).G + n.GetPixel(j, i).B);
                    if (v>0) 
                        n.SetPixel(j, i, Color.FromArgb(
                            (768*n.GetPixel(j, i).R/(int)v)%256, 
                            (768*n.GetPixel(j, i).G/(int)v)%256, 
                            (768*n.GetPixel(j, i).B/(int)v)%256));
                }
                pictureBox1.Image = n;
                pictureBox1.Update();

            }
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.Checked)
            {
                normalise();
            }
            else
            {
                loadimage();
            }

        }

        private void reLoadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            input.Text = File.ReadAllText(fname.Text);
            output.Text = "Reloading .. " + fname.Text;
            syntaxcheck(); //!
            editfl = false;
        }

        private void button23_Click(object sender, EventArgs e)
        {
            if (bm) return;
 
            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 1 }, 0, 1); //A
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)1).ToString());
            }
        }

        private void button24_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 2 }, 0, 1); //B
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)2).ToString());
            }
        }

        private void button15_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 7 }, 0, 1); //[] stop
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)7).ToString());
            }
        }

        private void button17_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 3 }, 0, 1); //TL
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)3).ToString());
            }
        }

        private void button14_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 4 }, 0, 1); //up
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)4).ToString());
            }
        }

        private void button20_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 5 }, 0, 1); //up
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)5).ToString());
            }
        }

        private void button18_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 6 }, 0, 1); //left
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)6).ToString());
            }
        }

        private void button21_Click(object sender, EventArgs e)
        {
            if (bm) return;

             if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 8 }, 0, 1); //right
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)8).ToString());
            }
        }

        private void button19_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 9 }, 0, 1); //PL
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)9).ToString());
            }
        }

        private void button16_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 10 }, 0, 1); //down
            }
            if (rlbasic)
            {
                basic_setibuf(((Char)10).ToString());
            }
        }

        private void button22_Click(object sender, EventArgs e)
        {
            if (bm) return;

            if (s != null && s.IsOpen)
            {
                s.Write(new byte[] { 11 }, 0, 1); //Pr
            }

            if (rlbasic)
            {
                basic_setibuf(((Char)11).ToString());
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //One - Zer0 on keypad
            if (bm) return;
            byte[] n = new byte[1];

            int v = ((Button)sender).Text[0] - '0';
            if (v == 0) v = 10;
            n[0]=(byte)(11+v);
            if (star_push) n[0] += 16;
            if (hash_push) n[0] += 32;

            if (s != null && s.IsOpen)
            {
                s.Write(n, 0, 1); //
            }

            if (rlbasic)
            {
                basic_setibuf(n[0].ToString());
            }

            hash_push = star_push = false;
        }

        private void button11_Click(object sender, EventArgs e)
        {
            // * pushed
            star_push = true;

        }

        private void button13_Click(object sender, EventArgs e)
        {
            // # pushed
            hash_push = true;
        }

        private Thread _myThread;
 
        private void SomeThreadMethod()
        {
            // do whatever you want
            basic_api();
        }

        //[SecurityPermissionAttribute(SecurityAction.Demand, ControlThread = true)]
        private void KillThatThread()
        {
           // _myThread.Abort();
            button26.Text = "START";
            rlbasic = false;
        }

        private void button26_Click(object sender, EventArgs e)
        {
            //run local basic
            if (button26.Text == "START")
            {
                basicio.Text = "";
                button26.Text = "Stop";
                rlbasic = true;
                timer1.Enabled = true;

               //_myThread = new Thread(SomeThreadMethod);
               basic_api();

               rlbasic = false;

            }
            else
            {
                basic_stop();
                //basic_setibuf(new byte[] {255}.ToString());
                //KillThatThread();
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (btf != null && progressBar1.Visible)
            {
                progressBar1.Increment((int)(100.0 * btf.progress));
            }
            this.Update();

            if (rlbasic)
            {
                StringBuilder str = new StringBuilder(8000);

                basic_getobuf(str, str.Capacity);
                string rx=str.ToString();
                if (rx.Length > 0)
                {
                    int p = rx.IndexOf("[2J");
                    Debug.WriteLine("p=" + p);
                    if ( p >= 0)
                    {
                        rx= rx.Substring(rx.LastIndexOf("[2J") + 3);
                        basicio.Text = "";
                    }
                    if (rx[0] == 8)
                    {
                        basicio.Text = basicio.Text.Substring(0, basicio.Text.Length - 1);
                        basicio.SelectionStart = basicio.Text.Length;
                    }

                    for (int i = 0; i < rx.Length; i++)
                    {
                        if (rx[i] == 10)
                        {
                            rx.Insert(i, "\r");
                            i = i + 1;
                        }
                    }
                    basicio.AppendText(rx);

                }

                //servos
                if (servoUC1.pm)
                {
                    servoUC1.val = read_servo(svp);
                }
                else
                    set_servo(svp, servoUC1.val);

                if (servoUC2.pm) 
                {
                    servoUC2.val = read_servo(svp+1);
                }
                else
                    set_servo(svp+1, servoUC2.val);

                if (servoUC3.pm)
                {
                    servoUC3.val = read_servo(svp + 2);;
                }
                else
                    set_servo(svp+2, servoUC2.val);

                tabPage5.Update();
            }
        }

        private void basicio_KeyPress(object sender, KeyPressEventArgs e)
        {
            basic_setibuf(e.KeyChar.ToString());
        }

        private LiveJob _job;

        /// <summary>
        /// Device for live source
        /// </summary>
        private LiveDeviceSource _deviceSource;

        void StopJob()
        {
            // Has the Job already been created ?
            if (_job != null)
            {
                _job.StopEncoding();

                // Remove the Device Source and destroy the job
                _job.RemoveDeviceSource(_deviceSource);

                // Destroy the device source
                _deviceSource.PreviewWindow = null;
                _deviceSource = null;

                _job = null;
            }
        }

        private void GetSelectedVideoAndAudioDevices(out EncoderDevice video, out EncoderDevice audio)
        {
            video = null;
            audio = null;

            if (listBox2.SelectedIndex < 0)
            {
                MessageBox.Show("No Video and Audio capture devices have been selected.\nSelect an audio and video devices from the listboxes and try again.", "Warning");
                return;
            }

            // Get the selected video device            
            foreach (EncoderDevice edv in EncoderDevices.FindDevices(EncoderDeviceType.Video))
            {
                if (String.Compare(edv.Name, listBox2.SelectedItem.ToString()) == 0)
                {
                    video = edv;
                    break;
                }
            }

            // Get the selected audio device            
            foreach (EncoderDevice eda in EncoderDevices.FindDevices(EncoderDeviceType.Audio))
            {
                //if (String.Compare(eda.Name, lstAudioDevices.SelectedItem.ToString()) == 0)
                {
                    audio = eda;
                    break;
                }
            }
        }

        private void button29_Click(object sender, EventArgs e)
        {
            if (button29.Text!="Connect")
            {
                StopJob();
                button29.Text = "Connect";
                try
                {
                    Stream stream = File.OpenRead("temp.jpg");
                    n = new Bitmap(stream);
                    stream.Close();
                    pictureBox1.Image = n;
                }
                catch (Exception e1) { }
                return;
            }

            //Connect
            button29.Text = "Stop";

            EncoderDevice video = null;
            EncoderDevice audio = null;

            GetSelectedVideoAndAudioDevices(out video, out audio);
            StopJob();

            if (video == null)
            {
                return;
            }

            // Starts new job for preview window
            _job = new LiveJob();

            // Checks for a/v devices
            if (video != null && audio != null)
            {
                // Create a new device source. We use the first audio and video devices on the system
                _deviceSource = _job.AddDeviceSource(video, audio);
                _deviceSource.PickBestVideoFormat(new Size(640, 480), 15);

                // Get the properties of the device video
                SourceProperties sp = _deviceSource.SourcePropertiesSnapshot();

                // Resize the preview panel to match the video device resolution set
               // pictureBox1.Size = new Size(sp.Size.Width, sp.Size.Height);

                // Setup the output video resolution file as the preview
                _job.OutputFormat.VideoProfile.Size = new Size(sp.Size.Width, sp.Size.Height);

                // Display the video device properties set
                textBox3.Text = sp.Size.Width.ToString() + "x" + sp.Size.Height.ToString() + "  " + sp.FrameRate.ToString() + " fps";

                // Sets preview window to winform panel hosted by xaml window
                _deviceSource.PreviewWindow = new PreviewWindow(new System.Runtime.InteropServices.HandleRef(pictureBox1, pictureBox1.Handle));

                // Make this source the active one
                _job.ActivateSource(_deviceSource);
            }
            else
            {
                // Gives error message as no audio and/or video devices found
                MessageBox.Show("No Video/Audio capture devices have been found.", "Warning");
            }
        }

        private void button30_Click(object sender, EventArgs e)
        {
            //Grab
            // Create a Bitmap of the same dimension of panelVideoPreview (Width x Height)

            using (Bitmap bitmap = new Bitmap(pictureBox1.Width, pictureBox1.Height))
            {
                using (Graphics g = Graphics.FromImage(bitmap))
                {
                    // Get the paramters to call g.CopyFromScreen and get the image
                    Rectangle rectanglePanelVideoPreview = pictureBox1.Bounds;
                    Point sourcePoints = pictureBox1.PointToScreen(new Point(pictureBox1.ClientRectangle.X, pictureBox1.ClientRectangle.Y));
                    g.CopyFromScreen(sourcePoints, Point.Empty, rectanglePanelVideoPreview.Size);
                }

                string strGrabFileName = String.Format("Snapshot_{0:yyyyMMdd_hhmmss}.jpg", DateTime.Now);
                textBox3.Text = strGrabFileName;
                bitmap.Save(strGrabFileName, System.Drawing.Imaging.ImageFormat.Jpeg);
                bitmap.Save("temp.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
            }
        }

        private void tabControl1_Enter(object sender, EventArgs e)
        {
            //enter tab
            listBox2.ClearSelected();
            foreach (EncoderDevice edv in EncoderDevices.FindDevices(EncoderDeviceType.Video))
            {
                listBox2.Items.Add(edv.Name);
            }
        }

        private void remoteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //remoteToolStripMenuItem.Enabled = !remoteToolStripMenuItem.Enabled;
            groupBox1.Visible = remoteToolStripMenuItem.Checked;
            this.Width = (groupBox1.Visible) ? 1028 : 868;
        }

        private void storefn()
        {
            sv[svp].id  = servoUC1.id;
            sv[svp].val = servoUC1.val;
            sv[svp].pm  = servoUC1.pm;
            sv[svp].io  = servoUC1.io;

            sv[svp+1].id  = servoUC2.id;
            sv[svp+1].val = servoUC2.val;
            sv[svp+1].pm  = servoUC2.pm;
            sv[svp+1].io  = servoUC2.io;

            sv[svp+2].id  = servoUC3.id;
            sv[svp+2].val = servoUC3.val;
            sv[svp+2].pm  = servoUC3.pm;
            sv[svp+2].io  = servoUC3.io;


        }

        private void copyfn()
        {
            servoUC1.id = sv[svp].id;
            servoUC1.val = sv[svp].val;
            servoUC1.pm = sv[svp].pm;
            servoUC1.io = sv[svp].io;
            servoUC1.Update();

            servoUC2.id = sv[svp + 1].id;
            servoUC2.val = sv[svp + 1].val;
            servoUC2.pm = sv[svp + 1].pm;
            servoUC2.io = sv[svp+1].io;
            servoUC2.Update();

            servoUC3.id = sv[svp + 2].id;
            servoUC3.val = sv[svp + 2].val;
            servoUC3.pm = sv[svp + 2].pm;
            servoUC3.io = sv[svp+2].io;
            servoUC3.Update();
        }

        private void button25_Click(object sender, EventArgs e)
        {
            // <<
           storefn() ;

            if (svp>0) svp-=1;

            copyfn();
        }

        private void button27_Click(object sender, EventArgs e)
        {
            // >>

            storefn();

            if (svp <sv.Length-3 ) svp += 1;

            copyfn();
        }

        private void Basic_frm_FormClosed(object sender, FormClosedEventArgs e)
        {
            basic_stop();
            this.Dispose();
        }

        private void Basic_frm_Load(object sender, EventArgs e)
        {

        }

        private void uploadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            input.Text = compiler.LoadBin("");
            output.Text = "Binary upload";
            fname.Text = "";
            //download_btn.Enabled = true;
            readyDownload = true;
        }

        private void hScrollBar1_ValueChanged_1(object sender, EventArgs e)
        {
            label16.Text = "<" + hScrollBar1.Value + ">";
            set_servo(32, hScrollBar1.Value);

            label17.Text = "<" + hScrollBar2.Value + ">";
            set_servo(33, hScrollBar2.Value);

            label18.Text = "<" + hScrollBar3.Value + ">";
            set_servo(34, hScrollBar3.Value);

            label19.Text = "<" + hScrollBar4.Value + ">";
            set_servo(35, hScrollBar4.Value);
        }

        private void button28_Click(object sender, EventArgs e)
        {
            rm.UpdateDisplay();
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            rm.showrightarm = checkBox3.Checked;
            rm.UpdateDisplay();
        }

        private void checkBox4_CheckedChanged(object sender, EventArgs e)
        {
            rm.showrightleg = checkBox4.Checked;
            rm.UpdateDisplay();
        }

        private void checkBox5_CheckedChanged(object sender, EventArgs e)
        {
            rm.showleftleg = rm.showleftarm = checkBox5.Checked;
            rm.UpdateDisplay();
        }

        private void tabPage6_Paint(object sender, PaintEventArgs e)
        {
            //rm.UpdateDisplay();
        }

        private void hScrollBar5_ValueChanged(object sender, EventArgs e)
        {
            int v = hScrollBar5.Value;
            label20.Text = "" + v;
            int n = Convert.ToInt32(textBox4.Text);
            if (n<0 || n>31)
            {
                n = 0;
                textBox4.Text = "0";
            }

            rm.servos[n] = v;
            rm.UpdateDisplay();
        }

    }
}
