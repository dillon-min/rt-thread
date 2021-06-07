#ifndef __USB_H
#define __USB_H
#include <stdint.h>
#define EP_MCU_OUT 	0x01
#define EP_MCU_IN	0x81
#define EP_IMU_OUT	0x01
#define EP_IMU_IN	0x81
#define INTF_MCU	0x02
#define INTF_IMU	0x01
void *open_usb(uint8_t intf);
void close_usb(void *dev, uint8_t intf);
int ctl_xfer(void *dev, int dir, int cmd, int val, int index,
		char *data, int len);
int hid_xfer(void *dev, uint8_t ep, uint8_t* cmd, uint32_t len, int timeout);
int usb_xfer(void *dev, int dir, int cmd, int val, int index,
		char *data, int len);
#endif
