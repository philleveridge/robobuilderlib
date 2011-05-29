import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;

import java.io.*;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

//

/**
 *
 * @author phil
 */
public class Rloader {

    /**
     * @param args the command line arguments
     */
	
    public static void main(String[] args) 
    {
            if (args.length<1)
            {
            	System.out.print("Please supply HEX file for load\n");
            	System.exit(1);
            }
            else
            	System.out.print("Load File : " + args[0] + "\n");

            
            Rloader test = new Rloader("/dev/ttyUSB0", 115200);
            
            if (!test.IsOpen)
            {
            	System.out.print("Can't open serial port\n");
            	System.exit(1);
            }
            
            try {
            
                BufferedReader input =  new BufferedReader(new FileReader(args[0]));
                
            	System.out.println("Reset Robobuilder Now");
	            
                if (!test.header())
                {
                	System.out.println("Download failed - invalid header sequence");
                	System.exit(1);
                }
	            
	            while (true)
	            {
		            String b = input.readLine();
		            if (b==null) break;
		            
		            if (!test.send(b))
		            {
	                	System.out.println("Download failed - incorrect line"); 
	                	System.exit(1);
		            }
	            }	                       	           
            } 
            catch (Exception e)
            {
            	System.out.println("Download failed" + e.getMessage());               	
            }

            test.close();

            System.out.print("Done.\n");
            System.exit(0);
    }
    
    // The three character entry sequence is 0x40, 0x26, 0x24 
    // The protocol is described here : https://www.priio.com/AVRBL-128.pdf
    //  '~' on every line, and  a '@' at the end. 
    
    SerialPort serialPort;
    OutputStream out;
    InputStream in;
    boolean IsOpen = false;
    char status;
    
    char BOOTLOADER_ACTIVE_CHAR	='^';  	    
    char READY_CHAR				='?';
    // define line complete with no error character
    char LINE_COMPLETE_CHAR		='~';
    // define checksum error character
    char CS_ERROR_CHAR			='-';
    // define flash page error character
    char PAGE_ERROR_CHAR		='%';
    // define file complete, no errors character
    char FILE_COMPLETE_CHAR		='@';
    // define file complete, with errors character
    char FILE_ERROR_CHAR		='#';
    
    public Rloader  (String portName, int n)
	{
		try 
		{
            CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(portName);
            CommPort commPort = portIdentifier.open("SevoController",2000);
            serialPort = (SerialPort) commPort;
            serialPort.setSerialPortParams(n, SerialPort.DATABITS_8,SerialPort.STOPBITS_1,SerialPort.PARITY_NONE);
            serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_XONXOFF_OUT);

            out = serialPort.getOutputStream();
            in  = serialPort.getInputStream();
            
            IsOpen=true;
        }
        catch (Exception e) 
        {
			System.out.println("connect failed " + e.getMessage());
            IsOpen=false;
        }
	}
    

    public void waitforany() throws IOException
    {
    	int ch;
		while (true)
		{
			ch = in.read();

			System.out.print((char)ch);
			System.out.flush();
			
			if (    (char)ch ==  LINE_COMPLETE_CHAR ||
					(char)ch ==  CS_ERROR_CHAR      ||
					(char)ch ==  PAGE_ERROR_CHAR    ||
					(char)ch ==  FILE_COMPLETE_CHAR ||
					(char)ch ==  FILE_ERROR_CHAR)
				break;
		}  
		status = (char)ch;    	
    }

    public void waitfor(char c) throws IOException
    {
    	int ch;
		while (true)
		{
			ch = in.read();

			System.out.print((char)ch);
			System.out.flush();
			
			if ((char)ch ==  c)
				break;
		}  
		status = (char)ch;
    }
    
    public Boolean header() 
    {
    	byte[] buff = new byte[] {0x40, 0x26, 0x24 };
    	if (IsOpen)
    	{
    		try 
    		{
    			waitfor(BOOTLOADER_ACTIVE_CHAR);   			
	    		out.write(buff); 					// send 3 byte sequence	    			
	    		waitfor(READY_CHAR);
	    			
	        	return true;
    		}
    		catch (Exception e)
    		{
    			System.out.println("Waiting for start?" + e.getMessage());    			
    		}
    	}
    	return false;
    }

    public boolean send(String bytes)
    {
            if (IsOpen)
            {
                try {
                	int c;               
                    
                    for (int i=0; i<bytes.length(); i++)
                    {
                    	char b1 =  bytes.charAt(i);
                    	out.write(b1);
                        System.out.write(b1);
                    }
                	out.write(13);
                	out.write(10);
                    
                	waitforany();
                	
                	if (status == LINE_COMPLETE_CHAR || status == FILE_COMPLETE_CHAR)
                	{
                        System.out.println("");    
                        return true;
                	}                	

                    System.out.println("Received fail = " + status);    
                    return false;
                }
                catch (Exception e)
                {
                    System.out.println("send fail");
                    return false;
                }
            }
            return false;
    }

    public void close()
    {
        if (IsOpen)
        {
            serialPort.close();
            IsOpen=false;
        }

    }

}
