using System;
using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using System.Text.RegularExpressions;

/*
An elementry basic language for Robobuilder Humanoid Robot
$Revision$
 
Language Spec:
See wiki for details

http://code.google.com/p/robobuilderlib/wiki/Basic

*/

namespace RobobuilderLib
{
    class Basic
    {
        Hashtable labels = new Hashtable();

        byte[] code;
        int codeptr;

        public int errno;
        public int lineno; 
        public string curline;

        /**********************************************************/

        public string[] error_msgs = new string[] {
	        "OK",
	        "Syntax error",
	        "Invalid Command",
	        "Illegal var",
	        "Bad number",
	        "Next without for",
	        };
	
        string[] specials = new string[] { "PF", "MIC", "X", "Y", "Z", "PSD", "VOLT", "IR", "KBD", "RND", "SERVO", "TICK", 
		        "PORT", "ROM", "TYPE", "ABS"  };
        
        enum KEY {
	        LET=0, FOR, IF, THEN, 
	        ELSE, GOTO, PRINT, GET, 
	        PUT, END, LIST, XACT, 
	        WAIT, NEXT, SERVO, MOVE,
	        GOSUB, RETURN, POKE, STAND,
            PLAY, OUT, OFFSET, RUN
	        };
	
        string[] tokens = new string[] {
            "LET", "FOR", "IF","THEN", 
            "ELSE","GOTO","PRINT","GET",
            "PUT", "END", "LIST", "XACT",
            "WAIT", "NEXT", "SERVO", "MOVE",
            "GOSUB", "RETURN", "POKE", "STAND",
            "PLAY", "OUT", "OFFSET", "RUN"
        };

        enum SKEY {sPF1=0, sMIC, sGX, sGY, sGZ, sPSD, sVOLT, sIR, sKBD,
        sRND, sSERVO, sTICK, sPORT, sROM, sABS, sIR2ACT, sKIR, sFIND
        };
							
        struct basic_line {
            public int lineno;
            public byte token;
            public byte var;
            public int value;
            public string text; // rest of line - unproceesed
        };

        public Basic()
        {
            code = new byte[4096];
            codeptr = 0;
        }

        /*------------------------------------- -------------------------------------*/

        string GetWord(ref string w)
        {
            int i;
            string n = "";
            if (w.Length == 0) return "";
            for (i = 0; i < w.Length; i++)
            {
                if (!((w[i] >= 'A' && w[i] <= 'Z') || (w[i] >= 'a' && w[i] <= 'z') || (w[i] >= '0' && w[i] <= '9')))
                {
                    break;
                }
                n += w[i];
            }
            w = w.Substring(i);
            n = n.ToUpper();
            return n;
        }

        string GetNext(ref string w)
        {
            if (w.Length == 0) return "";
            string n = w.Substring(0,1);
            if (w.Length > 0) w = w.Substring(1);
            return n;
        }

        bool IsNumber(string s)
        {
            for (int i = 0; i < s.Length; i++)
            {
                if (s[i] < '0' || s[i] > '9') return false;
            }
            return true;
        }

        int GetNumber(string s)
        {
            if (IsNumber(s))
                return Convert.ToInt32(s);
            else
                return 0;
        }

        int GetVar(string s)
        {
            s = s.ToUpper();
            if (s.Length == 1 && s[0] >= 'A' && s[0] <= 'Z')
                return (int)(s[0] - 'A');
            else
                return -1;
        }

        int GetSpecial(ref string w)
        {
            if (w.Length == 0) return -1;

            string t = GetWord(ref w);

            for (int i = 0; i < tokens.Length; i++)
            {
                if (t.Equals(specials[i], StringComparison.CurrentCultureIgnoreCase))
                {
                    return i;
                }
            }
            return -1;
        }

        int IsToken(string w)
        {
            if (w.Length == 0) return -1;

            for (int i = 0; i < tokens.Length; i++)
            {
                if (w.Equals(tokens[i], StringComparison.CurrentCultureIgnoreCase)) return i;
            }
            return -1;
        }

        string upperIt(string txt)
        {
            string r = "";
            bool uf=true;
            for (int i = 0; i < txt.Length; i++)
            {
                if (txt[i] == '"') uf = !uf;
                if (uf && Char.IsLower(txt[i]))
                    r += Char.ToUpper(txt[i]);
                else
                    r += txt[i];
            }
            return r;
        }

        string process_arg(string a)
        {
            string r = "";
            bool sflag = false;
            for (int i = 0; i < a.Length; i++)
            {
                string c=a.Substring(i,1);
                if (c == "\"") sflag = !sflag;

                if (sflag && c == "\\")
                {
                    char ch = a[i + 1];
                    int n=0;
                    if (ch >= '0' && ch <= '9') n = ch - '0';
                    if (ch >= 'A' && ch <= 'Z') n = ch - 'A';
                    ch = a[i + 2];
                    if (ch >= '0' && ch <= '9') n = n * 16 + ch - '0';
                    if (ch >= 'A' && ch <= 'Z') n = n * 16 + ch - 'A';
                    r += Convert.ToChar(n);
                    i += 2;
                }
                else
                    r += c;
            }
            if (sflag) r += "\""; // add terminating quote if missing
            return r;
        }

        public bool Compile(string prog)
        {
            errno = 0;
            lineno = 0;
            codeptr = 0;

            basic_line ln;
            ln.token = 0;
            ln.lineno = lineno;
            string[] forbuf = new string[5];
            int fb = 0;
            labels.Clear();

            string[] lines = prog.Split('\n');
            foreach (string s in lines)
            {
                String z = s;
                lineno++;
                curline = s;
                ln.lineno = lineno;
                Console.Write(s);

                if (z.IndexOf('\'') >= 0) z = z.Substring(0, z.IndexOf('\''));
                z = z.Trim();
                if (z.Length <= 0) continue;

                string tok = GetWord(ref z);

                if (z != "" && z[0]==':')
                {
                    //label
                    labels.Add(tok, lineno);
                    z = z.Substring(1);
                    if (GetNext(ref z) != " ") { errno = 1; return false; }
                    z = z.Trim();
                    tok = GetWord(ref z);
                }
                else if (IsNumber(tok))
                {
                    ln.lineno = Convert.ToInt32(tok);
                    if (GetNext(ref z) != " ") { errno = 1; return false; }
                    z = z.Trim();
                    tok = GetWord(ref z);
                }

                int t = IsToken(tok);
                if (t<0)
                {
                    errno = 1;
                    return false;
                }
                ln.token = (byte)t;
                ln.var = (byte)'\0';
                ln.text = "";
                ln.value = 0;

                if ((z != "") && (GetNext(ref z) != " ")) { errno = 1; return false; }

                z = z.Trim();

                /*************/

                switch ((KEY)t)
                {
                    case KEY.LET:
                    case KEY.GET:
                    case KEY.FOR:
                        int t1= GetVar(GetWord(ref z));
                        if (t1 < 0) errno = 3; else ln.var = (byte)t1;
                        if (GetNext(ref z) != "=") { errno = 1; }
                        if ((KEY)t == KEY.FOR)
                        {
                            int p = z.ToUpper().IndexOf(" TO ");
                            if (p > 0)
                            {
                                ln.text = z.Substring(0, p);
                                forbuf[fb++] = z.Substring(p + 4);
                            }
                            else
                            {
                                errno = 1;
                            }
                        }
                        else
                            ln.text = upperIt(z);
                        break;
                    case KEY.PRINT:
                        z = upperIt(z);
                        if (z[0] == '#') {ln.var=1; z=z.Substring(1); }
                        ln.text = process_arg(z);
                        break;
                    case KEY.MOVE:
                    case KEY.OUT:
                    case KEY.XACT:
                    case KEY.RUN:
                    case KEY.OFFSET:
                        ln.text = upperIt(z);
                        break;
                    case KEY.LIST:
                        t1 = GetVar(GetWord(ref z));
                        if (t1 < 0) errno = 3; else ln.var = (byte)t1;
                        if (GetNext(ref z) != "=") { errno = 1; }
                        ln.text = upperIt(z);
                        break;
                    case KEY.SERVO:
                        t = GetNumber(GetWord(ref z));
                        if (t < 0) errno = 3; else ln.var = (byte)t;
                        if (GetNext(ref z) != "=") { errno = 1; }
                        ln.text = upperIt(z);
                        break;
                    case KEY.POKE:
                        t = GetNumber(GetWord(ref z));
                        if (t < 0) errno = 3; else ln.var = (byte)t;
                        if (GetNext(ref z) != ",") { errno = 1; }
                        ln.text = upperIt(z);
                        break;
                    case KEY.PUT:
                        t = GetSpecial(ref z);
                        if (t < 0) errno = 3; else ln.var = (byte)t;
                        if ((SKEY)t == SKEY.sPORT)
                        {
                            if (GetNext(ref z) != ":") { errno = 3; }
                            string pn = GetNext(ref z);
                            if (pn[0] < 'A' || pn[0] > 'G') { errno = 3; }
                            if (GetNext(ref z) != ":") { errno = 3; }
                            string pb = GetNext(ref z);
                            if (pb[0] < '0' || pb[0] > '8') { errno = 3; }
                            ln.var = (byte)(30 + (int)(pn[0] - 'A') * 10 + (int)(pb[0] - '0'));
                        }
                        if (GetNext(ref z) != "=") { errno = 1; }
                        ln.text = upperIt(z);
                        break;
                    case KEY.WAIT:
                    case KEY.STAND:
                    case KEY.GOTO:
                    case KEY.GOSUB:
                    case KEY.PLAY: 
                        tok = GetWord(ref z);

                        if (labels.Contains(tok))
                        {
                            ln.value = (int)labels[tok];
                        }
                        else
                        {
                            ln.value = GetNumber(tok);
                        }
                        break;
                    case KEY.END:
                    case KEY.RETURN:
                        ln.text = "";
                        break;
                    case KEY.NEXT:
                        t = GetVar(GetWord(ref z));
                        if (t < 0) errno = 3; else ln.var = (byte)t;
                        if (fb > 0)
                        {
                            ln.text = forbuf[--fb];
                        }
                        else
                        {
                            errno = 5;
                        }
                        break;
                    case KEY.IF:
                        // IF A THEN B ELSE C =>  GOTO (A)?B:C
                        z = z.ToUpper();
                        z = Regex.Replace(z, "(.*) THEN (.*) ELSE (.*)", "($1)?$2:$3");
                        z = Regex.Replace(z, "(.*) THEN (.*)", "($1)?$2:0");
                        //Console.WriteLine("IF=" + z);
                        ln.text = upperIt(z);
                        break;
                    default:
                        errno = 2;
                        break;
                }

                if (errno != 0) return false;

                /*************/

                if (codeptr == 0)
                {
                    code[codeptr++] = (byte)0xAA; // start of code
                    code[codeptr++] = (byte)0x03; // start pointer
                    code[codeptr++] = (byte)0x00; // 
                }

                code[codeptr++] = (byte)(ln.lineno % 256);
                code[codeptr++] = (byte)(ln.lineno / 256);
                code[codeptr++] = ln.token;
                code[codeptr++] = ln.var;
                code[codeptr++] = (byte)(ln.value % 256);
                code[codeptr++] = (byte)(ln.value / 256);
                int l = codeptr + ln.text.Length + 3;
                code[codeptr++] = (byte)(l % 256);
                code[codeptr++] = (byte)(l / 256);
                for (int k = 0; k < ln.text.Length; k++)
                {
                    code[codeptr++] = (byte)ln.text[k];
                }
                code[codeptr++] = 0;
                code[codeptr] = (byte)0xCC; // start of code

                /*************/

            }
            
            return true;
        }

        public string Download()
        {
            // transfer code --> robot
            codeptr++; // (include terminator 0xCC )

            string s = codeptr.ToString("X4");
            s = "";

            for (int i = 0; i < codeptr; i++)
            {
                s += code[i].ToString("X2");
            }

            Console.WriteLine(s);

            return s;
        }

        public string Dump()
        {
            string m = "";
            for (int i = 0; i < codeptr + 1; i+=24)
            {
                m += (i.ToString("X4") + " - ");
                for (int j = 0; j < 24; j++)
                {
                    m += (code[i+j].ToString("X2") + "  ");
                }
                m += "\r\n" ;
            }
            Console.Write(m);
            return m;
        }
    }
}
