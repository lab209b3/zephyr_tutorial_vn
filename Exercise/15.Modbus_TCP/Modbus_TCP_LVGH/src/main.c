#include "main.h"

LOG_MODULE_REGISTER(tcp_modbus, LOG_LEVEL_INF);

static const struct gpio_dt_spec led_dev[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios)
};


///////////////////// VARIABLES ////////////////////

// SCREEN: ui_Screen1
lv_obj_t * ui_Screen1;
lv_obj_t * ui_Label1;

lv_obj_t * slider_map[4];
lv_obj_t * switch_map[5];

void ui_Screen1_screen_init(void);
void ui_event_Slider1(lv_event_t * e);
void ui_event_Slider2(lv_event_t * e);
void ui_event_Slider3(lv_event_t * e);
void ui_event_Slider4(lv_event_t * e);

void ui_event_Switch1(lv_event_t * e);
void ui_event_Switch2(lv_event_t * e);
void ui_event_Switch3(lv_event_t * e);
void ui_event_Switch4(lv_event_t * e);
void ui_event_Switch5(lv_event_t * e);

lv_obj_t * ui____initial_actions0;

/////////////////////////////////////////////

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
	bool on;

	if (state == true) {
		coils_state |= BIT(addr);
		on = true;
		lv_obj_add_state(switch_map[addr], LV_STATE_CHECKED);
	} else {
		coils_state &= ~BIT(addr);
		on = false;
		lv_obj_clear_state(switch_map[addr], LV_STATE_CHECKED);
	}

	if(addr == 0)
		gpio_pin_set(led_dev[addr].port, led_dev[addr].pin, (int)on);
	

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
	if (addr >= ARRAY_SIZE(holding_reg)) {
		return -ENOTSUP;
	}

	holding_reg[addr] = reg;
	lv_slider_set_value(slider_map[addr],reg,0);
	LOG_INF("Holding register write, addr %u", addr);

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

///////////////////// FUNCTIONS ////////////////////
void ui_event_Slider1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_VALUE_CHANGED) {
		holding_reg_wr(0,(int)lv_slider_get_value(target));
    }
}
void ui_event_Slider2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_VALUE_CHANGED) {
		holding_reg_wr(1,(int)lv_slider_get_value(target));
    }
}
void ui_event_Slider3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_VALUE_CHANGED) {
		holding_reg_wr(2,(int)lv_slider_get_value(target));
    }
}
void ui_event_Slider4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_VALUE_CHANGED) {
		holding_reg_wr(3,(int)lv_slider_get_value(target));
    }
}
void ui_event_Switch1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED) {
		
		coil_wr(0,lv_obj_has_state(switch_map[0], LV_STATE_CHECKED));
    }
}
void ui_event_Switch2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED) {
    	coil_wr(1,lv_obj_has_state(switch_map[1], LV_STATE_CHECKED));
    }
}
void ui_event_Switch3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED) {
    	coil_wr(2,lv_obj_has_state(switch_map[2], LV_STATE_CHECKED));
    }
}
void ui_event_Switch4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED) {
    	coil_wr(3,lv_obj_has_state(switch_map[3], LV_STATE_CHECKED));
    }
}
void ui_event_Switch5(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED) {
    	coil_wr(4,lv_obj_has_state(switch_map[4], LV_STATE_CHECKED));
    }
}

void ui_Screen1_screen_init(void)
{
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_Label1 = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_Label1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Label1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Label1, 0);
    lv_obj_set_y(ui_Label1, -102);
    lv_obj_set_align(ui_Label1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label1, "MODBUS TCP/IP");

    slider_map[0] = lv_slider_create(ui_Screen1);
    lv_obj_set_width(slider_map[0], 150);
    lv_obj_set_height(slider_map[0], 10);
    lv_obj_set_x(slider_map[0], -100);
    lv_obj_set_y(slider_map[0], 20);
    lv_obj_set_align(slider_map[0], LV_ALIGN_CENTER);


    slider_map[1] = lv_slider_create(ui_Screen1);
    lv_obj_set_width(slider_map[1], 150);
    lv_obj_set_height(slider_map[1], 10);
    lv_obj_set_x(slider_map[1], 100);
    lv_obj_set_y(slider_map[1], 20);
    lv_obj_set_align(slider_map[1], LV_ALIGN_CENTER);


    slider_map[2] = lv_slider_create(ui_Screen1);
    lv_obj_set_width(slider_map[2], 150);
    lv_obj_set_height(slider_map[2], 10);
    lv_obj_set_x(slider_map[2], -100);
    lv_obj_set_y(slider_map[2], 80);
    lv_obj_set_align(slider_map[2], LV_ALIGN_CENTER);


    slider_map[3] = lv_slider_create(ui_Screen1);
    lv_obj_set_width(slider_map[3], 150);
    lv_obj_set_height(slider_map[3], 10);
    lv_obj_set_x(slider_map[3], 100);
    lv_obj_set_y(slider_map[3], 80);
    lv_obj_set_align(slider_map[3], LV_ALIGN_CENTER);


    switch_map[0] = lv_switch_create(ui_Screen1);
    lv_obj_set_width(switch_map[0], 50);
    lv_obj_set_height(switch_map[0], 25);
    lv_obj_set_x(switch_map[0], -160);
    lv_obj_set_y(switch_map[0], -40);
    lv_obj_set_align(switch_map[0], LV_ALIGN_CENTER);


    switch_map[1] = lv_switch_create(ui_Screen1);
    lv_obj_set_width(switch_map[1], 50);
    lv_obj_set_height(switch_map[1], 25);
    lv_obj_set_x(switch_map[1], -80);
    lv_obj_set_y(switch_map[1], -40);
    lv_obj_set_align(switch_map[1], LV_ALIGN_CENTER);


    switch_map[2] = lv_switch_create(ui_Screen1);
    lv_obj_set_width(switch_map[2], 50);
    lv_obj_set_height(switch_map[2], 25);
    lv_obj_set_x(switch_map[2], 0);
    lv_obj_set_y(switch_map[2], -40);
    lv_obj_set_align(switch_map[2], LV_ALIGN_CENTER);


    switch_map[3] = lv_switch_create(ui_Screen1);
    lv_obj_set_width(switch_map[3], 50);
    lv_obj_set_height(switch_map[3], 25);
    lv_obj_set_x(switch_map[3], 80);
    lv_obj_set_y(switch_map[3], -40);
    lv_obj_set_align(switch_map[3], LV_ALIGN_CENTER);


    switch_map[4] = lv_switch_create(ui_Screen1);
    lv_obj_set_width(switch_map[4], 50);
    lv_obj_set_height(switch_map[4], 25);
    lv_obj_set_x(switch_map[4], 160);
    lv_obj_set_y(switch_map[4], -40);
    lv_obj_set_align(switch_map[4], LV_ALIGN_CENTER);


    lv_obj_add_event_cb(slider_map[0], ui_event_Slider1, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(slider_map[1], ui_event_Slider2, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(slider_map[2], ui_event_Slider3, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(slider_map[3], ui_event_Slider4, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(switch_map[0], ui_event_Switch1, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(switch_map[1], ui_event_Switch2, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(switch_map[2], ui_event_Switch3, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(switch_map[3], ui_event_Switch4, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(switch_map[4], ui_event_Switch5, LV_EVENT_ALL, NULL);

}

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_Screen1_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_Screen1);
}

void TFT(void)
{
	while(1)
	{
		lv_task_handler();
		k_msleep(10);
	}
}

K_THREAD_DEFINE(TFT_id, 2000, TFT, 
				NULL, NULL, NULL,
				7, 0, 0);

int main(void)
{
	int serv;
	struct sockaddr_in bind_addr;
	static int counter;

	const struct device *display_dev;
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	ui_init();

	lv_task_handler();
	display_blanking_off(display_dev);

	if (init_modbus_server()) {
		LOG_ERR("Modbus TCP server initialization failed");
		return 0;
	}

	if (init_leds()) {
		LOG_ERR("Modbus TCP server initialization failed");
		return 0;
	}

	serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (serv < 0) {
		LOG_ERR("error: socket: %d", errno);
		return 0;
	}

	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(MODBUS_TCP_PORT);

	if (bind(serv, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
		LOG_ERR("error: bind: %d", errno);
		return 0;
	}

	if (listen(serv, 5) < 0) {
		LOG_ERR("error: listen: %d", errno);
		return 0;
	}

	LOG_INF("Started MODBUS TCP server example on port %d", MODBUS_TCP_PORT);

	while(1) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		char addr_str[INET_ADDRSTRLEN];
		int client;
		int rc;

		client = accept(serv, (struct sockaddr *)&client_addr,
				&client_addr_len);

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
		LOG_INF("Connection from %s closed, errno %d",
			addr_str, rc);
	};
	return 0;
}
