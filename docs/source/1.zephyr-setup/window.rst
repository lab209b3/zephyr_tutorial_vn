Window
===============================================================================

.. contents::
    :local:
    :depth: 2

.. note::

    Tên user bắt buộc không có khoảng trắng ở giữa

Bước 1: Cài đặt các tool phụ thuộc
---------------------------------------------------------------------------

-   Mở PowerShell[Win10]/Terminal[Win11] với quyền **Administrator**.
-   Nhập lệnh

.. code-block:: bat

    Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

-   Check:

.. code-block:: bat

    Choco -version

-   Tắt cửa sổ PowerShell[Win10]/Terminal[Win11] , và mở lại với quyền **Administrator**.

-   Nhập lệnh:

.. code-block:: bat

    choco feature enable -n allowGlobalConfirmation

.. code-block:: bat

    choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'

.. code-block:: bat

    choco install ninja gperf python311 git dtc-msys2 wget 7zip

-   Sau khi cài đặt tắt cửa sổ.


Bước 2: Cài môi trường ảo và các *dependencies* của zephyr project
---------------------------------------------------------------------------

.. code-block:: bat

    cd C:\Users\dongkhoa

.. note::

    ``dongkhoa`` là tên user thay đổi lại cho phù với máy tính của bạn.

-   Tạo môi trường ảo:

.. code-block:: bat

    python -m venv zephyrproject\.venv

-   Kích hoạt môi trường ảo:

.. code-block:: bat

    zephyrproject\.venv\Scripts\activate.bat

**Sau khi kích hoạt môi trường ảo thì shell sẽ có tiền tố** ``(.venv)``. Để 
thoát khỏi chế độ kích hoạt sử dụng lệnh ``deactivate``:

.. code-block:: bat

    (.venv) C:\Users\dongkhoa

.. note::

    Luôn kích hoạt môi trường ảo trước khi bắt đầu làm việc với zephyr.

-   Cài đặt ``west``

.. code-block:: bat

    pip install west

-   Lấy source code của zephyr về máy tính:

.. code-block:: bat

    west init zephyrproject
    cd zephyrproject
    west update

-   Giải nén các gói **CMake Zephyr**:

..  code-block:: bat

    west zephyr-export

..  code-block:: bat

    pip install -r C:\Users\dongkhoa\zephyrproject\zephyr\scripts\requirements.txt


Bước 3: Cài Zephyr SDK
---------------------------------------------------------------------------

-   Mở PowerShell[Win10]/Terminal[Win11] với quyền **User**.

.. code-block:: bat

    cd C:\Users\dongkhoa

-   Tải ``Zephyr SDK``:

.. code-block:: bat

    wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/zephyr-sdk-0.16.5_windows-x86_64.7z

-   Giải nén ``Zephyr SDK``:

.. code-block:: bat

    7z x zephyr-sdk-0.16.5_windows-x86_64.7z

-   Chạy file setup ``Zephyr SDK``:

.. code-block:: bat

    cd zephyr-sdk-0.16.5
    setup.cmd

Bước 4: Build một project sample
---------------------------------------------------------------------------

-   Trở về thư mục *Project*:

.. note::

    Lưu ý phải ở trạng thái .venv

.. code-block:: bat

    (.venv) C:\User\dongkhoa\zephyrproject\zephyr

-   Chọn một project sample:

.. code-block:: bat

    cd ./samples/basic/blinky

-   Chọn Board để build:

**Các board mà zephyr hổ trở:** `Supported Boards <https://docs.zephyrproject.org/latest/boards/index.html#boards>`_.

.. code-block:: bat

    west build -p always -b <your-board-name>

**Example: nếu bạn sử dụng board:** ``STM32F746G_DISCO`` thì ghi như sau:

.. code-block:: bat

    west build -p always -b stm32f746g_disco

**Nếu bạn thêm lệnh** ``set(BOARD <your-board-name>)`` **trong file CMakeLists.txt trong project thì chỉ cần ghi:** ``west build``

-   Nạp vào board:

.. code-block:: bat

    west flash