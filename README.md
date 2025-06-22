# Задание 4.2

## Сканирование Wi-Fi сетей

---

### Цель: Научиться обнаруживать доступные Wi-Fi сети.

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

## Теория

---

```cpp
WiFi.scanNetworks() -> int16_t
```

Возвращает количество найденных сетей

---

```cpp
WiFi.SSID(uint8_t i) -> String
```

Возвращает название i-той сети

---

```cpp
WiFi.RSSI(i) -> int32_t
```

Возвращает уровень сигнала сети

---