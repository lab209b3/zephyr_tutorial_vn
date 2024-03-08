Ngắt Timer
==========

Trong một số trường hợp, khi timer được sử dụng để thực hiện một công việc phức tạp một cách định kỳ. Tuy nhiên, vì công việc này không thể hoàn thành ngay tại mức ngắt, nên khi timer hết thời gian, hàm được gọi là **“expiry function”** (hàm kết thúc thời gian của timer) không thực hiện công việc mà nó sẽ thực hiện. Thay vào đó, hàm này sẽ gửi một **“work item”** đến **“system workqueue”**. 

**System workqueue** là một cơ chế trong Zephyr để lập lịch và thực thi các công việc không thể hoàn thành ngay lập tức tại mức ngắt. Khi một work item được gửi đến workqueue, nó sẽ được thực hiện bởi một luồng của hệ thống (system thread). Việc sử dụng workqueue cho phép các công việc phức tạp hoặc tài nguyên tốn kém được thực hiện mà không làm gián đoạn các hoạt động quan trọng khác trong hệ thống. 

Cho nên việc gọi ``k_timer_start(&my_timer, TIMER_INTERVAL, TIMER_INTERVAL);`` có thể là một kiểu ngắt timer hoặc là không. 

.. Note:: Xem thêm: `Chương 4.Timer <https://github.com/lab209b3/zephyr_tutorial_vn/tree/master/docs/source/4.Timers>`__
