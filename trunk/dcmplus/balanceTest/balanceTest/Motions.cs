using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Demo
{

    class Motions
    {
        bool first = false;
        RobobuilderLib.trigger trg;

        byte[][] rstep = new byte[][] {
            new byte[] {123, 156, 212,  80, 108, 126, 73, 40, 150, 141,  68, 44, 40, 138, 208, 195},
            new byte[] {130, 165, 201,  81, 115, 134, 81, 31, 147, 149,  72, 44, 40, 145, 209, 201},
            new byte[] {132, 171, 197,  83, 117, 137, 86, 28, 148, 152,  78, 43, 41, 154, 209, 206},
            new byte[] {132, 175, 195,  87, 117, 139, 91, 27, 152, 154,  87, 43, 43, 164, 209, 211},
            new byte[] {132, 178, 197,  91, 117, 137, 95, 28, 157, 152,  97, 43, 48, 172, 209, 213},
            new byte[] {130, 179, 201,  95, 115, 134, 96, 31, 161, 149, 105, 43, 53, 179, 210, 214},
            new byte[] {127, 178, 206,  98, 112, 130, 95, 35, 166, 145, 111, 42, 57, 182, 210, 214},
            new byte[] {124, 175, 212, 100, 109, 127, 92, 40, 170, 142, 113, 42, 59, 183, 210, 214}
        };

        byte[][] lstep = new byte[][] {
            new byte[] {124, 175, 212, 100, 109, 127, 92, 40, 170, 142, 113, 42, 59, 183, 210, 214},
            new byte[] {120, 172, 217, 102, 105, 123, 88, 46, 170, 138, 111, 42, 57, 182, 210, 214},
            new byte[] {116, 167, 221, 103, 101, 120, 83, 51, 169, 135, 106, 43, 53, 179, 210, 214},
            new byte[] {113, 162, 224, 102,  98, 118, 77, 55, 167, 133, 97,  43, 48, 173, 209, 213},
            new byte[] {111, 157, 225, 98,   96, 118, 73, 57, 163, 133, 87,  43, 43, 164, 209, 211},
            new byte[] {113, 153, 224, 93,   98, 118, 70, 55, 159, 133, 79,  43, 41, 154, 209, 206},
            new byte[] {116, 152, 221, 89,  101, 120, 69, 51, 155, 135, 72,  44, 40, 146, 209, 201},
            new byte[] {120, 153, 217, 84,  105, 123, 70, 46, 152, 138, 69,  44, 40, 140, 208, 197},
            new byte[] {123, 156, 212, 80,  108, 126, 73, 40, 150, 141, 68,  44, 40, 138, 208, 195}
            };

        void trgAct (bool st, RobobuilderLib.wckMotion w)
        {
            trg = new RobobuilderLib.trigger();

            trg.set_PSD(0, 51);     //trigger if less than 0 or more than 51 i.e. never - but exercise routine
            trg.timer = 250;     //every 250 ms
            trg.print();
            w.set_trigger(trg);
            trg.activate(st);   //activate trigger (if true)
            trg.dbg = true;      // debug on
            trg.DCMP = true;
        }
 
        public void dtest (RobobuilderLib.wckMotion w) 
        {
           while (!Console.KeyAvailable)
           {
               w.wckReadPos(30, 5);
               Console.WriteLine("*".PadLeft(w.respnse[0],'='));
            }
        }

        byte[][] reverse(byte[][] z)
        {
            byte[][] r = z;
            // tbd
            return r;
        }

        byte[] cv18(byte[] a) // hip conversion
        {
            a[0] += 18;
            a[5] -= 20; 
            return a;
        }

        bool lpose(int dur, int stp, byte[][] pos, bool lpst, RobobuilderLib.wckMotion w)
        {
            for (int i=0; i<pos.Length; i++)
            {
                first = false;
                if (!w.PlayPose(dur, stp, cv18(pos[i]), first))
                    return false;
            }
            return true;
        }

        public void panda (Program p)
        {
            bool lpst = false;
            int cnt= 0;
            bool dir = true;

            p.readdistance();  
            Program.pause();

            //(setallLeds 15 dir (not dir))
   
            p.w.reset_timer();
   
            while (! (Console.KeyAvailable))
            {
                if (dir)
                {
                    lpst = lpose(25, 1, rstep, false,p.w);
                    Console.Write("R");
                    lpst = lpose(25, 1, lstep, false, p.w);
                    Console.Write("L");
                }
                else
                {
                    lpst = lpose(25, 1, reverse(lstep), false, p.w);
                    Console.Write("l");
                    lpst = lpose(25, 1, reverse(rstep), false, p.w);
                    Console.Write("r");
                }
				if (cnt++ >4) 
                {
					cnt= 0; 
					dir = !dir;
					//	;(setallLeds 15 dir (not dir))
                }
            }
	        //;(lpose 25 1 (list basic) false)
	        p.standup();
            //(setallLeds 15 false false)
	        //(dcmodeOff)
        }

    }
}
