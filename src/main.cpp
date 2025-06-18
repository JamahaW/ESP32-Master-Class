/*
 * Занятие 3
 * Задание 2.2 - Задача мигания светодиода и рефакторинг
 */

#include <Arduino.h>
#include <GyverOLED.h>


GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

// Глобальная переменная, хранит последнее значение потенциометра
volatile int pot = 0;

// Задача потенциометра
void potUpdate(void *) {
    const int pin_pot = 4;
    pinMode(pin_pot, INPUT);

    while (true) {
        pot = analogRead(pin_pot);
        delay(10);
    }
}

// Задача дисплея
void oledUpdate(void *) {
    oled.init();
    oled.setScale(2);

    while (true) {
        oled.clear();
        oled.setCursor(0, 0);
        oled.printf("Pot:%d", pot);
        oled.update();
        delay(100); // 10 Гц
    }
}

// Задача мигания светодиода
void blink(void *) {
    const int pin = 13;
    pinMode(pin, OUTPUT);

    while (true) {
        digitalWrite(pin, HIGH);
        delay(500);
        digitalWrite(pin, LOW);
        delay(500);
    }
}

void setup() {
    Serial.begin(115200);

    // Создаём задачи
    xTaskCreate(potUpdate, "PotUpdate", 2048, nullptr, 1, nullptr);
    xTaskCreate(oledUpdate, "OledUpdate", 2048, nullptr, 1, nullptr);
    xTaskCreate(blink, nullptr, 2048, nullptr, 1, nullptr);
    // Поскольку размер стека мал, а количество задач небольшое, в целях лаконичность обработкой ошибок можно пренебречь
}

void loop() {}