/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <app_version.h>

#include <zephyr/logging/log.h>

#include "flecs.h"

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

void my_abort() {
	printk("myabort\n");
	abort();
}


void my_logger(int32_t level, const char *file, int32_t line, const char *msg) {
	printk("level:%i file:%s:%i msg:%s\n", level, file, line, msg);
	if (level < -3) {
		my_abort();
	}
}


static int total_size = 0;

static void* my_malloc(ecs_size_t size) {
	void * p = malloc((size_t)size);
	total_size += size;
	printk("malloc: sum:%10i %10i, %p,\n", total_size, size, p);
	return p;
}

static void* my_calloc(ecs_size_t size) {
	void * p = calloc(1, (size_t)size);
	total_size += size;
	printk("calloc: sum:%10i %10i, %p\n", total_size, size, p);
	return p;
}

static void my_free(void * p) {
	printk("free: %x\n", p);
	free(p);
}

/*
void on_heap_free(uintptr_t heap_id, void *mem, size_t bytes)
{
  LOG_INF("Memory freed at %p, size %ld", mem, bytes);
}

HEAP_LISTENER_FREE_DEFINE(my_listener, HEAP_ID_LIBC, on_heap_free);
*/

int main(void)
{
    ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_api;
    os_api.log_ = my_logger;
    os_api.abort_ = my_abort;
    os_api.malloc_ = my_malloc;
    os_api.calloc_ = my_calloc;
    os_api.free_ = my_free;
    ecs_os_set_api(&os_api);

	printk("ecs_mini\n");
	ecs_world_t * world = ecs_mini();

	int ret;
	const struct device *sensor;

	printk("Zephyr Example Application %s\n", APP_VERSION_STRING);

	sensor = DEVICE_DT_GET(DT_NODELABEL(examplesensor0));
	if (!device_is_ready(sensor)) {
		LOG_ERR("Sensor not ready");
		return 0;
	}

	while (1) {
		struct sensor_value val;

		ret = sensor_sample_fetch(sensor);
		if (ret < 0) {
			LOG_ERR("Could not fetch sample (%d)", ret);
			return 0;
		}

		ret = sensor_channel_get(sensor, SENSOR_CHAN_PROX, &val);
		if (ret < 0) {
			LOG_ERR("Could not get sample (%d)", ret);
			return 0;
		}

		printk("Sensor value: %d\n", val.val1);

		k_sleep(K_MSEC(1000));
	}

	return 0;
}

