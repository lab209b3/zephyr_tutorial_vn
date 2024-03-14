Yêu cầu
=======

Cần phải kết nối nút nhấn vào chân GPIO ta muốn sử dụng. 

Cấu hình app.overlay
====================

.. block-code::

   / {
	buttons {
		compatible = "gpio-keys";
		button0: button_0 {
			gpios = <&gpioa 1 (GPIO_PULL_UP | GPIO_ACTIVE_HIGH)>;
			label = "User button";
		};
	};
	aliases {
		sw0 = &button0;
	};
   };   

Ta chọn nút nhấn:
   * Port = gpioa
   * Pin = 1
   * Flags = GPIO_PULL_UP | GPIO_ACTIVE_HIGH

Code trên dùng cho board ``stm32f103c8t6``. Nếu muốn chạy trên board khác thì vào ``CMakeLists.txt``
và sửa ``set(BOARD <your_board>)``. Kiểm tra led và button trên devicetree rồi có thể chỉnh sửa tùy ý. Sau đó build và flash lên board.

Ta cài đặt nút nhấn là ngắt cạnh xuống. Khi nút nhấn được giữ thì led sáng, khi thả nút nhấn thì led sẽ tắt.

