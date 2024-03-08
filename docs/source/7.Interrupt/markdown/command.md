$$
\begin{array}{|c|c|c|}
\hline
   \textbf{''} & \textbf{ Tiêu đề } & \textbf{Thư mục cha} \\ 
\hline
   \text{''} & \text{7.5 Danh sách lệnh} & \text{7. Ngắt - Interrupts} \\
\hline
\end{array}
$$

Các API liên quan đến ngắt sau được cung cấp bởi tệp irq.h:

| Lệnh              | Mô tả     |
| ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IRQ_CONNECT        | Khởi tạo một trình xử lý ngắt. Hàm này khởi tạo một  ngắt cho một IRQ. IRQ phải được kích hoạt sau đó trước khi trình xử lý ngắt bắt đầu phục vụ các ngắt.                                                     |
| IRQ_DIRECT_CONNECT | Khởi tạo một trình xử lý ngắt 'trực tiếp'. Hàm này khởi tạo một trình xử lý ngắt cho một IRQ. IRQ phải được kích hoạt sau đó thông qua irq_enable() trước khi trình xử lý ngắt bắt đầu phục vụ các ngắt.                            |
| ISR_DIRECT_HEADER  | Công việc chung trước khi thực thi một ISR. Macro này phải ở đầu của tất cả các ngắt trực tiếp và thực hiện các nhiệm vụ tối thiểu trước khi ISR chính thức ngắt. Nó không chấp nhận đối số và không trả về giá trị nào. |
| ISR_DIRECT_FOOTER  | Công việc  một ISR. Macro này phải ở cuối của ngắt trực tiếp và thực hiện các nhiệm vụ cụ thể như EOI. Nó không trả về giá trị.                                                     |
| ISR_DIRECT_PM      | Logic Power Management       |
| ISR_DIRECT_DECLARE | Macro hỗ trợ để khai báo  ngắt trực tiếp.                                                                                                                                                                                   |
| irq_lock()         | Khóa ngắt. Hàm này vô hiệu hóa tất cả các ngắt trên CPU.                                                                                                                                                                             |
| irq_unlock()       | Mở khóa ngắt.                              |
| irq_enable()       | Kích hoạt một IRQ.  |
| irq_disable()      | Vô hiệu hóa một IRQ.                                                                                              |
| irq_is_enabled()   | Lấy trạng thái kích hoạt của IRQ.                           |


Các API liên quan đến ngắt sau được cung cấp bởi kernel.h:

| Lệnh               | Mô tả                                           |
| --------------------- | ----------------------------------------------- |
| k_is_in_isr()         | Xác định xem có đang ngắt hay không.      |
| k_is_preempt_thread() | Xác định xem ngắt có gián đoạn Thread không. |
