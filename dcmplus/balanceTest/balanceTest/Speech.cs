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
        Program p;

        string greet;

        public Speech(Program p1)
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


        }

        void speak(string txt)
        {
            sp.Speak(txt);
            Console.WriteLine(txt);
        }

        public void voicemenu()
        {
            String[] c = new String[] { "stand", "close", "open", "left", "right", "forward", "look", "exit" };
            voicemenu(c);
        }

        public void voicemenu(String[] c)
        {
            if (rec == null)
            {
                System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo("en-GB");
                rec = new SpeechRecognitionEngine();
                Grammar g = new Grammar(new GrammarBuilder(new Choices(c)));
                rec.LoadGrammar(g);
                rec.SetInputToDefaultAudioDevice();
            }


            Grip gr = new Grip(p);
            Scan sn = new Scan(p);

            sp.Speak(greet);
            Console.WriteLine("Options:");
            foreach (string pmpt in c)
                Console.WriteLine("> " + pmpt);

            bool loop = true;
            while (loop)
            {
                speak("Ready");
                Console.Write("> ");
                RecognitionResult r = rec.Recognize();

                if (r == null)
                {
                    speak("Timeout");
                    return;
                }
                
                Console.WriteLine("{0} : {1}%", r.Text, r.Confidence);

                switch (r.Text)
                {
                    case "exit":
                        speak("Goodbye");
                        loop = false;
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
            }
        }
    }
}
