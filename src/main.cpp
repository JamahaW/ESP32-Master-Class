#include <Arduino.h>


// Задача для мигания
[[noreturn]] void blink(void *args) {
    auto *p = static_cast<std::pair<int, int> *>(args);

    const int pin = p->first, period = p->second / 2;

    delete p;

    pinMode(p->first, OUTPUT);

    while (true) {
        digitalWrite(pin, HIGH);
        delay(period);
        digitalWrite(pin, HIGH);
        delay(period);
    }
}

void setup() {
    // Размер стека для задач избыточен
    const int stack_depth = 4096;

    // Создание задачи
    xTaskCreate(
        // Указатель на функцию, которая будет работать на задачу
        blink,
        // Имя задачи не обязательно, но можно использовать для отладки
        "blink 1",
        // Указываем глубину стека для задачи
        stack_depth,
        // передача параметров задачу
        new std::pair<int, int>(4, 1000),
        // Приоритет задачи = 0 (как у setup, loop)
        0,
        // Handler задачи не возвращаем
        nullptr
    );

    // Handler второй задачи
    TaskHandle_t blink_2 = nullptr;

    // Аналогично создаём задачу
    xTaskCreate(
        blink,
        nullptr,
        stack_depth,
        new std::pair<int, int>(15, 1000 / 3),
        0,
        // Передаём handler этой задачи в blink_2
        &blink_2
    );

    // ждём 5 секунд и удаляем задачу
    delay(5000);
    vTaskDelete(blink_2);
}

void loop() {

}