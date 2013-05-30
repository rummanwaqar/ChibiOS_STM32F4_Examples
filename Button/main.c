/* Name: Button Blink
   Desc: Uses button to get the speed input and blinks LEDs
   Author: Rumman Waqar
*/

#include "ch.h"
#include "hal.h"

static int leds[] = {GPIOD_LED4, GPIOD_LED3, GPIOD_LED5, GPIOD_LED6 };
static int speeds[] = { 50, 250, 500, 1000 };
static int selectedSpeed = 0;
/* Mutex is a threads sync object that can be in two distict states:
   1. Not owned(unlocked)       2. Owned by a thread(locked)    */
static Mutex speedMutex;

static WORKING_AREA(waInputThread, 128);
static msg_t InputThread(void *arg __attribute__((unused)) ) {
    int button = FALSE;
    chRegSetThreadName( "speed selector" ); // sets the current thread name in the thread reg
    while ( TRUE ) {
        int buttonState = palReadPad(GPIOA, GPIOA_BUTTON); // read user button
        if ( !button && buttonState ) { // button is False and buttonState is Pressed
            chMtxLock( &speedMutex );   // lock speedMutex so it selectedSpeed  be used by the thread
            selectedSpeed = (selectedSpeed + 1) & 3;
            chMtxUnlock(); // speedMutex unlocked
            button = TRUE;
        } else if ( button && !buttonState) { // button True and button not pressed
            button = FALSE;
        }
        chThdYield(); // yields CPU control to the next thread in the ready list with equal priority
    }
}

int main(void) {
	halInit();
	chSysInit();

    chMtxInit( &speedMutex ); // initialize the Mutex 
    chThdCreateStatic( waInputThread, sizeof(waInputThread), NORMALPRIO, InputThread, NULL );

    int i = 0;
    while( TRUE ) {
        int speed;
        chMtxLock( &speedMutex );
        speed = speeds[selectedSpeed];
        chMtxUnlock();

        palSetPad(GPIOD, leds[i]);
        chThdSleepMilliseconds(speed);
        palClearPad(GPIOD, leds[i++]);
        i &= 3; // making sure i stays between 0 and 3 by masking i with 11
    }
}
