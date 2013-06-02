/* 
 * Compile gcc -lusb-1.0 -o client client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <libusb-1.0/libusb.h>

#define USB_VENDOR_ID 0x0483 // USB venfor IS 0x0483 = STM
#define USB_PRODUCT_ID 0xFFFF // USB product ID used by the device
#define USB_ENDPOINT_IN (LIBUSB_ENDPOINT_IN | 1) // endpoint address
#define USB_ENDPOINT_OUT (LIBUSB_ENDPOINT_OUT | 2) // endpoint address
#define USB_TIMEOUT 3000 // connection timeout (in ms)

static libusb_context *ctx = NULL;
static libusb_device_handle *handle;

static uint8_t receiveBuf[64];
uint8_t transferBuf[64];

// read a packet
static int usb_read(void) {
  int nread, ret;
  // perform a bulk read. endpoint specifies read/write direction
  // nread is no of bytes actually transfered
  ret  = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf), &nread, USB_TIMEOUT);
  if(ret) {
    printf("[*] ERROR in bulk read :%d\n", ret);
    return -1;
  } else {
    printf("%s", receiveBuf);
    return 0;
  }
}

// write a few bytes to the device
static int usb_write(int n) {
  int ret;
  ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n, &n, USB_TIMEOUT);
  // error handling
  switch(ret) {
    case 0:
      printf("data sent");
      return 0;
    case LIBUSB_ERROR_TIMEOUT:
      printf("[*] ERROR in bulk write: %d Timeout\n", ret);
      break;
    case LIBUSB_ERROR_PIPE:
      printf("[*] ERROR in bulk write: %d Pipe\n", ret);
      break;
    case LIBUSB_ERROR_OVERFLOW:
      printf("[*] ERROR in bulk write: %d Overflow\n", ret);
      break;
    case LIBUSB_ERROR_NO_DEVICE:
      printf("[*] ERROR in bulk write: %d No Device\n", ret);
      break;
    default:
      printf("ERROR in bulk write : %d\n", ret);
      break;
  }
  return -1;
}

// on SIGINT: close USB interface
static void sighandler(int signum) {
  printf("\nInterrupt signal received\n");
  if (handle) {
    libusb_release_interface(handle, 0);
    libusb_close(handle);
  }
  libusb_exit(NULL);
  exit(0);
}

// main
int main(int argc, char **argv) {
  //pass interrupt signal to out handler
  signal(SIGINT, sighandler);

  // init libusb. param context pointer
  libusb_init(&ctx);
  libusb_set_debug(ctx, 3); // highest verbosity

  // open device with VendorID and ProductID
  handle = libusb_open_device_with_vid_pid(ctx, USB_VENDOR_ID, USB_PRODUCT_ID);
  if (!handle) {
    printf("[*] ERROR device not found.\n");
    return 1;
  }

  int r = 1;
  //claims the interface with the OS
  r = libusb_claim_interface(handle, 0);
  if (r < 0) { // error
    printf("[*] ERROR USB claim interface %d\n", r);
    return 2;
  }
  printf("Interface claimed\n");

  while (1) {
    int a, n;
    printf("> ");
    scanf("%d", &a);
    n = sprintf(transferBuf, "%d\0", a);
    if (usb_write(n) != 0) {
      return -1;
    }
    usb_read();
  }
  libusb_close(handle);
  libusb_exit(NULL);
}
