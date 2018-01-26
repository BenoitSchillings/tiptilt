#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <math.h>
#include <unistd.h>

#include <libusb-1.0/libusb.h>


using namespace std;

//--------------------------------------------------------------------

#define USB_VENDOR_ID         0x0104d
#define USB_PRODUCT_ID        0x4000                       /* USB product ID used by the device */
#define USB_ENDPOINT_IN       (LIBUSB_ENDPOINT_IN  | 1)    /* endpoint address */
#define USB_ENDPOINT_OUT      (LIBUSB_ENDPOINT_OUT | 2)    /* endpoint address */
#define USB_TIMEOUT            3000                        /* Connection timeout (in ms) */

//--------------------------------------------------------------------

libusb_context *ctx = NULL;
libusb_device_handle *handle;
uint8_t receiveBuf[64];
uint8_t transferBuf[64];


//--------------------------------------------------------------------


int usb_write(const char *txt)
{
    int n, ret;
    
    n = sprintf((char*)transferBuf, "%s\r", txt);
    ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, transferBuf, n, &n, USB_TIMEOUT);
    
    if (ret != 0) {
        printf("Error in USB write %d\n", ret);
        return -1;
    }
    return ret;
}

//--------------------------------------------------------------------

int usb_read(void)
{
    int nread, ret;
    
    ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf), &nread, USB_TIMEOUT);
    if (ret) {
        printf("ERROR in bulk read: %d\n", ret);
        return -1;
    } else {
         return 0;
    }
}

//--------------------------------------------------------------------

int wait_complete(int motor)
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
        //        printf("[%d %c %c]\n", nread, receiveBuf[0], receiveBuf[1]);
        if (receiveBuf[0] == '1') return 0;
        usleep(500);
    } while(1);
}

//--------------------------------------------------------------------

int init_usb_connection()
{
    printf("init\n");
    libusb_init(&ctx);
    libusb_set_debug(ctx, 3);
    printf("init1\n");
    
    //Open Device with VendorID and ProductID
    handle = libusb_open_device_with_vid_pid(ctx, USB_VENDOR_ID, USB_PRODUCT_ID);
    
    if (!handle) {
        perror("device not found");
        exit(-1);
    }
    printf("init2\n");
    
    libusb_reset_device(handle);
    
    int r = 1;
    
    r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        fprintf(stderr, "usb_claim_interface error %d\n", r);
        return 2;
    }
    return 0;
}

//--------------------------------------------------------------------

void move(int motor, int delta)
{
    char    buf[256];
    
    sprintf(buf, "%dPR%d", motor, delta);
    usb_write(buf);
}

//--------------------------------------------------------------------


void sighandler(int signum)
{
    if (handle) {
        libusb_release_interface(handle, 0);
        libusb_close(handle);
     }
    libusb_exit(NULL);
    exit(0);
}

//--------------------------------------------------------------------

// x = (2/3)*a
// y = (-1/3) * a + tan(30)*b;
// z = (-1/3) * a - tan(30)*b;


//--------------------------------------------------------------------


class	tiptilt {
public:;

		tiptilt();
		~tiptilt();

	void	Move(float d_alpha, float d_beta);
	void	MoveTo(float alpha, float beta);	
	void	Reset();

	void	MoveFocus(float delta);
	void	MoveToFocus(float value);	

private:
	int     InitUSB();
	void	CloseUSB();

	int	WriteCommand(string cmd);
	string	ReadResult();
    void    upd_tilt();
    void    setxyz(float x, float y, float z);
    
    float   c_x;
    float   c_y;
    float   c_z;
    
    float   cal_x;
    float   cal_y;
    float   cal_z;
    
    float   c_a;
    float   c_b;
    float   d_focus;
};


//--------------------------------------------------------------------



   tiptilt::tiptilt()
{
    c_x = 0;
    c_y = 0;
    c_z = 0;
    d_focus = 0;
    c_a = 0;
    c_b = 0;
    
    cal_x = 1.0;
    cal_y = 1.0;
    cal_z = 1.0;
    InitUSB();
    
    usb_write("1VA2000");
    usb_write("1AC52000");
    usb_write("2VA2000");
    usb_write("2AC52000");

}


//--------------------------------------------------------------------

	tiptilt::~tiptilt()
{
    CloseUSB();
}

//--------------------------------------------------------------------

void    tiptilt::setxyz(float x, float y, float z)
{
    float ddx = x - c_x;
    float ddy = y - c_y;
    float ddz = z - c_y;
    
    if (ddx < 0) ddx *= cal_x;
    if (ddy < 0) ddy *= cal_y;
    if (ddz < 0) ddz *= cal_z;
    
    Move(1, round(ddx));
    wait_complete(1);
    Move(2, round(ddy));
    wait_complete(2);
    Move(3, round(ddz));
    wait_complete(3);
}

//--------------------------------------------------------------------

void    tiptilt::upd_tilt()
{
    float x,y,z;
    
    float tan30 = 0.57735026;
    
    x = (2.0 / 3.0) * c_a;
    y = (-1.0 / 3.0) * c_a + tan30 * c_b;
    z = (-1.0 / 3.0) * c_a - tan30 * c_b;

    x += d_focus;
    y += d_focus;
    z += d_focus;
    
    setxyz(x, y, z);
}

//--------------------------------------------------------------------

void	tiptilt::Move(float delta_alpha, float delta_beta)
{
    c_a += delta_alpha;
    c_b += delta_beta;
    upd_tilt();
}

//--------------------------------------------------------------------

void	tiptilt::MoveTo(float alpha, float beta)
{
    c_a = alpha;
    c_b = beta;
    upd_tilt();
}


//--------------------------------------------------------------------


void	tiptilt::MoveFocus(float delta)
{
    d_focus += delta;
    upd_tilt();
}

//--------------------------------------------------------------------

void	tiptilt::MoveToFocus(float value)
{
    d_focus = value;
    upd_tilt();
}

//--------------------------------------------------------------------

int	tiptilt::InitUSB()
{
	return init_usb_connection();
}

//--------------------------------------------------------------------

void	tiptilt::CloseUSB()
{
}

//--------------------------------------------------------------------

int	tiptilt::WriteCommand(string cmd)
{
    usb_write(cmd.c_str());
	return 0;
}


//--------------------------------------------------------------------

string	tiptilt::ReadResult()
{
    usb_read();
    
	return string((char*)receiveBuf);
}

//--------------------------------------------------------------------

tiptilt *tt;


int main()
{
    signal(SIGINT, sighandler);

    tt = new tiptilt();
    tt->Move(20, 0);
    delete tt;
    
	return 0;
}
