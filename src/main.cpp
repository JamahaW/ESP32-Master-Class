// Занятие 4.4 - Прием сообщений от Telegram-бота

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