using System;
using System.Drawing;
using System.Windows.Forms;
using RobobuilderLib;

namespace Demo
{
    class Scan
    {
        Program p;
        CList coords = new CList();

        public Scan(Program p1)
        {
            p = p1;
        }

        void scanw(int a, int b, int inc, Utility u)
        {
            int i = a;

            coords.reset_history();

            while ((inc > 0 && i < b) || (inc < 0 && i > b))
            {
                int d = p.readdistance();
                int n;
                if (inc > 0)
                    n = -100 + (i - a) * (200 / (b - a));
                else
                    n = -100 + (i - b) * (200 / (a - b));

                u.plot(String.Format("D={0:#}", d), n, d);
                coords.store(n, d);
                u.drawlist(coords.getAll(), coords.count(), new Pen(Color.FromName("Blue")));

                // int x,y; 
                // conv (d, i, a, b, out x, out y));
                // u.plot(String.Format("D={0:#}", d), x, y); 

                p.setServoPos(20, i, 4);
                i += inc;
            }
        }

        public void scan()
        {
            scan(5);
        }
        public void scan(int num)
        {
            Utility u = new Utility();
            u.p1 = new Pen(Color.FromName("Black"));
            u.p1.DashStyle = System.Drawing.Drawing2D.DashStyle.DashDot;
            u.createwindow("scan demo", 250, 250);
            u.win.Show();

            int centre = 142;
            int left = centre + 40;
            int right = centre - 40;

            while (!Console.KeyAvailable && num-->=0)
            {
                scanw(right, left, 1, u);
                scanw(left, right, -1, u);
            }
            u.cwin();
            p.headfw();
            p.psdoff();
        }

        public void conv (int d, int p, int ml, int mr, out int x, out int y)
        {
            double ppd = 270.0/ 255;
            double cn = ml + ((mr- ml) / 2);
            double r =(((p - cn )*ppd) * Math.PI) / 90 ; //2r;;
 
            x =(int)(d * Math.Sin(r));
            y =(int)(d * Math.Cos(r));

            Console.WriteLine( "d="+d +", p=" +p +" -> r=" +r+ " -> (x,y) =" +x+ "," +y);
        }
    }
}
