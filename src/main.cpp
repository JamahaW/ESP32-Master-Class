// Занятие 4.6 - Вывод сообщений Telegram на OLED

#include <WiFi.h>
#include <FastBot.h>
#include <GyverOLED.h>


GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

auto ssid = "ИМЯ_СЕТИ", password = "ПАРОЛЬ_СЕТИ";

FastBot bot("ТОКЕН_БОТА");

void handleMessage(FB_msg &msg) {
    oled.clear();
    oled.setCursor(0, 0);
    oled.print(msg.text);
}

void setup() {
    Serial.begin(115200);
    oled.init();
    oled.autoPrintln(true);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    Serial.println("Connected");

    bot.attach(handleMessage);
}

void loop() { bot.tick(); }