# Задание 4.3

## Синхронизация времени по NTP

---

### Цель: Научиться получать точное время с NTP-сервера.

```cpp
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


auto ssid = "ИМЯ_СЕТИ", password = "ПАРОЛЬ_СЕТИ";

WiFiUDP udp;

NTPClient timeClient(udp, "pool.ntp.org", 3 * 3600);  // UTC+3

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    Serial.println("Connected!");

    timeClient.begin();
}

void loop() {
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    delay(5000);
}
```

## Пояснения

- `pool.ntp.org` - публичный NTP-сервер
- `3 * 3600` - смещение для часового пояса UTC+3

---

```cpp
client.getFormattedTime()
```

Возвращает время в формате HH:MM:SS
