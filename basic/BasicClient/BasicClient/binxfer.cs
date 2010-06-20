using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Text;

namespace RobobuilderLib
{
    public class binxfer
    {
        SerialPort sp1;
        const byte MAGIC_RESPONSE = 0xEA;
        const byte MAGIC_REQUEST = 0xCD;

        public byte[] buff;

        public bool dbg = false;

        public double progress = 0.0;

        public binxfer(SerialPort s)
        {
            sp1 = s;
            sp1.WriteTimeout = 2000;
            sp1.ReadTimeout = 5000;
        }

        public void send_msg_basic(char mt)         // covers 'q', 'v', 'p'
        {
            byte[] buff = new byte[3];
            buff[0] = MAGIC_REQUEST;
            buff[1] = (byte)mt;
            buff[2] = (byte)((buff[0] ^ buff[1]) & 0x7f);   // checksum
            sp1.Write(buff, 0, 3);
            if (dbg) Console.WriteLine("DBG: sendb={0}", BitConverter.ToString(buff));
        }

        public void send_msg_move(byte[] buffer, int bfsz)
        {
            byte cs = (byte)bfsz;
            byte[] header = new byte[5];
            header[0] = MAGIC_REQUEST;
            header[1] = (byte)'m';

            header[2] = (byte)(bfsz & 0xFF);
            header[3] = (byte)((bfsz >> 8) & 0xFF);

            for (int j = 0; j < bfsz; j++)
            {
                cs ^= buffer[j];
            }
            header[4] = (byte)(cs & 0x7f);

            sp1.Write(header, 0, 4);

            // slow down output - sp1.Write(buffer, 0, bfsz);
            for (int i = 0; i < bfsz; i++)
            {
                sp1.Write(buffer, i, 1);
                System.Threading.Thread.Sleep(5);
                Console.Write(".");
            }
            sp1.Write(header, 4, 1);
            
            if (dbg) Console.WriteLine("DBG: sendm={0}", BitConverter.ToString(buffer));
        }

        public void send_msg_raw(char mt, string abytes)
        {
            int n = abytes.Length / 2;
            int cs = 0;
            byte[] header = new byte[4];
            header[0] = MAGIC_REQUEST;
            header[1] = (byte)mt;
            header[2] = (byte)(n & 0xFF);
            header[3] = (byte)((n >> 8) & 0xFF);


            cs ^= header[2]; cs ^= header[3];

            byte[] b = new byte[n+1];

            for (int j = 0; j < n; j++)
            {
                b[j] = (byte)Convert.ToInt16(abytes.Substring(j * 2, 2), 16);
                cs ^= b[j];
            }
            b[n] = (byte)(cs & 0x7f);

            if (dbg)
            {
                Console.WriteLine("DBG: [{1}] : [{2}] send={0}", BitConverter.ToString(header), 4, header.Length);
                Console.WriteLine("DBG: [{1}] : [{2}] send={0}", BitConverter.ToString(b), n + 1, b.Length);
            }

            sp1.Write(header, 0, 4);
            // slow down output
            for (int i = 0; i < n + 1; i++)
            {
                sp1.Write(b, i, 1);
                progress = (double)i / (double)(n + 1); 
                System.Threading.Thread.Sleep(8);
            }
        }

        public void send_msg_raw_bin(char mt, byte[] bytes)
        {
            byte n = (byte)bytes.Length;
            byte cs = n;
            byte[] header = new byte[3];
            header[0] = MAGIC_REQUEST;
            header[1] = (byte)mt;
            header[2] = n;

            for (int j = 0; j < n; j++)
            {
                cs ^= bytes[j];
            }
            cs = (byte)(cs & 0x7f);

            sp1.Write(header, 0, 3);
            if (dbg) Console.WriteLine("DBG: sendrb={0} {1}", BitConverter.ToString(header), BitConverter.ToString(bytes));

            // slow down output
            for (int i = 0; i < n; i++)
            {
                sp1.Write(bytes, i, 1);
                System.Threading.Thread.Sleep(2);
            }
            sp1.Write(new byte[] {cs}, 0, 1);
        }

        public byte readByte()
        {
            byte b = (byte)sp1.ReadByte();
            if (dbg) Console.WriteLine("DBG: byte={0}", b);
            return b;
        }

        public int bytesToRead()
        {
            int n = sp1.BytesToRead;
            if (n>0 && dbg) Console.WriteLine("DBG: num={0}", n); 
            return n;
        }

        public int readBytes(byte[] buff, int n)
        {
            int r = sp1.Read(buff, 0, n);
            if (dbg) Console.WriteLine("DBG: bytes={0}", BitConverter.ToString(buff));
            return r;
        }

        private bool test_packet(byte mt, int n)
        {
            // read n bytes (including cs and test
            buff = new byte[n];
            while (bytesToRead() < n) ;
            readBytes(buff, n);

            byte cs = mt;
            for (int i = 0; i < n-1; i++)
                cs ^= buff[i];
            return ((cs & 0x7f) == buff[n-1]);
        }
        
        public bool recv_packet()
        {
            int i;
            bool good_packet = false;
            string ignore = "";
            byte b;

            try
            {
                while ((b=readByte()) != MAGIC_RESPONSE) 
                   ignore += b; // ignore till start of message
            }
            catch (Exception e)
            {
                Console.WriteLine("Read error " + e + " ignore = [" + ignore + "]");
                return false;
            }

            byte mt = readByte();
            byte cs;

            switch (mt)
            {
                case (byte)'q':
                    // 43 packets
                    int n = readByte();
                    n = n * 2 + 11;
                    buff = new byte[n];
                    while (bytesToRead() < n) ;
                    readBytes(buff, n);

                    cs = (byte)'q';
                    for (i = 0; i < n-1; i++) 
                           cs ^= buff[i];
                    good_packet = ((cs &0x7f) == buff[n-1]);
                    break;
                case (byte)'x':
                case (byte)'X':
                case (byte)'I':
                    good_packet = test_packet(mt, 3);
                    break;
                case (byte)'v':
                case (byte)'l':
                case (byte)'z':
                case (byte)'P':
                case (byte)'D':
                    good_packet = test_packet(mt, 2);
                    if (mt == 'z')
                    {
                        Console.WriteLine("Protocol error {0}", buff[0]);
                        return false;
                    }
                    break;
                case (byte)'Q':
                    good_packet = test_packet(mt, 8);
                    break;
                case (byte)'A':
                    good_packet = test_packet(mt, 7);
                    break;
                default:    // un-known error
                    return false;
            }

            return good_packet;
        }
    }
}
