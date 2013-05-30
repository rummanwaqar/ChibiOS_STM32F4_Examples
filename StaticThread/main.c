/* Name: Static Thread 
   Desc: Use a static thread to make two LEDs blinkr
   Author: Rumman Waqar
*/

/* NOTE ON THREADS 
There are two classes of threads in ChibiOS:
* Static Threads - statically allocated in memory at compile time
* Dynamic Threads - created by allocating memory at run time from memory 
                    heap or memory pool
*/

#include "ch.h"
#include "hal.h"

/* to create a static thread a working area must be declared 
This macro reserves 180 bytes of stack for the thread and space for all 
thread related structures. */
static WORKING_AREA(waThread1, 128);

static msg_t Thread1 (void *arg) {
    palSetPadMode(GPIOD, GPIOD_LED5, PAL_MODE_OUTPUT_PUSHPULL);

    while (TRUE) {
        palSetPad(GPIOD, GPIOD_LED5); // Red LED
        chThdSleepMilliseconds(100);
        palClearPad(GPIOD, GPIOD_LED5);
        chThdSleepMilliseconds(100);
    }
}

int main(void) {
	halInit();
	chSysInit();
	
    /* it takes thread working area, size of working area, proiority, thread 
        function and thread parameters and returns a pointer to the thread. */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	palSetPadMode(GPIOD, GPIOD_LED6, PAL_MODE_OUTPUT_PUSHPULL);

	while (TRUE) {
        palSetPad(GPIOD, GPIOD_LED6); // Blue LED
		chThdSleepMilliseconds(500);
		palClearPad(GPIOD, GPIOD_LED6);
		chThdSleepMilliseconds(500);
	}
}
