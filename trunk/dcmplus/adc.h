//==============================================================================
// include file for adc.c
//==============================================================================

#define NUM_OF_AD_SAMPLE		10
#define ADC_VREF_TYPE			0x20
#define PSD_CH					0
#define VOLTAGE_CH				1
#define MIC_CH					0x0F
#define ADC_MODE_DISABLE		0x00     
#define ADC_MODE_SINGLE			0xDC
#define ADC_MODE_FREERUN_NOINT	0xE6
#define ADC_MODE_FREERUN_SP		0xED


BYTE 	adc_psd(void);
WORD 	adc_volt(void);
BYTE 	adc_mic(void);
int 	adc_test(BYTE);

void 	PSD_on(void);
void 	PSD_off(void);
void 	Get_AD_PSD(void);

void 	Get_AD_MIC(void);
void 	Get_VOLTAGE(void);

extern 	volatile WORD	gVOLTAGE;
extern 	volatile BYTE	gDistance;
extern 	volatile BYTE	gSoundLevel;



