//==============================================================================
// DATA TYPE
//==============================================================================
#define	BYTE	unsigned char
#define	WORD	unsigned int
#define	DWORD	unsigned long
#define	BYTEP	unsigned char*
#define	WORDP	unsigned int*
#define	SBYTE	signed char
#define	SWORD	signed int
//==============================================================================
// BIT SET
//==============================================================================
#define         SET_BIT0(x)     (x |= 0x01)
#define         SET_BIT1(x)     (x |= 0x02)
#define         SET_BIT2(x)     (x |= 0x04)
#define         SET_BIT3(x)     (x |= 0x08)
#define         SET_BIT4(x)     (x |= 0x10)
#define         SET_BIT5(x)     (x |= 0x20)
#define         SET_BIT6(x)     (x |= 0x40)
#define         SET_BIT7(x)     (x |= 0x80)
//==============================================================================
// BIT CLEAR
//==============================================================================
#define         CLR_BIT0(x)     (x &= 0xFE)
#define         CLR_BIT1(x)     (x &= 0xFD)
#define         CLR_BIT2(x)     (x &= 0xFB)
#define         CLR_BIT3(x)     (x &= 0xF7)
#define         CLR_BIT4(x)     (x &= 0xEF)
#define         CLR_BIT5(x)     (x &= 0xDF)
#define         CLR_BIT6(x)     (x &= 0xBF)
#define         CLR_BIT7(x)     (x &= 0x7F)
//==============================================================================
// BIT CHECK
//==============================================================================
#define         CHK_BIT0(x)     (x & 0x01)
#define         CHK_BIT1(x)     (x & 0x02)
#define         CHK_BIT2(x)     (x & 0x04)
#define         CHK_BIT3(x)     (x & 0x08)
#define         CHK_BIT4(x)     (x & 0x10)
#define         CHK_BIT5(x)     (x & 0x20)
#define         CHK_BIT6(x)     (x & 0x40)
#define         CHK_BIT7(x)     (x & 0x80)
//==============================================================================
// BIT MASK
//==============================================================================
#define         MSK_BIT0(x)     (x &= 0x01)
#define         MSK_BIT1(x)     (x &= 0x02)
#define         MSK_BIT2(x)     (x &= 0x04)
#define         MSK_BIT3(x)     (x &= 0x08)
#define         MSK_BIT4(x)     (x &= 0x10)
#define         MSK_BIT5(x)     (x &= 0x20)
#define         MSK_BIT6(x)     (x &= 0x40)
#define         MSK_BIT7(x)     (x &= 0x80)

