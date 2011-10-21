extern int dbg;
extern char devicename[20];

typedef struct wckResponse {
	int b1; int b2;
} wck_t;

typedef struct result {
	int x; int y; int z;
} result_t;

typedef struct servos {
	int nos;
	unsigned char values[32];
} servos_t;

void 		setdevname	(char *n);

void 		initserial	(int f);
int 		openport	(char *device, int baudrate);
void 		closeport	();

int   		wckPosRead	(char ServoID);
int  		readPSD		();   
int 		readIR		();
void 		standup 	(int n);
void 		setdh		(int n);
void 		PlayMotion	(int n);
void 		wckWriteIO	(unsigned char ServoID, unsigned char IO);

result_t	*readXYZ	();
wck_t   	*wckReadPos	(int, int);
wck_t 		*wckMovePos	(int id, int pos, int torq);
wck_t 		*wckPassive	(int id);

void 		SyncPosSend	(int SpeedLevel, servos_t *s);
void 		PlayPoseA	(int d, int f, int tq, int flag, servos_t *s);
servos_t 	*readservos	(int n);
