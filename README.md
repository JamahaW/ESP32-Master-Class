# Задание 4.1

## Подключение к Wi-Fi сети

---

### Цель: Научиться подключать ESP32 к существующей Wi-Fi сети.

```cpp
#include <WiFi.h>

auto ssid = "ИМЯ_СЕТИ", passphrase = "ПАРОЛЬ_СЕТИ";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, passphrase);

    while (WL_CONNECTED != WiFi.status()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nПодключено!");
}

void loop() {}
```

### Инструкция:

- Замените `ВАШ_SSID` и `ВАШ_ПАРОЛЬ` на реальные данные сети
- Загрузите скетч на ESP32
- Откройте монитор порта для наблюдения за процессом подключения