using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Demo
{
    class Grip
    {
        Program p;

        static public int SERVOID = 18;

        public Grip(Program p1)
        {
            p = p1;
        }

        int widepos = 70;
        int closepos = 120;
        int maxit = 15;
        double dt = 0.15;

        public void opengripper(int d)
        {
            int cnt = 0;
            int cp = p.getServoPos(SERVOID);
            int np = 0;
            int delta = 5;

            if (cp < widepos)
                cp = widepos;

            while ((cnt < maxit) && (Math.Abs(cp - widepos) > 2) && (delta > 0))
            {
                cnt++;
                Console.WriteLine(cnt + ", " + cp + ", " + delta);

                p.setServoPos(SERVOID, cp - d, 4);
                Program.sleep(dt);
                np = p.getServoPos(SERVOID);
                delta = Math.Abs(cp - np);
                cp = np;
            }
        }

        public void closegripper(int d)
        {
            int cnt = 0;
            int cp = p.getServoPos(SERVOID);
            int np = 0;
            int delta = 5;

            if (cp > closepos)
                cp = closepos;

            while ((cnt < maxit) && (Math.Abs(cp - closepos) > 2) && (delta > 0))
            {
                cnt++;
                Console.WriteLine(cnt + ", " + cp + ", " + delta);
                p.setServoPos(SERVOID, d + cp, 2);
                Program.sleep(dt);
                np = p.getServoPos(SERVOID);
                delta = Math.Abs(cp - np);
                cp = np;
            }
        }

        public void gripper_test()
        {
            Console.WriteLine("Gripper test (servo: {0}) - [o] open, [c] close [x/q] exit", SERVOID);
            bool loop = true;

            while (loop)
            {
                int n = 5;
                char ch;
                while (!Console.KeyAvailable) ;
                ch = Console.ReadKey(true).KeyChar;

                if (!(ch == '+' || ch == '-'))
                    ch = Char.ToUpper(ch);

                switch (ch)
                {
                    case 'E':
                        Console.WriteLine("Exercise", n);
                        for (int c = 0; c < 4; c++)
                        {
                            for (int i = widepos; i < closepos; i++)
                            {
                                Console.WriteLine("Pos {0}", i);
                                p.setServoPos(18, i, 4);
                                Program.sleep(.025);
                            }
                            for (int i = closepos; i > widepos; i--)
                            {
                                Console.WriteLine("Pos {0}", i);
                                p.setServoPos(18, i, 4);
                                Program.sleep(.025);
                            }
                        }
                        break;
                    case 'O':
                        Console.WriteLine("Open {0}", n);
                        opengripper(n);
                        break;
                    case 'C':
                        Console.WriteLine("Close {0}", n);
                        closegripper(n);
                        break;
                    case 'X':
                    case 'Q':
                        loop = false;
                        break;
                    case '+':
                        Console.WriteLine("Inc", ++n);
                        break;
                    case '-':
                        Console.WriteLine("Dec", --n);
                        break;
                }
            }
        }
    }
}
