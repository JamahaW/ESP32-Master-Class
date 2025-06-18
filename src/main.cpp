#include <Arduino.h>

#include <GyverOLED.h>


GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

// Глобальная переменная, хранит последнее значение потенциометра
volatile int pot = 0;
// Квалификатор volatile гарантирует работу атомарными операциями

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

void setup() {
    Serial.begin(115200);

    // Пытаемся создать задачу для потенциометра
    BaseType_t result = xTaskCreate(
        // Функция, выполняемая задачей
        potUpdate,
        // Наименование задачи для дальнейшей отладки
        "PotUpdate",
        // Размер стека для этой задачи
        2048,
        // Параметры (Не передаём)
        nullptr,
        // Приоритет задачи (= 1 - Выше, чем IDLE)
        1,
        // Возвращаемый дескриптор (Не используем)
        nullptr
    );

    // Если не удалось создать задачи (Не хватает памяти)
    if (pdFAIL == result) {
        Serial.println("Не удалось создать задачу 'PotUpdate'");
        return;
    }
    // Если задачу удалось создать, то она будет добавлена в очередь задач
    // и она будет выполнена как только планировщик задач выделит процессорное время

    // Аналогично, но кратко
    if (pdFAIL == xTaskCreate(oledUpdate, "OledUpdate", 2048, nullptr, 1, nullptr)) {
        Serial.println("Не удалось создать задачу 'OledUpdate'");
        return;
    }
}

void loop() {}