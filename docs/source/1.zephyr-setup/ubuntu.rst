Ubuntu
===============================================================================

.. contents::
    :local:
    :depth: 2

Bước 1: Cài đặt các tool phụ thuộc (CMake, Python, Devicetree compiler)
---------------------------------------------------------------------------
-   Nếu máy tính bạn cài đặt ubuntu version cũ hơn ``22.04`` thì cần cài đặt thêm 
    các gói sau:

.. code-block:: bash

    wget https://apt.kitware.com/kitware-archive.sh
    sudo bash kitware-archive.sh

-   Sử dụng ``apt`` để cài đặt các tool:

.. code-block:: bash

    sudo apt install --no-install-recommends git cmake ninja-build gperf \
    ccache dfu-util device-tree-compiler wget \
    python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
    make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1


-   Check lại xem đã cài được chưa:

.. code-block:: bash

    cmake --version
    python3 --version
    dtc --version


Bước 2: Cài môi trường ảo và zephyr project
---------------------------------------------------------------------------

-   Sử dụng ``apt`` để cài gói ``venv`` của Python:

.. code-block:: bash

    sudo apt install python3-venv

-   Tạo môi trường ảo: 

.. code-block:: bash

    python3 -m venv ~/zephyrproject/.venv

-   Activate môi trường ảo:

.. code-block:: bash

    source ~/zephyrproject/.venv/bin/activate

.. note::
    Lưu ý: mỗi khi muốn build hoặc flash zephyr project đều phải sử dụng lệnh này.

-   Cài đặt ``west``:

.. code-block:: bash
    
    pip install west

-   Lấy source code của zephyr về máy tính:

.. code-block:: bash
    
    west init ~/zephyrproject
    cd ~/zephyrproject
    west update

.. note::
    Lưu ý: đây là nguồn để học về các hàm sử dụng trong zephyr rất hiệu quả nên hãy cố gắng khai thác hết mức có thể.

-   Giải nén các gói **CMake Zephyr**:

.. code-block:: bash
    
    west zephyr-export

-   Cài đặt các requirement:

.. code-block:: bash
    
    pip install -r ~/zephyrproject/zephyr/scripts/requirements.txt


Bước 3: Cài Zephyr SDK
---------------------------------------------------------------------------

-   Tải và verify Zephyr SDK:

.. code-block:: bash

    cd ~
    wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/zephyr-sdk-0.16.5_linux-x86_64.tar.xz
    wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/sha256.sum | shasum --check --ignore-missing

-   Giải nén tệp vừa tải:

.. code-block:: bash

    tar xvf zephyr-sdk-0.16.5_linux-x86_64.tar.xz

-   Cài đặt ``Zephyr SDK``:

.. code-block:: bash

    cd zephyr-sdk-0.16.5
    ./setup.sh

Bước 4: Build một project sample
---------------------------------------------------------------------------

-   Chọn một project sample:

.. code-block:: bash

    cd ~
    cd ./zephyrproject/zephyr/samples/basic/blinky

-   Chọn Board để build:

**Các board mà zephyr hổ trở:** `Supported Boards <https://docs.zephyrproject.org/latest/boards/index.html#boards>`_.

.. code-block:: bash

    west build -p always -b <your-board-name>

**Example: nếu bạn sử dụng board:** ``STM32F746G_DISCO`` thì ghi như sau:

.. code-block:: bash

    west build -p always -b stm32f746g_disco

**Nếu bạn thêm lệnh** ``set(BOARD <your-board-name>)`` **trong file CMakeLists.txt trong project thì chỉ cần ghi:** ``west build``

-   Nạp vào board:

.. code-block:: bash

    west flash