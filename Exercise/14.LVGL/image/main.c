#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>
#include <zephyr/logging/log.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

LOG_MODULE_REGISTER(app);
LV_IMG_DECLARE(zephyr_logo);
lv_obj_t * img1;

void lv_example_img_1(void)
{
    
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &zephyr_logo);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, -20);
}

int main(void)
{
	const struct device *display_dev;
	

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	lv_example_img_1();
	
	lv_task_handler();
	display_blanking_off(display_dev);

	while (1) {
		
		lv_task_handler();
		k_sleep(K_MSEC(10));
	}
}
