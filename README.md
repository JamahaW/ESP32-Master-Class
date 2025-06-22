# Задание 4.5

## Эхо-бот в Telegram

---

### Цель: Создать бота, отвечающего на сообщения.

```cpp
#include <WiFi.h>
#include <FastBot.h>

auto ssid = "ИМЯ_СЕТИ", password = "ПАРОЛЬ_СЕТИ";

FastBot bot("ТОКЕН_БОТА");

void handleMessage(FB_msg &msg) {
    bot.sendMessage(msg.text, msg.chatID);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    Serial.println("Connected");

    bot.attach(handleMessage);
}

void loop() { bot.tick(); }
```

## Особенности:
- Бот отвечает тем же текстом, что прислал пользователь
- `message.chatID` содержит идентификатор чата
