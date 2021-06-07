#include <rtthread.h>
#include <rthw.h>
#include "mem_list.h"

#define MAX_LIST_CNT	100
#define MEM_USED	0x01
#define MEM_FREE	0x00

static rt_list_t rt_d2h_list;
static rt_list_t rt_h2d_list;
struct rt_mem_list
{
	rt_list_t list;
	uint8_t data[64];
	uint8_t used;
	uint16_t len;
};
typedef struct rt_mem_list *rt_memlist_t;

static struct rt_mem_list d2h_mem_list[MAX_LIST_CNT];
static struct rt_mem_list h2d_mem_list[MAX_LIST_CNT];

void rt_memlist_init()
{
	int i;

	rt_list_init(&rt_d2h_list);
	rt_list_init(&rt_h2d_list);

	for (i = 0; i < MAX_LIST_CNT; i++) {
		rt_list_init(&(d2h_mem_list[i].list));
		rt_list_init(&(h2d_mem_list[i].list));
		d2h_mem_list[i].used = MEM_FREE;
		h2d_mem_list[i].used = MEM_FREE;
		h2d_mem_list[i].len = 0;
		d2h_mem_list[i].len = 0;
	}
}

rt_bool_t d2h_list_isempty()
{
	register rt_base_t level;
	rt_bool_t res = RT_FALSE;

	level = rt_hw_interrupt_disable();
	res = rt_list_isempty(&rt_d2h_list);
	rt_hw_interrupt_enable(level);

	return res;
}

rt_bool_t h2d_list_isempty()
{
	register rt_base_t level;
	rt_bool_t res = RT_FALSE;

	level = rt_hw_interrupt_disable();
	res = rt_list_isempty(&rt_h2d_list);
	rt_hw_interrupt_enable(level);

	return res;
}

rt_bool_t insert_mem(uint8_t type, uint8_t* data, uint16_t len)
{
	int i;
	rt_memlist_t ptr;
	rt_list_t *insert;
	register rt_base_t level;

	level = rt_hw_interrupt_disable();

	if (type == TYPE_D2H) {
		ptr = d2h_mem_list;
		insert = &rt_d2h_list;
	} else {
		ptr = h2d_mem_list;
		insert = &rt_h2d_list;
	}

	for (i = 0; i < MAX_LIST_CNT; i++) {
		if (ptr[i].used == MEM_FREE) {
			ptr[i].used = MEM_USED;
			ptr[i].len = len;
			rt_memcpy(ptr[i].data, data, len);
			rt_list_insert_after(insert, &(ptr[i].list));
			break;
		}
	}

	rt_hw_interrupt_enable(level);

	if (i == MAX_LIST_CNT)
		return RT_FALSE;

	return RT_TRUE;
}

void remove_mem(uint8_t type, uint8_t **out, uint16_t *len)
{
	rt_list_t *insert;
	register rt_base_t level;

	level = rt_hw_interrupt_disable();
	*len = 0;
	if (type == TYPE_D2H) {
		insert = &rt_d2h_list;
	} else {
		insert = &rt_h2d_list;
	}

	*out = RT_NULL;

	if (!rt_list_isempty(insert)) {
		rt_memlist_t to_remove = rt_list_entry(insert->next,
				struct rt_mem_list,
				list);
		to_remove->used = MEM_FREE;
		*out = to_remove->data;
		*len = to_remove->len;
		rt_list_remove(&(to_remove->list));
	}

	rt_hw_interrupt_enable(level);
}
