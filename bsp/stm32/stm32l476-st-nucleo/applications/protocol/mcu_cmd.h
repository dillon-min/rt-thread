#ifndef __MCU_CMD_H
#define __MCU_CMD_H
#include <stdint.h>
typedef struct _float_val {
	union {
		float m_float;
		uint8_t m_bytes[sizeof(float)];
	};
} float_val;

typedef struct _int_data {
	union {
		uint32_t m_int;
		uint8_t m_bytes[sizeof(uint32_t)];
	};
} int_val;

typedef struct _int64_data {
	union {
		uint64_t m_int64;
		uint8_t m_bytes[sizeof(uint64_t)];
	};
} int64_val;
typedef struct _acc_gyro_data {
	union {
		uint32_t m_uint32;
		int16_t m_int16;
	};
} acc_gyro_val;
uint16_t handle_brightness(uint8_t *in, uint16_t in_len, uint8_t *out);	/* 0 - 7 */
uint16_t handle_dp_mode(uint8_t *in, uint16_t in_len, uint8_t *out);	/* 1:2d_1080, 2:3d_540, 3:3d_1080 */
uint16_t handle_dp_version(uint8_t *in, uint16_t in_len, uint8_t *out);	/* 7211 version */
uint16_t handle_host_id(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_glasses_id(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_psensor_near(uint8_t *in, uint16_t in_len, uint8_t *out);	/* psensor near value */
uint16_t handle_psensor_far(uint8_t *in, uint16_t in_len, uint8_t *out);	/* psensor far value */
uint16_t handle_get_temp(uint8_t *in, uint16_t in_len, uint8_t *out);	/* temperature */
uint16_t handle_oled_duty(uint8_t *in, uint16_t in_len, uint8_t *out);	/* oled duty */
uint16_t handle_sleep_mode(uint8_t *in, uint16_t in_len, uint8_t *out);	/* 0:close, 1:open */
uint16_t handle_magnetic(uint8_t *in, uint16_t in_len, uint8_t *out);	
uint16_t handle_vsync(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_env_light(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_rgb_led(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_sleep_time(uint8_t *in, uint16_t in_len, uint8_t *out);	/* time before glasses enter into sleep mode */
uint16_t handle_dp_new_version(uint8_t *in, uint16_t in_len, uint8_t *out);	/* dp ota, new version */
uint16_t handle_dp_ota(uint8_t *in, uint16_t in_len, uint8_t *out);		/* dp ota */
uint16_t handle_reset(uint8_t *in, uint16_t in_len, uint8_t *out);	/* reset mcu */
uint16_t handle_brightness_v2(uint8_t *in, uint16_t in_len, uint8_t *out);	/* 0 - 120 */
uint16_t handle_temp(uint8_t *in, uint16_t in_len, uint8_t *out);		/* 0:close, 1:open */
uint16_t handle_try_oled_power(uint8_t *in, uint16_t in_len, uint8_t *out);	/* 0:close, 1:open, used in heartbreat */
uint16_t handle_get_temp_v2(uint8_t *in, uint16_t in_len, uint8_t *out);	/* temperature ext */
uint16_t handle_brightness_max(uint8_t *in, uint16_t in_len, uint8_t *out);	/* set brightness max */
uint16_t handle_speaker_level(uint8_t *in, uint16_t in_len, uint8_t *out);	/* speaker gain value */
uint16_t handle_cpu(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_rom(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_ram(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_left_oled_h_orbit(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_left_oled_v_orbit(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_right_oled_h_orbit(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_right_oled_v_orbit(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_orbit_adjust(uint8_t *in, uint16_t in_len, uint8_t *out);	/* 0:close, 1:open */
uint16_t handle_color(uint8_t *in, uint16_t in_len, uint8_t *out);		/* 1:red, 2:blue, 3:white */
uint16_t handle_factory(uint8_t *in, uint16_t in_len, uint8_t *out);	/* use default setting */
uint16_t handle_bri_test(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_machine_id(uint8_t *in, uint16_t in_len, uint8_t *out);
uint16_t handle_rgb_reset(uint8_t *in, uint16_t in_len, uint8_t *out);
#endif
