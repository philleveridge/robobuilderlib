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

        int gAction = 0;

        RobobuilderLib.wckMotion w;
        Program p;

        string greet;
        String[] chs = new String[] { "stop", "stand", "close", "open", "left", "right", "forward", "look", "exit" };


        public Speech(Program p1, String[] c)
        {
            if (c != null) chs = c;
            setup(p1);
        }

        public Speech(Program p1)
        {
            setup(p1);
        }

        public void setup(Program p1)
        {
            p = p1;
            w = p.w;



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


            System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo("en-GB");
            rec = new SpeechRecognitionEngine();
            Grammar g = new Grammar(new GrammarBuilder(new Choices(chs)));
            rec.UnloadAllGrammars();
            rec.LoadGrammar(g);
            rec.SetInputToDefaultAudioDevice();

            g.SpeechRecognized += new EventHandler<SpeechRecognizedEventArgs>(SpeechRecognised);

        }

        void speak(string txt)
        {
            sp.Speak(txt);
            Console.WriteLine(txt);
        }

        private void SpeechRecognised(object sender, RecognitionEventArgs e)
        {
                foo(e.Result);

                speak("Ready");
                Console.Write("> ");
        }

        private bool foo(RecognitionResult r)
        {
            bool rslt = true;
            Grip gr = new Grip(p);
            Scan sn = new Scan(p);

            Console.WriteLine("{0} : {1}%", r.Text, r.Confidence);

            switch (r.Text)
            {
                case "stop":
                    gAction = 1;
                    break;
                case "exit":
                    speak("Goodbye");
                    gAction = 1;
                    rslt = false;
                    break;
                case "stand":
                    p.standup();
                    break;
                case "open":
                    if (p.gripservo)
                        gr.opengripper(5);
                    else
                        speak("no");
                    break;
                case "close":
                    if (p.gripservo)
                        gr.closegripper(5);
                    else
                        speak("no");
                    break;
                case "left":
                    if (p.headservo)
                        p.headleft();
                    else
                        speak("no");
                    break;
                case "right":
                    if (p.headservo)
                        p.headright();
                    else
                        speak("no");
                    break;
                case "forward":
                    if (p.headservo)
                        p.headfw();
                    else
                        speak("no");
                    break;
                case "look":
                    if (p.headservo)
                        sn.scan();
                    else
                        speak("no");
                    break;
            }
            return rslt;
        }

        public void voicemenu() // synchronous voice menu
        {
            sp.Speak(greet);
            Console.WriteLine("Options:");
            foreach (string pmpt in chs)
                Console.WriteLine("> " + pmpt);

            speak("Ready");
            Console.Write("> ");

            gAction = 0;

            while (gAction!=1)
            {
                RecognitionResult r = rec.Recognize();

                if (r == null)
                {
                    speak("Timeout");
                    return;
                }
            }
        }

        public void voicemenu2() // A-synchronous voice menu
        {
            sp.Speak(greet);
            Console.WriteLine("Options:");
            foreach (string pmpt in chs)
                Console.WriteLine("> " + pmpt);

            rec.RecognizeAsync(RecognizeMode.Multiple);

            while (gAction != 1)
            {
                System.Threading.Thread.Sleep(50);
            }
        }
    }
}
