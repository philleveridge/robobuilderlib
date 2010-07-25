//==============================================================================
// include file for accelerometer.c
//==============================================================================

extern volatile int x_value;
extern volatile int y_value;
extern volatile int z_value;

extern void  I2C_read    (int addr, int ocnt, BYTE * outbuff, int icnt, BYTE * inbuff);
extern int   I2C_write   (int addr, int ocnt, BYTE * outbuff) ;

extern void  Acc_init    (void);
extern void  Acc_GetData (void);