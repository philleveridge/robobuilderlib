using System;
using System.Collections.Generic;
using System.Text;

namespace RobobuilderLib
{
    class test
    {
        static void Main(string[] argv)
        {
            string fn = "";
            string pn = "COM40";

            if (argv.Length > 0) pn = argv[0];
            if (argv.Length > 1) fn = argv[1];

            Console.WriteLine("Robos test client {0} - {1}", pn, fn);

            try
            {

                PCremote r = new PCremote(pn);

                Console.WriteLine("Ver={0}", r.readVer());

                int[] n = r.readXYZ();
                Console.WriteLine("Acc={0},{1},{2}", n[0], n[1], n[2]);

                Console.WriteLine("PSD={0}", r.readPSD());

                wckMotion w = new wckMotion(r);

                if (fn != "")
                    w.PlayFile(fn);
                else
                    w.PlayPose(1000, 10, new byte[] { 143, 179, 198, 83, 106, 106, 69, 48, 167, 141, 47, 47, 49, 199, 204, 204, 122, 125, 127 }, true);

            }
            catch (Exception e)
            {
                Console.WriteLine("Test failed = " + e.Message);
            }

            Console.WriteLine("Finished - press a key"); while (!Console.KeyAvailable) ;

        }
    }
}
