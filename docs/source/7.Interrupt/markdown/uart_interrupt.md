$$
\begin{array}{|c|c|c|}
\hline
   \textbf{''} & \textbf{ Tiêu đề } & \textbf{Thư mục cha} \\ 
\hline
   \text{''} & \text{7.4 Ngắt UART} & \text{7. Ngắt - Interrupts} \\
\hline
\end{array}
$$
>>>***Tựa:*** ***Bài viết này giả sử người đọc đã đọc qua Chương 5.UART***
##  Tổng quan
Trong Zephyr, có 3 cách khác nhau để truy cập Ngoại vi UART, thông qua việc sử dụng API khác nhau: Polling, Interrupts-Driven và asynchronous-không đồng bộ.

* Polling là cách cơ bản nhất để truy cập ngoại vi UART. Hàm đọc `uart_poll_in()`, là một hàm không thể bị chặn và trả về 1 ký tự, -1 khi không hợp lệ. Hàm ghi `uart_poll_out()`, là một hàm block, các luồng khác sẽ phải đợi cho đến khi ký tự đã được gửi
* API Không đồng bộ Asynchronous là cách hiệu quả nhất để sử dụng UART, nó cho phép đọc và ghi dữ liệu ở chế độ nền bằng EasyDMA.
* Với Ngắt, ISR của trình điều khiển UART sẽ quản lý dữ liệu, trong khi chương trình vẫn có thể thao tác với function khác. Một vài tính năng truyền dữ liệu của Kernel như FIFO có thể áp dụng trong trường hợp này.

## Tổng quan Ngắt UART
- Việc nhận dữ liệu liên tục trong vòng while như vậy khiến cho MCU không thể làm được việc khác được. Vì vậy, chúng ta sẽ sử dụng chức năng ngắt UART để nhận dữ liệu. Ở trong vòng lập while(1) chúng ta sẽ làm nhiệm vụ cố định nào đó, khi có dữ liệu đến chân Rx của UART thì sẽ nhảy vào hàm ngắt UART để nhận dữ liệu. Sau khi thực hiện xong tác vụ ngắt, MCU sẽ quay về chương trình chính trong vòng lập while(1) để lại hoạt động bình thường.
- Trong Zephyr, để sử dụng ngắt UART, cần định cấu hình ngắt UART cho phần cứng cụ thể và sau đó xác định các hàm xử lý ngắt để xử lý dữ liệu khi ngắt xảy ra. Điều này thường được thực hiện thông qua các API và hàm được cung cấp bởi Zephyr.

## Một số hàm liên quan
1. `uart_irq_callback_set`
```C
static inline void uart_irq_callback_set(const struct device *dev, uart_irq_callback_user_data_t cb)
```
- Chức năng: Đặt chức năng Callback của ngắt IRQ

| Tên biến |      Kiểu     |       Tác dụng     |
|----|-------------------|-------------------|
| dev | `const struct device*` | Con trỏ tới trình điều khiển UART| 
| cb | `uart_irq_callback_user_data_t` | Hàm Callback | 

```C
static void func_irq(const struct device *dev, void *user_data)
```
* Hàm trên sẽ được gọi khi phần cứng UART tạo ra ngắt

2. `uart_irq_callback_user_data_set`
```C
static inline void uart_irq_callback_user_data_set(const struct device *dev, uart_irq_callback_user_data_t cb, void *user_data)
```
* Đặt chức năng gọi lại IRQ và hỗ trợ truyền tham số `*user_data`

3. `uart_irq_tx_enable`
```C
void uart_irq_tx_enable(const struct device *dev)
```
* Bật Cờ TX của thanh ghi IER (Thanh ghi kích hoạt ngắt), cho phép tạo ngắt trên chân TX

4. `uart_irq_rx_enable`
```C
void uart_irq_rx_enable(const struct device *dev)
```
* Bật Cờ RX của thanh ghi IER (Thanh ghi kích hoạt ngắt), cho phép tạo ngắt trên chân RX

5. `uart_irq_tx_disable`
6. `uart_irq_rx_disable`
7. `uart_irq_update`
```C
int uart_irq_update(const struct device *dev)
```
* Cập nhật BIT cờ ngắt, trả về 1 nếu thành công, 0 nếu thất bại
8. `uart_irq_tx_ready`
```C
static inline int uart_irq_tx_ready(const struct device *dev)
```
* Kiểm tra BIT TX Ready có đang bằng 1 không, có ý nghĩa là kiểm tra xem bộ đệm đầu ra có dữ liệu mới - có thể xuất ra hay không.
* Trả về 1 nếu có dữ liệu mới xuất ra.

> **Nói chung**:
> Khi sử dụng các chức năng liên quan tới IRQ, Zephyr có một số yêu cầu trong xử lý ngắt:
> - Trước tiên `uart_irq_update` được gọi để cập nhật thanh ghi cờ ngắt
> - Sau đó `uart_irq_tx_ready` hoặc `uart_irq_rx_ready` được gọi để kiểm tra xem đầu vào và đầu ra có hợp lệ hay không
> - Kế đó mới đọc và ghi liên quan tới các hoạt động.

> **Lưu ý**:
> `uart_irq_callback_set` khác với `uart_callback_set`.
> * `uart_irq_callback_set` được sử dụng để đặt lệnh Callback cho các ngắt phần cứng
> * Trong khi đó `uart_callback_set` được sử dụng cho các sự kiện Callback nội bộ của trình điều khiển UART Zephyr.
> 
>Ví dụ, khi trình điều khiển UART Ghi dữ liệu vào một thiết bị, nó sẽ tạo ra một thông báo cho sự kiện DONE, tức là sự kiện ghi đã hoàn thành, sau đó gọi lại hàm và truyền sự kiện vào đó, có thể dùng để truyền tiếp dữ liệu mới.

>>Để sử dụng IRQ UART cần có: `CONFIG_UART_INTERRUPT_DRIVEN=y`

Ngoài ra có một số hàm quan trọng như:
```C
static inline int uart_fifo_fill(const struct device *dev, const uint8_t *tx_data, int size)
static inline int uart_fifo_read(const struct device *dev, const uint8_t *rx_data, const int size)
```
## Phương pháp cơ bản
1. Một số Cờ Config cần được bật:
```
CONFIG_SERIAL=y
CONFIG_GPIO=y
CONFIG_UART_INTERRUPT_DRIVEN=y
```
2. Lấy thiết bị giao tiếp UART từ DeviceTree
```C
#define UART1_NODE DT_NODELABEL(usart1)
const struct device *uart_0 = DEVICE_DT_GET(UART1_NODE);
```
3. Hàm tạo ngắt:

Trong Zephyr, để sử dụng ngắt UART, ta cần thiết lập một hàm gọi lại (callback function) để xử lý sự kiện xảy ra khi có dữ liệu được nhận hoặc truyền qua UART. Đầu tiên, ta cần đăng ký hàm gọi lại này với trình điều khiển UART, thường được thực hiện thông qua hàm `uart_irq_callback_set()` hoặc `uart_irq_callback_user_data_set()`.
```C
uart_irq_callback_user_data_set(uart_0, callback_function, NULL);
uart_irq_rx_enable(uart_0);
```
Như đã nhắc tới ở trên, ở đây tạo cờ ngắt trên RX.

4. Hàm xử lý sự kiện ngắt:
```C
void callback_function(const struct device *dev, void *user_data)
{
    // Xử lý sự kiện UART
}
```
Thông thường trong hàm này có sử dụng các hàm như:
```C
uart_irq_update(uart_0)
uart_fifo_read(uart_0, &c, 1);
```
Dùng để đọc giá trị từ UART

>> Xem thêm: [Chương 5.UART](https://github.com/lab209b3/zephyr_tutorial_vn/tree/master/docs/source/5.UART) 