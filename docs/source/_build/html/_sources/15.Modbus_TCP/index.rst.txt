.. modbus_tcp documentation master file, created by Thanh Binh
   

Modbus - TCP/IP
===============
Bài viết đề cập tới một ứng dụng đơn giản việc triển khai hệ thống máy chủ Modbus TCP trên nền tảng Zephyr RTOS.

---------------------------------------------------------------------------
Yêu cầu về phần cứng
---------------------------------------------------------------------------
- Hướng dẫn này được chạy trên board STM32F746G_Disco.
- Máy tính được sử dụng Ubuntu 23.10 (VMware Workstation Pro) và phần mềm Modbus Poll 5.0.1.
  
---------------------------------------------------------------------------
Building and Running
---------------------------------------------------------------------------
- Code mẫu có thể được tìm thấy với đường dẫn `samples/subsys/modbus/tcp_server <https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/subsys/modbus/tcp_server/src/main.c>`__
- Code mẫu được thử trên mạch FRDM-K64F và sẽ lỗi khi rebuild trên bo STM32F746G_Disco. Ta cần thêm 2 dòng lệnh sau vào file ``prj.conf``
  
.. code-block:: bash
    
    CONFIG_ENTROPY_GENERATOR = y
    CONFIG_TESTS_RANDOM_GENERATOR = y

- Cài đặt địa chỉ IP của thiết bị & máy tính trong ``prj.conf``
  
.. code-block:: bash

   CONFIG_NET_CONFIG_MY_IPV4_ADDR = "192.168.1.220" //địa chỉ thiết bị
   CONFIG_NET_CONFIG_PEER_IPV4_ADDR = "192.168.1.107" //địa chỉ máy tính

- Đồng thời, xóa bỏ 2 led1 & led2 trong ``main.c/gpio_dt_spec led_dev[]`` do bo chỉ có 1 led0~~.

- Nạp thoi ~
  
.. code-block:: bash

   west build -b stm32f746g_disco 
   west flash

- Kết nối jack RJ45 với máy tính có cài Modbus Poll và thiết lập kết nối ở Connection/Connect.
  
.. code-block:: bash

   Connection -> Modbus TCP/IP
   Remote Server -> IP Address  = 192.168.1.220, Port = 502

.. image:: img/img2.png
  :width: 400
  :align: center


---------------------------------------------------------------------------
Đọc/ghi Coil
---------------------------------------------------------------------------
- Các led được mô phỏng như trạng thái các Coil được đánh địa chỉ từ 0 đến ``n led – 1``,có thể mở rộng thêm các GPIO khác để tạo nhiều Coil hơn thông qua ``app.overlay``. Tác động bật/tắt hay đọc trạng thái các Led (Coil) thông qua phần mềm Modbus Poll với FC theo bảng sau:

.. list-table:: 
   :widths: 25 50
   :header-rows: 1
   :align: center

   * - Function Code (Hex)
     - Chức năng
   * - 0x01
     - Đọc trạng thái coil
   * - 0x05
     - Ghi trạng thái 1 coil 
   * - 0x0F
     - Ghi trạng thái nhiều coil 

.. code-block:: bash

   Setup -> Read/Write Definition
   Slave ID -> 1

.. image:: img/img4.png
  :width: 400
  :align: center


---------------------------------------------------------------------------
Đọc/ghi thanh ghi
---------------------------------------------------------------------------
- Cũng như việc đọc trạng thái các coil để người điều khiển nắm được trạng thái của các thiết bị được bật/tắt trong hệ thống, người sử dụng còn cần đọc được các giá trị của các cảm biến, điện áp, công suất hay các cài đặt của 1 thiết bị trên mạng MODBUS. MODBUS cũng hỗ trợ các chức năng đọc/ghi như sau: 

.. list-table:: 
   :widths: 25 50
   :header-rows: 1
   :align: center

   * - Function Code (Hex)
     - Chức năng
   * - 0x03
     - Đọc giá trị thanh ghi
   * - 0x06
     - Ghi giá trị 1 thanh ghi
   * - 0x10
     - Ghi giá trị nhiều thanh ghi

.. code-block:: bash

   Setup -> Read/Write Definition
   Slave ID -> 1

.. image:: img/img3.png
  :width: 400
  :align: center

---------------------------------------------------------------------------
Tài liệu tham khảo:
---------------------------------------------------------------------------
Chuẩn MODBUS: https://www.modbustools.com/modbus.html

Modbus TCP server: https://docs.zephyrproject.org/latest/samples/subsys/modbus/tcp_server/README.html

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
