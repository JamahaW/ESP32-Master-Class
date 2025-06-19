#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>


#pragma pack(push, 1)  // Гарантированное выравнивание без паддинга

struct Message {
    char text[32];
    int32_t counter;

    void print() const {
        Serial.printf("Message{.text=\"%s\", .counter=%d}", text, counter);
    }
};

#pragma pack(pop)

using u8 = uint8_t;

void printMac(const uint8_t *mac) {
    Serial.printf(
        "[%02X:%02X:%02X:%02X:%02X:%02X]",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );
}

void onDataReceive(const u8 *mac, const u8 *data, int size) {
    printMac(mac);

    if (size != sizeof(Message)) {
        Serial.println(" (Неопределенный формат)");
        for (int i = 0; i < size; i += 1) {
            Serial.printf("%02X ", data[i]);
            if (i % 6 == 1) { Serial.println(); }
        }
        Serial.println();

        return;
    }

    const auto &msg = *reinterpret_cast<const Message *>(data);
    msg.print();
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Не удалось инициализировать ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(onDataReceive);
    Serial.println("Start!");
}

void loop() {}