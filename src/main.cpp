/*
 * Занятие 2.
 * Задача 2 - Использование встроенных ёмкостных датчиков
 * Управление светодиодом с помощью встроенной сенсорной кнопки
 */

/*
T0: GPIO 4
T1: GPIO 0  (Boot pin - осторожно!)
T2: GPIO 2  (Встроенный LED)
T3: GPIO 15
T4: GPIO 13
T5: GPIO 12
T6: GPIO 14
T7: GPIO 27
T8: GPIO 33
T9: GPIO 32
*/

#include <Arduino.h>


const int pin_led = 26, touch_pin = 14;

void setup() {
    pinMode(pin_led, OUTPUT);

    Serial.begin(115200);
    Serial.println("Старт!");
}

/*

 Калибровка порога
 - Запустите код и считайте значения без касания
 - Коснитесь сенсора и запишите новое значение
 - Установите порог на 20-30% ниже минимального значения при касании

 */

void loop() {
    // Пороговое значение
    const int threshold = 30;

    int value = touchRead(touch_pin);
    bool touched = value < threshold;

    digitalWrite(pin_led, touched);

    Serial.printf("Value: %d\tTouched: %s\n", value, (touched ? "Yes" : "No"));

    delay(100);
}