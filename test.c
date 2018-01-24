/*
 * Simple libusb-1.0 test programm
 * It openes an USB device, expects two Bulk endpoints,
 *   EP1 should be IN
 *   EP2 should be OUT
 * It alternates between reading and writing a packet to the Device.
 * It uses Synchronous device I/O
 *
 * Compile:
 *   gcc -lusb-1.0 -o test test.c
 * Run:
 *   ./test
 * Thanks to BertOS for the example:
 *   http://www.bertos.org/use/tutorial-front-page/drivers-usb-device
 *
 * For Documentation on libusb see:
 *   http://libusb.sourceforge.net/api-1.0/modules.html
 */
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <libusb-1.0/libusb.h>


#define USB_VENDOR_ID	    	0x0104d	
#define USB_PRODUCT_ID		0x4000				/* USB product ID used by the device */
#define USB_ENDPOINT_IN		(LIBUSB_ENDPOINT_IN  | 1)	/* endpoint address */
#define USB_ENDPOINT_OUT	(LIBUSB_ENDPOINT_OUT | 2)	/* endpoint address */
#define USB_TIMEOUT	        3000				/* Connection timeout (in ms) */

static libusb_context *ctx = NULL;
static libusb_device_handle *handle;

static uint8_t receiveBuf[64];
uint8_t transferBuf[64];

uint16_t counter = 0;


uint16_t count = 0;



static int usb_write(char *txt)
{
	int n, ret;

	n = sprintf(transferBuf, "%s\r", txt);
	ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n, &n, USB_TIMEOUT);

	//Error handling
	switch (ret) {
	case 0:
		return 0;
	case LIBUSB_ERROR_TIMEOUT:
		printf("ERROR in bulk write: %d Timeout\n", ret);
		break;
	case LIBUSB_ERROR_PIPE:
		printf("ERROR in bulk write: %d Pipe\n", ret);
		break;
	case LIBUSB_ERROR_OVERFLOW:
		printf("ERROR in bulk write: %d Overflow\n", ret);
		break;
	case LIBUSB_ERROR_NO_DEVICE:
		printf("ERROR in bulk write: %d No Device\n", ret);
		break;
	default:
		printf("ERROR in bulk write: %d\n", ret);
		break;

	}
	return -1;

}


static int usb_read(void)
{
	int nread, ret;
	ret =
	    libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf,
				 sizeof(receiveBuf), &nread, USB_TIMEOUT);
	if (ret) {
		printf("ERROR in bulk read: %d\n", ret);
		return -1;
	} else {
		printf("%d receive %d bytes from device: %s\n", ++counter,
		       nread, receiveBuf);
		//printf("%s", receiveBuf);  //Use this for benchmarking purposes
		return 0;
	}
}


static int wait_complete(int motor)
{
	int nread, ret;

	do {
		char buf[256];

		sprintf(buf, "%dMD?", motor);
		usb_write(buf);

		ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf), &nread, USB_TIMEOUT);
		if (ret) {
			printf("ERROR in bulk read: %d\n", ret);
			return -1;
		}
//		printf("[%d %c %c]\n", nread, receiveBuf[0], receiveBuf[1]);
		if (receiveBuf[0] == '1') return 0;
		usleep(500);
	} while(1);
}



/*
 * on SIGINT: close USB interface
 * This still leads to a segfault on my system...
 */
static void sighandler(int signum)
{
	printf("\nInterrupt signal received\n");
	if (handle) {
		libusb_release_interface(handle, 0);
		printf("\nInterrupt signal received1\n");
		libusb_close(handle);
		printf("\nInterrupt signal received2\n");
	}
	printf("\nInterrupt signal received3\n");
	libusb_exit(NULL);
	printf("\nInterrupt signal received4\n");

	exit(0);
}


void move(int motor, int delta)
{
	char	buf[256];

	sprintf(buf, "%dPR%d", motor, delta);
	usb_write(buf);
}

int main(int argc, char **argv)
{
	//Pass Interrupt Signal to our handler
	signal(SIGINT, sighandler);

	libusb_init(&ctx);
	libusb_set_debug(ctx, 3);

	//Open Device with VendorID and ProductID
	handle = libusb_open_device_with_vid_pid(ctx,
						 USB_VENDOR_ID, USB_PRODUCT_ID);

	if (!handle) {
		perror("device not found");
		return 1;
	}
	libusb_reset_device(handle);

	int r = 1;

	r = libusb_claim_interface(handle, 0);
	if (r < 0) {
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		return 2;
	}
	printf("Interface claimed\n");

	usb_write("VE?");
	usb_read();

	usb_write("1VA2000");
	usb_write("1AC52000");
	usb_write("2VA2000");
	usb_write("2AC52000");

	int delta;
	int motor;

	for (int i = 0; i < 33500; i++) {
		motor = rand()%2 + 1;

		delta = rand()%3 + 1;
		move(motor, delta);
		usleep(500);
		wait_complete(motor);
		if (motor == 1)
			move(motor, -delta*1.41);
		else
			move(motor, -delta*1.51);
		usleep(500);
		wait_complete(motor);
		usleep(50000);
	}

/*
	for (int i = 0; i < 400; i++) {
		move(1, 1);
		wait_complete(1);
		move(1, -1);
		wait_complete(1);
		printf("%d\n", i);
	}
*/
	usb_write("AB");
	r = libusb_release_interface(handle, 0);
	printf("release %d\n", r);
	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}
