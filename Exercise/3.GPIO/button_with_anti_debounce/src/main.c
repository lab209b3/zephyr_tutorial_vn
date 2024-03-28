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
	bool button_state;
	bool last_button_state = true;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	if (!gpio_is_ready_dt(&button)) {
		return 0;
	}

	ret1 = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	// ret2 = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
	ret2 = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
	ret1 = gpio_pin_set_dt(&led, 0);
	if (ret1 < 0 || ret2 < 0) {
		return 0;
	}

	while (1) {
		button_state = gpio_pin_get_dt(&button);

		// Kiểm tra nút nhấn có thay đổi trạng thái không
		if (button_state != last_button_state) {
			// Nếu nút nhấn được nhấn, thực hiện chống debounce
			if (!button_state) {
				// Chờ một khoảng thời gian ngắn trước khi kiểm tra lại trạng thái
				// nút nhấn
				k_msleep(50);
				// Kiểm tra lại trạng thái nút nhấn sau khoảng thời gian chờ
				if (!gpio_pin_get_dt(&button)) {
					// Thực hiện chỉ thay đổi trạng thái LED nếu nút nhấn vẫn
					// được nhấn sau thời gian chờ
					gpio_pin_toggle_dt(&led);
				}
			}
			// Cập nhật trạng thái nút nhấn cuối cùng
			last_button_state = button_state;
		}
	}
}
// them giai thich