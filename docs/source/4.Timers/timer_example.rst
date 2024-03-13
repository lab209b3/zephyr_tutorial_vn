Examples
========

Blink led_0 trên board STM332F746G_Disco step by step
-----------------------------------------------------

* Bước 1: Tạo một project với đủ các file như hình.

.. figure:: imagines/timer_example.png
   :align: center
   :alt: Priority
   :scale: 100%

* Bước 2: Viết file CMakeLists.txt giống với các sample cơ bản của zephyr.

.. code-block:: 

    # SPDX-License-Identifier: Apache-2.0

    cmake_minimum_required(VERSION 3.20.0)

    find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
    project(timer_test_v2)

    FILE(GLOB app_sources src/*.c)
    target_sources(app PRIVATE ${app_sources})

* Bước 3: Config cho board của bạn theo ứng dụng bạn muốn sử dụng.

.. code-block:: c
   
   CONFIG_PWM=y

   CONFIG_LED=y

   CONFIG_STDOUT_CONSOLE=y
   CONFIG_PRINTK=y
   CONFIG_LOG=y

* Bước 4: Hoàn thành đoạn code sau.

.. code-block:: c

    #include <zephyr/kernel.h>
    #include <zephyr/sys/printk.h>
    #include <zephyr/drivers/gpio.h>

    #define LED0_NODE DT_ALIAS(led0)

    static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

    struct k_timer my_timer;
    extern void my_expiry_function(struct k_timer *timer_id, bool led_state);

    void my_expiry_function(struct k_timer *timer_id, bool led_state)
    {
        gpio_pin_toggle_dt(&led);
        led_state = !led_state;
        printk("timeout %d\n", k_cycle_get_32());
    }

    int main(void)
    {
        bool led_state = true;

        k_timer_init(&my_timer, my_expiry_function, NULL);

        k_timer_start(&my_timer, K_MSEC(5000), K_MSEC(1000));
        
    }

* Bước 5: Quan sát kết quả.

