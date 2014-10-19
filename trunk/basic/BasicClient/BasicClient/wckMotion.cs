using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Ports;
using System.Text.RegularExpressions;


namespace RobobuilderLib
{
    public class wckMotion
    {
        public const int MAX_SERVOS = 21;

        static public int[] ub_Huno = new int[] {
        /* ID
          0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
        174,228,254,130,185,254,180,126,208,208,254,224,198,254,228,254};

        static public int[] lb_Huno = new int[] {
        /* ID
          0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
          1, 70,124, 40, 41, 73, 22,  1,120, 57,  1, 23,  1,  1, 25, 40};

        static public byte[] basicdh = new byte[] {
            143, 179, 198, 83, 105, 106, 68, 46, 167, 140, 77, 70, 152, 165, 181, 98, 120, 124, 99};
        
        static public byte[] basic18 = new byte[] { 
            143, 179, 198, 83, 106, 106, 69, 48, 167, 141, 47, 47, 49, 199, 192, 204, 122, 125, 255};

        static public byte[] basic16 = new byte[] { 
            125, 179, 199, 88, 108, 126, 72, 49, 163, 141, 51, 47, 49, 199, 205, 205 };

        static public byte[] basic_pos = basic18;

        public bool DCMP { get; set; }  // this must be true for DCMP high speed mode (custom firmware)

        public MoveTypes cmt { get; set; }

        /**********************************************
         * 
         * direct Command mode  - wcK prorocol
         * 
         * ********************************************/

        int[] sids = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
        private SerialPort serialPort;

        public byte[] respnse = new byte[32];
        public string Message;
        public byte[] pos;

        public double kfactor = 1.0f;

        public wckMotion(SerialPort p)
        {
            DCMP = true;
            cmt = MoveTypes.Linear; //default 
            serialPort = p;

            if (!serialPort.IsOpen)
            {
                Console.WriteLine("No serial port open");
            }

        }

        public void set_kfactor(double k)
        {
            kfactor =k;
        }

        public void servoStatus(int id, bool f)
        {
            sids[id] = (f) ? id : -1;
        }

        public bool wckPassive(int id)
        {
            byte[] buff = new byte[4];
            buff[0] = 0xFF;
            buff[1] = (byte)(6 << 5 | (id % 31));
            buff[2] = 0x10; // Mode=1, arbitary
            buff[3] = (byte)((buff[1] ^ buff[2]) & 0x7f);

            try
            {
                serialPort.Write(buff, 0, 4);
                respnse[0] = (byte)serialPort.ReadByte();
                respnse[1] = (byte)serialPort.ReadByte();
                Message = "Passive " + id + " = " + respnse[0] + ":" + respnse[1];
                System.Diagnostics.Debug.WriteLine(Message); // debug
                return true;
            }
            catch (Exception e1)
            {
                Message = "Failed" + e1.Message;
                return false;
            }
        }

        public bool wckReadAll()
        {
            // requires DCMP >229
            byte[] buff = new byte[4];
            buff[0] = 0xFF;
            buff[1] = (byte)(5 << 5 | (30 % 31));
            buff[2] = (byte)0x0f;      // 
            buff[3] = (byte)((buff[1] ^ buff[2]) & 0x7f);

            try
            {
                serialPort.Write(buff, 0, 4);
                respnse[0] = (byte)serialPort.ReadByte(); // x
                respnse[1] = (byte)serialPort.ReadByte(); // y
                respnse[2] = (byte)serialPort.ReadByte(); // z
                respnse[3] = (byte)serialPort.ReadByte(); // PSD
                respnse[4] = (byte)serialPort.ReadByte(); // ir
                respnse[5] = (byte)serialPort.ReadByte(); // buttons
                respnse[6] = (byte)serialPort.ReadByte(); // snd

                Message = "ReadAll = " + respnse[0] + ":" + respnse[1] + ":" + respnse[2] + ":" + respnse[3] + ":" + respnse[4] + ":" + respnse[5] + ":" + respnse[6];
                //System.Diagnostics.Debug.WriteLine(Message); // debug
                return true;
            }
            catch (Exception e1)
            {
                Message = "Failed" + e1.Message;
                return false;
            }
        }

        public bool wckReadPos(int id)
        {
            return wckReadPos(id, 0);
        }

        public bool wckReadPos(int id, int d1)
        {
            byte[] buff = new byte[4];
            buff[0] = 0xFF;
            buff[1] = (byte)(5 << 5 | (id % 31));
            buff[2] = (byte) d1; // arbitary
            buff[3] = (byte)((buff[1] ^ buff[2]) & 0x7f);

            try
            {
                serialPort.Write(buff, 0, 4);
                respnse[0] = (byte)serialPort.ReadByte();
                respnse[1] = (byte)serialPort.ReadByte();
                Message = "ReadPos " + id + " = " + respnse[0] + ":" + respnse[1];
                //System.Diagnostics.Debug.WriteLine(Message); // debug
                return true;
            }
            catch (Exception e1)
            {
                Message = "Failed" + e1.Message;
                return false;
            }
        }

        public bool wckMovePos(int id, int pos, int torq)
        {
            byte[] buff = new byte[4];
            buff[0] = 0xFF;
            buff[1] = (byte)(((torq % 5) << 5) | (id % 31));
            buff[2] = (byte)(pos % 254); // arbitary
            buff[3] = (byte)((buff[1] ^ buff[2]) & 0x7f);

            try
            {
                serialPort.Write(buff, 0, 4);
                respnse[0] = (byte)serialPort.ReadByte();
                respnse[1] = (byte)serialPort.ReadByte();
                Message = "MovePos " + id + " = " + respnse[0] + ":" + respnse[1];

                return true;
            }
            catch (Exception e1)
            {
                Message = "Failed" + e1.Message ;
                return false;
            }
        }

        public void SyncPosSend(int LastID, int SpeedLevel, object[] TargetArray, int Index)
        {
            byte[] b = new byte[TargetArray.Length];
            for (int i = 0; i < TargetArray.Length; i++) 
                b[i] = Convert.ToByte(TargetArray[i]);
            SyncPosSend(LastID, SpeedLevel, b, Index);
        }

        public void SyncPosSend(int LastID, int SpeedLevel, byte[] TargetArray, int Index)
        {
            int i;
            byte CheckSum;
            byte[] buff = new byte[5 + LastID];

            i = 0;
            CheckSum = 0;
            buff[0] = 0xFF;
            buff[1] = (byte)((SpeedLevel << 5) | 0x1f);
            buff[2] = (byte)(LastID + 1);

            while (true)
            {
                if (i > LastID) break;
                buff[3 + i] = TargetArray[Index * (LastID + 1) + i];
                CheckSum ^= (byte)(TargetArray[Index * (LastID + 1) + i]);
                i++;
            }
            CheckSum = (byte)(CheckSum & 0x7f);
            buff[3 + i] = CheckSum;

            //now output buff[]
            //Debug info :: for (i = 0; i < buff.Length - 1; i++) Console.Write(buff[i] + ","); Console.WriteLine(buff[i]);

            try
            {
                serialPort.Write(buff, 0, buff.Length);
                Message = "MoveSyncPos";

                return;
            }
            catch (Exception e1)
            {
                Message = "Failed" + e1.Message;
                return;
            }
        }

        public bool wckBreak()
        {
            byte[] buff = new byte[4];
            buff[0] = 0xFF;
            buff[1] = (byte)((6 << 5) | 31);
            buff[2] = (byte)0x20;
            buff[3] = (byte)((buff[1] ^ buff[2]) & 0x7f);

            try
            {
                serialPort.Write(buff, 0, 4);
                respnse[0] = (byte)serialPort.ReadByte();
                respnse[1] = (byte)serialPort.ReadByte();
                Message = "Break = " + respnse[0] + ":" + respnse[1];
                return true;
            }
            catch (Exception e1)
            {
                Message = "Break Failed" + e1.Message;
                return false;
            }
        }

        /* 
         * wck set operation(s)
         * 
         */ 
        public bool wckSetOper(byte d1,byte d2, byte d3, byte d4)
        {
            byte[] buff = new byte[6];
            buff[0] = 0xFF;
            buff[1] = d1;
            buff[2] = d2;
            buff[3] = d3;
            buff[4] = d4;
            buff[5] = (byte)((buff[1] ^ buff[2] ^ buff[3] ^ buff[4]) & 0x7f);

            try
            {
                serialPort.Write(buff, 0, 6);
                respnse[0] = (byte)serialPort.ReadByte();
                respnse[1] = (byte)serialPort.ReadByte();
                Message = "Set Oper = " + respnse[0] + ":" + respnse[1];
                return true;
            }
            catch (Exception e1)
            {
                Message = "Set Op Failed" + e1.Message;
                return false;
            }
        }

        public bool wckSetBaudRate(int baudrate, int id)
        {
            byte d1, d3;
            d1=(byte)((7<<5) | (id %31));
            //0(921600bps), 1(460800bps), 3(230400bps), 7(115200bps),
            //15(57600bps), 23(38400bps), 95(9600bps), 191(4800bps),
            switch (baudrate)
            {
                case 115200:
                    d3 = 7;
                    break;
                case 57600:
                    d3 = 15;
                    break;
                case 9600:
                    d3 = 95;
                    break;
                case 4800:
                    d3 = 191;
                    break;
                default:
                    return false;
            }
            return wckSetOper(d1, 0x08, d3, d3);
        }

        public bool wckSetSpeed(int id, int speed, int acceleration)
        {
            byte d1, d3, d4;
            if (speed < 0 || speed > 30) 
                return false;
            if (acceleration < 20 || acceleration > 100) 
                return false;
            d1 = (byte)((7 << 5) | (id % 31));
            d3 = (byte)speed;
            d4 = (byte)acceleration;
            return wckSetOper(d1, 0x0D, d3, d4);
        }

        public bool wckSetPDgain(int id, int pGain, int dGain)
        {
            return false; // not implemented
        }

        public bool wckSetID(int id, int new_id)
        {
            return false; // not implemented
        }

        public bool wckSetIgain(int id, int iGain)
        {
            return false; // not implemented
        }

        public bool wckSetPDgainRT(int id, int pGain, int dGain)
        {
            return false; // not implemented
        }

        public bool wckSetIgainRT(int id, int iGain)
        {
            return false; // not implemented
        }

        public bool wckSetSpeedRT(int id, int speed, int acceleration)
        {
            return false; // not implemented
        }

        public bool wckSetOverload(int id, int overT)
        {
            /*
            1 33 400
            2 44 500
            3 56 600
            4 68 700
            5 80 800
            6 92 900
            7 104 1000
            8 116 1100
            9 128 1200
            10 199 1800
             */
            byte d1, d3=33;
            d1 = (byte)((7 << 5) | (id % 31));
            switch (overT)
            {
                case 400:
                    d3 = 33;
                    break;
                case 500:
                    d3 = 44;
                    break;
                case 600:
                    d3 = 56;
                    break;
                case 700:
                    d3 = 68;
                    break;
                case 800:
                    d3 = 80;
                    break;                 
            }
            return wckSetOper(d1, 0x0F, d3, d3);
        }

        public bool wckSetBoundary(int id, int UBound, int LBound)
        {
            byte d1, d3, d4;
            d1 = (byte)((7 << 5) | (id % 31));
            d3 = (byte)LBound;
            d4 = (byte)UBound;
            return wckSetOper(d1, 0x11, d3, d4);
        }

        public bool wckWriteIO(int id, bool ch0, bool ch1 )
        {
            byte d1, d3;
            d1 = (byte)((7 << 5) | (id % 31));
            d3 = (byte)((byte)((ch0) ? 1 : 0) | (byte)((ch1) ? 2 : 0));
            return wckSetOper(d1, 0x64, d3, d3);
        }

        /* 
         * wck - Read operation(s)
         */ 

        public bool wckReadPDgain(int id)
        {
            return wckSetOper((byte)((7 << 5) | (id % 31)), 0x0A, 0x00, 0x00);
        }

        public bool wckReadIgain(int id)
        {
            return wckSetOper((byte)((7 << 5) | (id % 31)), 0x16, 0x00, 0x00);
        }

        public bool wckReadSpeed(int id)
        {
            return wckSetOper((byte)((7 << 5) | (id % 31)), 0x0E, 0x00, 0x00);
        }

        public bool wckReadOverload(int id)
        {
            return wckSetOper((byte)((7 << 5) | (id % 31)), 0x10, 0x00, 0x00);
        }

        public bool wckReadBoundary(int id)
        {
            return wckSetOper((byte)((7 << 5) | (id % 31)), 0x12, 0x00, 0x00);
        }

        public bool wckReadIO(int id)
        {
            return wckSetOper((byte)((7 << 5) | (id % 31)), 0x65, 0x00, 0x00);
        }

        public bool wckReadMotionData(int id)
        {
            return wckSetOper((byte)((7 << 5) | (id % 31)), 0x97, 0x00, 0x00);
        }

        public bool wckPosRead10Bit(int id)
        {
            return wckSetOper((byte)(7 << 5), 0x09, (byte)id, (byte)id);
        }

        /* 
         * special extended / 10 bit commands
         */ 

        public bool wckWriteMotionData(int id, int pos, int torq)
        {
            return false; // not implemented
        }
        
        public bool wckPosMove10Bit(int id, int pos, int torq)
        {
            return false; // not implemented
        }


        /*********************************************************************************************
         * 
         * I2C hardware functions (require DCMP)
         * 
         *********************************************************************************************/


        public int cbyte(byte b)
        {
            int i;
            if (b > 127)
            {
                i = (int)b - 256;
            }
            else
            {
                i = (int)b;
            }
            return i;
        }

        public bool I2C_write(int addr, byte[] outbuff)
        {
            if (!DCMP)
            {
                Message = "Special Op requires DCMP"; 
                return false;
            }

            int n = 0;
            if (outbuff != null) n = outbuff.Length;

            byte[] buff = new byte[n+6];
            buff[0] = 0xFF;
            buff[1] = 0xBE;                     // servo address 30
            buff[2] = 0x0E;                     // cmd = 0x0E (IC2_out)
            buff[3] = (byte)(addr%256);         // IC2 slave address
            buff[4] = (byte)(outbuff.Length);   // no of bytes tos end
            int cs = 0;

            for (int i = 0; i < n; i++)
            {
                buff[5 + i] = outbuff[i];
                cs ^= outbuff[i];
            }
            buff[5 + n] = (byte)(cs & 0x7f);

            try
            {
                //serialPort.Write(buff, 0, buff.Length);
                for (int cb = 0; cb < buff.Length; cb++)
                {
                    serialPort.Write(buff, cb, 1); // send each byte 
                }
                respnse[0] = (byte)serialPort.ReadByte();
                respnse[1] = (byte)serialPort.ReadByte();
                return true;
            }
            catch (Exception e1)
            {
                Message = "Special Op Failed" + e1.Message;
                return false;
            }
        }

        public byte[] I2C_read(int addr, byte[] outbuff, int cnt)
        {
            if (!DCMP)
            {
                Message = "Special Op requires DCMP" ;
                return null;
            }

            byte[] inbuff = new byte[cnt];

            byte[] buff = new byte[outbuff.Length + 7];
            buff[0] = 0xFF;
            buff[1] = 0xBE;                         // servo address 30
            buff[2] = 0x0D;                         // cmd = 0xD (IC2_in)
            buff[3] = (byte)(addr % 256);           // IC2 slave address
            buff[4] = (byte)(outbuff.Length + 1);   // no of bytes tos end 
            buff[5] = (byte)(cnt);                  // input bytes required added as first byte

            int cs = cnt;

            for (int i = 0; i < outbuff.Length; i++)
            {
                buff[6 + i] = outbuff[i];
                cs ^= outbuff[i];
            }

            buff[6 + outbuff.Length] = (byte)(cs & 0x7f);

            try
            {
                serialPort.Write(buff, 0, buff.Length);

                for (int j = 0; j < cnt; j++)
                {
                    respnse[j] = (byte)serialPort.ReadByte();
                    inbuff[j] = respnse[j];
                }
                return inbuff;
            }
            catch (Exception e1)
            {
                Message = "Special Op Failed" + e1.Message;
                return null;
            }
        }

        /*********************************************************************************************
         * 
         * higher level functions
         * 
         *********************************************************************************************/

        bool initpos = false;

        public void servoID_readservo(int num)
        {
            if (num == 0) num = sids.Length;
            
            pos = new byte[num];

            for (int id = 0; id < num; id++)
            {
                int n = sids[id];

                if (n>=0 && wckReadPos(n))                 //readPOS (servoID)
                {
                    if (respnse[1] < 255)
                    {
                        pos[id] = respnse[1];
                    }
                    else
                    {
                        pos[id] = 0;
                        System.Diagnostics.Debug.WriteLine(String.Format("Id {0} = {1}", id, respnse[1]));
                    }
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine("Id " + id + "not connected" );
                    //sids[id] = -1; // not connected
                }
            }
            initpos = true;
        }

        public void delay_ms(int t1)
        {
            //if (pcR.dbg) Console.WriteLine("dly=" + t1);
            System.Threading.Thread.Sleep(t1);
        }

        public void BasicPose(int duration, int no_steps)
        {
            PlayPose(duration, no_steps, basic_pos, true);
        }

        /*
        public bool PlayFile(string filename)
        { 
            return PlayFile(filename, 0, 0);
        }

        public bool PlayMatrix(matrix m)
        {
            return PlayMatrix(m, 0, 0);
        }

        public bool PlayMatrix(matrix m, int s, int f)
        { 
            // assume col 0=duration, col 1= no steps
            // start on row 's' finish on row 'f'
            // s >= f or s <= f 
            // f < 0 assume all remaining rows

            int n = 0;
            if (f < 0) f = m.getr()-1;

            if (s < 0 || s >= m.getr() || f >= m.getr())
            {
                Console.WriteLine("Bad params S={0}-F={1) M[{2},{3}]",s, f, m.getc(), m.getr());
                return false;
            }

            int sp = (s < f) ? 1 : -1;
            int ns = (f - s) * sp + 1;

            for (int i = 0; i < ns; i++)
            {
                int r = s + i * sp;
                double[] row = m.getrow(r);
                int duration = (int)row[0];
                int steps    = (int)row[1];

                byte[] servo_pos = new byte[m.getc()-2];
                for (int j = 0; j < servo_pos.Length; j++)
                {
                    servo_pos[j] = (byte)row[j+2];
                }
                if (!PlayPose(duration, steps, servo_pos, (n == 1)))
                    return false;
                n = 1;
            }
            return true;
        }

        public bool PlayFile(string filename, int startrow, int endrow)
        {
            byte[] servo_pos;
            int steps;
            int duration;
            int nos = 0;
            int n = 0;
            tcnt = 0;
            int linecount = 0;

            try
            {
                TextReader tr = new StreamReader(filename);
                string line = "";

                while ((line = tr.ReadLine()) != null)
                {
                    line = line.Trim();
                    linecount++;

                    if (!(linecount>=startrow && ( linecount<=endrow || endrow==0)))
                        continue;

                    if (pcR.dbg) Console.WriteLine("{0} - {1}", linecount, line);

                    if (line.StartsWith("#")) // comment
                    {
                        if (trig != null && trig.dbg)  Console.WriteLine(line);
                        if (line.StartsWith("#V=01,,"))
                            nos = 20;

                        Match m = Regex.Match(line, @"#V=01,N=([0-9]+)");
                        if (m.Success)
                        {
                            nos = Convert.ToInt32(m.Groups[1].Value);
                            if (pcR.dbg) Console.WriteLine("nos = {0}", nos);
                        }
                        continue;
                    }

                    string[] r = line.Split(',');

                    if (nos == 0)
                    {
                        if (r.Length > 20)
                            nos = r.Length - 5; // assume XYZ have been appended
                        else 
                            nos = r.Length - 2;
                    }

                    if (nos > 0)
                    {
                        servo_pos = new byte[nos];
                        n++;

                        duration = Convert.ToInt32(r[0]);
                        steps = Convert.ToInt32(r[1]);

                        for (int i = 0; i < nos; i++)
                            servo_pos[i] = (byte)Convert.ToInt32(r[i + 2]);

                        if (!PlayPose(duration, steps, servo_pos, (n == 1))) 
                            return false;
                    }

                }
                tr.Close();
            }
            catch (Exception e1)
            {
                Console.WriteLine("Error - can't load file " + e1.Message);
                return false;
            }
            return true;
        }
 * 
 * */

        // Different type of move interpolation
        // from http://robosavvy.com/forum/viewtopic.php?t=5306&start=30
        // orinial by RN1AsOf091407

        public enum MoveTypes { AccelDecel, Accel, Decel, Linear };

        double CalculatePos_AccelDecel(int Distance, double FractionOfMove)
        {
            if ( FractionOfMove < 0.5 )     // Accel:
                return CalculatePos_Accel(Distance /2, FractionOfMove * 2);
            else if (FractionOfMove > 0.5 ) //'Decel:
                return CalculatePos_Decel(Distance/2, (FractionOfMove - 0.5) * 2) + (Distance * 0.5);
            else                            //'= .5! Exact Middle.
                return Distance / 2;
        }

        double CalculatePos_Accel(int Distance, double FractionOfMove) 
        {
            return FractionOfMove * (Distance * FractionOfMove);
        }

        double CalculatePos_Decel(int Distance, double FractionOfMove)
        {
            FractionOfMove = 1 - FractionOfMove;
            return Distance - (FractionOfMove * (Distance * FractionOfMove));
        }

        double CalculatePos_Linear(int Distance, double FractionOfMove)
        {
            return (Distance * FractionOfMove);
        }

        double GetMoveValue(MoveTypes mt, int StartPos, int EndPos, double FractionOfMove)
        {
            int Offset,Distance;
            if (StartPos > EndPos)
            {
                Distance = StartPos - EndPos;
                Offset = EndPos;
                switch (mt)
                {
                    case MoveTypes.Accel:
                        return Distance - CalculatePos_Accel(Distance, FractionOfMove) + Offset;
                    case MoveTypes.AccelDecel:
                        return Distance - CalculatePos_AccelDecel(Distance, FractionOfMove) + Offset;
                    case MoveTypes.Decel:
                        return Distance - CalculatePos_Decel(Distance, FractionOfMove) + Offset;
                    case MoveTypes.Linear:
                        return Distance - CalculatePos_Linear(Distance, FractionOfMove) + Offset;
                }
            }
            else
            {
                Distance = EndPos - StartPos;
                Offset = StartPos;
                switch (mt)
                {
                    case MoveTypes.Accel:
                        return CalculatePos_Accel(Distance, FractionOfMove) + Offset;
                    case MoveTypes.AccelDecel:
                        return CalculatePos_AccelDecel(Distance, FractionOfMove) + Offset;
                    case MoveTypes.Decel:
                        return CalculatePos_Decel(Distance, FractionOfMove) + Offset;
                    case MoveTypes.Linear:
                        return CalculatePos_Linear(Distance, FractionOfMove) + Offset;
                }
            }
            return 0.0;
        }

        // NEW:: if byte = 255 = use current position
        // NEW:: check limits / bounds before sending
        // NEW:: move types added, will use linear interpolation between points unless specified

        public bool PlayPose(int duration, int no_steps, Object[] spodobj, bool first)
        {
            byte[] s = new byte[spodobj.Length];
            for (int i = 0; i < spodobj.Length; i++)
                s[i] = (byte)spodobj[i];
            return PlayPose(duration, no_steps, s, first, cmt);
        }

        public bool PlayPose(int duration, int no_steps, byte[] spod, bool first)
        {
            return PlayPose(duration, no_steps, spod, first, cmt);
        }

        public bool PlayPose(int duration, int no_steps, byte[] spod, bool first, MoveTypes ty)
        {
            int cnt = 0;

            byte[] temp = new byte[spod.Length];

            if (first || !initpos)
            {
                //if (pcR.dbg) Console.WriteLine("Debug:  read servo positions {0}", tcnt);

                servoID_readservo(spod.Length); // read start positons
            }

            //double[] intervals = new double[spod.Length];

            duration = (int)(0.5+(double)duration * kfactor);

            if (kfactor != 1.0f) { Console.WriteLine("Kfactor set (0) = Duration= (1)", kfactor, duration); }

            // bounds check
            for (int n = 0; n < spod.Length ; n++)
            {
                if (spod[n] != 255)
                {
                    if (n < lb_Huno.Length)
                    {
                        if (spod[n] < lb_Huno[n]) spod[n] = (byte)lb_Huno[n];
                        if (spod[n] > ub_Huno[n]) spod[n] = (byte)ub_Huno[n];
                    }
                    cnt++;
                }
            }

            //Console.WriteLine("Debug: diff = " + cnt);
            for (int s = 1; s <= no_steps; s++)
            {
                //
                for (int n = 0; n < spod.Length; n++)
                {
                    if (spod[n] == 255)
                        temp[n] = pos[n];
                    else
                        temp[n] = (byte)GetMoveValue(ty, pos[n], spod[n], (double)s / (double)no_steps);

                }

                //timed --- start
                long z = DateTime.Now.Ticks;
                SyncPosSend(temp.Length - 1, 4, temp, 0);
                //  if (pcR.dbg) { Console.WriteLine("Dbg: Timed = {0}", (DateTime.Now.Ticks - z) / TimeSpan.TicksPerMillisecond); }
                //timed --- end

                int td = duration / no_steps;
                if (td < 25) td = 25;
            }

            for (int n = 0; n < spod.Length; n++)
            {
                if (spod[n] != 255) pos[n] = spod[n];
            }

            return true; // complete
        }

    }
}
