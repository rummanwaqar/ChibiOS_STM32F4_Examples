#include "ch.h"
#include "hal.h"

#include "chprintf.h" // implements printf, used by shell
#include "shell.h" // implements command line shell

#include "usbcfg.h" // usb vcp settings

#define isUSBActive() SDU1.config->usbp->state == USB_ACTIVE

void USBInit(void);

/* Virtual serial port over USB.*/
SerialUSBDriver SDU1;

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

// allocate space for shell
#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

// mem command implementation for the shell
static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

// threads command implementation for shell
static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_about(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if(argc > 0) {
    chprintf(chp, "Usage: about\r\n");
    return;
  }
  chprintf(chp, "CDC VCP Demo\r\n");
  chprintf(chp, "By Rumman Waqar\r\n");
}
// table of shell commands
// nameString, function
static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"about", cmd_about},
  {NULL, NULL}
};

// Shell descriptor
static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1, // channel associated to Shell SerialUSB
  commands // extra shell commands table
};

/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/

/*
 * Application entry point.
 */
int main(void) {
  // thread representing the shell
  Thread *shelltp = NULL;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  USBInit();

  // main activity - shell respawn upon its termination.
  while (TRUE) {
    if (!shelltp && isUSBActive()) { // Checks if shell doesnt exist and  USB is active
      /* Spawns a new shell.*/
      shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    }
    else {
      /* If the previous shell exited.*/
      if (chThdTerminated(shelltp)) {
        /* Recovers memory of the previous shell.*/
        chThdRelease(shelltp);
        shelltp = NULL;
      }
    }
    chThdSleepMilliseconds(500);
  }
}

void USBInit() {
  /*
   * Shell manager initialization.
   */
  shellInit();

  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU1); // Init full duplex driver object. Pointer to SerialUSBDriver Struct
  sduStart(&SDU1, &serusbcfg); // Configures and starts the driver. Pointer to SerialUSBDriver
                               // and serial over usb configuration.

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp); 
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg); // Configures and activates the USB peripheral
  usbConnectBus(serusbcfg.usbp); // Connects the USB Device
}
