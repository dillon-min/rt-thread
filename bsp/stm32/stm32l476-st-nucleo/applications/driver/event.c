#include <rtthread.h>
#include <stdio.h>
#include "mcu.h"
#include "mcu_cmd.h"
#include "rc4.h"
#include "utils.h"
#include "param.h"

#define DRV_DEBUG
#define LOG_TAG             "protocol.event"
#include <drv_log.h>

static uint16_t temp_trigger = 0;
#define EVENT_PAYLOAD_OFS 10 
int64_val vsync_seq = {{0}};

uint8_t local_protocol_version = 1;
uint8_t remote_protocol_version = 1;
void handle_int_vsync()
{
	uint8_t vsync[32] = {0};
	vsync_seq.m_int64++;
	read_ts_64(vsync+EVENT_PAYLOAD_OFS);
	rt_memcpy(vsync+EVENT_PAYLOAD_OFS+8, vsync_seq.m_bytes, 8);
	build_event(vsync, DEVICE_EVENT_VSYNC, 16);
	mcu_msg_send(vsync); 
}

static void _handle_status(uint16_t msg_id, uint8_t event)
{
	uint8_t data[32] = {0};
	data[EVENT_PAYLOAD_OFS] = 0x00;
	data[EVENT_PAYLOAD_OFS+1] = event;
	build_event(data, msg_id, 2);
	mcu_msg_send(data); 
}

void handle_event_psensor(uint8_t type)
{
	_handle_status(DEVICE_EVENT_PSENSOR, type);
}

void handle_event_bri_key(uint32_t ev_stat)
{
	uint8_t event = MCU_BIN_KEY_UP;
	if (ev_stat & 0x01)
		event = MCU_BIN_KEY_DOWN;
	_handle_status(DEVICE_EVENT_KEY, event);
}

void handle_event_strob()
{
	LOG_D("cam strob irq\r\n");
}

void handle_event_dp_stat()
{
	LOG_D("dp stat irq\r\n");
}

void handle_event_temp()
{
	uint8_t temp_data[32] = {0};
	float temp = 0;//(float)temp_read(TEMP_ADC) / 1000.0f;
	float_val data;
	data.m_float = temp;
	
	temp_data[EVENT_PAYLOAD_OFS] = 0x00;
	rt_memcpy(temp_data+EVENT_PAYLOAD_OFS+1, data.m_bytes, 4);
	build_event(temp_data, NR_EVENT_TEMP_F, 5);
	mcu_msg_send(temp_data); 
}

void handle_event_env_light()
{
	uint8_t env_data[32] = {0};
	uint16_t env = 0;//cm3232a_read();
	
	env_data[EVENT_PAYLOAD_OFS] = 0x00;
	env_data[EVENT_PAYLOAD_OFS+1] = env & 0xff;
	env_data[EVENT_PAYLOAD_OFS+2] = (env >> 8) & 0xff;
	build_event(env_data, NR_EVENT_ENV_LIGHT, 3);
	mcu_msg_send(env_data); 
}

void handle_event_mag()
{
	uint8_t mag[32] = {0};
	uint8_t mag_data[6];
	float_val data[3];
	/*TODO: store 3 mag data*/
	//while (get_mag_data(mag_data) == 1);
	rt_memset(data[0].m_bytes, 0, 4);
	rt_memset(data[1].m_bytes, 0, 4);
	rt_memset(data[2].m_bytes, 0, 4);
	data[0].m_bytes[0] = mag_data[0];
	data[0].m_bytes[1] = mag_data[1];
	data[1].m_bytes[0] = mag_data[2];
	data[1].m_bytes[1] = mag_data[3];
	data[2].m_bytes[0] = mag_data[4];
	data[2].m_bytes[1] = mag_data[5];
	mag[EVENT_PAYLOAD_OFS] = 0x00;
	read_ts_64(mag+EVENT_PAYLOAD_OFS+1);
	rt_memcpy(mag+EVENT_PAYLOAD_OFS+9,  data[0].m_bytes, 4);
	rt_memcpy(mag+EVENT_PAYLOAD_OFS+13, data[1].m_bytes, 4);
	rt_memcpy(mag+EVENT_PAYLOAD_OFS+17, data[2].m_bytes, 4);
	build_event(mag, DEVICE_EVENT_MAG, 21);
	mcu_msg_send(mag); 
}

void handle_key_vp()
{
	LOG_D("key brightness up\r\n");
	handle_event_bri_key(0x00);
}

void handle_key_vd()
{
	LOG_D("key brightness down\r\n");
	handle_event_bri_key(0x01);
}

void handle_key_lp()
{
	LOG_D("key volume up\r\n");
}

void handle_key_ld()
{
	LOG_D("key volume down\r\n");
}

void handle_timer()
{
	if (g_param.magic)
		handle_event_mag();
	
	if (g_param.env_light)
		handle_event_env_light();

	if (g_param.temp) {
		temp_trigger++;
		if (temp_trigger == 2000) {
			handle_event_temp();
			temp_trigger = 0;
		}
	}

	if (remote_protocol_version >= 2) {
		LOG_D("version 2 event\r\n");
	}
}
