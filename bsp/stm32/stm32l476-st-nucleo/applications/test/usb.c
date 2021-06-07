#ifdef __LINUX__
#include <libusb-1.0/libusb.h>
#else
#include "lusb0_usb.h"
#endif
#include <stdio.h>
#include <time.h> 
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define MY_VID 		0x3318
#define MY_PID 		0x0003
#define MY_CONFIG 	1
#define MY_DEV 		0x0200

#define EP_IN 		0x81
#define EP_OUT 		0x01
#define INTF 		0

#ifdef __LINUX__
libusb_device **devs;
static void open_dev(libusb_device **devs,
	libusb_device_handle **handle)
{
    libusb_device *dev;
    int i = 0, j = 0;
    uint8_t path[8]; 

    while ((dev = devs[i++]) != NULL) {
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
	    fprintf(stderr, "failed to get device descriptor");
	    return;
	}

/*	printf("%04x:%04x (bus %d, device %d)",
		desc.idVendor, desc.idProduct,
		libusb_get_bus_number(dev), libusb_get_device_address(dev));
*/
	if (desc.idVendor == MY_VID
		&& desc.idProduct == MY_PID)
	    libusb_open(dev, handle);
	r = libusb_get_port_numbers(dev, path, sizeof(path));
/*	if (r > 0) {
	    printf(" path: %d", path[0]);
	    for (j = 1; j < r; j++)
		printf(".%d", path[j]);
	}
	printf("\n");*/
    }

    return;
}
#else
static usb_dev_handle *open_dev(uint8_t intf)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
	for (dev = bus->devices; dev; dev = dev->next)
	{
/*	    printf("filename %s vid %x, pid %x,intf %x, dev %x\n",dev->filename,
		    dev->descriptor.idVendor,dev->descriptor.idProduct,
		    dev->config->interface->altsetting->bInterfaceNumber,
		    dev->descriptor.bcdDevice);*/
	    if (dev->descriptor.idVendor == MY_VID
		    && dev->descriptor.idProduct == MY_PID
		    && dev->config->interface->altsetting->bInterfaceNumber==intf
		    && dev->descriptor.bcdDevice == MY_DEV)
	    {
		return usb_open(dev);
	    }
	}
    }
    return NULL;
}
#endif
#ifdef __LINUX__
int hid_async_read(void *dev, uint8_t ep, uint8_t* cmd, uint32_t len, libusb_transfer_cb_fn callback)
{
	struct libusb_transfer *transfer = libusb_alloc_transfer(0);
	
	if (transfer == NULL) {
	    printf("alloc transfer failed\r\n");
	    return -1;
	}
	transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

	libusb_fill_interrupt_transfer(transfer, dev,
							ep | LIBUSB_ENDPOINT_IN,
			                cmd, len,
			                callback, NULL,
			                0); // timeout

	if (libusb_submit_transfer(transfer) != LIBUSB_SUCCESS)
	{
		libusb_free_transfer(transfer);
	    	printf("Error initiating transfer.\n");
	    return -1;
	}
	
	libusb_handle_events(NULL);
}
#endif
int hid_xfer(void *dev, uint8_t ep, uint8_t* cmd, uint32_t len, int timeout)
{
	int act_len;
#ifdef __LINUX__
	libusb_interrupt_transfer((libusb_device_handle *)dev, ep,
			cmd, len, &act_len, timeout);
	return act_len;
#else
	if ((ep & 0x80) != 0x00)
		act_len = usb_interrupt_read((usb_dev_handle *)dev, ep,
				cmd, len, timeout);
	else
    		act_len = usb_interrupt_write((usb_dev_handle *)dev, ep,
    				cmd, len, timeout);
#endif
	return act_len;
}
int usb_xfer(void *dev, int dir, int cmd, int val, int index,
		char *data, int len)
{
#ifdef __LINUX__
	int ret = libusb_control_transfer(
			(libusb_device_handle *) dev,
			LIBUSB_REQUEST_TYPE_VENDOR |
			LIBUSB_RECIPIENT_DEVICE |
			dir, cmd,
			val,
			index,
			data,
			len,
			1000);
	if (ret < 0) {
		printf("usb_xfer ERROR %s", libusb_error_name(ret));
		return -1;
	}
#else
	int ret = usb_control_msg((usb_dev_handle *)dev,
			USB_TYPE_VENDOR|USB_RECIP_DEVICE|dir, cmd, 
			val, index, data, len, 1000); 
	if (ret < 0)
		printf("usb control error %s\n", usb_strerror());
#endif

	return ret;
}


int ctl_xfer(void *dev, int dir, int cmd, int val, int index,
		char *data, int len)
{
#ifdef __LINUX__
	int ret = libusb_control_transfer(
			(libusb_device_handle *) dev,
			LIBUSB_REQUEST_TYPE_VENDOR |
			LIBUSB_RECIPIENT_DEVICE |
			dir, cmd,
			val,
			index,
			data,
			len,
			1000);
	if (ret < 0) {
		printf("usb_xfer ERROR %s", libusb_error_name(ret));
		return -1;
	}
#else
	int ret = usb_control_msg((usb_dev_handle *)dev,
			USB_TYPE_VENDOR|USB_RECIP_DEVICE|dir, cmd, 
			val, index, data, len, 1000); 
	if (ret < 0)
		printf("usb control error %s\n", usb_strerror());
#endif

	return ret;
}

void *open_usb(uint8_t intf)
{
#ifdef __LINUX__
	libusb_device *dev = NULL;
	int r,cnt;
	libusb_device_handle *dev1;
	
	r = libusb_init(NULL);
	if (r < 0)
		return NULL;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0){
		libusb_exit(NULL);
		return NULL;
	}
	
	open_dev(devs, &dev1);
	if (dev1 == NULL) {
		printf("can't find cyusb devices\n");
		libusb_free_device_list(devs, 1);
		libusb_exit(NULL);
		return NULL;
	}
    	
    	libusb_set_configuration(dev1, MY_CONFIG);
	
	if (libusb_kernel_driver_active(dev1, intf) == 1) {
		if (0 > libusb_detach_kernel_driver(dev1, intf))
		{
		    	printf("can't dettach cyusb devices\n");
			libusb_free_device_list(devs, 1);
			libusb_exit(NULL);
			return NULL;
		}
    	}
    	libusb_claim_interface(dev1, intf);
    	return dev1;
#else
	usb_dev_handle *dev = NULL;
	usb_init();
	usb_find_busses();
	usb_find_devices();

	if (!(dev = open_dev(intf)))
	{
		printf("error opening device: \n%s\n", usb_strerror());
		return NULL;
	}

	if (usb_set_configuration(dev, MY_CONFIG) < 0)
	{
		printf("error setting config #%d: %s\n", MY_CONFIG,
				usb_strerror());
		usb_close(dev);
		return NULL;
	}
    	
    	if (usb_claim_interface(dev, intf) < 0)
    	{
		printf("error claiming interface #%d:\n%s\n", intf, usb_strerror());
		usb_close(dev);
		return NULL;
	}

    	return dev;
#endif
}

void close_usb(void *dev, uint8_t intf)
{
#ifdef __LINUX__
    libusb_release_interface((libusb_device_handle *)dev, intf);
    libusb_attach_kernel_driver((libusb_device_handle *)dev, intf);
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
#else

    usb_release_interface((usb_dev_handle *)dev, intf);
    if (dev)
    {
	usb_close((usb_dev_handle *)dev);
    }
#endif
}
