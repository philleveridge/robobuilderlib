using System.Drawing;
using System.Windows.Forms;
using System;
using System.Text;

namespace Demo
{
    class Utility
    {
        public Form win = null;
        public Pen p1, p2;
        Graphics g;
        Panel pb;


        public Form createwindow(string title, int h, int w)
        {
            win = new Form();

            win.Text = title;  
            win.AutoSize =  true;
            win.TopMost = false;

            pb = new System.Windows.Forms.Panel();
            pb.Size= new Size (h,w);
            pb.Location = new Point(24, 16);
            win.Controls.Add( pb);
            g=pb.CreateGraphics();
            return win;
        }

        public int[] history = new int[1000];
        public int hn = 0;

        public void store(int x, int y)
        {
            if (hn > 998) hn = 0;
            history[hn++] = x;
            history[hn++] = y;
        }

        public void reset_history()
        {
            hn = 0;
        }

        public int[] getlast(int n)
        {
            int[] p = new int[n];
            for (int i = 0; i < n; i++)
            {
                if (hn-n+i>0) 
                    p[i] = history[hn - n + i];
            }
            return p;
        }

        public void cwin()
        {
            win.Close();
        }

        public void plot(string txt, int x, int y)
        {
            int w  = pb.Width;
            int h  = pb.Height;

            x += (w/2);
            y = (h/2)-y;

            win.Show();
  
            g.Clear(Color.FromName ("White"));

            Pen axis  = new Pen (Color.FromName ("Black"));
            Pen pen   = new Pen (Color.FromName ("Red"));
            Font font = new Font ("Arial", (Single) 8.25 );

            g.DrawLine    (axis, 0, h/2, w, h/2);
            g.DrawLine    (axis, w/2, 0, w/2, h);
            g.DrawEllipse (pen, (x-6),  (y-6), 14, 14);
            g.DrawString  (txt, font, (pen.Brush), new PointF( 10, 10));     
        }

        public void drawline(Graphics g, int fx, int fy, int tx, int ty, Pen c)
        {       
            int w  = pb.Width;
            int h  = pb.Height;

            fx += (w/2);
            fy = (h/2) - fy;
            tx += (w / 2);
            ty = (h/2) - ty;
   
            g.DrawLine(c, fx, fy, tx, ty);
        }

        public void drawlist(int[] xy, int n, Pen c)
        {
            int i = 0;
            while (i + 4 <= xy.Length && i + 4 <= n)
            {
                drawline(g, xy[i], xy[i + 1], xy[i + 2], xy[i + 3], c);
                i += 2;
            }
        }
    }
}

