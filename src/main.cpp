#include <Arduino.h>

#include <GyverOLED.h>


GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

// Глобальная переменная, хранит последнее значение потенциометра
volatile int pot = 0;

// Флаг управления дисплеем
volatile bool is_oled_on = true;


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

    bool current_power_state = true;

    while (true) {
        // Проверяем, нужно ли изменить состояние питания
        if (is_oled_on != current_power_state) {
            oled.setPower(is_oled_on);
            current_power_state = is_oled_on;
        }

        // Если питание включено и обновление разрешено, обновляем дисплей
        if (is_oled_on) {
            oled.clear();
            oled.setCursor(0, 0);
            oled.printf("Pot:%d", pot);
            oled.update();
        }

        delay(100); // 10 Гц
    }
}

// Задача мигания светодиодом
void blink(void *) {
    const int pin = 13;
    pinMode(pin, OUTPUT);

    while (true) {
        // Вычисляем полупериод
        int half_period = map(pot, 0, 4095, 50, 500);

        digitalWrite(pin, LOW);
        delay(half_period);
        digitalWrite(pin, HIGH);
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
    is_oled_on = !is_oled_on;
    Serial.printf("Click: %s\n", is_oled_on ? "On" : "Off");
}

void setup() {
    Serial.begin(115200);

    // Создаем задачи
    xTaskCreate(potUpdate, "PotUpdate", 2048, nullptr, 1, nullptr);
    xTaskCreate(oledUpdate, "OledUpdate", 2048, nullptr, 1, nullptr);
    xTaskCreate(blink, "Blink", 2048, nullptr, 1, nullptr);

    // Настройка кнопки
    const int pin_button = 14;
    pinMode(pin_button, INPUT_PULLDOWN);
    attachInterrupt(pin_button, onClick, RISING);
}

void loop() {}