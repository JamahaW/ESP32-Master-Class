#include "Arduino.h"


const int PIN_JOY_X = 32;   // Аналоговый пин для оси X (ADC1)
const int PIN_JOY_Y = 33;   // Аналоговый пин для оси Y (ADC1)
const int PIN_JOY_BTN = 15; // Цифровой пин для кнопки (с подтяжкой)

void setup() {
    pinMode(PIN_JOY_X, INPUT);
    pinMode(PIN_JOY_Y, INPUT);
    pinMode(PIN_JOY_BTN, INPUT_PULLUP); // Включение внутренней подтяжки к VCC

    Serial.begin(115200);
}

void loop() {
    int x = analogRead(PIN_JOY_X);
    int y = analogRead(PIN_JOY_Y);
    bool b = digitalRead(PIN_JOY_BTN); // LOW при нажатии

    Serial.printf("x: %d\ty: %d\tb: %s\n", x, y, b ? "HIGH" : "LOW");

    delay(100);
}