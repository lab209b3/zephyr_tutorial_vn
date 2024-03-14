#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>

#define DEVICE_I2C_ADDRESS      0X27

// lcd instruction
#define WRITE_INSTRUCTION       0X00
#define WRITE_DATA              0X01

#define LCD_ENABLE              0X04

#define RESET_INIT_1            0X30
#define RESET_INIT_2            0X20
#define FUNCTION_SET            0X28
#define ENTRY_MODE_SET          0X06

#define CLEAR_DISPLAY           0X01

#define TWO_LINE_MODE           0X38

#define LCD_ON                  0X0C
#define LCD_OFF                 0X08

#define BACKLIGHT_ON            0X08
#define BACKLIGHT_OFF           0X00

#define END_OF_LINE             0X00

/////////////////////////////////////////////////////////////////////////////////////
// Basic interface functions for communicating with the MFRC522
/////////////////////////////////////////////////////////////////////////////////////
void lcd_write_instruction(uint8_t lcd_instruction);
void lcd_write_data(uint8_t lcd_data);

/////////////////////////////////////////////////////////////////////////////////////
// Functions for init/setup the LCD 1602
/////////////////////////////////////////////////////////////////////////////////////
void lcd_init(const struct i2c_dt_spec* i2c_dev);
void lcd_set_cursor_position(uint8_t vi_tri_cot, uint8_t vi_tri_hang);
void lcd_control_backlight(uint8_t backlight_data);
void lcd_control_lcd_and_backlight(bool control_state);

/////////////////////////////////////////////////////////////////////////////////////
// Application functions for the LCD 1602
/////////////////////////////////////////////////////////////////////////////////////
void lcd_clear_display();
void lcd_wait_and_clear(uint16_t delay_time_ms);
void lcd_print_string(const char string_array[], uint8_t vi_tri_cot, uint8_t vi_tri_hang);
void lcd_print_full_screen(const char string_array[], uint8_t vi_tri_cot, uint8_t vi_tri_hang, uint16_t delay_time_ms);
#endif
