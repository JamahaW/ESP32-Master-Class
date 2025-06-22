// Занятие 4.3 - Синхронизация времени по NTP

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


auto ssid = "ИМЯ_СЕТИ", password = "ПАРОЛЬ_СЕТИ";

WiFiUDP udp;

NTPClient timeClient(udp, "pool.ntp.org", 3 * 3600);  // UTC+3

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WL_CONNECTED != WiFi.status()) { delay(500); }
    Serial.println("Connected!");

    timeClient.begin();
}

void loop() {
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    delay(5000);
}