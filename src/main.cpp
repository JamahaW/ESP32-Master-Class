/*
 * Занятие 3
 * Задание 2.2 - Задача мигания светодиода и рефакторинг
 */

#include <Arduino.h>
#include <GyverOLED.h>


GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

// Глобальная переменная, хранит последнее значение потенциометра
volatile int pot = 0;

// Флаг для управления дисплеем
volatile bool oled_enabled = true;

// Задача потенциометра (Обновляет pot с частотой 100 Гц)
[[noreturn]] void potUpdate(void *) {
    const int pin_pot = 4;
    pinMode(pin_pot, INPUT);

    while (true) {
        pot = analogRead(pin_pot);
        delay(10);
    }
}

// Задача дисплея (Использует pot, обновляет дисплей с частотой 10 Гц)
[[noreturn]] void oledUpdate(void *) {
    // Предыдущее значение питания
    bool oled_last_enabled = oled_enabled;

    oled.init();
    oled.setScale(2);

    while (true) {

        // Если значение изменилось с прошлого раза, то обновляем и запоминаем
        if (oled_enabled != oled_last_enabled) {
            oled_last_enabled = oled_enabled;
            oled.setPower(oled_enabled);
        }

        if (oled_enabled) {
            oled.clear();
            oled.setCursor(0, 0);
            oled.printf("Pot:%d", pot);
            oled.update();
        }
        delay(100); // 10 Гц
    }
}

// Задача мигания светодиода (Использует pot для определения частоты мигания)
[[noreturn]] void blink(void *) {
    const int pin = 13;
    pinMode(pin, OUTPUT);

    while (true) {
        // Вычисляем полупериод пропорционально значению потенциометра
        int half_period = map(pot, 0, 4095, 50, 500);

        digitalWrite(pin, HIGH);
        delay(half_period);
        digitalWrite(pin, LOW);
        delay(half_period);
    }
}

// Обработчик прерывания кнопки
void IRAM_ATTR onClick() {
    const auto debounce = 200; // длительность дребезга контактов мс
    static auto last_click = millis(); // момент предыдущего вызова обработчика

    // Антидребезг
    if (millis() - last_click < debounce) { return; }
    last_click = millis();

    // Инвертируем состояние дисплея
    oled_enabled = not oled_enabled;
    Serial.printf("Click: %s\n", oled_enabled ? "On" : "Off");
}

void setup() {
    Serial.begin(115200);

    // Создаём задачи
    xTaskCreate(potUpdate, "PotUpdate", 2048, nullptr, 1, nullptr);
    xTaskCreate(oledUpdate, "OledUpdate", 2048, nullptr, 1, nullptr);
    xTaskCreate(blink, nullptr, 2048, nullptr, 1, nullptr);

    // Настройка кнопки
    const int pin_button = 14;
    pinMode(pin_button, INPUT_PULLDOWN);
    attachInterrupt(pin_button, onClick, RISING);
}

void loop() {}