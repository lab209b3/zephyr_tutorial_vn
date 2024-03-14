#include "lcd_driver.h" 
#include "zephyr/kernel.h"
#include "zephyr/sys/printk.h"

uint8_t backlight_state = BACKLIGHT_OFF;
const struct i2c_dt_spec* lcd_i2c_dev;

/////////////////////////////////////////////////////////////////////////////////////
// Basic interface functions for communicating with the MFRC522
/////////////////////////////////////////////////////////////////////////////////////
static void lcd_i2c_write_4bit(uint8_t write_data);
static void lcd_pulse_enable(uint8_t write_data);

/*
 * @brief Viết 4 bit cho lcd
 *
 * @param write_data nhận giá trị từ lcd_data hoặc lcd_instruction
 *
 * @note Không xài hàm này cho application.
 */
static void lcd_i2c_write_4bit(uint8_t write_data)
{
    uint8_t data = write_data | backlight_state;
    uint8_t error;

    error = i2c_write_dt(lcd_i2c_dev, &data, 1);

    if (error != 0)
    {
        printk("Fail to write 4 bits to lcd\r\n");
        return;
    }
}

/*
 * @brief Tạo một xung cho chân enable của lcd
 *
 * @param write_data nhận giá trị có thể là lcd_instruction hoặc lcd_data.
 *
 * @note Không sử dụng hàm này cho application.
 */
static void lcd_pulse_enable(uint8_t write_data)
{
    lcd_i2c_write_4bit(write_data | LCD_ENABLE);
    k_msleep(1);

    lcd_i2c_write_4bit(write_data & (~LCD_ENABLE));
    k_msleep(1);
}

/*
 * @brief viết 1 byte instruction cho lcd
 *
 * @param lcd_instruction nhận một trong các giá trị được define trong file
 * lcd_driver.h
 *
 * @note Không sử dụng hàm này cho application.
 */
void lcd_write_instruction(uint8_t lcd_instruction)
{   
    uint8_t high_nibble = (lcd_instruction & 0xF0) | WRITE_INSTRUCTION;
    uint8_t low_nibble  = ((lcd_instruction << 4) & 0xF0) | WRITE_INSTRUCTION;

    lcd_i2c_write_4bit(high_nibble);
    lcd_pulse_enable(high_nibble);

    lcd_i2c_write_4bit(low_nibble);
    lcd_pulse_enable(low_nibble);
}

/*
 * @brief viết 1 byte data cho lcd
 *
 * @param lcd_data Nhận giá trị ascii để viết lên màn hình.
 *
 * @note Chúng ta sẽ sử dụng các hàm application ở phía cuối, nếu muốn xài trực
 * tiếp vẫn được.
 */
void lcd_write_data(uint8_t lcd_data)
{   
    uint8_t high_nibble = (lcd_data & 0xF0) | WRITE_DATA;
    uint8_t low_nibble  = ((lcd_data << 4) & 0xF0) | WRITE_DATA;

    lcd_i2c_write_4bit(high_nibble);
    lcd_pulse_enable(high_nibble);

    lcd_i2c_write_4bit(low_nibble);
    lcd_pulse_enable(low_nibble);
}

/////////////////////////////////////////////////////////////////////////////////////
// Functions for init/setup the LCD 1602
/////////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Khởi động lcd 1602 để chạy mode 4bit
 *
 * @param i2c_dev nhận địa chỉ của struct i2c_dt_spec mà các bạn cài đặt ở hàm
 * main() trong folder source.
 */
void lcd_init(const struct i2c_dt_spec* i2c_dev)
{
    lcd_i2c_dev = i2c_dev;

    //reset khoi dong nguon
    //delay 100ms sau khi cap nguon
    k_msleep(55);
    // Reset LCD_ENABLE
    lcd_i2c_write_4bit(0x00);
    // Turn backlight off
    backlight_state = BACKLIGHT_OFF;
    //delay 4.5 ms sau khi gui lenh 0x30 lan 1
    lcd_i2c_write_4bit(RESET_INIT_1);
    lcd_pulse_enable(RESET_INIT_1);
    k_msleep(5);
    //delay 100us sau khi gui lenh 0x30 lan 2
    lcd_i2c_write_4bit(RESET_INIT_1);
    lcd_pulse_enable(RESET_INIT_1);
    k_msleep(5);
    //delay 100us sau khi gui lenh 0x30 lan 3
    lcd_i2c_write_4bit(RESET_INIT_1);
    lcd_pulse_enable(RESET_INIT_1);
    k_msleep(5);
    //delay 100us sau khi gui lenh 0x20
    lcd_i2c_write_4bit(RESET_INIT_2);
    lcd_pulse_enable(RESET_INIT_2);
    k_msleep(1);

    //khoi dong LCD
    //delay 100us sau khi gui lenh function set
    lcd_write_instruction(FUNCTION_SET);
    k_msleep(1); 
    //delay 2 ms sau khi gui lenh clear display
    lcd_write_instruction(CLEAR_DISPLAY);
    k_msleep(2);
    //delay 2ms sau khi gui lenh display control
    lcd_write_instruction(LCD_OFF);
    k_msleep(1); 
    //delay 100us sau khi gui lenh entry mode set
    lcd_write_instruction(ENTRY_MODE_SET);
    k_msleep(1);
}

/*
 * @brief Cài đặt vị trí con trỏ trong lần viết ra màn hình tiếp theo
 *
 * @param vi_tri_cot Nhận giá trị 1 hoặc 2 tương ưng với hàng 1 hoặc 2, lưu ý
 * giá trị 0 không mang ý nghĩa nào và lcd sẽ không hoạt động theo mong muốn
 * 
 * @param vi_tri_hang Nhận giá trị từ 0 -> 15 tương ứng với vị trí của con trỏ
 * trên 1 hàng gồm 16 ô của lcd 1602
 */
void lcd_set_cursor_position(uint8_t vi_tri_cot, uint8_t vi_tri_hang)
{
    uint8_t cursor_address_array[3] = {0x0, 0x80, 0xC0};
    uint8_t cursor_address = cursor_address_array[vi_tri_cot] + vi_tri_hang;
    lcd_write_instruction(cursor_address);
    k_msleep(1); 
}

/*
 * @brief Điều khiển đèn led nền của lcd
 *
 * @param backlight_data Nhận 2 giá trị BACKLIGHT_ON và BACKLIGHT_OFF
 */
void lcd_control_backlight(uint8_t backlight_data)
{
    backlight_state = backlight_data;
}

/////////////////////////////////////////////////////////////////////////////////////
// Application functions for the LCD 1602
/////////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Xóa màn hình. 
 */
void lcd_clear_display()
{
    lcd_write_instruction(CLEAR_DISPLAY);
	k_msleep(2);
}

/*
 * @brief Chờ một khoảng thời gian rồi xóa màn hình.
 *
 * @param delay_time_ms khoảng thời gian ms phải chờ.
 */
void lcd_wait_and_clear(uint16_t delay_time_ms)
{
    k_msleep(delay_time_ms);
    lcd_write_instruction(CLEAR_DISPLAY);
    k_msleep(2);
}

/*
 * @brief In ra trên một dòng của lcd
 *
 * @param string_array Có thể pass vào một string "abc" hoặc địa chỉ của một
 * array cần chuyền, nếu truyển vào array thì phải có giá trị END_OF_LINE.
 *
 * @param vi_tri_cot Nhận 2 giá 1 và 2 tương ứng cột 1 và 2 của lcd.
 *
 * @param vi_tri_hang Nhận giá trị từ 0 -> 15 tường ưng ô số 0 đến 15 trên 1
 * hàng của lcd.
 */
void lcd_print_string(const char string_array[], uint8_t vi_tri_cot, uint8_t vi_tri_hang)
{
    lcd_set_cursor_position(vi_tri_cot, vi_tri_hang);

    while (*string_array != END_OF_LINE)
    {
        lcd_write_data(*string_array);
        string_array++;
    }       
}

/*
 * @brief In ra màn hình một dòng duy nhất mỗi lần gọi hàm, nếu muốn in ra 2
 * dòng cùng lúc thì xài hàm lcd_print_string(), hàm này cho phép đặt thời gian
 * chờ rồi xóa màn hình sau khi đã in lên lcd, hoặc nếu pass vào thời gian chờ
 * là 0 thì sẽ luôn luôn hiển thị.
 *
 * @param string_array Có thể pass vào một string "abc" hoặc địa chỉ của một
 * array cần chuyền, nếu truyển vào array thì phải có giá trị END_OF_LINE.
 *
 * @param vi_tri_cot Nhận 2 giá 1 và 2 tương ứng cột 1 và 2 của lcd.
 *
 * @param vi_tri_hang Nhận giá trị từ 0 -> 15 tường ưng ô số 0 đến 15 trên 1
 * hàng của lcd.
 *
 * @param delay_time_ms khoảng thời gian ms phải chờ.
 */
void lcd_print_full_screen(const char string_array[], uint8_t vi_tri_cot, uint8_t vi_tri_hang, uint16_t delay_time_ms)
{
	lcd_write_instruction(CLEAR_DISPLAY);
	k_msleep(2);

	lcd_print_string(string_array, vi_tri_cot, vi_tri_hang);

	if (delay_time_ms != 0)
        {
            k_msleep(delay_time_ms);

            lcd_write_instruction(CLEAR_DISPLAY);
            k_msleep(2);
        }
}
