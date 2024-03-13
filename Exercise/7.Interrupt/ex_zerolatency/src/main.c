
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include "interrupt_util.h"

#include <inttypes.h>


#define SENSOR_IRQ 25 // Số IRQ của cảm biến

volatile int sensor_value = 0;

void sensor_read(void)
{
    // Đọc giá trị từ cảm biến và lưu vào sensor_value
    // (Giả sử rằng giá trị được đọc từ cảm biến được lưu vào biến sensor_value)
    sensor_value = sensor_value + 20; 
}

void sensor_isr(const struct device *dev, void *data)
{
    // Xử lý ngắt từ cảm biến
            printk("Warning: Sensor value exceeded 100! at %" PRIu32 "\n", k_cycle_get_32());
             sensor_value = 0;

}

void sensor_isr_installer(void)
{
    // Kết nối ngắt và kích hoạt ngắt
    IRQ_CONNECT(SENSOR_IRQ, 0, sensor_isr, NULL, IRQ_ZERO_LATENCY);
    irq_enable(SENSOR_IRQ);
}

void main(void)
{
    sensor_isr_installer();

    while (1) {
        // Đọc giá trị từ cảm biến mỗi 10 giây
        sensor_read();
        printk("%d at %" PRIu32 "\n",sensor_value, k_cycle_get_32());
    if (sensor_value >= 100) {

       trigger_irq(SENSOR_IRQ);
    }
        
        k_sleep(K_SECONDS(1));
    }
}
