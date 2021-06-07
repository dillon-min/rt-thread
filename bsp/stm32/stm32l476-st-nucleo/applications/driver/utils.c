#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include "mcu.h"
#include "mcu_cmd.h"

#define DRV_DEBUG
#define LOG_TAG             "utils"
#include <drv_log.h>
rt_device_t ts_device;
static rt_err_t timer_timeout(rt_device_t dev, rt_size_t size)
{
	notify_event(EVENT_TIMER);
	return RT_EOK;
}
int normal_timer_init()
{
	int err = 0;
	rt_device_t tim_dev;
	rt_hwtimer_mode_t mode;
	rt_hwtimerval_t val;

	if ((tim_dev = rt_device_find("timer16")) == RT_NULL)
	{
		LOG_E("No Device: timer16");
		return -1;
	}

	if (rt_device_open(tim_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
	{
		LOG_E("Open timer16 Fail");
		return -1;
	}

	rt_device_set_rx_indicate(tim_dev, timer_timeout);
	mode = HWTIMER_MODE_PERIOD;
	err = rt_device_control(tim_dev, HWTIMER_CTRL_MODE_SET, &mode);
	val.sec = 0;
	val.usec = 10000;
	LOG_I("SetTime: Sec %d, Usec %d", val.sec, val.usec);
	if (rt_device_write(tim_dev, 0, &val, sizeof(val)) != sizeof(val))
		LOG_E("set timer failed");
	
	return 0;
}
int timestamp_init()
{
	int err = 0;
	rt_hwtimer_mode_t mode;
	rt_hwtimerval_t val;

	if ((ts_device = rt_device_find("timer15")) == RT_NULL)
	{
		LOG_E("No Device: timer15");
		return -1;
	}

	if (rt_device_open(ts_device, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
	{
		LOG_E("Open timer15 Fail");
		return -1;
	}

	mode = HWTIMER_MODE_PERIOD;
	err = rt_device_control(ts_device, HWTIMER_CTRL_MODE_SET, &mode);
	val.sec = 10*60*60;
	val.usec = 0;
	LOG_I("SetTime: Sec %d, Usec %d", val.sec, val.usec);
	if (rt_device_write(ts_device, 0, &val, sizeof(val)) != sizeof(val))
		LOG_E("set timer failed");
	
	return 0;
}

uint32_t read_ts()
{
	rt_hwtimerval_t val,val1;
	rt_device_read(ts_device, 0, &val, sizeof(val));
	return (val.sec*1000000+val.usec);
}

void read_ts_64(uint8_t *buf)
{
	static uint32_t old_sensor_ts = 0;
	static uint32_t up_ts = 0;
	uint32_t ts = read_ts();
	
	if (ts < old_sensor_ts) {
		up_ts++;
	}
	
	old_sensor_ts = ts;
	buf[0] = ts & 0xff;
	buf[1] = (ts >> 8) & 0xff;
	buf[2] = (ts >> 16) & 0xff;
	buf[3] = (ts >> 24) & 0xff;
	buf[4] = up_ts & 0xff;
	buf[5] = (up_ts >> 8) & 0xff;
	buf[6] = (up_ts >> 16) & 0xff;
	buf[7] = (up_ts >> 24) & 0xff;
}
void dump_mcu_cmd(uint16_t msg_id, uint16_t cmd_id,
		uint8_t *payload, uint16_t len)
{
	int64_val ts;
	uint16_t i = 0;
	uint8_t tmp[64] = {0};
	read_ts_64(ts.m_bytes);
	switch (msg_id) {
		case MSG_ID_HW_VER:
		strcpy(tmp, "MSG_ID_HW_VER: ");
		break;
		
		case MSG_ID_SW_VER:
		strcpy(tmp,"MSG_ID_SW_VER: ");
		break;
		
		case HEART_CMD:
//		strcat(tmp,"HEART_CMD: \r\n");
		break;
		
		case HOST_CMD_GET:
		strcpy(tmp,"HOST_CMD_GET: ");
		break;
		
		case HOST_CMD_SET:
		strcpy(tmp,"HOST_CMD_SET: ");
		break;
		
		case DEVICE_EVENT_PSENSOR:
		strcpy(tmp,"Event Psensor : ");
		break;
		
		case DEVICE_EVENT_KEY:
		strcpy(tmp,"Event Brightness Key : ");
		break;
		
		case NR_EVENT_ENV_LIGHT:
//		strcat(tmp,"Event Env Light : \r\n");
		break;
		
		case DEVICE_EVENT_MAG:
//		strcat(tmp,"Event Mag : \r\n");
		break;
		
		case DEVICE_EVENT_VSYNC:
//		strcat(tmp,"Event Vsync : \r\n");
		break;
		
		case DEVICE_EVENT_TMP_HEAD:
		strcpy(tmp,"Event Temp H : ");
		break;
		
		case DEVICE_EVENT_TMP_TAIL:
		strcpy(tmp,"Event Temp T : ");
		break;
		
		default:
		sprintf(tmp,"UnKnown MsgId(%x): ", msg_id);
		break;
	}

	if (msg_id == HOST_CMD_GET || msg_id == HOST_CMD_SET) {
		switch (cmd_id) {
			case NR_BRIGHTNESS:
				 strcat(tmp,"NR_BRIGHTNESS: ");
				 break;
			case NR_DISPLAY_2D_3D:
				 strcat(tmp,"NR_DISPLAY_2D_3D: ");
				 break;
			case NR_DISPLAY_VERSION:
				 strcat(tmp,"NR_DISPLAY_VERSION: ");
				 break;
			case NR_HOST_ID:
				 strcat(tmp,"NR_HOST_ID: ");
				 break;
			case NR_GLASSID:
				 strcat(tmp,"NR_GLASSID: ");
				 break;
			case NR_PSENSOR_CLOSED:
				 strcat(tmp,"NR_PSENSOR_CLOSED: ");
				 break;
			case NR_PSENSOR_NOCLOSED:
				 strcat(tmp,"NR_PSENSOR_NOCLOSED: ");
				 break;
			case NR_TEMPERATURE:
				 strcat(tmp,"NR_TEMPERATURE: ");
				 break;
			case NR_DISPLAY_DUTY:
				 strcat(tmp,"NR_DISPLAY_DUTY: ");
				 break;
			case NR_POWER_FUCTION:
				 strcat(tmp,"NR_POWER_FUCTION: ");
				 break;
			case NR_MAGNETIC_FUCTION:
				 strcat(tmp,"NR_MAGNETIC_FUCTION: ");
				 break;
			case NR_VSYNC_FUCTION:
				 strcat(tmp,"NR_VSYNC_FUCTION: ");
				 break;
			case NR_ENV_LIGHT:
				 strcat(tmp,"NR_ENV_LIGHT: ");
				 break;
			case NR_WORLD_LED:
				 strcat(tmp,"NR_WORLD_LED: ");
				 break;
			case NR_SLEEP_TIME:
				 strcat(tmp,"NR_SLEEP_TIME: ");
				 break;
			case NR_7211_VERSION:
				 strcat(tmp,"NR_7211_VERSION: ");
				 break;
			case NR_7211_UPDATE:
				 strcat(tmp,"NR_7211_UPDATE: ");
				 break;
			case NR_REBOOT:
				 strcat(tmp,"NR_REBOOT: ");
				 break;
			case NR_BRIGHTNESS_EXT:
				 strcat(tmp,"NR_BRIGHTNESS_EXT: ");
				 break;
			case NR_TEMPERATURE_FUNCTION:
				 strcat(tmp,
				 		 "NR_TEMPERATURE_FUNCTION: ");
				 break;
			case NR_TRY_CTRL_DISPLAY_STATUS:
				 strcat(tmp,
				 	"NR_TRY_CTRL_DISPLAY_STATUS: ");
				 break;
			case NR_TEMPERATURE_EXT:
				 strcat(tmp,"NR_TEMPERATURE_EXT: ");
				 break;
			case NR_BRIGHTNESS_MAX:
				 strcat(tmp,"NR_BRIGHTNESS_MAX: ");
				 break;
			case NR_SPEAKER_LEVEL:
				 strcat(tmp,"NR_SPEAKER_LEVEL: ");
				 break;
			case NR_CPU_INFO:
				 strcat(tmp,"NR_CPU_INFO: ");
				 break;
			case NR_ROM_INFO:
				 strcat(tmp,"NR_ROM_INFO: ");
				 break;
			case NR_RAM_INFO:
				 strcat(tmp,"NR_RAM_INFO: ");
				 break;
			case NR_LEFT_OLED_H_ORBIT:
				 strcat(tmp,
				 		 "NR_LEFT_OLED_H_ORBIT: ");
				 break;
			case NR_LEFT_OLED_V_ORBIT:
				 strcat(tmp,
				 		 "NR_LEFT_OLED_V_ORBIT: ");
				 break;
			case NR_RIGHT_OLED_H_ORBIT:
				 strcat(tmp,
				 		 "NR_RIGHT_OLED_H_ORBIT: ");
				 break;
			case NR_RIGHT_OLED_V_ORBIT:
				 strcat(tmp,
				 		 "NR_RIGHT_OLED_V_ORBIT: ");
				 break;
			case NR_ORBIT_ADJUST:
				 strcat(tmp,"NR_ORBIT_ADJUST: ");
				 break;
			case NR_COLOR:
				 strcat(tmp,"NR_COLOR: ");
				 break;
			case NR_RECOVERY_FACTORY:
				 strcat(tmp,
				 		 "NR_RECOVERY_FACTORY: ");
				 break;
			case NR_GLASS_BRI_TEST:
				 strcat(tmp,"NR_GLASS_BRI_TEST: ");
				 break;
			default:
				 strcat(tmp,"UnKnow CmdId(): ");
				 break;

		}
	}

	LOG_D("[%10lld]: %s", ts.m_int64/1000, tmp);
	if (len != 0 &&
		msg_id != DEVICE_EVENT_MAG &&
		msg_id != HEART_CMD &&
		msg_id != DEVICE_EVENT_VSYNC &&
		msg_id != NR_EVENT_ENV_LIGHT) {
	for (i=0; i<len; i++)
		rt_kprintf("0x%x \r\n", payload[i]);
	rt_kprintf("\r\n");
	}
}
