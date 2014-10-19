using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Windows.Forms;
using System.Drawing;
using CPI.Plot3D;

namespace RobobuilderLib
{
    class RobotModel
    {
        float x = 0.0f, y = -30.0f, z = -600.0f;

        float cx = 300.0f, cy = 50.0f, cz = 200.0f;

        int sh = 30, sw = 30;

        public bool showrightarm = true;
        public bool showleftarm = true;
        public bool showrightleg = true;
        public bool showleftleg = true;
        public bool showaxis = true;
        public int[] servos;

        Graphics g;

        public RobotModel(Graphics g1)
        {
            g = g1;
            servos = new int[32];
            for (int i=0; i<32; i++) servos[i]=0;
        }

        public void DrawSquare(Plotter3D p, float sideLength)
        {
            p.Forward(-sideLength / 2);
            p.TurnLeft(90);
            p.Forward(-sideLength / 2);
            for (int i = 0; i < 4; i++)
            {
                p.Forward(sideLength);  // Draw a line sideLength long
                p.TurnRight(90);        // Turn right 90 degrees
            }
            p.Forward(sideLength / 2);
            p.TurnRight(90);
            p.Forward(sideLength / 2);
        }

        public void DrawRect(Plotter3D p, float a, float b)
        {
            for (int i = 0; i < 2; i++)
            {
                p.Forward(a);  // Draw a line sideLength long
                p.TurnRight(90);        // Turn right 90 degrees

                p.Forward(b);  // Draw a line sideLength long
                p.TurnRight(90);        // Turn right 90 degrees
            }
        }

        public void DrawCube(Plotter3D p, float sideLength)
        {
            p.PenUp();

            p.Forward(-sideLength / 2);
            p.TurnLeft(90);
            p.Forward(-sideLength / 2);
            p.TurnUp(90);
            p.Forward(-sideLength / 2);
            p.PenDown();

            for (int i = 0; i < 4; i++)
            {
                DrawRect(p, sideLength, sideLength);
                p.Forward(sideLength);
                p.TurnDown(90);
            }

            p.PenUp();
            p.Forward(sideLength / 2);
            p.TurnDown(90);
            p.Forward(sideLength / 2);
            p.TurnRight(90);
            p.Forward(sideLength / 2);
        }

        public void DrawServo(Plotter3D p)
        {
            DrawCube(p, sw);
            //DrawCube(p, sw/2);
            //p.Forward(sw / 4);
            //DrawCube(p, sw/1.3f);
            //p.Forward(-sw / 4);
        }

        public void DrawHandL(Plotter3D p)
        {
            int sideLength = sw;

            p.PenUp();
            p.Forward(-sideLength / 2);
            p.TurnLeft(90);
            p.Forward(-sideLength / 2);
            p.TurnUp(90);
            p.Forward(-sideLength / 2);
            p.PenDown();

            for (int i = 0; i < 4; i++)
            {
                if (i == 2) DrawRect(p, sideLength, sideLength);
                p.Forward(sideLength);
                p.TurnDown(90);
            }
            p.PenUp();
            p.Forward(sideLength / 2);
            p.TurnDown(90);
            p.Forward(sideLength / 2);
            p.TurnRight(90);
            p.Forward(sideLength / 2);
        }

        public void DrawHandR(Plotter3D p)
        {
            int sideLength = sw;

            p.PenUp();
            p.Forward(sideLength / 2);
            p.TurnLeft(90);
            p.Forward(sideLength / 2);
            p.TurnUp(90);
            p.Forward(sideLength / 2);
            p.PenDown();

            for (int i = 0; i < 4; i++)
            {
                if (i == 0) DrawRect(p, -sideLength, -sideLength);
                p.Forward(-sideLength);
                p.TurnDown(90);
            }
            p.PenUp();
            p.Forward(-sideLength / 2);
            p.TurnDown(90);
            p.Forward(-sideLength / 2);
            p.TurnRight(90);
            p.Forward(-sideLength / 2);
        }



        public void DrawArmR(Plotter3D p)
        {
            double ang1 = 360.0 * servos[10] / 254.0;
            double ang2 = 360.0 * servos[11] / 254.0;
            double ang3 = 360.0 * servos[12] / 254.0; 

            DrawServo(p);
            p.PenUp();
            p.Forward(40);

            p.Orientation.RollLeft(-ang1);
            p.TurnLeft(-ang2);
            DrawServo(p);

            p.TurnUp(ang3);
            p.Forward(50);
            DrawServo(p);
            p.Forward(50);
            DrawHandR(p);
            p.Forward(-100);
            p.TurnDown(ang3);
            p.TurnRight(-ang2);
            p.Orientation.RollLeft(ang1);

            p.Forward(-40);
        }


        public void DrawArmL(Plotter3D p)
        {
            double ang1 = 360.0 * servos[13] / 254.0;
            double ang2 = 360.0 * servos[14] / 254.0;
            double ang3 = 360.0 * servos[15] / 254.0; 

            DrawServo(p);
            p.PenUp();
            p.Forward(-40);

            p.Orientation.RollRight(ang1);
            p.TurnLeft(ang2);
            DrawServo(p);
            p.TurnDown(ang3);
            p.Forward(-50);

            DrawServo(p);
            p.Forward(-50);
            DrawHandL(p);
            p.Forward(100);

            p.TurnUp(ang3);
            p.TurnRight(ang2);
            p.Orientation.RollRight(-ang1);

            p.Forward(40);
        }

        public void DrawFoot(Plotter3D p)
        {
            p.Forward(20);
            p.TurnUp(90);
            p.TurnRight(-90);
            p.PenDown();
            DrawRect(p, -sw, sw * 2);
            p.PenUp();
            p.TurnLeft(-90);
            p.TurnUp(-90);
            p.Forward(-20);

        }

        public void DrawRightLeg(Plotter3D p)
        {
            DrawServo(p);
            p.Forward(45);
            DrawServo(p);
            p.Forward(45);
            DrawServo(p);
            DrawFoot(p);
            p.Forward(-90);
        }

        public void DrawLeftLeg(Plotter3D p)
        {
            DrawServo(p);
            p.Forward(45);
            DrawServo(p);
            p.Forward(45);
            DrawServo(p);
            DrawFoot(p);
            p.Forward(-90);
        }

        public void DrawBody(Plotter3D p)
        {
            //p.Orientation = new Orientation3D();
            //p.Location = new CPI.Plot3D.Point3D(cx, cy, cz);
            //DrawCube(p, 50);
        }

        public void DrawRobot(Plotter3D p)
        {
            //DrawBody(p);
            DrawBody(p);
            p.PenUp();
            p.Forward(40);
            if (showrightarm) DrawArmR(p);

            p.TurnRight(90);
            p.Forward(80);
            if (showrightleg) DrawRightLeg(p);

            p.PenUp();
            p.Forward(-80);
            p.TurnLeft(90);
            p.Forward(-80);
            if (showleftarm) DrawArmL(p);

            p.PenUp();

            p.TurnRight(90);
            p.Forward(80);

            if (showleftleg) DrawLeftLeg(p);
        }

        private void axis(Plotter3D p)
        {
            p.Orientation = new Orientation3D();
            p.Location = new CPI.Plot3D.Point3D(cx, cy, cz);

            p.Pen = new Pen(Color.Green);
            p.PenDown();

            p.Forward(100);
            p.Forward(-200);
            p.Forward(100);
            p.TurnRight(90);

            p.Pen = new Pen(Color.Blue);

            p.Forward(100);
            p.Forward(-200);
            p.Forward(100);
            p.TurnUp(90);

            p.Pen = new Pen(Color.Red);

            p.Forward(100);
            p.Forward(-200);
            p.Forward(100);

            p.Pen = new Pen(Color.Black);
            p.Orientation = new Orientation3D();

        }

        public void UpdateDisplay()
        {
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
            using (CPI.Plot3D.Plotter3D p = new CPI.Plot3D.Plotter3D(g, new Point3D(x, y, z)))
            {
                g.Clear(Color.Beige);

                if (showaxis) axis(p);

                DrawRobot(p);
            }
        }

        public void SpinDisplay()
        {
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
            using (CPI.Plot3D.Plotter3D p = new CPI.Plot3D.Plotter3D(g, new Point3D(x, y, z)))
            {
                for (int angle = 0; angle <= 360; angle += 10)
                {
                    System.Threading.Thread.Sleep(50);
                    g.Clear(Color.Beige);

                    if (showaxis) axis(p); 
                    p.TurnUp(angle);
                    DrawRobot(p);
                }
            }
        }
    }
}
