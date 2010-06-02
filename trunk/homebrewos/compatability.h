//Include file for compatability.c

#define NUM_OF_WCK_HUNO		16
#define NUM_OF_WCK_DINO		16
#define NUM_OF_WCK_DOGY		16
#define	MAX_wCK				31
#define	POS_MARGIN			10


#define NUM_OF_REMOCON		5
#define IR_BUFFER_SIZE		4
#define IR_HEADER_LT		63
#define IR_HEADER_UT		81
#define IR_LOW_BIT_LT		10
#define IR_LOW_BIT_UT		18
#define IR_HIGH_BIT_LT		19
#define IR_HIGH_BIT_UT		26


#define	BTN_NOT_PRESSED		0
#define	PF1_BTN_SHORT		1
#define	PF2_BTN_SHORT		2
#define	PF12_BTN_SHORT		3
#define	PF1_BTN_LONG		4
#define	PF2_BTN_LONG		5
#define	PF12_BTN_LONG		6

#define	PF1_HUNO			1
#define	PF1_DINO			2
#define	PF1_DOGY			3
#define	PF2_B				4

#define	NO_ERR				0xff
#define	SEEPROM_WR_ERR		1
#define	CHKSUM_ERR			2
#define	OVER_NUM_OF_M	    3
#define	OVER_NUM_OF_A	    4
#define	PF_MATCH_ERR		5
#define	INTERVAL_ERR		6
#define	ZERO_SET_ERR		7
#define	ZERO_DATA_ERR		8
#define	WCK_NUM_ERR			9
#define	WCK_POS_ERR			10
#define	WCK_NO_ACK_ERR		11


#define BTN_A		    0x01		// IR ������ A
#define BTN_B		    0x02		// IR ������ B
#define BTN_LR		    0x03		// IR ������ ��ȸ��
#define BTN_U		    0x04		// IR ������ ����
#define BTN_RR		    0x05		// IR ������ ��ȸ��
#define BTN_L		    0x06		// IR ������ ��
#define BTN_C		    0x07		// IR ������ �߾�
#define BTN_R		    0x08		// IR ������ ��
#define BTN_LA		    0x09		// IR ������ �°���
#define BTN_D		    0x0A		// IR ������ �Ʒ���
#define BTN_RA		    0x0B		// IR ������ �����
#define BTN_1		    0x0C		// IR ������ 1
#define BTN_2		    0x0D		// IR ������ 2
#define BTN_3		    0x0E		// IR ������ 3
#define BTN_4		    0x0F		// IR ������ 4
#define BTN_5		    0x10		// IR ������ 5
#define BTN_6		    0x11		// IR ������ 6
#define BTN_7		    0x12		// IR ������ 7
#define BTN_8		    0x13		// IR ������ 8
#define BTN_9		    0x14		// IR ������ 9
#define BTN_0		    0x15		// IR ������ 0

#define BTN_STAR_A		0x16		// IR ������ *+A
#define BTN_STAR_B		0x17		// IR ������ *+B
#define BTN_STAR_LR		0x18		// IR ������ *+��ȸ��
#define BTN_STAR_U		0x19		// IR ������ *+����
#define BTN_STAR_RR		0x1A		// IR ������ *+��ȸ��
#define BTN_STAR_L		0x1B		// IR ������ *+��
#define BTN_STAR_C		0x1C		// IR ������ *+�߾�
#define BTN_STAR_R		0x1D		// IR ������ *+��
#define BTN_STAR_LA		0x1E		// IR ������ *+�°���
#define BTN_STAR_D		0x1F		// IR ������ *+�Ʒ���
#define BTN_STAR_RA		0x20		// IR ������ *+�����
#define BTN_STAR_1		0x21		// IR ������ *+1
#define BTN_STAR_2		0x22		// IR ������ *+2
#define BTN_STAR_3		0x23		// IR ������ *+3
#define BTN_STAR_4		0x24		// IR ������ *+4
#define BTN_STAR_5		0x25		// IR ������ *+5
#define BTN_STAR_6		0x26		// IR ������ *+6
#define BTN_STAR_7		0x27		// IR ������ *+7
#define BTN_STAR_8		0x28		// IR ������ *+8
#define BTN_STAR_9		0x29		// IR ������ *+9
#define BTN_STAR_0		0x2A		// IR ������ *+0

#define BTN_SHARP_A		0x2B		// IR ������ #+A
#define BTN_SHARP_B		0x2C		// IR ������ #+B
#define BTN_SHARP_LR    0x2D		// IR ������ #+��ȸ��
#define BTN_SHARP_U		0x2E		// IR ������ #+����
#define BTN_SHARP_RR	0x2F		// IR ������ #+��ȸ��
#define BTN_SHARP_L		0x30		// IR ������ #+��
#define BTN_SHARP_C		0x31		// IR ������ #+�߾�
#define BTN_SHARP_R		0x32		// IR ������ #+��
#define BTN_SHARP_LA	0x33		// IR ������ #+�°���
#define BTN_SHARP_D		0x34		// IR ������ #+�Ʒ���
#define BTN_SHARP_RA	0x35		// IR ������ #+�����
#define BTN_SHARP_1		0x36		// IR ������ #+1
#define BTN_SHARP_2		0x37		// IR ������ #+2
#define BTN_SHARP_3		0x38		// IR ������ #+3
#define BTN_SHARP_4		0x39		// IR ������ #+4
#define BTN_SHARP_5		0x3A		// IR ������ #+5
#define BTN_SHARP_6		0x3B		// IR ������ #+6
#define BTN_SHARP_7		0x3C		// IR ������ #+7
#define BTN_SHARP_8		0x3D		// IR ������ #+8
#define BTN_SHARP_9		0x3E		// IR ������ #+9
#define BTN_SHARP_0		0x3F		// IR ������ #+0

#define P_BMC504_RESET(A)		if(A) SET_BIT6(PORTB);else CLR_BIT6(PORTB)

void DetectPower	(void);
void ChargeNiMH		(void);
void ProcButton		() {}
void AccGetData		(void);
void BasicPose		(int,int,int,int);
char wckPowerDown	(void);
