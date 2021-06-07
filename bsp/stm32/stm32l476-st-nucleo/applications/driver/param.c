#include <fal.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "param.h"

#define DEFAULT_PARAM_BASE	"param"
#define DEFAULT_PARAM_LEN	4096

static uint8_t *local_buf = RT_NULL;
static const struct fal_partition *part_dev = NULL;

void param_get(uint32_t ofs, uint8_t *buf, uint32_t len)
{
	if (ofs + len > DEFAULT_PARAM_LEN)
		return;

	rt_memcpy(buf, local_buf+ofs, len);
}

void param_set_once(uint32_t ofs, uint8_t *buf, uint32_t len)
{
	if (ofs + len > DEFAULT_PARAM_LEN)
		return;

	if (rt_memcmp(buf, local_buf+ofs, len) == 0)
		return;
	
	rt_memcpy(local_buf+ofs, buf, len);
}

void param_set(uint32_t ofs, uint8_t *buf, uint32_t len)
{
	register rt_base_t level;
	if (ofs + len > DEFAULT_PARAM_LEN)
		return;

	if (rt_memcmp(buf, local_buf+ofs, len) == 0)
		return;
	
	level = rt_hw_interrupt_disable();
	if (0 > fal_partition_erase(part_dev, 0, DEFAULT_PARAM_LEN)) {
		rt_hw_interrupt_enable(level);
		rt_kprintf("erase param failed\r\n");
		return;
	}
	
	rt_memcpy(local_buf+ofs, buf, len);

	if (0 > fal_partition_write(part_dev, 0, local_buf, DEFAULT_PARAM_LEN)) {
		rt_kprintf("write param failed\r\n");
	}
	rt_hw_interrupt_enable(level);
}

rt_bool_t param_init()
{
	int i;
	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	local_buf = (uint8_t *)rt_malloc(DEFAULT_PARAM_LEN*sizeof(uint8_t));
	if (local_buf == RT_NULL) {
		rt_kprintf("Init param failed, not enough memory\r\n");
		return RT_FALSE;
	}

	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	part_dev = fal_partition_find(DEFAULT_PARAM_BASE);
	if (part_dev == NULL) {
		rt_kprintf("Can't find %s zone\r\n", DEFAULT_PARAM_BASE);
		return RT_FALSE;
	}
	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	
	if (0 > fal_partition_read(part_dev, 0, local_buf, DEFAULT_PARAM_LEN)) {
		rt_kprintf("Can't get param\r\n");
		return RT_FALSE;
	}
	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	for (i=0; i<512; i++) {
		if (i!= 0 && i%16 == 0)
			rt_kprintf("\r\n");
		rt_kprintf("%02x ", local_buf[i]);
	}
	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	return RT_TRUE;
}
