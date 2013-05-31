/* Name: General Purpose Timer
   Author: Rumman Waqar

   Timer must be enabled in HALconf and the correct timer must be selected in mcuconf
*/

#include "ch.h"
#include "hal.h"

// Call back function for the timer
static void toggleLED(GPTDriver *gptp) {
    palTogglePad(GPIOD, GPIOD_LED6);
}

// GPT2 Configuration
static GPTConfig gptcfg = 
{
    700000, // 700 KHz timer
    toggleLED
};

int main(void) {
    // performs all the init required to use various peripherals
	halInit();
    // gets the operating system code up and running
    // one of the timer unit is init to generate interrupts at a spec rate
	chSysInit();
	
    // configure the I/O pin
	palSetPadMode(GPIOD, GPIOD_LED6, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOD, GPIOD_LED5, PAL_MODE_OUTPUT_PUSHPULL);

    // configure the GPT Timer
    gptStart(&GPTD2, &gptcfg);

    // start timer in continuous mode
    gptStartContinuous(&GPTD2, 10); // 70KHz

	while (TRUE) {
		// sets a pin high
        palSetPad(GPIOD, GPIOD_LED5);
        // generates a 500ms delay
        // OS immediately shutsdown processor core & switches to low power mode
		chThdSleepMilliseconds(500);
        // sets a pin low
		palClearPad(GPIOD, GPIOD_LED5);
		chThdSleepMilliseconds(500);
	}
}
