/*
 * @Author: andy.chang
 * @Date: 2025-03-09 02:55:17
 * @Last Modified by: andy.chang
 * @Last Modified time: 2025-03-10 00:50:01
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(btn, LOG_LEVEL_INF);

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

static const struct gpio_dt_spec btn =
    GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);
static struct gpio_callback btn_cb_data;

/* Semaphore to indicate that there was an update to the gpio */
static K_SEM_DEFINE(btn_update, 0, 1);

/**
 * @brief btn_chg
 *
 */
struct k_work_delayable btn_chg;

/**
 * @brief Button work
 *
 * @param work
 */
static void btn_work(struct k_work *work) {
    // Send semaphore to button thread
    k_sem_give(&btn_update);
}

static void btn_callback(const struct device *dev, struct gpio_callback *cb,
                         uint32_t pins) {
    // Debounce with kwork
    k_work_reschedule(&btn_chg, K_MSEC(3));
}

static void btn_init(void) {
    int ret;

    // Initial peripherial
    if (!device_is_ready(btn.port)) {
        LOG_ERR("Error: button device %s is not ready", btn.port->name);
        return;
    }

    k_work_init_delayable(&btn_chg, btn_work);

    ret = gpio_pin_configure_dt(&btn, GPIO_INPUT | GPIO_PULL_UP);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d", ret, btn.port->name,
                btn.pin);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&btn, GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n", ret,
                btn.port->name, btn.pin);
        return;
    }

    gpio_init_callback(&btn_cb_data, btn_callback, BIT(btn.pin));
    gpio_add_callback(btn.port, &btn_cb_data);
    LOG_INF("Set up button at %s pin %d, level: %d\n", btn.port->name, btn.pin,
            gpio_pin_get_dt(&btn));

    while (1) {
        // Wait for semaphore with K_FOREVER
        k_sem_take(&btn_update, K_FOREVER);

        // Check button state
        uint8_t btn_state = gpio_pin_get_dt(&btn);
        if (btn_state) {
            LOG_INF("button pressed");
        } else {
            LOG_INF("button released");
        }
    }
}

K_THREAD_DEFINE(btn_init_id, STACKSIZE, btn_init, NULL, NULL, NULL, PRIORITY, 0,
                0);
