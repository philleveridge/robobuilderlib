# Introduction #

The bootloader used on the RBC appears to be the standard AVRBL from Progressive Resources.

The protocol is described here :
https://www.priio.com/AVRBL-128.pdf

The three character entry sequence is 0x40, 0x26, 0x24

The RBC upgrade tool is developed from the AVRBL code and works fine.
# Details #

According to the AVRBL spec, every line should be acknowledged with '~' if it was received and flashed successfully. Then, when the programming is complete, AVRBL should send '#' indicating that all is good, or '@' indicating that the program did not load successfully.

I'm getting a '~' on every line, but a '@' at the end. I'm not sure what to make of that.

I also observe pretty odd usage of the XON and XOFF character; the RBC appears to send these around every '~' — i.e., "

&lt;XOFF&gt;

~

&lt;XON&gt;

" is sent for every line, regardless of how much data is waiting. I've had to crank down my send rate in order to get a number of line acknowledgments that matches the number of lines sent. If the RBC (AVRBL?) were using XON and XOFF correctly, I think that shouldn't be necessary.

```
// define file complete, no errors character
#define FILE_COMPLETE_CHAR  '@'
// define file complete, with errors character
#define FILE_ERROR_CHAR  '#' 
```