#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <lvgl_input_device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>

#define MODBUS_TCP_PORT 502

static uint16_t holding_reg[4] = {1,2,3,4};
static uint8_t coils_state;

