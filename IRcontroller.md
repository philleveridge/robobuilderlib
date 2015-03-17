# Introduction #

The IR receive is done mainly in the interrupt routine for the external interrupt 6, though it also uses timer 2 for timing the receiver and for a timeout interrupt.

My IR controller has the address
'#define IR\_ADDRESS 0x2B // Address code from IR'

I don't know if they vary, so please try it and let me know. I do check the address, though this could be disabled.

gIRReady is TRUE when a code is received and gIRData contains the code. The code is according to the pdf file and ranges from 0x01 to 0x3f.

I am using the AVRlib from Pascal Stang, which contains lots of other useful bits of code.

# Details #

The format of the message from the IR controller is 4 bytes. The last byte contains the button pressed data.

On my controller the first byte is 0x2B and the second is 0x02, and I assumed the first is the address. All 4 bytes are in IRTemp[.md](.md), so can be read. You could read IRTemp[0](0.md) for your own address. It would be also good to know if IRTEmp[1](1.md) is also 0x02 for you.