using System;

namespace Demo
{
    public class cell 
    { 
        public object head; 
        public object tail;

        public cell()
        {
            head = null; tail = null;
        } 
    };

    public class function
    {
        public string name    = "";
        public bool   builtin = false;
    }

    public class environ
    {
        int cnt=0;
        const int MAX=30;
        string reserve = "car.cdr.prn.set.cons.plus.minus.quote.atomp.listp.";
        string[] name = new string[MAX];
        object[] val  = new object[MAX];

        public void set(string n, object v)
        {
            //update
            for (int i = 0; i < cnt; i++)
            {
                if (name[i] == n)
                {
                    val[i] = v;
                    return;
                }
            }
            //create
            name[cnt] = n;
            val[cnt] = v;
            if (cnt < MAX - 1) cnt++;
        }

        public object find(string n)
        {
            if (reserve.IndexOf(n) > 0)
            {
                function f=new function();
                f.name=n;
                f.builtin = true;
                return f;
            }

            for (int i = 0; i < cnt; i++)
            {
                if (name[i] == n)
                    return val[i];
            }
            return null;
        }
    }

    class Program
    {
        string buffer = "";
        environ env = new environ();

        void printstr(string s)
        {
            Console.Write(s);
        }

        void printline(string s)
        {
            printstr(s);
            printstr("\r\n");
        }

        object car(object list)
        {
            if (list is cell)
            {
                return ((cell)list).head;
            }
            else
                return null;
        }

        object plus(cell top)
        {
            if (top == null)
                return 0;

            if (top.head is int)
            {
                return (int)top.head + (int)plus((cell)top.tail);
            }
            else
                return 0;
        }

        object minus(cell top)
        {
            if (top == null)
                return 0;

            if (top.head is int)
            {
                return (int)top.head - (int)plus((cell)top.tail);
            }
            else
                return 0;
        }

        object cdr(cell list)
        {
            if (list != null)
            {
                return list.tail;
            }
            else
                return null;
        }

        object list(object a, object b)
        {
            return null; //tbd
        }

        object cons(object a, object b)
        {
            if (atomp(a) && listp(b))
            {
                cell n = new cell();
                n.head = a;
                n.tail = b;
                return n;
            }
            return null;
        }

        bool atomp(object x)
        {
            return !listp(x);
        }

        bool listp(object x)
        {
            return (x is cell);
        }

        int getch()
        {
            if (buffer.Length > 0)
            {
                int c = buffer[0];
                buffer = buffer.Substring(1);
                return c;
            }
            return -1;
        }

        void  ungetch(int ch)
        {
            buffer = (char)ch + buffer;
        }

        int readdigit()
        {
            int num = 0;
            int ch = 0;
            while ((ch = getch()) > 0)
            {
                if (!Char.IsDigit((char)ch))
                {
                    ungetch(ch);
                    return num;
                }
                num = num * 10 + (ch - '0');
            }
            return num;
        }

        bool isWhiteSpace(int ch)
        {
            return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
        }

        bool isSymbol(int ch)
        {
            return (ch == '+' || ch == '-' || ch == '_' || ch == '.');
        }

        bool isLetterorDigit(int ch)
        {
            return ((ch >= 'A' && ch <= 'Z' ) ||
             (ch >= 'a' && ch <= 'z' ) ||
             (ch >= '0' && ch <= '9' ) );
        }

        void readwhitespace()
        {
            int ch = 0;
            while ((ch = getch()) > 0 && isWhiteSpace((char)ch)) ;
            ungetch(ch);
        }

        string readstring()
        {
            string str = "";
            int ch = 0;
            while ((ch = getch()) > 0)
            {
                if (ch=='"')
                    return str;
                str += (char)ch;
            }
            return str;
        }

        string readtoken()
        {
            string str = "";
            int ch = 0;
            while ((ch = getch()) > 0)
            {
                if (isWhiteSpace((char)ch))
                    return str;
                str += (char)ch;
            }
            return str;
        }

        cell evalist()
        {
            int ch;
            cell top = new cell();

            cell nxt = top;            
            cell prv = null;

            while ((ch = getch()) > 0)
            {
                if ((char)ch == ')')
                {
                    if (prv != null)
                        prv.tail=null;
                    return top;
                }
                ungetch(ch);
                nxt.head = eval();
                nxt.tail = new cell();
                prv=nxt;
                nxt=(cell)nxt.tail;
            }
            return top;
        }

        public object eval()
        {
            object r = null;
            int ch;
            bool qf = false;

            while ((ch = getch()) > 0)
            {
                if (isWhiteSpace((char)ch))
                {
                    qf = false;
                    readwhitespace();
                    continue;
                }

                if (ch >= '0' && ch <= '9')
                {
                    ungetch(ch);       
                    return (object)readdigit();
                }
                if (ch == '"')
                {
                    return (object)readstring();
                }
                if (ch == '\'') //quote shortcut
                {
                    qf = true;
                }

                if (ch == '(')
                {
                    cell args = evalist();
                    if (!qf)
                        return callFunction(args);
                    else
                        return args;
                }

                if ( (ch>= 'A' && ch<= 'Z' ) || (ch>= 'a' && ch<= 'z' ))
                {
                    ungetch(ch);
                    string t=readtoken();

                    if (!qf)
                    {
                        object o = env.find(t);
                        if (o == null)
                        {
                            function f = new function();
                            f.name = t;
                            return f;
                        }
                        return o;
                    }
                    else
                    {
                        return t;
                    }
                }
            }
            return r;
        }

        cell set(cell x)
        { 
            // set x y
            env.set((string)x.head, x.tail);
            return (cell)x.tail;
        }

        object prn(cell x)
        {
            object r = pr(x);
            printline("");
            return r;
        }

        object pr(cell x)
        {
            cell nxt = x;
            object r=null;
            while (nxt != null)
            {
                r = nxt.head;
                if (nxt.head is string)
                {
                    printstr((string)nxt.head);
                }
                else if (nxt.head is int)
                {
                    printstr(((int)nxt.head).ToString());
                }
                else if (nxt.head is function)
                {
                    printstr("Function: " + ((function)nxt.head).name);
                }
                else if (nxt.head is cell)
                {
                    printstr("(");
                    pr((cell)nxt.head);
                    printstr(")");
                }
                if (nxt.tail is cell)
                {
                    nxt = (cell)nxt.tail;
                }
                else
                {
                    if (nxt.tail != null) 
                        printstr("." + (string)nxt.tail);
                    nxt = null;
                }
            }
            return r;
        }

        object callFunction(cell f)
        {
            if (f.head is function)
            {
                //primitives
                switch (((function)(f.head)).name)
                {
                    case "car":
                        return car(((cell)f.tail).head);
                    case "cdr":
                        return cdr((cell)((cell)f.tail).head);
                    case "plus":
                        return plus((cell)f.tail);
                    case "minus":
                        return minus((cell)f.tail);
                    case "atomp":
                        return atomp(((cell)f.tail).head);
                    case "listp":
                        return listp(((cell)f.tail).head);
                    case "quote":
                        return f.tail;
                    case "prn":
                        object r = prn((cell)f.tail);
                        return r;
                    case "set":
                        return set((cell)f.tail);
                    case "cons":
                        return null;
                }
                return null;
            }
            else
            {
                return null;
            }
        }

        public void repl()
        {
            while (true)
            {
                printstr("> ");
                buffer = Console.ReadLine();
                while (buffer.Length > 0)
                {
                    object r = eval();
                    if (r is cell)
                    {
                        pr((cell)r);
                        printline("");
                    }
                    else
                        printline((r != null) ? r.ToString() : "? error");
                }
            }
        }

        static void Main(string[] args)
        {
            Program p = new Program();
            p.printline("FemptoLisp 0.1");
            p.repl();
        }
    }
}
