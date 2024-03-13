I2C
+++

.. contents::
    :local:
    :depth: 2

1. Giới thiệu I2C 
==================


2. I2C setup
============

Khi thiết lập bất kỳ I2C nào, phải tuân theo các bước cơ bản sau:

Bước 1: Cấu hình Device Tree cho I2C 
____________________________________

- File Device Tree có thể được tìm thấy theo đường dẫn: ``zephyrproject/zephyr/boards/...``. 
- Trong dự án này, board sử dụng là STM32F746G_Disco, đường dẫn đến file Device Tree tương ứng: ``zephyrproject/zephyr/boards/arm/stm32f746g_disco.dts``.

.. code-block:: bash

    &i2c1 {
        pinctrl-0 = <&i2c1_scl_pb8 &i2c1_sda_pb9>;
        pinctrl-names = "default";
        status = "okay";
        clock-frequency = <I2C_BITRATE_FAST>;
    };

- Trong đoạn mã trên, ``&i2c1`` chỉ định I2C cụ thể (có thể thay đổi tùy thuộc
  vào thiết bị bạn đang sử dụng), ``clock-frequency`` đặt tốc độ ``I2C_BITRATE_FAST``,
  và ``status`` được đặt thành ``okay`` để bật I2C.


Bước 2: Yêu cầu Kconfig File cho I2C 
____________________________________

.. code-block::

    CONFIG_I2C=y

Bước 3: Set up I2C trong source
_______________________________

Các thư viện phải include:

.. code-block:: c

    #include <sys/_stdint.h>
    #include <zephyr/device.h>
    #include <zephyr/drivers/i2c.h>

Tiếp theo, device binding với node lable trong dts

.. code-block:: c

    #define I2C_NODE DT_NODELABEL(i2c1)
    const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

Toàn bộ code phía sau sẽ bỏ vào hàm ``main()``.
Sau khi đã binding với device, chúng ta sẽ config i2c

.. code-block:: c

    i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD)|I2C_MODE_CONTROLLER);

Hàm ``i2c_configure``, nhận 2 param là:

* ``dev``: Là pointer tới struct device chúng ta vừa tạo ở trên.
* ``dev_config``: Một gói 32 bit để config I2C, ở đây chúng ta sẽ dùng các flag
  được zephyr define sẵn.

Ở ví dụ trên chúng ta dùng 2 flag là ``I2C_MODE_CONTROLLER`` và
``I2C_SPEED_SET(I2C_SPEED_STANDARD)``:

* ``I2C_MODE_CONTROLLER`` sẽ cài đặt I2C là mode controller.
* ``I2C_SPEED_SET(I2C_SPEED_STANDARD)`` để cài đặt speed cho clock là
  ``100khz``.

Sau khi cài đặt xong, chúng ta có thể truyền dữ liệu:

.. code-block:: c

    uint8_t data = 1;
    uint16_t addr = 0X0034;
    i2c_write(i2c_dev, &data, 1, addr); 

Hàm ``i2c_write`` nhận các param sau:

* ``dev``: Pointer tới struct device của I2C.
* ``buf``: Pointer tới biến dữ liêu để truyền đi.
* ``num_bytes``: Số bytes phải truyền.
* ``addr``: Địa chỉ I2C dữ liệu ``uint16_t``.

4. Bài tập
==========

**Điều khiển LCD 1602 thông qua I2C**

- ``Code mẫu: exercise/I2C/index.rst``
