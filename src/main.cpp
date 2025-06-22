// Занятие 2.8 - ШИМ через ledcWrite
#include <Arduino.h>


const auto pin_led = 26;

const auto pwm_channel = 0;       // Канал ШИМ (0-15)
const auto pwm_frequency = 5000;  // Частота 5 КГц
const auto pwm_resolution = 8;    // Разрешение 8 бит

void setup() {
    // Настройка канала ШИМ
    ledcSetup(pwm_channel, pwm_frequency, pwm_resolution);
    ledcAttachPin(pin_led, pwm_channel);  // Привязка пина к каналу
}

void loop() {
    const auto max_value = (1 << pwm_resolution) - 1;

    // Плавное изменение яркости
    for (int i = -max_value; i < max_value; i++) {
        auto pwm_value = max_value - abs(i);
        ledcWrite(pwm_channel, pwm_value);  // Управление через LEDC
        delay(1000 / max_value);
    }
}