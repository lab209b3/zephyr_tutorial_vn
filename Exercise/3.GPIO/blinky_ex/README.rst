Yêu cầu
=======

Nếu chạy code trên board không có sẵn led thì ta cần phải kết nối chân
GPIO với led, tạo file ``app.overlay`` và cấu hình led đó trong. Ở ví dụ trên sử dụng led trên board (PC13)

Build và Run
============

Code trên dùng cho board ``stm32f103c8t6``. Nếu muốn chạy trên board khác thì vào ``CMakeLists.txt``
và sửa ``set(BOARD <your_board>)``. Kiểm tra led hỗ trợ cho board trên devicetree rồi có thể chỉnh sửa tùy ý. Sau đó build và flash lên board.

