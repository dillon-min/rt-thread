/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   change to new framework
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_gpio.h"
#include "crc.h"
#include "mem_list.h"
#include <fal.h>
#include "param.h"
#include "utils.h"
#include "mcu.h"
#include <finsh.h>
#define DRV_DEBUG
#define LOG_TAG             "main.mcu"
#include <drv_log.h>
rt_device_t hid_device;

/* defined the LED0 pin: PC13 */
#define LED0_PIN    GET_PIN(C, 13)
#define SCL_PIN    GET_PIN(E, 0)
#define SDA_PIN    GET_PIN(E, 1)

rt_uint32_t g_heart_t2 = 0;

int vcom_init(void)
{
    /* set console */
    rt_console_set_device("vcom");

#if defined(RT_USING_POSIX)
    /* backup flag */
    dev_old_flag = ioctl(libc_stdio_get_console(), F_GETFL, (void *) RT_NULL);
    /* add non-block flag */
    ioctl(libc_stdio_get_console(), F_SETFL, (void *) (dev_old_flag | O_NONBLOCK));
    /* set tcp shell device for console */
    libc_stdio_set_console("vcom", O_RDWR);

    /* resume finsh thread, make sure it will unblock from last device receive */
    rt_thread_t tid = rt_thread_find(FINSH_THREAD_NAME);
    if (tid)
    {
        rt_thread_resume(tid);
        rt_schedule();
    }
#else
    /* set finsh device */
    finsh_set_device("vcom");
#endif /* RT_USING_POSIX */

    return 0;
}

static rt_err_t event_hid_in(rt_device_t dev, void *buffer)
{
	notify_event(EVENT_ST2OV);
	//rt_sem_release(&tx_sem_complete);
	return RT_EOK;
}

void hid_out(uint8_t *data, uint16_t len)
{
	int i;
	rt_uint8_t *ptr = data;

	rt_kprintf("\r\n");
	for (i = 0; i < len; i++) {
		if (i%16 == 0 && i!=0)
			rt_kprintf("\r\n");
		rt_kprintf("%02x ", *ptr++);
	}
	rt_kprintf("\r\n");
	if (rt_device_write(hid_device, 0x01, data+1, (len-1)) != (len-1))
		rt_kprintf("hid out failed\r\n");
}

static void dump_data(rt_uint8_t *data, rt_size_t size)
{
	rt_size_t i;
	rt_uint8_t *ptr = data;

	rt_kprintf("\r\n");
	for (i = 0; i < size; i++) {
		if (i%16 == 0 && i!=0)
			rt_kprintf("\r\n");
		rt_kprintf("%02x ", *ptr++);
	}
	rt_kprintf("\r\n");

	if (!insert_mem(TYPE_H2D, data, 64))
		LOG_W("lost h2d packet\r\n");
	notify_event(EVENT_OV2ST);
}

static void dump_report(struct hid_report * report)
{
	dump_data(report->report,report->size);
}

void HID_Report_Received(hid_report_t report)
{
	g_heart_t2 = read_ts();
	dump_report(report);
}

static int generic_hid_init(void)
{
	int err = 0;
	hid_device = rt_device_find("hidd");

	RT_ASSERT(hid_device != RT_NULL);

	err = rt_device_open(hid_device, RT_DEVICE_FLAG_RDWR);

	if (err != RT_EOK)
	{
		rt_kprintf("open dev failed!\n");
		return -1;
	}

	//rt_event_init(&light_event, "event", RT_IPC_FLAG_FIFO);
	//rt_sem_init(&tx_sem_complete, "tx_complete_sem_hid", 1, RT_IPC_FLAG_FIFO);

	rt_device_set_tx_complete(hid_device, event_hid_in);
#if 0
	rt_pin_mode(VSYNC_INT_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_attach_irq(VSYNC_INT_PIN, PIN_IRQ_MODE_RISING, vsync_isr, RT_NULL);
	rt_pin_irq_enable(VSYNC_INT_PIN, RT_TRUE);
	rt_thread_init(&usb_thread,
			"hidd_app",
			usb_thread_entry, hid_device,
			usb_thread_stack, sizeof(usb_thread_stack),
			10, 20);

	rt_thread_startup(&usb_thread);
#endif
	return 0;
}

extern int fal_init(void);
INIT_COMPONENT_EXPORT(fal_init);

int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    if (!param_init())
        LOG_D("can't startup system\r\n");
    //rt_memlist_init();
    //timestamp_init();
    //protocol_init();
    generic_hid_init();

    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(5000);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(5000);
    }
}
