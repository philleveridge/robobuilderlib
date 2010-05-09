using System;
using System.Collections.Generic;
using System.Text;

using System.Drawing;
using System.Windows.Forms;

using RobobuilderLib;

namespace Demo
{
    class Program
    {
        public PCremote p;
        public wckMotion w;

        public int nos = 0;
        public bool headservo;
        public bool gripservo;

        byte[] basic18 = new byte[] { 143, 179, 198, 83, 106, 106, 69, 48, 167, 141, 47, 47, 49, 199, 192, 204, 122, 125, 255 };
        byte[] basic16 = new byte[] { 125, 179, 199, 88, 108, 126, 72, 49, 163, 141, 51, 47, 49, 199, 205, 205 };
        

        static public void sleep(double t)
        {
            System.Threading.Thread.Sleep((int)(t * 1000));
        }

        static public void pause()
        {
            Console.WriteLine("Paused - press any key");
            Console.Read();
        }

        public int getServoPos(int n)
        {
            if (w.wckReadPos(n))
                return w.respnse[1];
            else
                return 0;
        }

        public int setServoPos(int n, int p, int t)
        {
            if (w.wckMovePos(n,p,t))
                return 1;
            else
                return 0;
        }

        public void standup()
        {
            if (nos < 16) return;
            w.PlayPose(1000, 10, (nos<18)? basic16:basic18, true);
        }

        public byte[] getallServos(int p)
        {
            byte[] r = new byte[p];
            for (int i = 0; i < p; i++)
            {
                if (w.wckReadPos(i))
                    r[i] = w.respnse[1];
                else
                    r[i] = 0;
            }
            return r;
        }

        public int readdistance()
        {
            if (w.wckReadPos(30, 5))
            {
                return w.respnse[0];
            }
            else
                return 0;
        }

        public void headleft()  { setServoPos(20,  85, 4); }

        public void headright() { setServoPos(20, 205, 4); }

        public void headfw()    { setServoPos(20, 145, 4); }



        /*
         * 
         * 
         * 
         */

        int countServos(int m)
        {
            for (int i = 0; i < m; i++)
            {
                if (!w.wckReadPos(i))
                {
                    nos = i;
                    return i;
                }
            }
            nos = m;
            return m;
        }

        public bool testServo(int id)
        {
            return w.wckReadPos(id);
        }

        static public bool dotest(string s)
        {
            Console.Write("Run test  : " + s + " [Y | N]? ");

            char ch='\0';
            while (ch != 'Y' && ch != 'N')
            {
                while (!Console.KeyAvailable) ;
                ch = Char.ToUpper(Console.ReadKey(true).KeyChar);
            }
            Console.WriteLine(ch);
            return (ch == 'Y');
        }

        /*
         *  Main - 
         * 
         * 
         */

        static void Main(string[] args)
        {
            string port = "COM5";
            if (args.Length > 0) port = args[0];

            Program g = new Program();
            g.p = new PCremote(port);
            g.w = new wckMotion(g.p);

            g.headservo = g.testServo(20);
            g.gripservo = g.testServo(18);

            Console.WriteLine("Demo - Port: {0} - {1} servos", port, g.countServos(22));

            g.standup();

            if (Program.dotest("Balance test"))
            {
                Balance b = new Balance(g);
                b.balanceTest();
            }

            if (!g.headservo)
            {
                Console.WriteLine("head servo not connected");
            }
            else if (Program.dotest("Scan test"))
            {
                Scan s1 = new Scan(g);
                s1.scan();
            }

            if (Program.dotest("Speech Demo"))
            {
                Speech s = new Speech(g);
                s.voicemenu();
            }

            if (Program.dotest("Gripper Demo"))
            {
                Grip r = new Grip(g);
                r.gripper_test();
            }

            if (Program.dotest("Walk Demo"))
            {
                Walk m = new Walk(g);
                m.panda();
            }

        }
    }
}
