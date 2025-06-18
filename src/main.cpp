/*
 * Занятие 2.
 * Задача 3 - Управление яркостью светодиода с помощью ШИМ: analogWrite, ledcWrite
 */

#include <Arduino.h>


const int pin_led = 26;

void setup() {
    pinMode(pin_led, OUTPUT);

    Serial.begin(115200);
    Serial.println("Старт!");
}

void loop() {
    // Амплитуда
    const int max_pwm = 255;

    for (int i = -max_pwm; i <= max_pwm; i++) {
        // Вычисляем шим по треугольной функции
        int pwm = max_pwm - abs(i);

        analogWrite(pin_led, pwm);

        Serial.printf("PWM: %d\n", pwm);
        delay(4);
    }
}