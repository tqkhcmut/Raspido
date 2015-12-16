/*
 * usb_rf.c
 *
 *  Created on: Dec 15, 2015
 *      Author: kieutq
 */

#include "usb_rf.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <usb.h>
#include <pthread.h>

char COM[32];

int usbrf_fd = 0;

pthread_mutex_t usbrf_fd_access;
static inline void lock_usbrf_access(void)
{
	while (pthread_mutex_trylock(&usbrf_fd_access) != 0)
		usleep(100);
}
static inline void unlock_usbrf_access(void)
{
	pthread_mutex_unlock(&usbrf_fd_access);
}


#if USBRF_AUTO
pthread_t usb_polling_handler;
void * usb_polling(void * params)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    for(;;)
    {
    	usb_init();
    	usb_find_busses();
    	usb_find_devices();
    	for (bus = usb_busses; bus; bus = bus->next)
    	{
    		for (dev = bus->devices; dev; dev = dev->next)
    		{
    			printf("Trying device %s/%s\n", bus->dirname, dev->filename);
    			printf("\tID_VENDOR = 0x%04x\n", dev->descriptor.idVendor);
    			printf("\tID_PRODUCT = 0x%04x\n", dev->descriptor.idProduct);
    		}
    	}

    	sleep(1);
    }
    return NULL;
}
#endif

int set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		printf ("error %d from tcgetattr", errno);
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		printf ("error %d from tcsetattr", errno);
		return -1;
	}
	return 0;
}

void set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		printf ("error %d from tggetattr", errno);
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 1;            // 0.1 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		printf ("error %d setting term attributes", errno);
}


void USBRF_Init(void)
{
	memset(COM, 0, 32);
	memcpy(COM, "/dev/ttyUSB0", strlen("/dev/ttyUSB0"));
#if USBRF_AUTO
	pthread_create(&usb_polling_handler, NULL, usb_polling, NULL);
#else

#endif
}
int USBRF_ConnectAvailable(void)
{
	if( access(COM, F_OK ) != -1 ) {
	    return 1;
	} else {
	    return 0;
	}
}

int USBRF_Connect(void)
{
	int res = 0;
	lock_usbrf_access();
	usbrf_fd = open (COM, O_RDWR | O_NOCTTY | O_SYNC);

	if (usbrf_fd < 0)
	{
		printf("error %d opening %s: %s\r\n", errno, COM, strerror (errno));
		res = -1;
	}
	set_interface_attribs (usbrf_fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (usbrf_fd, 0);

	unlock_usbrf_access();
	return res;
}

int USBRF_DisConnect(void)
{
	int res = 0;
	lock_usbrf_access();
	if (usbrf_fd < 0)
	{
		res = -1;
	}
	else
	{
		close(usbrf_fd);
	}
	unlock_usbrf_access();
	return res;
}
int USBRF_DataAvailable(void)
{
	int res = 0;
	return res;
}
int USBRF_DataSend(uint8_t * buffer, int len)
{
	int res = 0;
	lock_usbrf_access();
	res = write(usbrf_fd, buffer, len);
	unlock_usbrf_access();
	return res;
}
int USBRF_DataGet(uint8_t * buffer, int len)
{
	int res = 0;
	lock_usbrf_access();
	res = read(usbrf_fd, buffer, len);
	unlock_usbrf_access();
	return res;
}
