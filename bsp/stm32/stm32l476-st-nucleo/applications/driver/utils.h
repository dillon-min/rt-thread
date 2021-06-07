#ifndef _UTILS_H
#define _UTILS_H
int timestamp_init();
void read_ts_64(uint8_t *buf);
int normal_timer_init();
uint32_t read_ts();
void dump_mcu_cmd(uint16_t msg_id, uint16_t cmd_id,
		uint8_t *payload, uint16_t len);
#endif
