#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "usb.h"
#include "rc4.h"
#define HID_REPORT_ID 0x01
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

enum {
	/* base 20200825 */
	NR_BRIGHTNESS = 1,
	NR_DISPLAY_2D_3D,
	NR_DISPLAY_VERSION,
	NR_HOST_ID,
	NR_GLASSID,
	NR_PSENSOR_CLOSED,
	NR_PSENSOR_NOCLOSED,
	NR_TEMPERATURE,
	NR_DISPLAY_DUTY,
	NR_POWER_FUCTION,
	NR_MAGNETIC_FUCTION,
	NR_VSYNC_FUCTION,
	NR_ENV_LIGHT,
	NR_WORLD_LED,
	NR_SLEEP_TIME,
	NR_7211_VERSION,
	NR_7211_UPDATE,
	NR_REBOOT,
	NR_BRIGHTNESS_EXT,
	NR_TEMPERATURE_FUNCTION,
	NR_TRY_CTRL_DISPLAY_STATUS,
	NR_TEMPERATURE_EXT,
	NR_BRIGHTNESS_MAX,
	NR_SPEAKER_LEVEL,
	NR_CPU_INFO,
	NR_ROM_INFO,
	NR_RAM_INFO,
	NR_LEFT_OLED_H_ORBIT,
	NR_LEFT_OLED_V_ORBIT,
	NR_RIGHT_OLED_H_ORBIT,
	NR_RIGHT_OLED_V_ORBIT,
	NR_ORBIT_ADJUST,
	NR_COLOR,
	NR_RECOVERY_FACTORY,
	NR_GLASS_BRI_TEST,
	NR_MACHINE_ID,
	NR_RGB_RESET,
	NR_MAX
};
static uint32_t crc32_tab[] =
{
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
};
#define USB_ENDPOINT_OUT               0x00
#define USB_ENDPOINT_IN                0x80

uint32_t crc32(const char* s, int len)
{
    int i;
    uint32_t crc32val = 0;
    crc32val ^= 0xFFFFFFFF;

    for (i = 0;  i < len;  i++) {
        crc32val = crc32_tab[(crc32val ^ s[i]) & 0xFF] ^
        	((crc32val >> 8) & 0x00FFFFFF);
    }

    return crc32val ^ 0xFFFFFFFF;
}

uint64_t get_ts()
{
	struct timespec time_start={0, 0};
	clock_gettime(CLOCK_REALTIME, &time_start);

	return (time_start.tv_sec*1000000 + time_start.tv_nsec);
}

int parse_rsp(const char *cmd, uint16_t msgid, uint16_t mcu_cmd,
		uint8_t *rsp, uint16_t len)
{
	uint8_t ofs;
	if (strcmp(cmd, "heart") == 0 && msgid == 0x2500) {
		ofs = 1;
		int64_val val1, val2, val3;
		memcpy(val1.m_bytes, rsp+ofs, 8);
		memcpy(val2.m_bytes, rsp+ofs+8, 8);
		memcpy(val3.m_bytes, rsp+ofs+16, 8);
		uint64_t t4 = get_ts();
		printf("heart: err %d, t1 %lld, t2 %lld, t3 %lld, t4 %lld\r\n",
				rsp[0], val1.m_int64, val2.m_int64*1000,
				val3.m_int64*1000, t4);
		return 0;
	} else if ((strcmp(cmd, "get_temp") == 0 && mcu_cmd == NR_TEMPERATURE ||
		   strcmp(cmd, "get_temp2") == 0 && mcu_cmd == NR_TEMPERATURE_EXT) &&
		   (msgid == 0x2001 || msgid == 0x2003)) {
		if (rsp[0] == 0)
		{
			float_val data;
			memcpy(data.m_bytes, rsp+3, 4);
			printf("temp get: %foC\r\n", data.m_float);
		} else
			printf("temp get failed\r\n");

		return 0;
	} else if (strcmp(cmd, "sw_ver") == 0 && msgid == 0x0003 || 
		strcmp(cmd, "hw_ver") == 0 && msgid == 0x0002) {
		uint16_t major, minor, patch;
		uint32_t builddate;
		ofs = 1;
		major = rsp[ofs] | (rsp[ofs+1] << 8);
		ofs += 2;
		minor = rsp[ofs] | (rsp[ofs+1] << 8);
		ofs += 2;
		patch = rsp[ofs] | (rsp[ofs+1] << 8);
		ofs += 2;
		builddate = (rsp[ofs] << 0) | (rsp[ofs+1] << 8) |
				(rsp[ofs+2] << 16) | (rsp[ofs+3] << 24);
		if (len == 11)
			printf("%s: err %d, major %d, minor %d, patch %d, "
				"build_date %d\r\n",
				cmd, rsp[0], major, minor, patch, builddate);
		else
			printf("%s: err %d, major %d, minor %d, patch %d, "
				"build_date %d, %c%c%c%c\r\n",
				cmd, rsp[0], major, minor, patch, builddate,
				rsp[ofs], rsp[ofs+1], rsp[ofs+2], rsp[ofs+3]);

		return 0;
	} else if ((strcmp(cmd, "brightness") == 0 && mcu_cmd == NR_BRIGHTNESS||
		   strcmp(cmd, "dp_mode") == 0 && mcu_cmd == NR_DISPLAY_2D_3D||
		   strcmp(cmd, "psensor_far") == 0 && mcu_cmd == NR_PSENSOR_NOCLOSED||
		   strcmp(cmd, "psensor_near") == 0 && mcu_cmd == NR_PSENSOR_CLOSED||
		   strcmp(cmd, "sleep_mode") == 0 && mcu_cmd == NR_POWER_FUCTION||
		   strcmp(cmd, "magnetic") == 0 && mcu_cmd == NR_MAGNETIC_FUCTION||
		   strcmp(cmd, "vsync") == 0 && mcu_cmd == NR_VSYNC_FUCTION||
		   strcmp(cmd, "env_light") == 0 && mcu_cmd == NR_ENV_LIGHT||
		   strcmp(cmd, "rgb_led") == 0 && mcu_cmd == NR_WORLD_LED||
		   strcmp(cmd, "sleep_time") == 0 && mcu_cmd == NR_SLEEP_TIME||
		   strcmp(cmd, "dp_ota") == 0 && mcu_cmd == NR_7211_UPDATE||
		   strcmp(cmd, "reset") == 0 && mcu_cmd == NR_REBOOT||
		   strcmp(cmd, "rgb_rst") == 0 && mcu_cmd == NR_RGB_RESET||
		   strcmp(cmd, "brightness_ext") == 0 && mcu_cmd == NR_BRIGHTNESS_EXT||
		   strcmp(cmd, "temp") == 0 && mcu_cmd == NR_TEMPERATURE_FUNCTION||
		   strcmp(cmd, "spk_gain") == 0 && mcu_cmd == NR_SPEAKER_LEVEL||
		   strcmp(cmd, "oled") == 0 && mcu_cmd == NR_TRY_CTRL_DISPLAY_STATUS||
		   strcmp(cmd, "oled_l_h") == 0 && mcu_cmd == NR_LEFT_OLED_H_ORBIT||
		   strcmp(cmd, "oled_l_v") == 0 && mcu_cmd == NR_LEFT_OLED_V_ORBIT||
		   strcmp(cmd, "oled_r_h") == 0 && mcu_cmd == NR_RIGHT_OLED_H_ORBIT||
		   strcmp(cmd, "oled_r_v") == 0 && mcu_cmd == NR_RIGHT_OLED_V_ORBIT||
		   strcmp(cmd, "oled_adjust") == 0 && mcu_cmd == NR_ORBIT_ADJUST||
		   strcmp(cmd, "color") == 0 && mcu_cmd == NR_COLOR||
		   strcmp(cmd, "factory") == 0 && mcu_cmd == NR_RECOVERY_FACTORY||
		   strcmp(cmd, "bri_test") == 0 && mcu_cmd == NR_GLASS_BRI_TEST||
		   strcmp(cmd, "oled_duty") == 0 && mcu_cmd == NR_DISPLAY_DUTY||
		   strcmp(cmd, "brightness_max") == 0 && mcu_cmd == NR_BRIGHTNESS_MAX) &&
		   (msgid == 0x2001 || msgid == 0x2003)) {
			if (len == 3) {
				printf("%s set: %s\r\n", cmd,
					(rsp[0] == 0) ? "success":"failed");
			} else if (len == 7) {
				int_val data;
				memcpy(data.m_bytes, rsp+3, 4);
				printf("%s get: %d\r\n", cmd, data.m_int);
			}
			return 0;
	} else if ((strcmp(cmd, "host_id") == 0 && mcu_cmd == NR_HOST_ID||
		   strcmp(cmd, "dp_ver") == 0 && mcu_cmd == NR_DISPLAY_VERSION||
		   strcmp(cmd, "cpu") == 0 && mcu_cmd == NR_CPU_INFO||
		   strcmp(cmd, "ram") == 0 && mcu_cmd == NR_RAM_INFO||
		   strcmp(cmd, "rom") == 0 && mcu_cmd == NR_ROM_INFO||
		   strcmp(cmd, "glasses_id") == 0 && mcu_cmd == NR_GLASSID||
		   strcmp(cmd, "sn") == 0 && mcu_cmd == NR_MACHINE_ID||
		   strcmp(cmd, "dp_new_ver") == 0&& mcu_cmd == NR_7211_VERSION) &&
		   (msgid == 0x2001 || msgid == 0x2003)) {
		if (len == 3) {
			printf("%s set: %s\r\n", cmd,
					(rsp[0] == 0) ? "success":"failed");
		} else {
			rsp[len] = 0;
			printf("%s get: %s\r\n", cmd, rsp+3);
		}
		return 0;
	} else {
		printf("rsp unsupported cmd %s\r\n", cmd);
	}

	return -1;
}
int main(int argc, void *argv[])
{
	int i;
	uint16_t msg_id = 0;
	uint16_t cmd_id = 0;
	uint16_t payload_len = 0;
	uint8_t payload[512] = {0};
	int rsp_len;
	void *dev = NULL;
	uint8_t type;
	uint64_t ts; 
	uint8_t rsp[512] = {0};
	uint8_t cmd[512] = {0};
	uint8_t cmd_ori[512] = {0};
	uint8_t debug = 0;
	uint8_t cnt = 0;
	uint8_t glasses_v = 0;
	uint8_t app_v = 1;
	
	for (i=0; i<argc; i++)
	{
		if (strstr(argv[i], "-v") != NULL)
			debug = 1;
	}

	if (argc < 2) {
		printf("uasge: ./mcu_cmd {opt} [-v]\r\n"
			" -v: verbose log\r\n"
			"opt: \r\n"
			"\tsw_ver:\t\tsoftware version\r\n"
			"\thw_ver:\t\thardware version\r\n"
			"\theart:\t\theartbeat\r\n"
			"\tbrightness:\tset/get brightness 0 - 7\r\n"
			"\tbrightness_max:\tset/get brightness max\r\n"
			"\thost_id:\tset/get hosid < 24\r\n"
			"\toled_duty:\tset/get oled duty\r\n"
			"\tdp_mode:\tset/get dp mode, 1:2d_1080, 2:3d_540, "
			"3:3d_1080\r\n"
			"\tdp_ver:\t\tget/set dp version\r\n"
			"\tglasses_id:\tset/get glasses id\r\n"
			"\tpsensor_near:\tset/get psensor near threshold\r\n"
			"\tpsensor_far:\tset/get psensor far threshold\r\n"
			"\tget_temp:\tget glasses temperature\r\n"
			"\tget_temp2:\tget glasses temperature2\r\n"
			"\tsleep_mode:\tset/get sleep mode, 0 close, 1 open\r\n"
			"\tmagnetic:\tset/get magnetic, 0 close, 1 open\r\n"
			"\tvsync:\t\tset/get vsync, 0 close, 1 open\r\n"
			"\tenv_light:\tset/get env light, 0 close, 1 open\r\n"
			"\trgb_led:\tset/get rgb camera flash led, 0 close, "
			"1 open\r\n"
			"\tsleep_time:\tset/get sleep time, must > 20s\r\n"
			"\tdp_new_ver:\tset/get dp new version, used in dp "
			"ota\r\n"
			"\tdp_ota:\t\tstart/end dp ota\r\n"
			"\treset:\t\tmcu reset\r\n"
			"\tbrightness_ext:\tset/get brightness 0 - 120\r\n"
			"\ttemp:\t\tset/get temperature auto event, 0 close, "
			"1 open\r\n"
			"\tspk_gain:\tset/get speaker gain\r\n"
			"\tcpu:\t\tget cpu info\r\n"
			"\tram:\t\tget ram info\r\n"
			"\trom:\t\tget rom info\r\n"
			"\toled_l_h:\tadjust oled left h orbit\r\n"
			"\toled_l_v:\tadjust oled left v orbit\r\n"
			"\toled_r_h:\tadjust oled right h orbit\r\n"
			"\toled_r_v:\tadjust oled right v orbit\r\n"
			"\toled_adjust:\tadjust oled orbit, 0 close, 1 open\r\n"
			"\tfactory:\tset cx3 to default paramters\r\n"
			"\tbri_test:\tset oled brightness loop test\r\n"
			"\tcolor:\t\tset/get color value r g b\r\n"
			"\tsn:\t\tset/get sn\r\n"
			"\trgb_rst:\t\treset rgb\r\n"

		);
		return 0;
	}

	msg_id = 0x2000;
	payload_len = 3;
	
	if (strcmp(argv[1], "heart") == 0) {
		msg_id = 0x2500;
		payload_len = 9;
	} else if (strcmp(argv[1], "sw_ver") == 0) {
		msg_id = 0x0003;
		payload_len = 0;
	} else if (strcmp(argv[1], "hw_ver") == 0) {
		msg_id = 0x0002;
		payload_len = 0;
	} else if (strcmp(argv[1], "dp_ver") == 0) {
		cmd_id = NR_DISPLAY_VERSION;
	} else if (strcmp(argv[1], "dp_mode") == 0) {
		cmd_id = NR_DISPLAY_2D_3D;	
	} else if (strcmp(argv[1], "get_temp") == 0) {
		cmd_id = NR_TEMPERATURE;
	} else if (strcmp(argv[1], "get_temp2") == 0) {
		cmd_id = NR_TEMPERATURE_EXT;
	} else if (strcmp(argv[1], "sn") == 0) {
		cmd_id = NR_MACHINE_ID;
	} else if (strcmp(argv[1], "glasses_id") == 0) {
		cmd_id = NR_GLASSID;
	} else if (strcmp(argv[1], "oled_duty") == 0) {
		cmd_id = NR_DISPLAY_DUTY;	
	} else if (strcmp(argv[1], "brightness_max") == 0) {
		cmd_id = NR_BRIGHTNESS_MAX;	
	} else if (strcmp(argv[1], "brightness") == 0) {
		cmd_id = NR_BRIGHTNESS;	
	} else if (strcmp(argv[1], "psensor_near") == 0) {
		cmd_id = NR_PSENSOR_CLOSED;	
	} else if (strcmp(argv[1], "psensor_far") == 0) {
		cmd_id = NR_PSENSOR_NOCLOSED;	
	} else if (strcmp(argv[1], "host_id") == 0) {
		cmd_id = NR_HOST_ID;	
	} else if (strcmp(argv[1], "sleep_mode") == 0) {
		cmd_id = NR_POWER_FUCTION;	
	} else if (strcmp(argv[1], "magnetic") == 0) {
		cmd_id = NR_MAGNETIC_FUCTION;	
	} else if (strcmp(argv[1], "vsync") == 0) {
		cmd_id = NR_VSYNC_FUCTION;	
	} else if (strcmp(argv[1], "env_light") == 0) {
		cmd_id = NR_ENV_LIGHT;	
	} else if (strcmp(argv[1], "rgb_led") == 0) {
		cmd_id = NR_WORLD_LED;	
	} else if (strcmp(argv[1], "spk_gain") == 0) {
		cmd_id = NR_SPEAKER_LEVEL;	
	} else if (strcmp(argv[1], "sleep_time") == 0) {
		cmd_id = NR_SLEEP_TIME;	
	} else if (strcmp(argv[1], "dp_new_ver") == 0) {
		cmd_id = NR_7211_VERSION;	
	} else if (strcmp(argv[1], "dp_ota") == 0) {
		cmd_id = NR_7211_UPDATE;	
	} else if (strcmp(argv[1], "rgb_rst") == 0) {
		cmd_id = NR_RGB_RESET;	
	} else if (strcmp(argv[1], "reset") == 0) {
		cmd_id = NR_REBOOT;	
	} else if (strcmp(argv[1], "factory") == 0) {
		cmd_id = NR_RECOVERY_FACTORY;	
	} else if (strcmp(argv[1], "bri_test") == 0) {
		cmd_id = NR_GLASS_BRI_TEST;	
	} else if (strcmp(argv[1], "oled") == 0) {
		cmd_id = NR_TRY_CTRL_DISPLAY_STATUS;	
	} else if (strcmp(argv[1], "cpu") == 0) {
		cmd_id = NR_CPU_INFO;	
	} else if (strcmp(argv[1], "ram") == 0) {
		cmd_id = NR_RAM_INFO;	
	} else if (strcmp(argv[1], "rom") == 0) {
		cmd_id = NR_ROM_INFO;	
	} else if (strcmp(argv[1], "oled_l_h") == 0) {
		cmd_id = NR_LEFT_OLED_H_ORBIT;	
	} else if (strcmp(argv[1], "oled_r_h") == 0) {
		cmd_id = NR_RIGHT_OLED_H_ORBIT;	
	} else if (strcmp(argv[1], "oled_l_v") == 0) {
		cmd_id = NR_LEFT_OLED_V_ORBIT;	
	} else if (strcmp(argv[1], "oled_r_v") == 0) {
		cmd_id = NR_RIGHT_OLED_V_ORBIT;	
	} else if (strcmp(argv[1], "oled_adjust") == 0) {
		cmd_id = NR_ORBIT_ADJUST;	
	} else if (strcmp(argv[1], "color") == 0) {
		cmd_id = NR_COLOR;	
	} else if (strcmp(argv[1], "brightness_ext") == 0) {
		cmd_id = NR_BRIGHTNESS_EXT;	
	} else if (strcmp(argv[1], "temp") == 0) {
		cmd_id = NR_TEMPERATURE_FUNCTION;	
	} else {
		printf("not supported \r\n");
		return 0;
	}
	if (msg_id == 0x2500) {
		int64_val tss;
		tss.m_int64 = get_ts();
		payload[0] = 0;
		memcpy(payload + 1, tss.m_bytes, 8);
	}
	if (cmd_id != 0) {
		payload[0] = 0;
		payload[1] = (cmd_id >> 0) & 0xff;
		payload[2] = (cmd_id >> 8) & 0xff;
		if (argc >= 3 && debug == 0 || argc >= 4 && debug == 1) {
			msg_id = 0x2002;
			if (cmd_id == NR_DISPLAY_2D_3D ||
			    cmd_id == NR_BRIGHTNESS ||
			    cmd_id == NR_BRIGHTNESS_MAX ||
			    cmd_id == NR_TEMPERATURE_FUNCTION ||
			    cmd_id == NR_BRIGHTNESS_EXT ||
			    cmd_id == NR_PSENSOR_CLOSED ||
			    cmd_id == NR_PSENSOR_NOCLOSED ||
			    cmd_id == NR_DISPLAY_DUTY ||
			    cmd_id == NR_POWER_FUCTION ||
			    cmd_id == NR_MAGNETIC_FUCTION ||
			    cmd_id == NR_VSYNC_FUCTION ||
			    cmd_id == NR_REBOOT ||
			    cmd_id == NR_ENV_LIGHT ||
			    cmd_id == NR_WORLD_LED ||
			    cmd_id == NR_7211_UPDATE ||
			    cmd_id == NR_SPEAKER_LEVEL ||
			    cmd_id == NR_RIGHT_OLED_V_ORBIT ||
			    cmd_id == NR_LEFT_OLED_V_ORBIT ||
			    cmd_id == NR_RIGHT_OLED_H_ORBIT ||
			    cmd_id == NR_LEFT_OLED_H_ORBIT ||
			    cmd_id == NR_TRY_CTRL_DISPLAY_STATUS ||
			    cmd_id == NR_SLEEP_TIME ||
			    cmd_id == NR_COLOR
			    ) {
				payload_len = 7;
				int_val data;
				data.m_int = atoi(argv[2]);
				memcpy(payload+3, data.m_bytes, 4);
			}
			
			if (cmd_id == NR_HOST_ID ||
				cmd_id == NR_DISPLAY_VERSION ||
				cmd_id == NR_GLASSID ||
				cmd_id == NR_MACHINE_ID) {
				payload_len = 3+strlen(argv[2]);
				memcpy(payload+3, argv[2], strlen(argv[2]));
			}
		}
	}
	
	dev = open_usb(INTF_MCU);
	if (dev == NULL) {
		printf("can not open usb device\r\n");
		return 0;
	}
	int64_val data;
	data.m_int64 = get_ts();
	
	cmd[0] = HID_REPORT_ID;
	cmd[1] = 0xfd;
	memcpy(cmd+2, data.m_bytes, 8);
	cmd[10] = (msg_id >> 0) & 0xff;
	cmd[11] = (msg_id >> 8) & 0xff;
	cmd[12] = 0x00;
	cmd[13] = 0x00;
	cmd[14] = 0x00;
	cmd[15] = 0x00;
	cmd[16] = (payload_len >> 0) & 0xff;
	cmd[17] = (payload_len >> 8) & 0xff;
	if (payload_len != 0) {
		memcpy(cmd+18, payload, payload_len);
	}

	if (debug) {
		printf("[%d]==>\r\n", payload_len+22);
		for (i=0; i<payload_len+22; i++) {
			if (i%16 == 0 && i != 0)
				printf("\r\n");
			printf("%02x ", cmd[i]);

		}
		printf("\r\n");
	}

	set_rc4_key(cmd[8]%10, msg_id, cmd+2);
	rc4(cmd+12, cmd+12, 2+4+payload_len);
	uint32_t crc = crc32((const char *)cmd+1, payload_len+17);
	cmd[payload_len+18]   = (crc >> 0) & 0xff;
	cmd[payload_len+19] = (crc >> 8) & 0xff;
	cmd[payload_len+20] = (crc >> 16) & 0xff;
	cmd[payload_len+21] = (crc >> 24) & 0xff;

	if (debug) {
		printf("enc[%d]==>\r\n", payload_len+21);
		for (i=0; i<payload_len+21; i++) {
			if (i%16 == 0 && i != 0)
				printf("\r\n");
			printf("%02x ", cmd[i]);
		}
		printf("\r\n");
	}
#if 0	
	do {
                rsp_len = hid_xfer(dev, EP_MCU_IN, rsp, 64, 1000);
                printf("rsp len %d\r\n", rsp_len);
                for (i=0; i<rsp_len; i++) {
                        if (i%16 == 0 && i != 0)
                                printf("\r\n");
                        printf("%02x ", rsp[i]);
                }
                printf("\r\n");
        } while (rsp_len > 0);
#endif
	//usb_xfer(dev, USB_ENDPOINT_OUT, 0xE2, 0x00, 0x01, NULL, 0);
	//usb_xfer(dev, USB_ENDPOINT_IN, 0xE1, 0x00, 0x01, &glasses_v, 1);
	//printf("glasses protocol version: %d\r\n", glasses_v);
	//usb_xfer(dev, USB_ENDPOINT_OUT, 0xE1, 0x00, 0x00, &app_v, 1);
	cmd[62] = ((payload_len +22) >> 8) & 0xff;	
	cmd[63] = ((payload_len +22) >> 0) & 0xff;	
	if (64 == hid_xfer(dev, EP_MCU_OUT, cmd, 64,
				5000)) {
		while (1) {
		rsp_len = hid_xfer(dev, EP_MCU_IN, rsp, 64, 5000);
		printf("rsp len %d\r\n", rsp_len);
		if (rsp_len != 0) {
			if (debug) {
			printf("<==[%d]enc\r\n", rsp_len);
			for(i=0; i<rsp_len; i++) {
				if (i%16 == 0 && i != 0)
				printf("\r\n");
				printf("%02x ", rsp[i]);
			}
			printf("\r\n");
			}
			rsp_len = (rsp[62] << 8) | rsp[63];
			printf("second rsp len %d\r\n", rsp_len);
			uint32_t crc = crc32((const char *)rsp+1, rsp_len - 5);
			uint32_t crc_dev = (rsp[rsp_len - 1] << 24) |
						(rsp[rsp_len - 2] << 16) |
						(rsp[rsp_len - 3] << 8) |
						(rsp[rsp_len - 4] << 0);
			if (crc != crc_dev)
				printf("rsp crc failed h %04x != c %04x\r\n",
					crc, crc_dev);
			uint16_t rsp_msg_id = (rsp[11] << 8)|rsp[10];
			set_rc4_key(rsp[8]%10, rsp_msg_id, rsp+2);
			rc4(rsp+12, rsp+12, rsp_len - 16);
			
			if (debug) {
			printf("<==[%d]\r\n", rsp_len);
			for (i=0; i<rsp_len; i++) {
				if (i%16 == 0 && i != 0)
				printf("\r\n");
				printf("%02x ", rsp[i]);

			}
			printf("\r\n");
			}
			int64_val ts;
			memcpy(ts.m_bytes, rsp+2, 8);
			printf("rsponse ts: %lld\r\n", ts.m_int64);
			uint16_t rsp_payload_len = (rsp[17] << 8) | rsp[16];
			uint16_t rsp_msg_id1, rsp_mcu_cmd;
			rsp_msg_id1 = (rsp[11] << 8) | rsp[10];
			rsp_mcu_cmd = (rsp[20] << 8) | rsp[19];
			if (0 == parse_rsp(argv[1], rsp_msg_id1, rsp_mcu_cmd,
						rsp+18, rsp_payload_len))
						break;
			cnt++;
			if (cnt > 10)
				break;
		} else {
			printf("rcv failed\r\n");
			break;
		}
		}
	}

	close_usb(dev, INTF_MCU);
	return 0;
}
