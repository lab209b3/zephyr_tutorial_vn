I2C
+++

.. contents::
    :local:
    :depth: 2

1. I2C setup
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

Trong đoạn mã trên, ``&i2c1`` chỉ định I2C cụ thể (có thể thay đổi tùy thuộc
vào thiết bị bạn đang sử dụng), ``clock-frequency`` đặt tốc độ ``I2C_BITRATE_FAST``,
và ``status`` được đặt thành ``okay`` để bật I2C.

Biến ``clock-frequency`` gồm các giá trị có thể thay đổi sau:

* ``I2C_BITRATE_STANDARD``:     100kbps
* ``I2C_BITRATE_FAST``:         400kbps
* ``I2C_BITRATE_FAST_PLUS``:    1000kbps

Tùy thuộc vào tốc độ i2c mong muốn mà mình có thể cài đặt đè lên các giá trị có
trong file ``dts`` của zephyr, bằng cách viết vào ``app.overlay`` như sau:

.. code-block:: bash

    &i2c1 {
        clock-frequency = <I2C_BITRATE_STANDARD>;
    };

Đoạn code trên được viết vào file ``app.overlay`` trong folder project của mình,
giá trị ``clock-frequency`` đã được chuyển thành ``I2C_BITRATE_STANDARD``, so
với giá trị mặc định là ``I2C_BITRATE_FAST``.

.. note::
   Lưu ý rằng chúng ta nên biết tên i2c chúng ta sẽ sử dụng trên thiết bị của
   mình, ở đây ta sẽ dùng i2c1 nên trong ``app.overlay``, chúng ta sẽ sử dụng
   ``&i2c1``, nếu xài i2c2 thì sẽ xài ``&i2c2``, để biết tên i2c của thiết 
   bị, chúng ta có thể vào file ``.dts`` của board của mình trong 
   ``zephyrproject/zephyr/boards/...``.

Bước 2: Yêu cầu Kconfig File cho I2C 
____________________________________

.. code-block::

    CONFIG_I2C=y

Bước 3: Set up I2C trong source
_______________________________

Có 2 cách để setup I2C trong source:

* Set up dùng ``struct i2c_dt_spec``.
* Set up dùng ``struct device``.

Setup dùng ``struct i2c_dt_spec``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Bước 1: Cấu hình I2C target trong ``app.overlay``
`````````````````````````````````````````````````

.. code-block:: bash

    &i2c1
    {
        my_lcd: my_lcd@27
        {
            compatible = "i2c-device";
            reg = <0x27>;
            label = "LCD_1602";
        };
    };

Đầu tiên, xác định i2c của thiết bị mà chúng ta sẽ sử dụng, sau đó tạo ra môt 
subnode - node con ``my_lcd``, có thành viên là ``my_lcd@27``, ``@27`` là địa
chỉ I2C của thiết bị này.

Tiếp theo, giá trị của ``my_lcd@27``, chúng ta chỉ cần
``reg = <0x27>``, còn các biến còn lại có thể có hoặc không.

.. note::
   Lưu ý rằng, địa chỉ I2C ở đây chúng ta sẽ sử dụng giá trị lấy thẳng từ
   datasheet, **không được** shift trái, phải.

Chúng ta có thể tạo thêm nhiều thiết bị trong node ``&i2c1`` này, tương đương
với nhiều slaver cho một i2c controller, tuy nhiên, chúng ta không thể lấy
subnode của i2c này xài cho i2c khác được, một là chúng ta copy qua i2c khác,
hai là đổi tên ``&i2c1`` này thành i2c mong muốn.

Bước 2: Cấu hình trong source
`````````````````````````````

Các thư viện phải include:

.. code-block:: c

    #include <sys/_stdint.h>
    #include <zephyr/device.h>
    #include <zephyr/kernel.h>
    #include <zephyr/drivers/i2c.h>

Đầu tiên, chúng ta sẽ nạp các giá trị của i2c target trong ``app.overlay`` vào
struct ``i2c_dt_spec``, sử dụng ``I2C_DT_SPEC_GET(node_id)``.

.. code-block:: c

    #define I2C1_NODE DT_NODELABEL(my_lcd)

    const struct i2c_dt_spec i2c_dev = I2C_DT_SPEC_GET(I2C1_NODE);

Để pass vào param ``node_id``, chúng ta sẽ sử dụng ``DT_NODELABEL(my_lcd)``, lưu
ý chúng ta muốn trỏ đến subnode nào đã được cài đặt ở ``app.overlay`` trên thì
**viết đúng** tên của subnode đó.

Bước 3: Giao tiếp I2C
`````````````````````
Để viết vào i2c target chúng ta sẽ sử dụng ``i2c_write_dt``.

.. code-block:: c

    int main(void)
    {
        uint8_t data = 0x10;
        i2c_write_dt(&i2c_dev, &data, 1);
    }

Hàm ``i2c_write_dt`` gồm có 3 param:

* ``*spec``: trỏ đến ``stuct i2c_dt_spec`` của chúng ta.
* ``*buf``: trỏ đến biến/array chứa các giá trị cần truyền.
* ``num_bytes``: số bytes cần phải truyền.

Để đọc từ i2c target chúng ta sẽ sử dụng ``i2c_read_dt``.

.. code-block:: c

    int main(void)
    {
        uint8_t data;
        i2c_read_dt(&i2c_dev, &data, 1);
    }

Hàm ``i2c_write_dt`` gồm có 3 param:

* ``*spec``: trỏ đến ``stuct i2c_dt_spec`` của chúng ta.
* ``*buf``: trỏ đến biến/array chứa các giá trị cần nhân.
* ``num_bytes``: số bytes cần phải nhân.

Để cả truyền và nhận chúng ta dùng ``i2c_write_read_dt``.

.. code-block:: c

    int main(void)
    {
        uint8_t write_data = 0x12;
        uint8_t read_data;

        i2c_write_read_dt(&i2c_dev, &write_data, 1, &read_data, 1);
    }


Hàm ``i2c_write_dt`` gồm có 3 param:

* ``*spec``: trỏ đến ``stuct i2c_dt_spec`` của chúng ta.
* ``*write_buf``: trỏ đến biến/array chứa các giá trị cần truyện.
* ``num_write``: số bytes cần phải truyền.
* ``*read_buf``: trỏ đến biến/array chứa các giá trị cần truyền.
* ``num_read``: số bytes cần phải nhận.

Setup dùng ``struct device``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

2. Bài tập
==========

.. toctree::
   :maxdepth: 1

   exercise/ex1/readme
