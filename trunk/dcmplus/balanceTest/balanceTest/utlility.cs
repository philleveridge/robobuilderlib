using System.Drawing;
using System.Windows.Forms;
using System;
using System.Text;

namespace Demo
{
    class Utility
    {
        Form win = null;
        Pen p1, p2;
        Graphics g;
        Panel pb;
        int[] history = new int[1000];
        int hn = 0;

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

        public void drawlist(Graphics g, int[] xy, Pen c)
        {
            int i = 0;
            while (i + 4 <= xy.Length)
            {
                drawline(g, xy[i], xy[i + 1], xy[i + 2], xy[i + 3], c);
                i += 2;
            }
        }

        public void swin()
        {
            p1 = new Pen(Color.FromName("Black"));
            p1.DashStyle = (System.Drawing.Drawing2D.DashStyle.DashDot);
            p2 = new Pen(Color.FromName("Red"));
            win = createwindow("Balance Demo", 250, 250);
            win.Show();
        }

        public void pwin(int coord, int n, double t)
        {
            int nx = ((n * 10) % 280) -  140 ;
            int ny = 4 * coord;
   
            string text = "(Acc=" + ny +  " Rate=" + String.Format ("{0:#.#}", t) + " ms)";
            plot (text, nx, ny);

            drawlist (g, new int[] {-125, 40, 125, 40},(Pen)((ny > 40)? p2 : p1)) ; //limit
            drawlist (g, new int[] {-125,-40, 125,-40},(Pen)((ny <-40)? p2 : p1)) ; //limit     
            //history (cons (list x y ) history))   
            if (hn < 998)
            {
                history[hn++] = nx; history[hn++] = ny;
            }
            else
                hn = 0;

            if (hn>6) 
                drawlist (g, new int[] { 
                    history[hn-6], 
                    history[hn-5], 
                    history[hn-4], 
                    history[hn-3], 
                    history[hn-2], 
                    history[hn-1]}, 
                    new Pen (Color.FromName("Blue")));
        }
    }
}

