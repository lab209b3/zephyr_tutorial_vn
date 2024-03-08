#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);
LV_IMG_DECLARE(power_on);
LV_IMG_DECLARE(power_off);

lv_obj_t * imgbtn;

void disp_btn(void)
{

    imgbtn = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_RELEASED, NULL, &power_off, NULL);
    lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_DISABLED, NULL, &power_off, NULL);
	lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &power_off, NULL);

    lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &power_on, NULL);
    lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &power_on, NULL);
	lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_PRESSED, NULL, &power_on, NULL);
    lv_obj_set_width(imgbtn, 50);
    lv_obj_set_height(imgbtn, 50);
    lv_obj_align(imgbtn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag( imgbtn, LV_OBJ_FLAG_CHECKABLE );   /// Flags
}

int main(void)
{
	const struct device *display_dev;
	

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}
	disp_btn();
	

	

	lv_task_handler();
	display_blanking_off(display_dev);

	while (1) {
		
		lv_task_handler();
		k_sleep(K_MSEC(10));
	}
}
