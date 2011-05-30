import java.net.*;
import java.io.*;

public class BServe 
{
    public enum RemoCon
    {
        FAILED,
        A, B, LeftTurn, Forward, RightTurn, Left, Stop, Right, Punch_Left, Back, Punch_Right,
        N1, N2, N3, N4, N5, N6, N7, N8, N9, N0,

        S_A, S_B, S_LeftTurn, S_Forward, S_RightTurn, S_Left, S_Stop, S_Right, S_Punch_Left, S_Back, S_Punch_Right,
        S_N1, S_N2, S_N3, S_N4, S_N5, S_N6, S_N7, S_N8, S_N9, S_N0,

        H_A, H_B, H_LeftTurn, H_Forward, H_RightTurn, H_Left, H_Stop, H_Right, H_Punch_Left, H_Back, H_Punch_Right,
        H_N1, H_N2, H_N3, H_N4, H_N5, H_N6, H_N7, H_N8, H_N9, H_N0
    };
    
    int MAX = 8;
    RemoCon ir_val = RemoCon.FAILED;

    Boolean stopNow = false;
    int startservo = 0;
    
    boolean[] leds = new boolean[8]; // RUN G/B, PWR R/G, ERROR R, PF1 R/B, PF2 O
    boolean[] but  = new boolean[2]; // PF1 / PF2
    
    BufferedReader in = null;
    PrintWriter   out = null;
    ServerSocket serverSocket = null;
    
    public static void main(String[] args)
    {
    	System.out.println("Basic Server start");
    	
    	BServe m = new BServe();
    	m.sock(8888);
    }
    
	public void Bserve()
	{
	}
	
    int handledata(String v)
    {
        String[] p = v.split(":");
        if (v.startsWith("S:"))
        {
            int n = Integer.parseInt(p[1]);
            if (n < 0) n = 0; if (n >= 32) n = 31;

            //servoUCA[n].val = Convert.ToInt32(p[2]);
            return n;
        }
        if (v.startsWith("P:"))
        {
            int n = Integer.parseInt(p[1]);
            return n;
        }
        if (v.startsWith("Y:")) // synch move
        {
            for (int i = 0; i < p.length; i++)
            {
                //servoUCA[i].val = Convert.ToInt32(p[i+1]);
            }
            return 0;
        }
        if (v.startsWith("H:")) // leds and button
        {
            if (p.length > 1 && p[1] != "")
            {
                int n = Integer.parseInt(p[1]);
                for (int i = 0; i < 8; i++)
                {
                    leds[i] = (n | (1 << i)) == 1;
                }
                set_leds();
            }
            return (but[0]?1:0) + (but[1]?2:0);
        }
        if (v.startsWith("R:"))
        {
        	int n = Integer.parseInt(p[1]); if (n < 0) n = 0; if (n >= 32) n = 31;
            //return servoUCA[n].val;
        }
        if (v.startsWith("O:"))
        {
        	int n = Integer.parseInt(p[1]); if (n < 0) n = 0; if (n >= 32) n = 31;
            //return servoUCA[n].io = Convert.ToInt32(p[2]);
        }
        if (v.startsWith("IR"))
        {
            //int t = (int)ir_val;
            //int ir_val = RemoCon.FAILED;
            //return ((t==0)?-1:t);
        }
        if (v.startsWith("X"))
        {
            //return xv.Value;
        }
        if (v.startsWith("Y"))
        {
            //return yv.Value;
        }
        if (v.startsWith("Z"))
        {
            //return zv.Value;
        }
        if (v.startsWith("V"))
        {
            return 101; // version 0.101 
        }
        if (v.startsWith("PSD"))
        {
            //return psdv.Value;
        } 
        return 0;
    }
    
    private void displ(String s)
    {
    	System.out.println(s);
    	System.out.flush();
    }
    
    public void sock(int port)
    {      
        try {
        	serverSocket = new ServerSocket(port);
        }
        catch (IOException e) {
	        System.out.println("Could not listen on port " + port);
	        System.exit(-1);
        }

        displ(" >> Server Started");
        int requestCount = 0;

        while (!stopNow)
        {           
            try
            {
                Socket client = serverSocket.accept();
                displ(" >> Accept connection from client");
                
                in = new BufferedReader(new InputStreamReader(client.getInputStream()));
                out = new PrintWriter(client.getOutputStream(), true);
                
                String line;
                               
                while(true)
                {
                    try
                    {
                      line="";
                      int ch;
                      
                      while (true)
                      {
                    	  ch = in.read();   

                    	  if ((char)ch=='$')
                    		  break;
                    	  
                    	  line += (char)ch;
                      }
                                           
                      displ(" >> Data from client - " + line);
                      
                      int r = handledata(line);                    

                      String serverResponse = "D:" + r + "$";
                      
                      displ(" >> Data to client - " + serverResponse);
                      
                      out.print(serverResponse);
                      out.flush();
                      
                    } 
                    catch (IOException e) 
                    {
                      System.out.println("Read failed");
                      System.exit(-1);
                    }
                }            
            } 
            catch (IOException e) 
            {
                System.out.println("Accept failed");
                System.exit(-1);
            }          
        }
        displ(" >> Done");
    }
    
    void set_leds()
    {
    }
    
    protected void finalize()
    {
        //Clean up 
	     try{
	        in.close();
	        out.close();
	        serverSocket.close();
	    } catch (IOException e) {
	        System.out.println("Could not close.");
	        System.exit(-1);
	    }
    }

}
