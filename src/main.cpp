// Занятие 2.1 - Базовое управление кнопкой
#include <Arduino.h>


// Определение пинов: светодиод и кнопка
const auto pin_led = 26, pin_button = 14;

void setup() {
    // Настройка режимов пинов
    pinMode(pin_led, OUTPUT);
    pinMode(pin_button, INPUT_PULLUP);  // Используем встроенную подтяжку к VCC
}

void loop() {
    // Чтение состояния кнопки и управление светодиодом
    bool button_state = digitalRead(pin_button);
    digitalWrite(pin_led, button_state);

    delay(100);
}