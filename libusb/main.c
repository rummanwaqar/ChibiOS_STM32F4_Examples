#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "usbdescriptor.h"

uint8_t receiveBuf[OUT_PACKETSIZE];
#define IN_MULT 4
uint8_t transferBuf[IN_PACKETSIZE * IN_MULT];

USBDriver * usbp = &USBD1;
uint8_t initUSB=0;
uint8_t usbStatus=0;

/*
 * data Transmitted Callback
 */
void dataTransmitted(USBDriver *usbp, usbep_t ep) {
  (void) usbp;
  (void) ep;
  // Toggle a status LED 
  palTogglePad(GPIOD, GPIOD_LED3);

  // exit on USB reset
  if(!usbStatus) return;

  // prepares for a transmit transaction on an IN endpoint
  usbPrepareTransmit(usbp, EP_IN, transferBuf, sizeof transferBuf);

  chSysLockFromIsr();
  // starts a transmit transaction on an IN endpoint
  usbStartTransmitI(usbp, EP_IN);
  chSysUnlockFromIsr();
}

static USBInEndpointState ep1instate;

// EP1 init struct (IN only)
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_BULK,  //Type & mode of the endpoint
  NULL,                   //Setup packet notification callback
  dataTransmitted,        //IN endpoint notification callback
  NULL,                   //OUT endpoint notification callback
  IN_PACKETSIZE,          //IN endpoint maximum packet size
  0x0000,                 //OUT endpoint maximum packet size
  &ep1instate,            //USBEndpointState associated to the IN endpoint
  NULL,                   //USBEndpointState associated to the OUT endpoint
  IN_MULT,                
  NULL                    //Pointer to a buffer for setup packets
};

// data received Callback
// toggles an LED
void dataReceived(USBDriver *usbp, usbep_t ep) {
  // select ep from active endpoint config
  USBOutEndpointState *osp = usbp->epc[ep]->out_state;
  (void) usbp;
  (void) ep;
  // exit on USB rest
  if(!usbStatus) return;

  // bytes recieved so far
  if(osp->rxcnt) {
    switch(receiveBuf[0]) {
      case '1':
        palTogglePad(GPIOD, GPIOD_LED3);
        break;
      case '2':
        palTogglePad(GPIOD, GPIOD_LED4);
        break;
      case '3':
        palTogglePad(GPIOD, GPIOD_LED5);
        break;
      case '4':
        palTogglePad(GPIOD, GPIOD_LED6);
        break;
    }
  }

  // init next receive
  // recieve size 64
  usbPrepareReceive(usbp, EP_OUT, receiveBuf, 64);

  chSysLockFromIsr();
  usbStartReceiveI(usbp, EP_OUT);
  chSysUnlockFromIsr();
}

USBOutEndpointState ep2outstate;
static const USBEndpointConfig ep2config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  NULL,
  dataReceived,
  0x0000,
  OUT_PACKETSIZE,
  NULL,
  &ep2outstate,
  1,
  NULL
};

// Handles the USB driver global events.
static void usb_event(USBDriver *usbp, usbevent_t event) {
  (void) usbp;
  switch (event) {
    case USB_EVENT_RESET:
      usbStatus = 0;
      palTogglePad(GPIOD, GPIOD_LED3);
      return;
    case USB_EVENT_ADDRESS:
      return;
    case USB_EVENT_CONFIGURED:
      chSysLockFromIsr();
      usbInitEndpointI(usbp, 1, &ep1config);
      usbInitEndpointI(usbp, 2, &ep2config);
      chSysUnlockFromIsr();
      initUSB = 1; // allow the main thread to init the transfers
      return;
    case USB_EVENT_SUSPEND:
      return;
    case USB_EVENT_WAKEUP:
      return;
    case USB_EVENT_STALLED:
      return;
  }
  palTogglePad(GPIOD, GPIOD_LED5);
  return;
}

// returns the USB descriptors
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype, 
                                           uint8_t dindex,
                                           uint16_t lang) {
  (void)usbp;
  (void)lang;

  switch (dtype) {
    case USB_DESCRIPTOR_DEVICE:
      return &vcom_device_descriptor;
    case USB_DESCRIPTOR_CONFIGURATION:
      return &vcom_configuration_descriptor;
    case USB_DESCRIPTOR_STRING:
      if (dindex < 4) {
        return &vcom_strings[dindex];
      } else {
        return &vcom_strings[4];
      }
  }
  palTogglePad(GPIOD, GPIOD_LED4);
  return NULL;
}

// requests hook callback. allows to be notified of standard requests
bool_t requestsHook(USBDriver *usbp) {
  (void) usbp;
  return FALSE;
}

// USBConfig
const USBConfig config = {
  usb_event, 
  get_descriptor,
  requestsHook,
  NULL // start of frame callback
};

int main(void) {
  uint16_t i;

  //fill the transfer buffer
  for (i=0; i<sizeof transferBuf;i++) 
    transferBuf[i] = 'a'+(i%26);

  halInit();
  chSysInit();

  // Start and Connect USB
  usbStart(usbp, &config);
  usbConnectBus(usbp);

  while (TRUE) {
    // initUSB is true, when the USB cable is disconnected
    // or on first iteration
    while (!initUSB)
      chThdSleepMilliseconds(1000);

    chThdSleepMilliseconds(100);
    usbStatus=1;

    // start first transaction.
    // further trabsactions handled by dataReceived callback
    usbPrepareReceive(usbp, EP_OUT, receiveBuf, 64);
    chSysLock();
    usbStartReceiveI(usbp, EP_OUT);
    chSysUnlock();

    // start first transfer
    // all further transactions are init by the dataTransmitted callback
    usbPrepareTransmit(usbp, EP_IN, transferBuf, sizeof transferBuf);
    chSysLock();
    usbStartTransmitI(usbp, EP_IN);
    chSysUnlock();
    initUSB=0;
  }
}
