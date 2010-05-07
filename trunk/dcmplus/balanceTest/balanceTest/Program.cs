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
        int gx=0, gy=0, gz=0;

        long   st;

        static public void sleep(double t)
        {
            System.Threading.Thread.Sleep((int)(t * 1000));
        }

        static public void pause()
        {
            Console.WriteLine("Paused - press any key");
            Console.Read();
        }

        int getServoPos(int n)
        {
            if (w.wckReadPos(n))
                return w.respnse[1];
            else
                return 0;
        }

        int setServoPos(int n, int p, int t)
        {
            if (w.wckMovePos(n,p,t))
                return 1;
            else
                return 0;
        }

        struct compare
        {
            public int   min;
            public int   max;
            public int[] dp;

            public compare(int a, int b, int[] c)
            { min = a; max = b; dp = c; }
        };

        /*
        (= Z4    '(( ( 14  20)  (0 0  0  3 0  0 0  0 0 -3 ))             
           ( ( 10  15)  (0 0  0  2 0  0 0  0 0 -2 ))
           ( ( 5   11)  (0 0  0  1 0  0 0  0 0 -1 ))
           ( (-11 -5)   (0 0  0 -1 0  0 0  0 0  1 ))
           ( (-14 -10)  (0 0  0 -2 0  0 0  0 0  2 ))
           ( (-20 -13)  (0 0  0 -3 0  0 0  0 0  3 )))
        )
         */

        int widepos  =70;
        int closepos =120;
        int maxit    =15;
        double dt    =   0.1;

        public void opengripper (int d)
        {
            int cnt =0;
            int cp = getServoPos(18);
            int np= 0;
            int delta=5 ;

            if (cp < widepos) 
                  cp =  widepos ;

            while ((cnt < maxit) && (Math.Abs(cp - widepos) > 2) && (delta > 0)) 
            {
                cnt++;
                Console.WriteLine( cnt + ", " + cp + ", "+  delta);

                setServoPos(18, cp - d, 4);
                sleep (dt);
                np = getServoPos(18);
                delta =  Math.Abs (cp - np);
                cp=np;
            }
        }

        public void closegripper (int d)
        {
            int cnt =0;
            int cp = getServoPos(18);
            int np = 0;
            int delta=5 ;
         
            if (cp > closepos) 
              cp =  closepos ;

            while ((cnt < maxit) && (Math.Abs (cp-closepos) > 2) && (delta > 2)) 
            {
                cnt++;
                Console.WriteLine( cnt + ", " + cp + ", "+  delta);
                w.wckMovePos(18 ,d  + cp, 4);
                sleep(dt);
                np = getServoPos(18);
                delta =  Math.Abs (cp - np);
                cp=np;
            }
        }

        byte[] basic18 = new byte[]  {143, 179, 198,  83, 106, 106,  69,  48, 167, 141,  47,  47,  49, 199, 192, 204, 122, 125, 127 };	
        byte[] ub_Huno = new byte[]  {174, 228, 254, 130, 185, 254, 180, 126, 208, 208, 254, 224, 198, 254, 200, 254};
        byte[] lb_Huno = new byte[]  {  1,  70, 124,  40,  41,  73,  22,   1, 120,  57,   1,  46,   1,   1,  25,  40};

        public void standup()
        {
            w.PlayPose(1000, 10, basic18, true);
        }

        int[] rmatch(int g, compare[] c)
        {
            int[] res = null;
            foreach (compare r in c)
            {
                if (g >= r.min && g < r.max)
                {
                    res = r.dp;
                    break;
                }
            }
            return res;
        }

        byte[] bcheck(int[] p, byte[] min, byte[] max)
        {
            byte[] r = new byte[p.Length];
            for (int i = 0; i < p.Length; i++)
            {
                if (i < min.Length && i < max.Length)
                    r[i] = (byte)((p[i] > max[i]) ? max[i] : ((p[i] < min[i]) ? min[i] : p[i]));
            }
            return r;
        }

        void spos(byte[] ps)
        {
            try {
                w.SyncPosSend(15, 4, ps, 0) ;
                System.Threading.Thread.Sleep(20); // wow need to slow this down !

            } catch {
                Console.WriteLine ("overflow");
            }
        }

        byte[] getallServos(int p)
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

        void getZX(out int x, out int z)
        {
            x = 0; z = 0;
            if (w.wckReadPos(30, 2))
            {
                x = w.respnse[0];
                z = w.respnse[1];
                if (x > 127) x -= 256;
                if (z > 127) z -= 256;
            }
            x = x - gx; z = z - gz;
            //Console.WriteLine("x={0}, z={1}", x, z);
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

        void calibrateXYZ()
        {
            gx = 0; gy = 0; gz = 0;
            if (w.wckReadPos(30, 1))
            {
                gy = w.respnse[0];
                gz = w.respnse[1];
                if (gy > 127) gy -= 256;
                if (gz > 127) gz -= 256;
            }
            if (w.wckReadPos(30, 2))
            {
                gx = w.respnse[0];
                if (gx > 127) gx =- 256;
            }
            Console.WriteLine("calibrated: {0},{1},{2}", gx, gy, gz);
        }

        int[] rmatch(int n)
        {
            int[] r = null;
            return r;
        }

        int[] add_delta(int[] a, int[] b)
        {
            int[] r = new int[a.Length];
            for (int i=0; i<a.Length; i++)
            {
                r[i] = a[i] + ((b != null && i < b.Length) ? b[i] : 0);
            }
            return r;
        }

        int[] add_delta(int[] a, byte[] b)
        {
            int[] r = new int[a.Length];
            for (int i = 0; i < a.Length; i++)
            {
                r[i] = a[i] + ((b != null && i<b.Length)? b[i] : 0);
            }
            return r;
        }

        byte[] autobalance(byte[] sp, int x, int z)
        {
            compare[] z2 = new compare[] {
                new compare( 14, 20, new int[] {0,0,0,0,0,0,0,0,0,0, 4,0,0,-4,0,0}),
                new compare( 10, 15, new int[] {0,0,0,0,0,0,0,0,0,0, 2,0,0,-2,0,0}),
                new compare(  5, 11, new int[] {0,0,0,0,0,0,0,0,0,0, 1,0,0,-1,0,0}),
                new compare(-11, -5, new int[] {0,0,0,0,0,0,0,0,0,0,-1,0,0, 1,0,0}),
                new compare(-14,-10, new int[] {0,0,0,0,0,0,0,0,0,0,-2,0,0, 2,0,0}),
                new compare(-20,-13, new int[] {0,0,0,0,0,0,0,0,0,0,-4,0,0, 4,0,0})
            };

            compare[] Zx = new compare[] {
                new compare( 6, 15, new int[] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, 0, 0, -2, 0 }),
                new compare( 4,  7, new int[] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0 }),
                new compare(-7, -4, new int[] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 0, 0,  1, 0 }),
                new compare(-15,-6, new int[] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  2, 0, 0,  2, 0 })    
            };

            int[] dz = rmatch(z, z2);
            int[] dx = rmatch(x, Zx);
            int[] nxt = null;

            if (dz != null) nxt = dz;
            if (dx != null) nxt = add_delta(dx, nxt);

            if (nxt != null)
            {
                return bcheck(add_delta(nxt, sp), lb_Huno, ub_Huno); ;
            }
            else
            {
                return sp;
            }      
        }



        void balanceTest()
        {
            int x, z;

            Double rt=0;
            Utility u = new Utility();
            u.swin();

            standup();

            byte[] sbase = getallServos(16);
            calibrateXYZ();
            pause();
            int nc = 0;
            st = DateTime.Now.Ticks;

            while (!Console.KeyAvailable)
            {
                if ((nc++ % 10) == 0)
                {
                    rt = (DateTime.Now.Ticks - st) / (10 * TimeSpan.TicksPerMillisecond);
                    Console.WriteLine("Rate = {0:#.#}", rt);
                    st = DateTime.Now.Ticks;
                }
                getZX(out x, out z);

                u.pwin(z, nc, rt );

                sbase = autobalance(sbase, x, z);
                spos(sbase);
            }
            standup();
            u.cwin();
        }

        static void Main(string[] args)
        {
            string port = "COM5";

            Program g = new Program();
            g.p = new PCremote(port);
            g.w = new wckMotion(g.p);

            int test = 1;

            Console.WriteLine("Demo - Port: " + port);

            if (test == 1)
            {
                Console.WriteLine("Balance test");
                g.balanceTest();
            }

            if (test == 2)
            {
                Console.WriteLine("Speech Demo");
                Speech s = new Speech();
                s.voicemenu(g);
            }

            if (test == 3)
            {
                Console.WriteLine("Walk Demo");
                Motions m = new Motions();
                m.dtest(g.w);
                pause();
                m.panda(g);
            }

        }
    }
}
