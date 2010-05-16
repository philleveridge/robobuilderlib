using System;
using System.Drawing;
using System.Windows.Forms;
using RobobuilderLib;

namespace Demo
{
    class Balance
    {
        int gx = 0, gy = 0, gz = 0;
        wckMotion w;
        Program p;

        CList coords = new CList();

        long st;

        public Balance(Program p1)
        {
            p = p1;
            w = p1.w;
        }

        byte[] ub_Huno = new byte[] { 174, 228, 254, 130, 185, 254, 180, 126, 208, 208, 254, 224, 198, 254, 200, 254 };
        byte[] lb_Huno = new byte[] { 1, 70, 124, 40, 41, 73, 22, 1, 120, 57, 1, 46, 1, 1, 25, 40 };

        struct compare
        {
            public int min;
            public int max;
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

        int[] rmatch(int g,  compare[] c)
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

        void spos(byte[] ps)
        {
            try
            {
                w.SyncPosSend(15, 4, ps, 0);
                System.Threading.Thread.Sleep(20); // wow need to slow this down !

            }
            catch
            {
                Console.WriteLine("overflow");
            }
        }
        public void dtest()
        {
            while (!Console.KeyAvailable)
            {
                w.wckReadPos(30, 5);
                Console.WriteLine("*".PadLeft(w.respnse[0], '='));
            }
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
                if (gx > 127) gx = -256;
            }
            Console.WriteLine("calibrated: {0},{1},{2}", gx, gy, gz);
        }


        int[] add_delta(int[] a, int[] b)
        {
            int[] r = new int[a.Length];
            for (int i = 0; i < a.Length; i++)
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
                r[i] = a[i] + ((b != null && i < b.Length) ? b[i] : 0);
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

        public void swin(Utility u)
        {
            u.p1 = new Pen(Color.FromName("Black"));
            u.p1.DashStyle = (System.Drawing.Drawing2D.DashStyle.DashDot);
            u.p2 = new Pen(Color.FromName("Red"));
            u.createwindow("Balance Demo", 250, 250);
            u.win.Show();
        }

        public void pwin(Utility u, int coord, int n, double t)
        {
            int nx = ((n * 10) % 280) - 140;
            int ny = 4 * coord;

            u.plot("(Acc=" + ny + " Rate=" + String.Format("{0:#.#}", t) + " ms)", nx, ny);
            u.drawlist(new int[] { -125, 40, 125, 40 }, 4, (Pen)((ny > 40) ? u.p2 : u.p1)); //limit
            u.drawlist(new int[] { -125, -40, 125, -40 }, 4, (Pen)((ny < -40) ? u.p2 : u.p1)); //limit    
            coords.store(nx, ny);
            u.drawlist(coords.getlast(6), 6, new Pen(Color.FromName("Blue")));
        }

        public void balanceTest()
        {
            int x, z;
            Double rt = 0;
            Utility u = new Utility();
            swin(u);

            byte[] sbase = p.getallServos(16);
            calibrateXYZ();
            Program.pause();
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
                pwin(u, z, nc, rt);

                sbase = autobalance(sbase, x, z);
                spos(sbase);
            }
            p.standup();
            u.cwin();
        }

    }
}
