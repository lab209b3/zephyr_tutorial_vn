Giới thiệu
===============================================================================

.. contents::
    :local:
    :depth: 2

Cấu trúc của một project
*******************************************************************************

-   Cấu trúc của một project cơ bản có những phần chính như sau:

.. code-block:: bash

    src
    └── main.c
    CMakeLists.txt
    prj.conf
    app.overlay

-   file ``app.overlay`` có thể có hoặc không do file này dùng để chèn thêm các config 
    cho board mà mặc định zephyr không hổ trợ.
-   file ``main.c`` có thể không cần để trong folder ``src``.

.. note::

    -   Đây chỉ là những file bắt buộc phải có trong một project để chạy project 
        này **bắt buộc** phải để trong folder ``zephyrproject``.
    -   Chỉ có thể chạy những board và chip được zephyr hổ trợ.

Tạo một project đầy đủ
*******************************************************************************

.. note:: 

    Cách tạo project này dựa trên một project mẫu mà zephyr cung cấp nên cần phải 
    custom lại những chỗ cần thiết *(như là thêm bớt folder)*


Bước 1: Khởi tạo workspace folder (*my-workspace*)
-------------------------------------------------------------------------------

.. note::
    
    - Trước khi khởi tạo workspace folder hãy active virtual environment:
    
    ``source ~/zephyrproject/.venv/bin/activate``.
    
    - Dung lượng nơi lưu trử  > 1GB.


-   Chạy command như sau:

.. code-block:: bash

    # initialize my-workspace for the example-application (main branch)
    west init -m https://github.com/zephyrproject-rtos/example-application --mr main my-workspace

Bước 2: Chỉnh sửa lại file set up thư viện cho project
-------------------------------------------------------------------------------

-   Mở thư mục file ``my-workspace/west.yml``. Mặc định nó sẽ như sau:

.. code-block:: yaml

    # Copyright (c) 2021 Nordic Semiconductor ASA
    # SPDX-License-Identifier: Apache-2.0

    manifest:
    self:
        west-commands: scripts/west-commands.yml

    remotes:
        - name: zephyrproject-rtos
          url-base: https://github.com/zephyrproject-rtos

    projects:
        - name: zephyr
          remote: zephyrproject-rtos
          revision: main
           import:
            # By using name-allowlist we can clone only the modules that are
            # strictly needed by the application.
            name-allowlist:
            - cmsis      # required by the ARM port
            - hal_nordic # required by the custom_plank board (Nordic based)
            - hal_stm32  # required by the nucleo_f302r8 board (STM32 based)

-   Do ở đẩy sử dụng board STM32F746G_Disco, Nên chỉ cần ``cmsis`` và ``hal_stm32`` => xóa ``hal_nordic`` (*dùng cho board nrf*).

Bước 3: Update thư viện cho project và build, flash trong project
-------------------------------------------------------------------------------

-   Chạy command sau:

.. code-block:: bash
    
    # update Zephyr modules
    cd my-workspace
    west update

-   Vào project có trong workspace:

.. code-block:: bash
    
    cd ./example-application

..  note::
    Đây là project mẫu không dành cho tất cả các board, thay đổi lại code 
    bên trong theo project của mình, có thể xóa những folder không cần trong 
    project.

-   Build Project:

.. code-block:: bash
    
    west build -b $BOARD app

-   Flash vào chip:

.. code-block:: bash
    
    west flash

Bước 4: Giải thích các folder chính có trong project
-------------------------------------------------------------------------------

*   ``boards``: Chứa các file config dành cho board mà zephyr không hổ trợ sẳn.
*   ``drivers``: Viết thư viện dành cho các driver chứa có thư viện như button...
*   ``dts``: Viết device tree cho chip mà zephyr không hổ trợ sẳn
*   ``include``: Chứa các file *header*
*   ``lib``: Chứa các file *.c*
*   ``app``: Chứa file *main.c*. folder này có chức năng chính dùng để build project (do có file CMakeLists.txt để 
    liên kết với các thư viện của zephyr).

Bước 5: Một số custom cở bản cho project
-------------------------------------------------------------------------------

Chỉnh sửa lại tên project
```````````````````````````````````````````````````````````````````````````````

-   Sửa lại tên folder project *(Mặc định là:* ``example-application`` *).*
-   Tiếp theo mở thư mục file ``.west/config``. Chỉnh sửa ``path`` lại theo tên 
    folder project.
-   Sau đó update lại bằng lệnh: ``west update``.

Tải thêm thư viện ngoài (không phải là thư viện chuẩn do zephyr phát triển)
```````````````````````````````````````````````````````````````````````````````

-   Mở thư mục file ``my-workspace/west.yml``. ở ví dụ này mình sẽ thêm hai thư 
    viện là ``lvgl`` và ``mbedtls``:

.. code-block:: yaml

    # Copyright (c) 2021 Nordic Semiconductor ASA
    # SPDX-License-Identifier: Apache-2.0

    manifest:
    self:
        west-commands: scripts/west-commands.yml

    remotes:
        - name: zephyrproject-rtos
          url-base: https://github.com/zephyrproject-rtos

    projects:
        - name: zephyr
          remote: zephyrproject-rtos
          revision: main
           import:
            # By using name-allowlist we can clone only the modules that are
            # strictly needed by the application.
            name-allowlist:
            - cmsis      # required by the ARM port
            - hal_stm32  # required by the STM32 board
        - name: lvgl
          remote: zephyrproject-rtos
          revision: 2b76c641749725ac90c6ac7959ca7718804cf356
          path: modules/lvgl
        - name: mbedtls
          remote: zephyrproject-rtos
          revision: 6ec4abdcda78dfc47315af568f93e5ad4398dea0
          path: modules/mbedtls

-   Sau đó update lại bằng lệnh: ``west update``.