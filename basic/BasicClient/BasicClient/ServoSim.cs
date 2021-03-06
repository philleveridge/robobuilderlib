﻿using System;
using System.Data;
using System.Drawing;
using System.Windows.Forms;
using System.Text;

using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;

namespace RobobuilderLib
{
    public enum RemoCon
    {
        FAILED = 0,
        A = 0x01, B, LeftTurn, Forward, RightTurn, Left, Stop, Right, Punch_Left, Back, Punch_Right,
        N1, N2, N3, N4, N5, N6, N7, N8, N9, N0,

        S_A = 0x16, S_B, S_LeftTurn, S_Forward, S_RightTurn, S_Left, S_Stop, S_Right, S_Punch_Left, S_Back, S_Punch_Right,
        S_N1, S_N2, S_N3, S_N4, S_N5, S_N6, S_N7, S_N8, S_N9, S_N0,

        H_A = 0x2B, H_B, H_LeftTurn, H_Forward, H_RightTurn, H_Left, H_Stop, H_Right, H_Punch_Left, H_Back, H_Punch_Right,
        H_N1, H_N2, H_N3, H_N4, H_N5, H_N6, H_N7, H_N8, H_N9, H_N0
    };

    public partial class ServoSim : Form
    {
        public string bfn="basic.exe";

        const int MAX = 8;
        RemoCon ir_val = RemoCon.FAILED;
        servoUC[] servoUCA = new servoUC[32]; 
        TcpListener serverSocket;
        bool stopNow = false;
        int startservo = 0;

        bool[] leds = new bool[8]; // RUN G/B, PWR R/G, ERROR R, PF1 R/B, PF2 O
        bool[] but  = new bool[2]; // PF1 / PF2

        Process process = null;

        public ServoSim()
        {
            InitializeComponent();
            intsuc();
            showuc();

            int i;
            for (i = 0; i < 8; i++) leds[i] = false;
            for (i = 0; i < 2; i++) but[i]  = false;
        }

        void intsuc()
        {
            for (int i = 0; i < 32; i++)
            {
                servoUCA[i] = new servoUC();

                servoUCA[i].id  = i;
                servoUCA[i].val = 127;

                servoUCA[i].Location = new System.Drawing.Point(10 + (i%MAX)*65, 10);
                servoUCA[i].Name = "servoUC" + i;
                servoUCA[i].Size = new System.Drawing.Size(60, 206);
                servoUCA[i].Visible = false;

                this.Controls.Add(servoUCA[i]);
            }
        }

        void showuc()
        {
            for (int i = 0; i < 32; i++)
            {
                if (i>=startservo && i<startservo+MAX)
                    servoUCA[i].Visible = true;
                else
                    servoUCA[i].Visible = false;
            }
        }

        void close()
        {
            serverSocket.Stop();
        }

        private void displ(string text)
        {
            textBox1.AppendText(text);
        }

        int handledata(string v)
        {
            string[] p = v.Split(':');
            if (v.StartsWith("S:"))
            {
                int n = Convert.ToInt32(p[1]);
                if (n < 0) n = 0; if (n >= 32) n = 31;

                servoUCA[n].val = Convert.ToInt32(p[2]);
                return n;
            }
            if (v.StartsWith("P:"))
            {
                int n = Convert.ToInt32(p[1]);
                return n;
            }
            if (v.StartsWith("Y:")) // synch move
            {
                for (int i = 0; i < p.Length; i++)
                {
                    servoUCA[i].val = Convert.ToInt32(p[i+1]);
                }
                return 0;
            }
            if (v.StartsWith("H:")) // leds and button
            {
                if (p.Length > 1 && p[1] != "")
                {
                    int n = Convert.ToInt32(p[1]);
                    for (int i = 0; i < 8; i++)
                    {
                        leds[i] = (n | (1 << i)) == 1;
                    }
                    set_leds();
                }
                return (but[0]?1:0) + (but[1]?2:0);
            }
            if (v.StartsWith("R:"))
            {
                int n = Convert.ToInt32(p[1]); if (n < 0) n = 0; if (n >= 32) n = 31;
                return servoUCA[n].val;
            }
            if (v.StartsWith("O:"))
            {
                int n = Convert.ToInt32(p[1]); if (n < 0) n = 0; if (n >= 32) n = 31;
                return servoUCA[n].io = Convert.ToInt32(p[2]);
            }
            if (v.StartsWith("IR"))
            {
                int t = (int)ir_val;
                ir_val = RemoCon.FAILED;
                return ((t==0)?-1:t);
            }
            if (v.StartsWith("X"))
            {
                return xv.Value;
            }
            if (v.StartsWith("Y"))
            {
                return yv.Value;
            }
            if (v.StartsWith("Z"))
            {
                return zv.Value;
            }
            if (v.StartsWith("V"))
            {
                return 101; // version 0.101 
            }
            if (v.StartsWith("PSD"))
            {
                return psdv.Value;
            } 
            return 0;
        }

        private void runbasic()
        {
            // start basic

            if (process != null)
                return;

            if (File.Exists(bfn))
            {
                process = new Process();
                process.StartInfo.FileName = bfn;
                process.EnableRaisingEvents = true;
                process.Exited += new EventHandler(process_Exited);
                process.Start();
            }
        }

        private void sock()
        {
            Int32 port = 8888;
            IPAddress localAddr = IPAddress.Parse("127.0.0.1");

            serverSocket = new TcpListener(localAddr, port);

            serverSocket.Start();
            displ(" >> Server Started");
            int requestCount = 0;

            runbasic();

            while (!stopNow)
            {
                while  (!serverSocket.Pending())
                {
                    Application.DoEvents();
                    if (stopNow)
                    {
                        serverSocket.Stop();
                        displ(" >> Server Stopped");
                        return;
                    }
                }

                TcpClient clientSocket = serverSocket.AcceptTcpClient();
                displ(" >> Accept connection from client");
                requestCount = 0;

                try
                {
                    requestCount = requestCount + 1;
                    NetworkStream networkStream = clientSocket.GetStream();
                    byte[] bytesFrom = new byte[10025];
                    networkStream.Read(bytesFrom, 0, (int)clientSocket.ReceiveBufferSize);
                    string dataFromClient = System.Text.Encoding.ASCII.GetString(bytesFrom);
                    dataFromClient = dataFromClient.Substring(0, dataFromClient.IndexOf("$"));

                    int r = handledata(dataFromClient);
                    Application.DoEvents();

                    displ(" >> Data from client - " + dataFromClient);
                    string serverResponse = "D:" + r.ToString() + "$";

                    Byte[] sendBytes = Encoding.ASCII.GetBytes(serverResponse);
                    networkStream.Write(sendBytes, 0, sendBytes.Length);
                    networkStream.Flush();
                    displ(" >> " + serverResponse);
                }
                catch (Exception ex)
                {
                    displ(ex.ToString());
                }
            }

            serverSocket.Stop();
            displ(" >> Done");

        }

        void process_Exited(object sender, EventArgs e)
        {
            process = null;
            MessageBox.Show("EXITED");
            stopNow = true;
            //button1.Text = "Go!";
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (button1.Text == "Go!")
            {
                button1.Text = "Stop";
                stopNow = false;
                sock();
                button1.Text = "Go!";
            }
            else
            {
                button1.Text = "Go!";
                stopNow = true;
                // ???
                if (process != null)
                {
                    process.Kill();
                    process = null;
                }
            }
        }

        private void pictureBox1_Click(object sender, MouseEventArgs e)
        {
            //FAILED=0,
            //A=0x01,B,LeftTurn,Forward,RightTurn,Left,Stop,Right,Punch_Left,Back,Punch_Right,
            //N1,N2,N3,N4,N5,N6,N7,N8,N9,B0,

            string[] spots = new string[] 
            {
            "A,43,29,54,42",
            "B,111,28,121,45",
            "",
            "F,76,42,87,63",
            "",
            "L,38,79,51,93",
            "@,73,81,87,94",
            "R,111,78,127,93",
            "",
            "B,75,110,88,129",
            "",
            "1,48,148,59,160",
            "2,75,149,89,160",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "0",
            "*,45,226,58,236",      // need to rememebr
            "#,105,225,118,235"     // need to rememebr
            };

            int ir;
            string h = "";

            for (ir = 0; ir < spots.Length; ir++)
            {
                string[] r = spots[ir].Split(',');
                if (r.Length == 5)
                {
                    if ((e.X > Convert.ToInt32(r[1])) && (e.X < Convert.ToInt32(r[3])) && (e.Y > Convert.ToInt32(r[2])) && (e.Y < Convert.ToInt32(r[4])))
                    {
                        h = r[0];
                        break;
                    }
                }
            }
            Console.WriteLine("Mouse - " + e.X + "," + e.Y + " : " + h);

            if (ir < spots.Length)
                ir_val = (RemoCon)(ir + 1);
            else
                ir_val = RemoCon.FAILED;

            textBox1.Text = "IR = " + ir_val;
            return;
        }

        void ValueChanged(object sender, System.EventArgs e)
        {
            textBox1.Text = "val changed to = " + ((VScrollBar)sender).Value;
            if (((VScrollBar)sender).Name == "psdv")
            {
                label4.Text = "PSD=" + ((VScrollBar)sender).Value;
            }

        }

        private void servoIter_Click(object sender, EventArgs e)
        {
            int n = Convert.ToInt32(servoIter.Text);
            n = n + 1;
            if (n > 2) n = 0;
            startservo = n * MAX;
            servoIter.Text = n.ToString();
            showuc();
        }

        //
        private void pf1_btn_Click(object sender, EventArgs e)
        {
            but[0] = true;
        }

        private void pf2_btn_Click(object sender, EventArgs e)
        {
            but[1] = true;
        }

        void set_leds()
        {
            checkBox1.Checked = leds[0];
            checkBox2.Checked = leds[1];
            checkBox3.Checked = leds[2];
            checkBox4.Checked = leds[3];
            checkBox5.Checked = leds[4];
            checkBox6.Checked = leds[5];
            checkBox7.Checked = leds[6];
            checkBox8.Checked = leds[7];
        }
    }
}
