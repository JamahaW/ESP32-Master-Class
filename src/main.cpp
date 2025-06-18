/*
 * Занятие 2.
 * Задача 3 - Управление яркостью светодиода с помощью ШИМ через analogWrite
 */

#include <Arduino.h>


const int pin_led = 26;

const int pwm_resolution_bits = 10;

void setup() {
    pinMode(pin_led, OUTPUT);

    // Меняем частоту ШИМ
    analogWriteFrequency(10000); // 10КГц
    // Изменяем разрешение ШИМ
    analogWriteResolution(pwm_resolution_bits);

    Serial.begin(115200);
    Serial.println("Старт!");
}

void loop() {
    // Амплитуда
    const int max_pwm = (1 << pwm_resolution_bits) - 1; // 2 ** pwm_resolution_bits - 1

    // Длительность шага
    const int step_ms = 1000 / max_pwm;

    for (int i = -max_pwm; i < max_pwm; i++) {
        // Вычисляем уровень заполнения шим по треугольной функции: /\/\/\/\/
        int pwm = max_pwm - abs(i);

        analogWrite(pin_led, pwm);

        Serial.printf("PWM: %d\n", pwm);
        delay(step_ms);
    }
}