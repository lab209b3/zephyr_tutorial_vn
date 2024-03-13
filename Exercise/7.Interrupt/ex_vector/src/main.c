#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/irq.h>
#define DEVICE_IRQ 25 // Số IRQ của thiết bị


ISR_DIRECT_DECLARE(device_isr)
{
   //
   ISR_DIRECT_PM(); /* PowerManagement được thực hiện sau khi ngắt dịch vụ để có độ trễ tốt nhất */
   return 1; 
}

void device_isr_installer(void)
{
    // Kết nối ngắt và kích hoạt ngắt
    printk("Đăng kí Device 25 IRQ");
    IRQ_DIRECT_CONNECT(DEVICE_IRQ, 0, device_isr, 0);
    irq_enable(DEVICE_IRQ);
}

int main(){
    device_isr_installer();

    while (1) {
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
