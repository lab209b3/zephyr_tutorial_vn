SETUP DEVICE TREE FILE
======================

Các giá trị cần biết khi config một timer
-----------------------------------------

.. table:: device tree

   +--------------------+--------------------+-----------------------------------------+
   | Name               | Type               | Details                                 |
   +====================+====================+=========================================+
   | ``reset``          | ``phandle-array``  |                                         |
   |                    |                    | Pin configuration/s for the first state |
   |                    |                    | .                                       |
   |                    |                    |                                         |
   |                    |                    | Content is specific to the selected     |
   |                    |                    | pin controller driver implementation.   |
   +--------------------+--------------------+-----------------------------------------+
   | ``st,prescaler``   | ``int``            |Clock prescaler at the input of the timer|
   |                    |                    |                                         |
   |                    |                    |Could be in range [0 .. 0xFFFF] for STM32| 
   |                    |                    |General Purpose Timers (CLK/(prescaler+1)| 
   |                    |                    |)                                        |
   +--------------------+--------------------+-----------------------------------------+
   | ``st,countermode`` | ``int``            |Sets timer counter mode.                 |              
   |                    |                    |                                         |
   |                    |                    |Use constants defined in dt-bindings/    |
   |                    |                    |timer/stm32-timer.h.                     |
   |                    |                    |                                         |
   |                    |                    |STM32_TIM_COUNTERMODE_UP: used as up     |
   |                    |                    |counter.                                 |
   |                    |                    |                                         |
   |                    |                    |STM32_TIM_COUNTERMODE_DOWN: used as      |
   |                    |                    |downcounter.                             |
   |                    |                    |                                         |      
   |                    |                    |STM32_TIM_COUNTERMODE_CENTER_DOWN: c     |
   |                    |                    |ounts up and down alternatively.         |
   |                    |                    |Output compare interrupt flags of output |
   |                    |                    |channels are set only when the counter   |
   |                    |                    |is counting down.                        |
   |                    |                    |                                         |
   |                    |                    |STM32_TIM_COUNTERMODE_CENTER_UP:         |
   |                    |                    |counts up and down alternatively.        |
   |                    |                    |Output compare interrupt flags of        |
   |                    |                    |output channels are set only when the    |
   |                    |                    |counter is counting up.                  |
   |                    |                    |                                         |      
   |                    |                    |STM32_TIM_COUNTERMODE_CENTER_UP_DOWN:    |
   |                    |                    |counts up and down alternatively.        |
   |                    |                    |Output compare interrupt flags of        |
   |                    |                    |output channels                          |
   |                    |                    |are set only when the counter            |
   |                    |                    |is counting up or down.                  |
   |                    |                    |                                         |   
   |                    |                    |                                         |   
   |                    |                    |                                         |   
   |                    |                    |                                         |
   |                    |                    |                                         |
   |                    |                    |                                         |
   +--------------------+--------------------+-----------------------------------------+
   | ``reset-names``    | ``string-array``   | Name of each reset                      |
   +--------------------+--------------------+-----------------------------------------+

Device tree for timer
---------------------

Đoạn code dưới đây là một ví dụ cho việc viết device tree cho TIM2, với chức năng là PWM

.. code-block:: c
   
   &timers2 {
      status = "okay";

      pwm2: pwm {
         status = "okay";
         pinctrl-0 = <&tim2_ch1_pa0>;
         pinctrl-names = "default";
      };
   };

Tương tự với ví dụ với TIM3, với presclaer = 10000, PWM tại PB4

.. code-block:: c
   &timers3 {
      st,prescaler = <10000>;
      status = "okay";

      pwm3: pwm {
         status = "okay";
         pinctrl-0 = <&tim3_ch1_pb4>;
         pinctrl-names = "default";
      };
   };

Config timer in Kconfig
-----------------------

