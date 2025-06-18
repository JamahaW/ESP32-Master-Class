## Пины ESP32

| Пин на плате | GPIO | Тип       | Функции                          | Особые примечания                        |
|--------------|------|-----------|----------------------------------|------------------------------------------|
| 3V3          | —    | Power     | Выход 3.3V                       | Макс. ток 600 мА                         |
| VIN          | —    | Power     | Внешнее питание (5-10V)          | Через стабилизатор AMS1117-3.3V          |
| GND          | —    | Ground    | Общий провод                     | Несколько выводов на плате               |
| EN           | —    | Control   | Сброс/включение                  | Подтянут к VCC через 10 кОм; LOW = сброс |
| SVP (VP)     | 36   | Analog In | ADC1_CH0, RTC_GPIO0              | Только вход! Не поддерживает подтяжку    |
| SVN (VN)     | 39   | Analog In | ADC1_CH3, RTC_GPIO3              | Только вход! Не поддерживает подтяжку    |
| IO34         | 34   | Analog In | ADC1_CH6, RTC_GPIO4              | Только вход!                             |
| IO35         | 35   | Analog In | ADC1_CH7, RTC_GPIO5              | Только вход!                             |
| IO32         | 32   | GPIO      | ADC1_CH4, RTC_GPIO9, Touch9      | Безопасен при загрузке                   |
| IO33         | 33   | GPIO      | ADC1_CH5, RTC_GPIO8, Touch8      | Безопасен при загрузке                   |
| IO25         | 25   | GPIO      | DAC1, ADC2_CH8, RTC_GPIO6        | Не использовать ADC2 с Wi-Fi             |
| IO26         | 26   | GPIO      | DAC2, ADC2_CH9, RTC_GPIO7        | Не использовать ADC2 с Wi-Fi             |
| IO27         | 27   | GPIO      | ADC2_CH7, RTC_GPIO17, Touch7     | Не использовать ADC2 с Wi-Fi             |
| IO14         | 14   | GPIO      | ADC2_CH6, RTC_GPIO16, Touch6     | Выводит ШИМ при загрузке                 |
| IO12         | 12   | GPIO      | ADC2_CH5, RTC_GPIO15, Touch5     | Должен быть LOW при загрузке!            |
| IO13         | 13   | GPIO      | ADC2_CH4, RTC_GPIO14, Touch4     | Безопасен при загрузке                   |
| IO15         | 15   | GPIO      | ADC2_CH3, RTC_GPIO13, Touch3     | Должен быть HIGH при загрузке!           |
| IO2          | 2    | GPIO      | Встроенный LED, ADC2_CH2, Touch2 | Подтянут к GND; LOW при загрузке         |
| IO0          | 0    | GPIO      | ADC2_CH1, Touch1, Boot mode      | Должен быть HIGH при загрузке!           |
| IO4          | 4    | GPIO      | ADC2_CH0, RTC_GPIO10, Touch0     | Безопасен при загрузке                   |
| IO16         | 16   | GPIO      | UART2_RX, RTC_GPIO6              | Безопасен при загрузке                   |
| IO17         | 17   | GPIO      | UART2_TX, RTC_GPIO7              | Безопасен при загрузке                   |
| IO5          | 5    | GPIO      | VSPI_CS0, ADC2_CH11              | Выводит ШИМ при загрузке                 |
| IO18         | 18   | GPIO      | VSPI_CLK                         | Безопасен при загрузке                   |
| IO19         | 19   | GPIO      | VSPI_MISO                        | Безопасен при загрузке                   |
| IO21         | 21   | GPIO      | Default SDA (I2C)                | Безопасен при загрузке                   |
| RX0          | 3    | UART      | UART0_RX (Debug)                 | HIGH при загрузке; избегать              |
| TX0          | 1    | UART      | UART0_TX (Debug)                 | Отладка при загрузке; избегать           |
| IO22         | 22   | GPIO      | Default SCL (I2C)                | Безопасен при загрузке                   |
| IO23         | 23   | GPIO      | VSPI_MOSI                        | Безопасен при загрузке                   |