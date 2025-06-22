// Занятие 4.2 - Сканирование доступных Wi-Fi сетей
#include <WiFi.h>


void setup() {
    Serial.begin(115200);

    int n = WiFi.scanNetworks();
    Serial.printf("Найдено %d сетей:\n", n);

    for (int i = 0; i < n; i++) {
        Serial.printf("%d: %s\n", i + 1, WiFi.SSID(i).c_str());
    }
}

void loop() {}