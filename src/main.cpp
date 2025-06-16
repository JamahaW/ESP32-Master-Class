#include <Arduino.h>


// Реализуем Функцию-помощник так как требуется создавать однотипные примитивные задачи
TaskHandle_t taskCreateHelper(const char *task_name, TaskFunction_t code, void *parameters) {
    // Размер стека для задач избыточен
    const int stack_depth = 4096;
    // Обычный приоритет для пользовательских задач
    const int default_task_priority = 0;

    // Возвращаемый дескриптор задачи (До попытки создания задачи предполагаем, что не удалось)
    TaskHandle_t ret = nullptr;

    // Пытаемся создать задачу
    BaseType_t result = xTaskCreate(
        // Передаём указатель на функцию, чтобы указать участок кода для выполнения задачей
        code,
        // Передаём наименование задачи для удобства отладки во время выполнения
        task_name,
        // Указываем размер стека для задачи
        stack_depth,
        // Передаём параметры для этой задачи
        parameters,
        // Указываем приоритет задачи
        default_task_priority,
        // Получаем дескриптор задачи
        &ret
    );
    // После создания задачи её выполнение начнётся сразу

    // Результатом может быть pdPASS (= 1) или pdFAIL (= 0)
    if (result == pdPASS) {
        Serial.printf("Успешно создана и запущена задача: %s\n", task_name);
    } else {
        Serial.printf("Не удалось создать задачу: %s\n", task_name);
    }

    // Если xTaskCreate возвращает pdPASS, то ret будет установлен в действительный дескриптор задачи.
    // Если xTaskCreate возвращает pdFAIL, то ret останется `nullptr`.

    return ret;
}

// Определим параметры для задачи
struct BlinkParameters {
    const int pin, period, iterations;
};

// Задача для мигания
void blink(void *parameters) {
    auto *p = static_cast<BlinkParameters *>(parameters);

    // Получаем имя этой задачи
    const char *name = pcTaskGetName(nullptr);

    pinMode(p->pin, OUTPUT);

    for (int i = 0; i < p->iterations; i++) {
        Serial.printf("Мигаем из задачи %s\n", name);

        digitalWrite(p->pin, HIGH);
        delay(p->period);
        digitalWrite(p->pin, LOW);
        vTaskDelay(p->period / portTICK_PERIOD_MS); // ровно то же самое, что и delay
    }

    // Удаляем ЭТУ задачу (nullptr сообщает об использовании контекста)
    vTaskDelete(nullptr);

    // Код ниже будет недоступен, т.к. выполнение задачи будет прекращено сразу же
}

void setup() {

    // Создаём параметры для первой задачи
    static BlinkParameters blink_1_parameters = {
        .pin = 4,
        .period = 1000,
        .iterations = 5
    };

    // Используем нашу функцию-помощник для создания и запуска задачи
    taskCreateHelper("Blink-1", blink, &blink_1_parameters);

    // аналогично и для второй задачи

    static BlinkParameters blink_2_parameters = {15, 1000 / 3, 15};
    taskCreateHelper("Blink-2", blink, &blink_2_parameters);
}

void loop() {

}