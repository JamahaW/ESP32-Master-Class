/*
 * Занятие 2.
 * Задача 1
 * 1.	Подключение кнопки к ESP32 (обзор возможных вариантов).
 * Управление светодиодом с помощью тактовой кнопки INPUT, INPUT_PULLUP, INPUT_PULLDOWN.
 */

#include <Arduino.h>


const int pin_led = 26, pin_button = 14;

void setup() {
    pinMode(pin_led, OUTPUT);
    //                                                                              : 14 - [R 10K] - GND
//    pinMode(pin_button, INPUT);             // Без встроенное подтяжки            : 14 - [B] - 3V3
//    pinMode(pin_button, INPUT_PULLUP);      // Включить встроенную подтяжку в VCC : 14 - [B] - GND
//    pinMode(pin_button, INPUT_PULLDOWN);    // Включить встроенную подтяжку в GND : 14 - [B] - 3V3

    Serial.begin(115200);
    Serial.println("Старт!");
}

void loop() {
    bool state = digitalRead(pin_button);

    digitalWrite(pin_led, state);
    Serial.printf("State: %s\n", (state ? "HIGH" : "LOW"));

    delay(100);
}