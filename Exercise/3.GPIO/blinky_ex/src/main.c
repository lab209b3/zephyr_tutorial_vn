#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE  DT_ALIAS(led0)
#define TIME_DELAY 1000
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

	if (ret < 0) {
		return 0;
	}

	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(TIME_DELAY);
	}
}
