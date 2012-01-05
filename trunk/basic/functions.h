#ifndef FUNCTIONS_H
#define FUNCTIONS_H

extern int					get_special(char **str, long *res);
extern void					set_bit(int p, int b, int n);
extern int					get_bit(int pn, int bn);
extern const unsigned char	map[];

extern int					sigmoid(int v, int t);

extern const  prog_char		*specials[];

extern int					Cos(BYTE d);
extern int					Sin(BYTE d);
extern int					fix_fft(short fr[], short fi[], short m, short inverse);

#endif