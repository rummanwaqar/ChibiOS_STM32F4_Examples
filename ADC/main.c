/* Name: ADC
   Author: Rumman Waqar
*/

#include "ch.h"
#include "hal.h"

// params are adc driver, pointer to most recent sample and rows available (buffer depth)
static void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n);

#define ADC_GRP1_NUM_CHANNELS 1 // no of channels
#define ADC_GRP1_BUF_DEPTH 1 // no of samples

// samples buffer
static adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

// ADC Conversion group
static const ADCConversionGroup adcgrpcfg = {
    FALSE, // Linear Buffer
    ADC_GRP1_NUM_CHANNELS, // No of channels
    adccb, // callback function
    NULL, // error callback function
    0, // control register 1
    ADC_CR2_SWSTART, // control register 2 - start ADC
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_480), // sample time register - AN11 Sample Rate = 480
    0, // sample time register 2
    ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS), // regular seq reg 1 - set number of channels
    0, // regular seq reg 2
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) // regular seq reg - 1st conversion
};

// adc callback function
void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
    (void)buffer; (void)n;
    if (adcp->state == ADC_COMPLETE) { // check if conv complete
        if (samples[0] < 1000) {
            palSetPad(GPIOD, GPIOD_LED4);
        } else if (samples[0] < 2000) {
            palSetPad(GPIOD, GPIOD_LED3);
        } else if (samples[0] < 3000) {
            palSetPad(GPIOD, GPIOD_LED5);
        } else {
            palSetPad(GPIOD, GPIOD_LED6);
        }
    }
}

int main(void) {
	halInit();
	chSysInit();
	
    // start the adc
    adcStart(&ADCD1, NULL);
	palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG); // PC1 set as Analog mode
    
    // start first conversion
    adcStartConversionI(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
	while (TRUE) {
        // start a new conversion
        adcStartConversionI(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
        chThdSleepMilliseconds(100);
	}
}
