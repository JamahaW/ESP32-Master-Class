# Задание 2.8: ШИМ через ledcWrite

## Цель: Освоить продвинутое управление ШИМ с помощью LEDC.

```cpp
const auto pin_led = 26;

const auto pwm_channel = 0;       // Канал ШИМ (0-15)
const auto pwm_frequency = 5000;  // Частота 5 КГц
const auto pwm_resolution = 8;    // Разрешение 8 бит

void setup() {
    // Настройка канала ШИМ
    ledcSetup(pwm_channel, pwm_frequency, pwm_resolution);
    ledcAttachPin(pin_led, pwm_channel);  // Привязка пина к каналу
}

void loop() {
    const auto max_value = (1 << pwm_resolution) - 1;

    // Плавное изменение яркости
    for (int i = -max_value; i < max_value; i++) {
        auto pwm_value = max_value - abs(i);
        ledcWrite(pwm_channel, pwm_value);  // Управление через LEDC
        delay(1000 / max_value);
    }
}
```

## Теория:

---

### Настраивает канал ШИМ
```cpp
ledcSetup(
    uint8_t channel,            // Номер канала (0..15)
    uint32_t freq,              // Частота в Гц
    uint8_t resolution_bits     // Разрешение (1..20 бит)
) -> void
```

---

### Привязывает пин к каналу ШИМ
```cpp
ledcAttachPin(uint8_t pin, uint8_t channel) -> void
```

---

### Устанавливает коэффициент заполнения для канала
```cpp
ledcWrite(uint8_t channel, uint32_t duty) -> void
```
