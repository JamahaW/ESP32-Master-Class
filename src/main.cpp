#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

void onDataReceive(const uint8_t *mac, const uint8_t *data, int size) {
    auto message = reinterpret_cast<const char*>(data);

    Serial.printf(
        "[%02X:%02X:%02X:%02X:%02X:%02X]: '%s' (%d B)\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        message, size
    );
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init fail");
        return;
    }

    esp_now_register_recv_cb(onDataReceive);
    Serial.println("Start!");
}

void loop() {}