/*
 * Занятие 2.
 * Задача 4 - analogRead
 */

#include <Arduino.h>


const int pin_led = 26, pin_potentiometer = 33;

const auto adc_resolution = 8; // Разрешение АЦП (1..16)

void setup() {
    pinMode(pin_led, OUTPUT);

    // Изменить разрешение АЦП
    analogReadResolution(adc_resolution);

    Serial.begin(115200);
    Serial.println("Старт!");
}

void loop() {
    int value = analogRead(pin_potentiometer);
    analogWrite(pin_led, value);

    Serial.printf("ADC: %d\n", value);

    delay(1000);
}