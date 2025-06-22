// Занятие 4.1 - Подключение к Wi-Fi сети
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