/* Name: Dynamic Thread using Heal Allocator
   Desc: Use a dynamic thread to make two LEDs blink
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

static msg_t Thread1 (void *arg) {
    palSetPadMode(GPIOD, GPIOD_LED5, PAL_MODE_OUTPUT_PUSHPULL);
    
    unsigned i = 10;
    while (i > 0) {
        palSetPad(GPIOD, GPIOD_LED5); // Red LED
        chThdSleepMilliseconds(100);
        palClearPad(GPIOD, GPIOD_LED5);
        chThdSleepMilliseconds(100);
        i--;
    }
    return (msg_t)i;
}

int main(void) {
	halInit();
	chSysInit();
	
    /* Null = Default Heap, Stack Size , ... */
    Thread *tp = chThdCreateFromHeap(NULL, THD_WA_SIZE(128), NORMALPRIO+1, Thread1, NULL);

	palSetPadMode(GPIOD, GPIOD_LED6, PAL_MODE_OUTPUT_PUSHPULL);

	while (TRUE) {
        if(tp == NULL)
            chSysHalt(); // Memory exausted 
        palSetPad(GPIOD, GPIOD_LED6); // Blue LED
		chThdSleepMilliseconds(500);
		palClearPad(GPIOD, GPIOD_LED6);
		chThdSleepMilliseconds(500);
	}
}
