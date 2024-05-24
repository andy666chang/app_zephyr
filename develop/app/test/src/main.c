
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void) {
    int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

    printf("Hello World!\n");

    while (1) {
		gpio_pin_set_dt(&led, 0);
		k_msleep(SLEEP_TIME_MS);
		// k_busy_wait(1000000);
		printf("LED state: %s\n", "ON");

		gpio_pin_set_dt(&led, 1);
		k_msleep(SLEEP_TIME_MS);
		// k_busy_wait(1000000);
		printf("LED state: %s\n", "OFF");
    }

    return 0;
}
