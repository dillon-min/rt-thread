#include <rtthread.h>
#include <stdio.h>
#include <board.h>
#include "mcu.h"
#include "mcu_cmd.h"
#include "param.h"

glasses_param g_param;
void set_g_param(uint16_t ofs, uint8_t param1, uint8_t param2)
{
	switch (ofs) {
		case PARAM_ORBIT:
			g_param.orbit = param1;
			break; 
		case PARAM_MAGIC:
			g_param.magic = param1;
			break; 
		case PARAM_SW_KEY:
			g_param.sw_key = param1;
			break; 
		case PARAM_POWER:
			g_param.power = param1;
			break; 
		case PARAM_MIC:
			g_param.mic = param1;
			break; 
		case PARAM_SW2D:
			g_param.sw2d = param1;
			break; 
		case PARAM_VSYNC:
			g_param.vsync = param1;
			//if (param1 == 0x01)
			//	cx3_enable_vsync();
			//else
			//	cx3_disable_vsync();
			break; 
		case PARAM_TEMP:
			g_param.temp = param1;
			break; 
		case PARAM_ENV_LIGHT:
			g_param.env_light = param1;
			break; 
		case PARAM_SLEEP_TIME:
			g_param.sleep_time[0] = param1;
			g_param.sleep_time[1] = param2;
			break; 
	}
}
void get_g_param()
{
	param_get(PARAM_ORBIT, &g_param.orbit, 1);
	param_get(PARAM_MAGIC, &g_param.magic, 1);
	param_get(PARAM_SW_KEY, &g_param.sw_key, 1);
	param_get(PARAM_POWER, &g_param.power, 1);
	param_get(PARAM_MIC, &g_param.mic, 1);
	param_get(PARAM_SW2D, &g_param.sw2d, 1);
	param_get(PARAM_VSYNC, &g_param.vsync, 1);
	param_get(PARAM_TEMP, &g_param.temp, 1);
	param_get(PARAM_ENV_LIGHT, &g_param.env_light, 1);
	param_get(PARAM_SLEEP_TIME, (uint8_t *)&g_param.sleep_time, 2);
}
#define PAYLOAD_LEN_BASE 3	//err+payload
static uint16_t handle_on_off(uint16_t ofs, uint8_t *in, uint16_t in_len,
		uint8_t *out)
{
	static uint32_t on[12] = {0};
	uint8_t state;

	out[0] = MCU_ERR_SUCCESS;

	if (in != NULL) {
		int_val data;
		rt_memcpy(data.m_bytes, in, sizeof(uint32_t));
		uint32_t host_data = data.m_int;

		if (host_data == 0 || host_data == 1) {
			state = (host_data == 0) ? 0x00 : 0x01;
			param_set(ofs, &state, 1);
			set_g_param(ofs, state, 0);
		} else
			out[0] = MCU_ERR_INV_ARG; 
	} else {
		int_val data;
		param_get(ofs, &state, 1);
		data.m_int = state;
		rt_memcpy(out+3, data.m_bytes, sizeof(uint32_t));
		return PAYLOAD_LEN_BASE + sizeof(uint32_t);
	}

	return PAYLOAD_LEN_BASE;
}

static uint16_t _handle_uint32_t_data(uint32_t *data, uint32_t min, uint32_t max,
		uint8_t *in, uint16_t in_len, uint8_t *out)
{
	out[0] = MCU_ERR_SUCCESS;

	if (in != NULL) {
		int_val data_val;
		rt_memcpy(data_val.m_bytes, in, sizeof(uint32_t));
		uint32_t host_data = data_val.m_int;

		if ((host_data > min && host_data < max && max > min) ||
			(host_data > min && max < min))
			*data = host_data;
		else
			out[0] = MCU_ERR_INV_ARG; 
	} else {
		int_val data_val;
		data_val.m_int = *data,
		rt_memcpy(out+3, data_val.m_bytes, sizeof(uint32_t));
		return PAYLOAD_LEN_BASE + sizeof(uint32_t);
	}

	return PAYLOAD_LEN_BASE;
}

static uint16_t _handle_info(uint8_t *info, uint16_t max, uint8_t *in,
		uint16_t in_len, uint8_t *out)
{
	out[0] = MCU_ERR_SUCCESS;

	if (in != NULL && in_len > 0) {
		rt_memset(info, 0, 24);
		rt_memcpy(info, in, (in_len >= max) ? max : in_len);		
	} else if (out != NULL) {
		rt_memcpy(out+3, info, rt_strlen(info));
		return PAYLOAD_LEN_BASE+rt_strlen(info);
	}

	return PAYLOAD_LEN_BASE;
}

static uint16_t _handle_get_temp(float temp, uint8_t *in, uint16_t in_len,
		uint8_t *out)
{
	float_val data;

	data.m_float = temp;

	out[0] = MCU_ERR_SUCCESS;

	if (in != NULL && in_len > 0) {
		out[0] = MCU_ERR_INV_ARG;
	} else if (out != NULL) {
		rt_memcpy(out+3, data.m_bytes, sizeof(float));
		return PAYLOAD_LEN_BASE+sizeof(float);
	}

	return PAYLOAD_LEN_BASE;
}

static uint16_t handle_just_set(uint8_t *set, uint8_t *in, uint16_t in_len,
		uint8_t *out)
{
	out[0] = MCU_ERR_SUCCESS;

	if (in != NULL) {
		*set = 1;
	} else {
		out[0] = MCU_ERR_INV_ARG; 
	}

	return PAYLOAD_LEN_BASE;
}


uint16_t handle_brightness(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 0 - 7 */
{
	static uint32_t bri = 0;
	uint16_t result;
	
	if (in == NULL)
		param_get(PARAM_BRIGHTNESS, (uint8_t *)&bri, 1);
	
	result = _handle_uint32_t_data(&bri, 0, 8, in, in_len, out);

	if (in != NULL && bri > 0 && bri < 8) {
		//oled_set_backlight(bri & 0xff);
		param_set(PARAM_BRIGHTNESS, (uint8_t *)&bri, 1);
	}

	return result;
}

uint16_t handle_dp_mode(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 1:2d_1080, 2:3d_540, 3:3d_1080 */
{
	static uint32_t mode = 0;
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_DP_MODE, (uint8_t *)&mode, 1);

	result = _handle_uint32_t_data(&mode, 0, 4, in, in_len, out);

	if (in != NULL && mode > 0 && mode < 4) {
		//lt7211b_set_mode(mode);
		param_set(PARAM_DP_MODE, (uint8_t *)&mode, 1);
	}

	return result;
}

uint16_t handle_dp_version(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 7211 version */
{
	uint8_t dp_ver[33] = {0x00};
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_DP_VERSION, dp_ver, 32);
	
	result = _handle_info(dp_ver, 32, in, in_len, out);

	if (in != NULL)
		param_set(PARAM_DP_VERSION, dp_ver, 32);

	return result;
}

uint16_t handle_host_id(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t host_id[33] = {0x00};
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_HOST_ID, host_id, 32);
	
	result = _handle_info(host_id, 32, in, in_len, out);
	
	if (in != NULL)
		param_set(PARAM_HOST_ID, host_id, 32);

	return result;
}

uint16_t handle_machine_id(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t machine_id[24] = {0x00};
	uint16_t result;
	
	if (in == NULL)
		param_get(PARAM_MACHINE_ID, machine_id, 14);
	
	result = _handle_info(machine_id, 14, in, in_len, out);
	
	if ( in != NULL)
		param_set(PARAM_MACHINE_ID, machine_id, 14);

	return result;
}

uint16_t handle_glasses_id(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t glasses_id[24] = {0x00};
	uint16_t result;
	
	if (in == NULL)
		param_get(PARAM_GLASSES_ID, glasses_id, 13);
	
	result = _handle_info(glasses_id, 13, in, in_len, out);
	
	if ( in != NULL)
		param_set(PARAM_GLASSES_ID, glasses_id, 13);

	return result;
}

uint16_t handle_psensor_near(uint8_t *in, uint16_t in_len, uint8_t *out)
/* psensor near value */
{
	static uint32_t near = 0;
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_PSENSOR_NEAR_THRO, (uint8_t *)&near, 2);

	result = _handle_uint32_t_data(&near, 79, 0, in, in_len, out);

	if (in != NULL && near > 79) {
		//set_psensor_near(near);
		param_set(PARAM_PSENSOR_NEAR_THRO, (uint8_t *)&near, 2);
	}

	return result;
}

uint16_t handle_psensor_far(uint8_t *in, uint16_t in_len, uint8_t *out)
/* psensor far value */
{
	static uint32_t far = 0;
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_PSENSOR_FAR_THRO, (uint8_t *)&far, 2);

	result = _handle_uint32_t_data(&far, 29, 0, in, in_len, out);

	if (in != NULL && far > 29) {
		//set_psensor_far(far);
		param_set(PARAM_PSENSOR_FAR_THRO, (uint8_t *)&far, 2);
	}
	return result;
}

uint16_t handle_get_temp(uint8_t *in, uint16_t in_len, uint8_t *out)
/* temperature */
{
	float temp = 0;//(float)temp_read(TEMP_ADC) / 1000.0f;
	return _handle_get_temp(temp, in, in_len, out);
}

uint16_t handle_oled_duty(uint8_t *in, uint16_t in_len, uint8_t *out)
/* oled duty */
{
	static uint32_t duty = 0;
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_DUTY, (uint8_t *)&duty, 1);

	result = _handle_uint32_t_data(&duty, 0, 101, in, in_len, out);

	if (in != NULL && duty > 0 && duty < 101) {
		//oled_set_duty(duty & 0xff);	
		param_set(PARAM_DUTY, (uint8_t *)&duty, 1);
	}

	return result;
}

uint16_t handle_sleep_mode(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 0:close, 1:open */
{
	return handle_on_off(PARAM_POWER, in, in_len, out);
}

uint16_t handle_magnetic(uint8_t *in, uint16_t in_len, uint8_t *out)	
{
	return handle_on_off(PARAM_MAGIC, in, in_len, out);
}

uint16_t handle_vsync(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	return handle_on_off(PARAM_VSYNC, in, in_len, out);
}

uint16_t handle_env_light(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	return handle_on_off(PARAM_ENV_LIGHT, in, in_len, out);
}

uint16_t handle_rgb_led(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	return handle_on_off(PARAM_RGB_LED, in, in_len, out);
}

uint16_t handle_sleep_time(uint8_t *in, uint16_t in_len, uint8_t *out)
/* time before glasses enter into sleep mode */
{
	static uint32_t time = 0;
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_SLEEP_TIME, (uint8_t *)&time, 2);
	
	result = _handle_uint32_t_data(&time, 20, 0, in, in_len, out);

	if (in != NULL && time > 20) {
		param_set(PARAM_SLEEP_TIME, (uint8_t *)&time, 2);
		set_g_param(PARAM_SLEEP_TIME, (time >> 8) & 0xff,
				time & 0xff);
	}

	return result;
}

uint16_t handle_rgb_reset(uint8_t *in, uint16_t in_len, uint8_t *out)
/* rgb reset */
{
	uint8_t reset;
	//rgb_power_ctl(CyFalse);
	//CyU3PThreadSleep(100);
	//rgb_power_ctl(CyTrue);
	//CyU3PThreadSleep(100);

	return handle_just_set(&reset, in, in_len, out);
}

uint16_t handle_dp_new_version(uint8_t *in, uint16_t in_len, uint8_t *out)
/* dp ota, new version */
{
	uint8_t update_dp_version;
	return handle_just_set(&update_dp_version, in, in_len, out);
}

uint16_t handle_dp_ota(uint8_t *in, uint16_t in_len, uint8_t *out)
/* dp ota */
{
	out[0] = MCU_ERR_SUCCESS;
	return PAYLOAD_LEN_BASE;
}

static void reboot_handler(void *param)
{
	rt_thread_mdelay(1000);
	rt_hw_interrupt_disable();
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}
uint16_t handle_reset(uint8_t *in, uint16_t in_len, uint8_t *out)
/* reset mcu */
{
	#define TYPE_WARM_BOOT 0x12345678
	#define TYPE_COLD_BOOT 0x00000000
	uint8_t reset;
	uint32_t tmp = 0U;
	int_val data;
	rt_memcpy(data.m_bytes, in, sizeof(uint32_t));
	tmp = (uint32_t)0x40002850;
	//tmp += (BackupRegister * 4U);
	rt_kprintf("reboot flag %d, bkp %x\r\n", data.m_int, *(__IO uint32_t *)tmp);
	HAL_PWR_EnableBkUpAccess();
	if (data.m_int == 1)
		*(__IO uint32_t *)tmp = (uint32_t)TYPE_WARM_BOOT;
	else
		*(__IO uint32_t *)tmp = (uint32_t)TYPE_COLD_BOOT;
	HAL_PWR_DisableBkUpAccess();
	rt_kprintf("bkp %x\r\n", *(__IO uint32_t *)tmp);
	rt_thread_t tid = rt_thread_create("reboot", reboot_handler, RT_NULL,
			1024, 28, 20);
	rt_thread_startup(tid);
	return handle_just_set(&reset, in, in_len, out);
}

uint16_t handle_brightness_v2(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 0 - 120 */
{
	static uint32_t bri = 0;
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_LIGHT_120_DEGRE, (uint8_t *)&bri, 2);

	result = _handle_uint32_t_data(&bri, 0, 120, in, in_len, out);

	if (in != NULL && bri > 0 && bri < 120) {
		//oled_set_backlight_ext(bri);
		param_set(PARAM_LIGHT_120_DEGRE, (uint8_t *)&bri, 2);
	}

	return result;
}

uint16_t handle_temp(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 0:close, 1:open */
{
	return handle_on_off(PARAM_TEMP, in, in_len, out);
}

uint16_t handle_try_oled_power(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 0:close, 1:open, used in heartbreat */
{
	uint8_t try;
	return handle_just_set(&try, in, in_len, out);
}

uint16_t handle_get_temp_v2(uint8_t *in, uint16_t in_len, uint8_t *out)
/* temperature ext */
{
	float temp = 0;//(float)temp_read(TEMP_ADC) / 1000.0f;
	return _handle_get_temp(temp, in, in_len, out);
}

uint16_t handle_brightness_max(uint8_t *in, uint16_t in_len, uint8_t *out)
/* set brightness max */
{
	static uint32_t max = 0;
	uint16_t result;

	if (in == NULL)
		param_get(PARAM_BRIGHTNESS_MAX, (uint8_t *)&max, 1);

	result = _handle_uint32_t_data(&max, 7, 120, in, in_len, out);

	if (in != NULL && max > 7 && max < 120)
		param_set(PARAM_BRIGHTNESS_MAX, (uint8_t *)&max, 1);

	return result;
}

uint16_t handle_speaker_level(uint8_t *in, uint16_t in_len, uint8_t *out)
/* speaker gain value */
{
	static uint32_t gain = 0;
	uint16_t result;
	
	if (in == NULL)
		param_get(PARAM_SPK_GAIN, (uint8_t *)&gain, 1);
	
	result = _handle_uint32_t_data(&gain, 0, 100, in, in_len, out);

	if (in != NULL && gain > 0 && gain < 100)
		param_set(PARAM_SPK_GAIN, (uint8_t *)&gain, 1);

	return result;
}

uint16_t handle_cpu(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t cpu[24] = "Cortex-M4";
	return _handle_info(cpu, 0, in, in_len, out);
}

uint16_t handle_rom(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t rom[24] = "512K bytes";
	return _handle_info(rom, 0, in, in_len, out);
}

uint16_t handle_ram(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t ram[24] = "128K bytes";
	return _handle_info(ram, 0, in, in_len, out);
}

uint16_t handle_left_oled_h_orbit(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t oled_l_h;
	uint16_t result = handle_just_set(&oled_l_h, in, in_len, out);

	param_set(PARAM_ORBIT1_H, &oled_l_h, 1);

	return result;
}

uint16_t handle_left_oled_v_orbit(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t oled_l_v;
	uint16_t result = handle_just_set(&oled_l_v, in, in_len, out);

	param_set(PARAM_ORBIT1_V, &oled_l_v, 1);

	return result;
}

uint16_t handle_right_oled_h_orbit(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t oled_r_h;
	uint16_t result = handle_just_set(&oled_r_h, in, in_len, out);
	
	param_set(PARAM_ORBIT2_H, &oled_r_h, 1);

	return result;
}

uint16_t handle_right_oled_v_orbit(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t oled_r_v;
	uint16_t result = handle_just_set(&oled_r_v, in, in_len, out);
	
	param_set(PARAM_ORBIT2_V, &oled_r_v, 1);
	
	return result;
}

uint16_t handle_orbit_adjust(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 0:close, 1:open */
{
	uint8_t orbit_adjust;
	uint16_t result = handle_just_set(&orbit_adjust, in, in_len, out);
	
	param_set(PARAM_ORBIT, &orbit_adjust, 1);

	return result;
}

uint16_t handle_color(uint8_t *in, uint16_t in_len, uint8_t *out)
/* 1:red, 2:blue, 3:white */
{
	out[0] = MCU_ERR_SUCCESS;

	if (in != NULL) {
		param_set(PARAM_COLOR, in, 3);
	} else {
		param_get(PARAM_COLOR, out+3, 3);
		out[6] = 0;
		return PAYLOAD_LEN_BASE + 4;
	}

	return PAYLOAD_LEN_BASE;
}

uint16_t handle_factory(uint8_t *in, uint16_t in_len, uint8_t *out)
/* use default setting */
{
	uint8_t factory;
	/* set default param to eeprom */
	return handle_just_set(&factory, in, in_len, out);
}

uint16_t handle_bri_test(uint8_t *in, uint16_t in_len, uint8_t *out)
{
	uint8_t bri_test;
	return handle_just_set(&bri_test, in, in_len, out);
}
