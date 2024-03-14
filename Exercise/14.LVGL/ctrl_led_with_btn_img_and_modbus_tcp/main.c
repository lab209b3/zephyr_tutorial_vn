#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <lvgl_input_device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#define STACKSIZE 2048
#define PRIORITY 7
#define MODBUS_TCP_PORT 502

LOG_MODULE_REGISTER(tcp_modbus, LOG_LEVEL_INF);


static uint16_t holding_reg[8];
static uint8_t multi_coil[4];
static uint8_t coils_state;
static uint16_t slider_data;

lv_obj_t * imgbtn[4];
lv_obj_t * img1;

lv_obj_t * slider_label;
lv_obj_t * slider;

LV_IMG_DECLARE(zephyr_logo);
LV_IMG_DECLARE(power_on);
LV_IMG_DECLARE(power_off);

static const struct gpio_dt_spec led_dev[] = {GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios)};
static int custom_read_count;
static bool custom_handler(const int iface,const struct modbus_adu *rx_adu,struct modbus_adu *tx_adu,uint8_t *const excep_code,void *const user_data)
{
	const uint8_t request_len = 2;
	const uint8_t response_len = 6;
	int *read_counter = (int *)user_data;
	uint8_t subfunc;
	uint8_t data_len;

	LOG_INF("Custom Modbus handler called");

	if (rx_adu->length != request_len) {
		LOG_WRN("Custom request length doesn't match");
		*excep_code = MODBUS_EXC_ILLEGAL_DATA_VAL;
		return true;
	}

	subfunc = rx_adu->data[0];
	data_len = rx_adu->data[1];

	LOG_INF("Custom function called with subfunc=%u, data_len=%u", subfunc, data_len);
	(*read_counter)++;
	sys_put_be16(0x5555, tx_adu->data);
	sys_put_be16(0xAAAA, &tx_adu->data[2]);
	sys_put_be16(*read_counter, &tx_adu->data[4]);
	tx_adu->length = response_len;

	return true;
}

MODBUS_CUSTOM_FC_DEFINE(custom, custom_handler, 101, &custom_read_count);

static int init_leds(void)
{
	int err;

	for (int i = 0; i < ARRAY_SIZE(led_dev); i++) {
		if (!gpio_is_ready_dt(&led_dev[i])) {
			LOG_ERR("LED%u GPIO device not ready", i);
			return -ENODEV;
		}

		err = gpio_pin_configure_dt(&led_dev[i], GPIO_OUTPUT_INACTIVE);
		if (err != 0) {
			LOG_ERR("Failed to configure LED%u pin", i);
			return err;
		}
	}

	return 0;
}

static int coil_rd(uint16_t addr, bool *state)
{
	if (addr >= ARRAY_SIZE(led_dev)) {
		return -ENOTSUP;
	}

	if (coils_state & BIT(addr)) {
		*state = true;
	} else {
		*state = false;
	}
	LOG_INF("Coil read, addr %u, %d", addr, (int)*state);

	return 0;
}

static int coil_wr(uint16_t addr, bool state)
{
	//bool on;

	if (addr >= ARRAY_SIZE(multi_coil)) {
		return -ENOTSUP;
	}

	multi_coil[addr] = (int)state;


	if(multi_coil[0])	
	{
		lv_imgbtn_set_state(imgbtn[0], LV_IMGBTN_STATE_PRESSED);
		gpio_pin_set(led_dev[0].port, led_dev[0].pin, 1);
	}
	else
	{
		lv_imgbtn_set_state(imgbtn[0], LV_IMGBTN_STATE_RELEASED);
		gpio_pin_set(led_dev[0].port, led_dev[0].pin, 0);
	}
	for(uint8_t i = 1; i <4;i++)
	{
		if(multi_coil[i])	lv_imgbtn_set_state(imgbtn[i], LV_IMGBTN_STATE_CHECKED_PRESSED);
		else				lv_imgbtn_set_state(imgbtn[i], LV_IMGBTN_STATE_RELEASED);
	}
	LOG_INF("Coil write, addr %u, %d", addr, (int)state);

	return 0;
}

static int holding_reg_rd(uint16_t addr, uint16_t *reg)
{
	if (addr >= ARRAY_SIZE(holding_reg)) {
		return -ENOTSUP;
	}
	*reg = holding_reg[addr];
	
	LOG_INF("Holding register read, addr %u", addr);

	return 0;
}

static int holding_reg_wr(uint16_t addr, uint16_t reg)
{
	char buf[8];
	if (addr >= ARRAY_SIZE(holding_reg)) {
		return -ENOTSUP;
	}

	holding_reg[addr] = reg;
	LOG_INF("Holding register write, addr %u, state %d", addr, holding_reg[addr]);
	lv_slider_set_value(slider,reg,LV_ANIM_ON);
	lv_snprintf(buf, sizeof(buf), "%d%%", reg);
	lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	return 0;
}

static struct modbus_user_callbacks mbs_cbs = {
	.coil_rd = coil_rd,
	.coil_wr = coil_wr,
	.holding_reg_rd = holding_reg_rd,
	.holding_reg_wr = holding_reg_wr,
};

static struct modbus_adu tmp_adu;
K_SEM_DEFINE(received, 0, 1);
static int server_iface;
static int server_raw_cb(const int iface, const struct modbus_adu *adu,
			void *user_data)
{
	LOG_DBG("Server raw callback from interface %d", iface);

	tmp_adu.trans_id = adu->trans_id;
	tmp_adu.proto_id = adu->proto_id;
	tmp_adu.length = adu->length;
	tmp_adu.unit_id = adu->unit_id;
	tmp_adu.fc = adu->fc;
	memcpy(tmp_adu.data, adu->data,
	       MIN(adu->length, CONFIG_MODBUS_BUFFER_SIZE));

	LOG_HEXDUMP_DBG(tmp_adu.data, tmp_adu.length, "resp");
	k_sem_give(&received);

	return 0;
}

const static struct modbus_iface_param server_param = {
	.mode = MODBUS_MODE_RAW,
	.server = {
		.user_cb = &mbs_cbs,
		.unit_id = 1,
	},
	.rawcb.raw_tx_cb = server_raw_cb,
	.rawcb.user_data = NULL
};

static int init_modbus_server(void)
{
	char iface_name[] = "RAW_0";
	int err;

	server_iface = modbus_iface_get_by_name(iface_name);

	if (server_iface < 0) {
		LOG_ERR("Failed to get iface index for %s",
			iface_name);
		return -ENODEV;
	}

	err = modbus_init_server(server_iface, server_param);

	if (err < 0) {
		return err;
	}

	return modbus_register_user_fc(server_iface, &modbus_cfg_custom);
}

static int modbus_tcp_reply(int client, struct modbus_adu *adu)
{
	uint8_t header[MODBUS_MBAP_AND_FC_LENGTH];

	modbus_raw_put_header(adu, header);
	if (send(client, header, sizeof(header), 0) < 0) {
		return -errno;
	}

	if (send(client, adu->data, adu->length, 0) < 0) {
		return -errno;
	}

	return 0;
}

static int modbus_tcp_connection(int client)
{
	uint8_t header[MODBUS_MBAP_AND_FC_LENGTH];
	int rc;
	int data_len;

	rc = recv(client, header, sizeof(header), MSG_WAITALL);
	if (rc <= 0) {
		return rc == 0 ? -ENOTCONN : -errno;
	}

	LOG_HEXDUMP_DBG(header, sizeof(header), "h:>");
	modbus_raw_get_header(&tmp_adu, header);
	data_len = tmp_adu.length;

	rc = recv(client, tmp_adu.data, data_len, MSG_WAITALL);
	if (rc <= 0) {
		return rc == 0 ? -ENOTCONN : -errno;
	}

	LOG_HEXDUMP_DBG(tmp_adu.data, tmp_adu.length, "d:>");
	if (modbus_raw_submit_rx(server_iface, &tmp_adu)) {
		LOG_ERR("Failed to submit raw ADU");
		return -EIO;
	}

	if (k_sem_take(&received, K_MSEC(1000)) != 0) {
		LOG_ERR("MODBUS RAW wait time expired");
		modbus_raw_set_server_failure(&tmp_adu);
	}

	return modbus_tcp_reply(client, &tmp_adu);
}

/*----------------------------------------------------------------------------------------------------------------*/
void disp_logo(void)
{
    img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &zephyr_logo);
    lv_obj_align(img1, LV_ALIGN_TOP_LEFT, 10, 10);
}
void disp_label(void)
{
	
    lv_obj_t * label1 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#00A000 Zephyr's Project#");

    lv_obj_set_width(label1, 300);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 0);

	
    lv_obj_t * label2 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     
    lv_obj_set_width(label2, 250);
    lv_label_set_recolor(label2, true);
    lv_label_set_text(label2, "#FF3333 Project made by Hung Nguyen Duy, # #00ff00 Thanh Binh Tran, # #FF33FF Thanh Sang Huynh, # #27A4F2 Minh Nguyen Nhat, #");
	lv_obj_set_style_text_font(label2, &lv_font_montserrat_14, 0);
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 100, 50);
}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if ( code == LV_EVENT_VALUE_CHANGED &&  lv_obj_has_state(obj,LV_STATE_CHECKED)  ) {
		gpio_pin_set(led_dev[0].port, led_dev[0].pin, 1);
    }
	if (code == LV_EVENT_VALUE_CHANGED &&  !lv_obj_has_state(obj,LV_STATE_CHECKED)  ) {
		gpio_pin_set(led_dev[0].port, led_dev[0].pin, 0);
	}
}


void disp_btn(void)
{

    imgbtn[0] = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn[0], LV_IMGBTN_STATE_RELEASED, NULL, &power_off, NULL);
    lv_imgbtn_set_src(imgbtn[0], LV_IMGBTN_STATE_DISABLED, NULL, &power_off, NULL);
	lv_imgbtn_set_src(imgbtn[0], LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &power_off, NULL);

    lv_imgbtn_set_src(imgbtn[0], LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &power_on, NULL);
    lv_imgbtn_set_src(imgbtn[0], LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &power_on, NULL);
	lv_imgbtn_set_src(imgbtn[0], LV_IMGBTN_STATE_PRESSED, NULL, &power_on, NULL);
    lv_obj_set_width(imgbtn[0], 50);
    lv_obj_set_height(imgbtn[0], 50);
    lv_obj_align(imgbtn[0], LV_ALIGN_CENTER, -150, 0);
    lv_obj_add_flag( imgbtn[0], LV_OBJ_FLAG_CHECKABLE );   /// Flags
	lv_obj_add_event_cb(imgbtn[0], event_handler, LV_EVENT_ALL, NULL);

	imgbtn[1] = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn[1], LV_IMGBTN_STATE_RELEASED, NULL, &power_off, NULL);
    lv_imgbtn_set_src(imgbtn[1], LV_IMGBTN_STATE_DISABLED, NULL, &power_off, NULL);
	lv_imgbtn_set_src(imgbtn[1], LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &power_off, NULL);

    lv_imgbtn_set_src(imgbtn[1], LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &power_on, NULL);
    lv_imgbtn_set_src(imgbtn[1], LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &power_on, NULL);
	lv_imgbtn_set_src(imgbtn[1], LV_IMGBTN_STATE_PRESSED, NULL, &power_on, NULL);
    lv_obj_set_width(imgbtn[1], 50);
    lv_obj_set_height(imgbtn[1], 50);
    lv_obj_align(imgbtn[1], LV_ALIGN_CENTER, -50, 0);
    lv_obj_add_flag( imgbtn[1], LV_OBJ_FLAG_CHECKABLE );   /// Flags

	imgbtn[2] = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn[2], LV_IMGBTN_STATE_RELEASED, NULL, &power_off, NULL);
    lv_imgbtn_set_src(imgbtn[2], LV_IMGBTN_STATE_DISABLED, NULL, &power_off, NULL);
	lv_imgbtn_set_src(imgbtn[2], LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &power_off, NULL);

    lv_imgbtn_set_src(imgbtn[2], LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &power_on, NULL);
    lv_imgbtn_set_src(imgbtn[2], LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &power_on, NULL);
	lv_imgbtn_set_src(imgbtn[2], LV_IMGBTN_STATE_PRESSED, NULL, &power_on, NULL);
    lv_obj_set_width(imgbtn[2], 50);
    lv_obj_set_height(imgbtn[2], 50);
    lv_obj_align(imgbtn[2], LV_ALIGN_CENTER, 50, 0);
    lv_obj_add_flag( imgbtn[2], LV_OBJ_FLAG_CHECKABLE );   /// Flags

	imgbtn[3] = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn[3], LV_IMGBTN_STATE_RELEASED, NULL, &power_off, NULL);
    lv_imgbtn_set_src(imgbtn[3], LV_IMGBTN_STATE_DISABLED, NULL, &power_off, NULL);
	lv_imgbtn_set_src(imgbtn[3], LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &power_off, NULL);

    lv_imgbtn_set_src(imgbtn[3], LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &power_on, NULL);
    lv_imgbtn_set_src(imgbtn[3], LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &power_on, NULL);
	lv_imgbtn_set_src(imgbtn[3], LV_IMGBTN_STATE_PRESSED, NULL, &power_on, NULL);
    lv_obj_set_width(imgbtn[3], 50);
    lv_obj_set_height(imgbtn[3], 50);
    lv_obj_align(imgbtn[3], LV_ALIGN_CENTER, 150, 0);
    lv_obj_add_flag( imgbtn[3], LV_OBJ_FLAG_CHECKABLE );   /// Flags
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * target = lv_event_get_target(e);
    char buf[8];
	slider_data = (int)lv_slider_get_value(target);
	holding_reg_wr(0, slider_data);
    lv_snprintf(buf, sizeof(buf), "%d%%", slider_data);
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void disp_slider(void)
{
    slider = lv_slider_create(lv_scr_act());
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, -35);
	lv_obj_set_style_bg_color(slider, lv_color_hex(0xff00a0), 0);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(slider_label, "0%");
	lv_obj_set_style_text_font(slider_label, &lv_font_montserrat_18, 0);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}


void tcp_serverr(void)
{
	int serv;
	struct sockaddr_in bind_addr;
	static int counter;

	if (init_modbus_server()) {
		LOG_ERR("Modbus TCP server initialization failed");
	}

	if (init_leds()) {
		LOG_ERR("Modbus TCP server initialization failed");
	}

	serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (serv < 0) {
		LOG_ERR("error: socket: %d", errno);
	}

	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(MODBUS_TCP_PORT);

	if (bind(serv, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
		LOG_ERR("error: bind: %d", errno);
	}

	if (listen(serv, 5) < 0) {
		LOG_ERR("error: listen: %d", errno);
	}
	
	while(1)
	{
		
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		char addr_str[INET_ADDRSTRLEN];
		int client;
		int rc;

		client = accept(serv, (struct sockaddr *)&client_addr, &client_addr_len);

		if (client < 0) {
			LOG_ERR("error: accept: %d", errno);
			continue;
		}

		inet_ntop(client_addr.sin_family, &client_addr.sin_addr,
			  addr_str, sizeof(addr_str));
		LOG_INF("Connection #%d from %s",
			counter++, addr_str);

		do {
			rc = modbus_tcp_connection(client);
		} while (!rc);

		close(client);
	}
}

void lvgl(void)
{
	const struct device *display_dev;
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
	}
	disp_logo();
	disp_label();
	disp_btn();
	disp_slider();
	lv_task_handler();
	display_blanking_off(display_dev);

	while (1)
	{
		lv_task_handler();
		k_sleep(K_MSEC(10));
	}
}

K_THREAD_DEFINE(lvgl_id, STACKSIZE, lvgl, NULL, NULL, NULL,PRIORITY, 0, 0);
K_THREAD_DEFINE(tcp_serverr_id, STACKSIZE, tcp_serverr, NULL, NULL, NULL,PRIORITY, 0, 0);