// define some notes
// Frequencies from http://www.phy.mtu.edu/~suits/notefreqs.html
// converted to half-periods (us) by calculating
// 	1000000/2/frequency
// where frequency is in Hz

#define D5 851
#define E5 758
#define Fsh5 675
#define G5 637
#define A5 568
#define B5 506
#define C6 477
#define D6 425
#define DUR 40

extern void play_tone(int delay, int duration);
 

