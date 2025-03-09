/*
 * Copyright (c) 2017 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void) {
    const struct device *const dev = DEVICE_DT_GET_ONE(st_vl53l0x);
    struct sensor_value value;
    int ret;

	printk("Hello World: %s\n", CONFIG_BOARD);

    if (!device_is_ready(dev)) {
        LOG_ERR("sensor: device not ready.");
        return 0;
    }

    while (1) {
        ret = sensor_sample_fetch(dev);
        if (ret) {
            LOG_ERR("sensor_sample_fetch failed ret %d", ret);
            return 0;
        }

        ret = sensor_channel_get(dev, SENSOR_CHAN_PROX, &value);
        LOG_INF("prox is %d", value.val1);

        ret = sensor_channel_get(dev, SENSOR_CHAN_DISTANCE, &value);
        LOG_INF("distance is %.3fm", sensor_value_to_double(&value));

        k_sleep(K_MSEC(1000));
    }
    return 0;
}
