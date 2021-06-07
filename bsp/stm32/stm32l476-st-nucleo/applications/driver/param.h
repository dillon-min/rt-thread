#ifndef _PARAM_H
#define _PARAM_H
#include <rtthread.h>
#define PARAM_PSENSOR_NEAR_THRO 0x14
#define PARAM_PSENSOR_FAR_THRO 	(PARAM_PSENSOR_NEAR_THRO + 2)
#define PARAM_LIGHT_120_DEGRE	(PARAM_PSENSOR_FAR_THRO + 2)
#define PARAM_ORBIT1_H		(PARAM_LIGHT_120_DEGRE + 2)
#define PARAM_ORBIT1_V		(PARAM_ORBIT1_H + 1)
#define PARAM_ORBIT2_H		(PARAM_ORBIT1_V + 1)
#define PARAM_ORBIT2_V		(PARAM_ORBIT2_H + 1)
#define PARAM_ORBIT		(PARAM_ORBIT2_V + 1)
#define PARAM_MAGIC		(PARAM_ORBIT + 1)
#define PARAM_GLASSES_ID	(PARAM_MAGIC + 1)
#define PARAM_DP_OTA_RESULT	(PARAM_GLASSES_ID + 13)
#define PARAM_SW_KEY		(PARAM_DP_OTA_RESULT + 1)
#define PARAM_SLEEP_TIME	(PARAM_SW_KEY + 1)
#define PARAM_POWER		(PARAM_SLEEP_TIME + 2)
#define PARAM_MIC		(PARAM_POWER + 1)
#define PARAM_SPK_GAIN		(PARAM_MIC + 1)
#define PARAM_COLOR		(PARAM_SPK_GAIN + 1)
#define PARAM_DUTY		(PARAM_COLOR + 3)
#define PARAM_SW2D		(PARAM_DUTY + 1)
#define PARAM_VSYNC		(PARAM_SW2D + 1)
#define PARAM_TEMP		(PARAM_VSYNC + 1)
#define PARAM_BRIGHTNESS	(PARAM_TEMP + 1)
#define PARAM_DP_MODE		(PARAM_BRIGHTNESS + 1)
#define PARAM_DP_VERSION	(PARAM_DP_MODE + 1)
#define PARAM_OTP		(PARAM_DP_VERSION + 32)
#define PARAM_OLED_1_CAL	(PARAM_OTP + 1)
#define PARAM_OLED_2_CAL	(PARAM_OLED_1_CAL + 16)
#define PARAM_HOST_ID		(PARAM_OLED_2_CAL + 16)
#define PARAM_OPERATORS		(PARAM_HOST_ID + 32)
#define PARAM_LIGHT_COMPONSATE	(PARAM_OPERATORS + 32)
#define PARAM_ERR_NUM		(PARAM_LIGHT_COMPONSATE + 2)
#define PARAM_SN		(PARAM_ERR_NUM + 2)
#define PARAM_TIMES		(PARAM_SN + 32)
#define PARAM_FACTORY		(PARAM_TIMES + 2)
#define PARAM_SLAM_FACTORY	(PARAM_FACTORY + 64)
#define PARAM_ENV_LIGHT		(PARAM_SLAM_FACTORY + 64)
#define PARAM_RGB_LED		(PARAM_ENV_LIGHT + 1) 
#define PARAM_BRIGHTNESS_MAX	(PARAM_RGB_LED + 1)
#define PARAM_MACHINE_ID	(PARAM_BRIGHTNESS_MAX + 1)
#define PARAM_XXXX		(PARAM_MACHINE_ID + 15)
typedef struct _glasses_param {
	uint8_t psensor_near[2]; /*psensor near thresold */
	uint8_t psensor_far[2]; /* psensor far thresold */
	uint8_t light[2]; /* brightness 120 degre */
	uint8_t orbit1_h;
	uint8_t orbit1_v;
	uint8_t orbit2_h;
	uint8_t orbit2_v;
	uint8_t orbit; /* factory calibration switcher, 0 off , 1 on */
	uint8_t magic; /* magic switcher */
	uint8_t glasses_id[13]; /* glasses id 12 bytes */
	uint8_t dp_ota_result; /* dp ota result */
	uint8_t sw_key; /* use brightness key to switch 2d */
	uint8_t sleep_time[2];  /* time to sleep from power on */
	uint8_t power; /* power safe switcher */
	uint8_t mic; /* mic switcher */
	uint8_t spk_gain; /* speaker gain value */
	uint8_t color[3]; /* color rgb */
	uint8_t duty; /* oled duty */
	uint8_t sw2d; /* 1 always 2d startup, 0 last 2d/3d state */
	uint8_t vsync; /* vsync switcher */
	uint8_t temp; /* temperature switcher */
	uint8_t brightness; /* oled brightness */
	uint8_t dp_mode; /* 1 2d_1080, 2 3d_540, 3 3d_1080 */
	uint8_t dp_version[32]; /* dp version */
	uint8_t otp; /* otp ? */
	uint8_t oled1_cal[16]; /* oled 1 calibration data */
	uint8_t oled2_cal[16]; /* oled 2 calibration data */
	uint8_t host_id[32]; /* host id */
	uint8_t operators[32]; /* operater id */
	uint8_t light_componsate[2]; /* factory componsate test */
	uint8_t err_num[2]; /* test failed count */
	uint8_t sn[32]; /* lgu sn */
	uint8_t times[2]; /* glasses startup count */
	uint8_t factory[64]; /* factory information */
	uint8_t slam_factory[64]; /* slam factory information */
	uint8_t env_light;
} *pglasses_param, glasses_param;
extern glasses_param g_param;
void param_get(uint32_t ofs, uint8_t *buf, uint32_t len);
void param_set_once(uint32_t ofs, uint8_t *buf, uint32_t len);
void param_set(uint32_t ofs, uint8_t *buf, uint32_t len);
rt_bool_t param_init();
#endif
