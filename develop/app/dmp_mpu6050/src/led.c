/*
 * @Author: andy.chang
 * @Date: 2025-03-09 02:55:17
 * @Last Modified by: andy.chang
 * @Last Modified time: 2025-03-09 02:56:23
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(led, LOG_LEVEL_INF);

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

static void blink(void) {
    const struct gpio_dt_spec spec =
        GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
    int ret;

    LOG_INF("Thread: %s", __func__);

    if (!device_is_ready(spec.port)) {
        LOG_ERR("Error: %s device is not ready", spec.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&spec, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        LOG_INF("Error %d: failed to configure pin %d", ret, spec.pin);
        return;
    }

    while (1) {
        gpio_pin_set(spec.port, spec.pin, 1);
        k_msleep(500);
        gpio_pin_set(spec.port, spec.pin, 0);
        k_msleep(500);
    }
}

K_THREAD_DEFINE(blink_id, STACKSIZE, blink, NULL, NULL, NULL, PRIORITY, 0, 0);
