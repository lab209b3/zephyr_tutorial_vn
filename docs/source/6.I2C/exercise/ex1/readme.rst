Exercise 1
++++++++++

Mục tiêu của Exercise 1 sẽ cho các bạn làm quen với thao tác cài đặt i2c để giao
tiếp với lcd 1602.

Các bạn sẽ làm theo hướng dẫn của doc từ ``app.overlay`` cho đến source
``main.c``, thư viện lcd 1602 sẽ do mình soạn và có sẵn, các bạn đọc qua thư
viện có gì không hiểu thì có thể hỏi mình.

Để có thể thực hiện được Exercise này, các bạn copy toàn bộ folder ``ex1`` và
pass vào trong folder ``zephyrproject`` trong máy của mình hoặc các bạn có thể
copy vào workspace của riêng mình. Nếu build hoặc chạy không đúng ý thì có thể
hỏi mình.

Hiện tại thư viện này chỉ hoạt động theo các setup bằng struct ``i2c_dt_spec``.
