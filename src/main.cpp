// Занятие 4.7 - Управление LED через Telegram

#include <WiFi.h>
#include <FastBot.h>
#include <GyverOLED.h>


const int pin_led = 2;

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

auto ssid = "ИМЯ_СЕТИ", password = "ПАРОЛЬ_СЕТИ";

FastBot bot("ТОКЕН_БОТА");

void handleMessage(FB_msg &msg) {
    oled.clear();
    oled.setCursor(0, 0);
    oled.print(msg.text);
    oled.update();

    if (msg.text == "/on") { digitalWrite(pin_led, HIGH); }
    if (msg.text == "/off") { digitalWrite(pin_led, LOW); }
}

void setup() {
    Serial.begin(115200);
    pinMode(pin_led, OUTPUT);
    oled.init();
    oled.autoPrintln(true);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    Serial.println("Connected");

    bot.attach(handleMessage);
}

void loop() { bot.tick(); }