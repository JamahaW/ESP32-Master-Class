#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>


struct Message : Printable {
    char text[32]{};
    int32_t counter{};

    size_t printTo(Print &p) const override {
        return p.printf("Message{.text=\"%s\", .counter=%d}", text, counter);
    }
};

using u8 = uint8_t;

void printMac(const u8 *mac) {
    Serial.write('[');
    for (int i = 0; i < 6; i++) { Serial.printf("%02X:", mac[i]); }
    Serial.write(']');
}

void onDataReceive(const u8 *mac, const u8 *data, int size) {
    printMac(mac);

    if (size != sizeof(Message)) {
        Serial.println(" (Неопределенный формат)");
        for (int i = 0; i < size; i += 1) {
            Serial.printf("%02X", data[i]);
            if (i % 6 == 0) { Serial.println(); }
        }
        Serial.println();

        return;
    }

    const auto &msg = *reinterpret_cast<const Message *>(data);

    Serial.println(msg);
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_recv_cb(onDataReceive);
    Serial.println("Receiver ready. Waiting for messages...");
}

void loop() {}