/*
 * Copyright (c) 2017 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/printk.h>

int main(void)
{
	#if 0
	const struct device *const dev = DEVICE_DT_GET_ONE(st_vl53l0x);
	struct sensor_value value;
	int ret;

	if (!device_is_ready(dev)) {
		printk("sensor: device not ready.\n");
		return 0;
	}

	while (1) {
		ret = sensor_sample_fetch(dev);
		if (ret) {
			printk("sensor_sample_fetch failed ret %d\n", ret);
			return 0;
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_PROX, &value);
		printk("prox is %d\n", value.val1);

		ret = sensor_channel_get(dev,
					 SENSOR_CHAN_DISTANCE,
					 &value);
		printf("distance is %.3fm\n", sensor_value_to_double(&value));

		k_sleep(K_MSEC(1000));
	}
	return 0;
	#endif
}


#include <zephyr/drivers/gpio.h>

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

static void blink(void)
{
	const struct gpio_dt_spec spec = GPIO_DT_SPEC_GET(DT_NODELABEL(bd_led), gpios);
	int ret;

	printk("Thread: %s\n", __func__);

	if (!device_is_ready(spec.port)) {
		printk("Error: %s device is not ready\n", spec.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&spec, GPIO_OUTPUT_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure pin %d\n",
			ret, spec.pin);
		return;
	}

	while (1) {
		gpio_pin_set(spec.port, spec.pin, 1);
		k_msleep(500);
		gpio_pin_set(spec.port, spec.pin, 0);
		k_msleep(500);
	}
}

K_THREAD_DEFINE(blink_id, STACKSIZE, blink, NULL, NULL, NULL,
	PRIORITY, 0, 0);
