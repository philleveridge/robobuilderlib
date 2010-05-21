using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

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
        public string name = "";
    }

    public class environ
    {
        int cnt=0;
        const int MAX=30;
        string[] name = new string[MAX];
        object[] val  = new object[MAX];

        public void set(string n, object v)
        {
            //update
            for (int i = 0; i < cnt; i++)
            {
                if (name[i] == n)
                {
                    val[cnt] = v;
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

                if (Char.IsLetter((char)ch))
                {
                    ungetch(ch);
                    string t=readtoken();

                    if (!qf)
                    {
                        function f = new function();
                        f.name = t;
                        return f;
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
            return (cell)x.head;
        }

        object print(cell x)
        {
            printstr("(");
            while (true)
            {
                if (x.head == null)
                {
                    printstr("NIL ");
                }
                else if (x.head is cell)
                {
                    print((cell)x.head);
                }
                else
                {
                    printstr(x.head.ToString());
                    printstr(" ");
                }

                if (x.tail == null)
                {
                    printstr(") ");
                    return x.head;
                }

                if (x.tail is cell)
                    x = (cell)x.tail;
                else
                {
                    printstr(".");
                    printstr((string)x.tail);
                    return x.tail;
                }
            }
        }

        object callFunction(cell f)
        {
            if (f.head is function)
            {
                //printstr("Function ");
                //printstr(((function)(f.head)).name);

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
                    case "pr":
                        return print((cell)f.tail);
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
                        print((cell)r);
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
