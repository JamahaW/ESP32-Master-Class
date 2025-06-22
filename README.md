# Задание 2.1: Базовое управление кнопкой

## Цель: Научиться читать состояние кнопки и управлять светодиодом.

```cpp
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
```

## Теория:

> ### Настроить режим работы GPIO-пина:
> ```cpp
> pinMode(
>    uint8_t pin, // GPIO
>    uint8_t mode // Режим*
> ) -> void
> ```

_Режимы:_

- `INPUT`: Вход без подтяжки
- `INPUT_PULLUP`: Вход с подтяжкой к `VCC`
- `INPUT_PULLDOWN`: Вход с подтяжкой к `GND` (доступно не для всех пинов)
- `OUTPUT`: Цифровой выход

---

> ### Считать текущее логическое состояние пина:
> ```cpp
> digitalRead(uint8_t pin) -> int
> ```

_Интерпретация возвращаемого значения:_

- `HIGH` (`1`): Напряжение > 2.5V
- `LOW` (`0`): Напряжение < 1.0V

---
