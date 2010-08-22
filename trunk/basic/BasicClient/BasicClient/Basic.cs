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
        public Hashtable labels = new Hashtable();
        public Hashtable constants = new Hashtable();
        public int errno;
        public int lineno;
        public string curline;
        public string precomp;

        byte[] code;
        int codeptr;

        /**********************************************************/

        public string[] error_msgs = new string[] {
	        "OK",
	        "Syntax error",
	        "Invalid Command",
	        "Illegal var",
	        "Bad number",
	        "Next without for",
	        };
	
        public static string[] specials = new string[] { 
            "PF",   "MIC", "X",    "Y",   "Z",     "PSD", 
            "VOLT", "IR",  "KBD",  "RND", "SERVO", "TICK", 
		    "PORT", "ROM", "TYPE", "ABS", "KIR",   "FIND"  };

        enum SKEY
        {
            sPF1 = 0, sMIC, sGX, sGY, sGZ, sPSD, sVOLT, sIR, sKBD,
            sRND, sSERVO, sTICK, sPORT, sROM, sABS, sIR2ACT, sKIR, sFIND
        };
        
         public static  string[] tokens = new string[] {
            "LET",   "FOR",    "IF",     "THEN", 
            "ELSE",  "GOTO",   "PRINT",  "GET",
            "PUT",   "END",    "LIST",   "XACT",
            "WAIT",  "NEXT",   "SERVO",  "MOVE",
            "GOSUB", "RETURN", "POKE",   "STAND",
            "PLAY",  "OUT",    "OFFSET", "RUN", 
            "I2CO",  "I2CI",   "STEP",   "SPEED", 
	        "MTYPE", "LIGHTS",
            "ENDIF" // leave at end
        };
        
        enum KEY {
	        LET=0, FOR, IF, THEN, 
	        ELSE, GOTO, PRINT, GET, 
	        PUT, END, LIST, XACT, 
	        WAIT, NEXT, SERVO, MOVE,
	        GOSUB, RETURN, POKE, STAND,
            PLAY, OUT, OFFSET, RUN,
            I2CO, I2CI, STEP, SPEED, MTYPE, LIGHTS,
            ENDIF
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

            if (constants.ContainsKey(n))
                return (string)(constants[n]);

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

        public string[]  parse(string[] l)
        {
            string[] r = new string[l.Length];
            constants.Clear();
            labels.Clear();

            int lnc = 5;
            int i = 0;
            precomp = "";
            foreach (string s in l)
            {
                string z = s;
                if (z.IndexOf('\'') >= 0) z = z.Substring(0, z.IndexOf('\''));
                z = z.Trim();
                if (z.Length <= 0) continue;

                string tok = GetWord(ref z);

                if (z != "" && z[0] == ':')
                {
                    labels.Add(tok, lnc);  //label
                    z = z.Substring(1);
                    if (GetNext(ref z) != " ")
                    {
                        continue;
                    }
                    z = z.Trim();
                    tok = GetWord(ref z);
                }
                else
                {
                    if (tok.ToLower() == "const")
                    {
                        //
                        if (GetNext(ref z) != " ")
                        {
                            continue;
                        }
                        string n = GetWord(ref z);

                        if (GetNext(ref z) != " ")
                        {
                            continue;
                        }
                        string v = GetWord(ref z);
                        try
                        {
                            constants.Add(n, v);
                        }
                        catch
                        {
                            Console.WriteLine("Constant {0} {1} failed",n,v);
                        }
                        continue;
                    }
                }

                Console.WriteLine(lnc + " " + tok + z);
                r[i]= "" + lnc + " " + tok + z;
                precomp += r[i] + "\r\n";
                i++;
                lnc += 5;
            }
            //

            if (constants.Count > 0)
            {
                precomp += "-----------\r\nConstant defined\r\n";
                foreach (string n in constants.Keys)
                {
                    precomp = Regex.Replace(precomp, n, constants[n].ToString());
                    precomp += n + " " + constants[n] + "\r\n";
                }
            }

            if (labels.Count > 0)
            {
                precomp += "-----------\r\nLabels defined\r\n";
                foreach (string n in labels.Keys)
                {
                    precomp = Regex.Replace(precomp, n, labels[n].ToString(), RegexOptions.IgnoreCase);
                    precomp += n + " " + labels[n] + "\r\n";
                }
            }

            Console.WriteLine(precomp);
            return r;
        }

        public bool Compile(string prog)
        {
            errno = 0;
            lineno = 0;
            codeptr = 0;

            int[] nestedif   = new int[10];
            int nif = 0;
            int lnc = 5;

            basic_line ln;
            ln.token = 0;
            ln.lineno = lineno;
            string[] forbuf = new string[5];
            int fb = 0;
            labels.Clear();

            string[] lines = prog.Split('\n');

            string[] r = parse(lines);

            foreach (string s in r)
            {
                if (s==null) continue;

                Console.WriteLine(s);
                lineno++;
                curline = s;
                ln.lineno = lnc;
                String z = s;

                string tok = GetWord(ref z);

                if (IsNumber(tok))
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
                lnc += 5;

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
                        if (z.Length>0 && z[0] == '#') {ln.var=1; z=z.Substring(1); }
                        ln.text = process_arg(z);
                        break;
                    case KEY.MOVE:
                    case KEY.OUT:
                    case KEY.XACT:
                    case KEY.RUN:
                    case KEY.OFFSET:
                    case KEY.I2CI:
                    case KEY.I2CO:
                    case KEY.SPEED:
                    case KEY.MTYPE:
                    case KEY.LIGHTS:
                        ln.text = upperIt(z);
                        break;
                    case KEY.LIST:
                        t1 = GetVar(GetWord(ref z));
                        if (t1 < 0) errno = 3; else ln.var = (byte)t1;
                        if (GetNext(ref z) != "=") { errno = 1; }
                        ln.text = upperIt(z);
                        break;
                    case KEY.SERVO:
                    case KEY.STEP: 
                    string tz = GetWord(ref z);
                        if ((t = GetVar(tz)) < 0)
                        {
                            t = GetNumber(tz);
                            if (t < 0) errno = 3; else ln.var = (byte)(t + 32);
                        }
                        else
                        {
                            ln.var = (byte)t;
                        }
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

                        // mutliline else
                        if (z.EndsWith("THEN"))
                        {
                            if (codeptr==0) // first line
                                nestedif[nif++] = 3;
                            else
                                nestedif[nif++]   = codeptr;
                            z = Regex.Replace(z, "(.*) THEN", String.Format("($1 )?   {0} :    0000",ln.lineno+5));
                        }
                        else
                        {
                            foreach (string n in labels.Keys)
                            {
                                z = Regex.Replace(z, n, labels[n].ToString());
                            }

                            z = Regex.Replace(z, "(.*) THEN (.*) ELSE (.*)", "($1 )?   $2 :   $3");
                            z = Regex.Replace(z, "(.*) THEN (.*)", "($1 )?    $2:0");
                        }
                        ln.text = z;
                        break;
                    case KEY.ELSE:
                        // multilines else
                        if (nif > 0)
                        {
                            // update IF THEN to ELSE+5 line
                            int n = nestedif[nif - 1];
                            int ls = code[n + 6] + code[n + 7] * 256;
                            //
                            string tx = (ln.lineno + 5).ToString();
                            for (int x = 0; x < 4; x++)
                            {
                                if (x < tx.Length)
                                    code[ls - 5+x] = (byte)tx[x];
                                else
                                    code[ls - 5+x] = (byte)0;
                            }

                            nestedif[nif-1] = codeptr;
                            ln.token = (byte)KEY.GOTO;
                            ln.value = 0;
                        }
                        break;
                    case KEY.ENDIF:
                        if (nif > 0)
                        {
                            // update ELSE/GOTO to here
                            nif--;
                            int n = nestedif[nif];
                            if (n > 0)
                            {
                                if (code[n + 2] == (byte)KEY.IF)
                                {
                                    int ls = code[n + 6] + code[n + 7] * 256;
                                    //
                                    string tx = (ln.lineno + 5).ToString();
                                    for (int x = 0; x < 4; x++)
                                    {
                                        if (x < tx.Length)
                                            code[ls - 5 + x] = (byte)tx[x];
                                        else
                                            code[ls - 5 + x] = (byte)0;
                                    }
                                }
                                else
                                {
                                    code[n + 4] = (byte)((ln.lineno + 5) % 256);
                                    code[n + 5] = (byte)((ln.lineno + 5) / 256);
                                }
                            }
                            continue;
                        }
                        else
                        {
                            errno = 5;
                        }
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
