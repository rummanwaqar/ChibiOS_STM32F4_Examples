/* Name: HelloWorld
   Desc: Simple LED Blinker
   Author: Rumman Waqar
*/

#include "ch.h"
#include "hal.h"

int main(void) {
    // performs all the init required to use various peripherals
	halInit();
    // gets the operating system code up and running
    // one of the timer unit is init to generate interrupts at a spec rate
	chSysInit();
	
    // configure the I/O pin
	palSetPadMode(GPIOD, GPIOD_LED6, PAL_MODE_OUTPUT_PUSHPULL);

	while (TRUE) {
		// sets a pin high
        palSetPad(GPIOD, GPIOD_LED6); // Blue LED
        // generates a 500ms delay
        // OS immediately shutsdown processor core & switches to low power mode
		chThdSleepMilliseconds(500);
        // sets a pin low
		palClearPad(GPIOD, GPIOD_LED6);
		chThdSleepMilliseconds(500);
	}
}
