//==============================================================================
// include file for adc.c
//==============================================================================

int adc_init(void);
BYTE adc_psd(void);
WORD adc_volt(void);
WORD adc_mic(void);

int adc_test(BYTE);

extern volatile BYTE psd_value;
extern volatile WORD volts;
extern volatile WORD mic_vol;

