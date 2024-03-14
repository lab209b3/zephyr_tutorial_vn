#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED1_NODE    DT_ALIAS(led0)
#define BUTTON0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(BUTTON0_NODE, gpios, {0});

int main(void)
{
	int ret1, ret2;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	if (!gpio_is_ready_dt(&button)) {
		return 0;
	}

	ret1 = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	ret2 = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
	gpio_pin_set_dt(&led, 0);
	if (ret1 < 0 || ret2 < 0) {
		return 0;
	}

	while (1) {
		if (!gpio_pin_get_dt(&button)) {
			gpio_pin_set_dt(&led, 1);
		} else {
			gpio_pin_set_dt(&led, 0);
		}
	}
}
