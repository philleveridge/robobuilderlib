using System;
using System.Text;

using System.Speech.Synthesis;
using System.Speech.Recognition;

namespace Demo
{
    class Speech
    {
        SpeechSynthesizer sp;
        SpeechRecognitionEngine rec = null;

        RobobuilderLib.wckMotion w;

        string greet;
        String[] c = new String[] { "stand", "close", "open", "left", "right", "forward", "scan", "exit" };

        public Speech()
        {
            sp = new SpeechSynthesizer();

            if (DateTime.Now.Hour < 12) 
            {
                greet = "Good Morning" ;
            }
            else if (DateTime.Now.Hour < 18)
            {
                greet = "Good Afternoon";
            }
            else
                greet = "Good Evening";

            if (rec == null)
            {
                System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo ("en-GB");
                rec = new SpeechRecognitionEngine();
                Grammar g = new Grammar(new GrammarBuilder(new Choices(c)));
                rec.LoadGrammar(g);
                rec.SetInputToDefaultAudioDevice();
            }
        }

        public void voicemenu(Program p)
        {
            w = p.w;
            sp.Speak(greet);
            Console.WriteLine("Options:");
            foreach (string pmpt in c)
                Console.WriteLine("> " + pmpt);

            bool loop = true;
            while (loop)
            {
                sp.Speak("Ready");
                Console.Write("Ready> ");
                RecognitionResult r = rec.Recognize();

                if (r == null)
                {
                    sp.Speak("Timeout");
                    return;
                }
                
                Console.WriteLine("{0} : {1}%", r.Text, r.Confidence);

                switch (r.Text)
                {
                    case "exit":
                        sp.Speak("Goodbye");
                        loop = false;
                        break;
                    case "stand":
                        p.standup();
                        break;
                    case "open":
                        p.opengripper(5);
                        break;
                    case "close":
                        p.closegripper(5);
                        break;
                    case "left":
                        p.headleft();
                        break;
                    case "right":
                        p.headright();
                        break;
                    case "forward":
                        p.headfw();
                        break;
                    case "scan":
                        p.scan();
                        break;
                }
            }
        }
    }
}
