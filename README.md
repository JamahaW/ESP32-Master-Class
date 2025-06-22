# Задание 4.4

## Создание Telegram-бота

---

### Цель: Настроить базового Telegram-бота для приема сообщений.

```cpp
#include <WiFi.h>
#include <FastBot.h>


auto ssid = "ИМЯ_СЕТИ", password = "ПАРОЛЬ_СЕТИ";

FastBot bot("ТОКЕН_БОТА");

void handleMessage(FB_msg &msg) {
    Serial.printf("От %s: %s\n", msg.username, msg.text);
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

## Инструкция по получению токена:

- Напишите `@BotFather` в Telegram
- Используйте команду `/newbot`
- Следуйте инструкциям для создания бота
- Скопируйте полученный токен в код
