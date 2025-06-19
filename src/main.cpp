#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "array"

/// Целевой MAC адрес
constexpr std::array<uint8_t, 6> target_mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Структура пакета
struct __attribute__((packed)) Packet {
    uint32_t timestamp;
};

// Упрощённая инициализация ESP NOW
bool initEspNow() {
    if (ESP_FAIL == esp_now_init()) { return false; }

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, target_mac.data(), 6);

    return esp_now_add_peer(&peer) == ESP_OK;
}

// Обработчик приёма данных
void handleReceive(const uint8_t *mac, const uint8_t *data, int size) {
    // Проверяем размер пакета
    if (size != sizeof(Packet)) {
        Serial.printf("Неверный размер пакета: %d\n", size);
        return;
    }

    // Интерпретируем сырые данные как пакет
    const auto &packet = *reinterpret_cast<const Packet *>(data);

    // Используем
    Serial.printf(
        "[%02X:%02X:%02X:%02X:%02X:%02X]: %u ms\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        packet.timestamp
    );
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());

    while (not initEspNow()) {
        Serial.println("ESP-NOW init failed!");
        delay(1000);
    }

    esp_now_register_recv_cb(handleReceive);
}

void loop() {
    Packet packet = {.timestamp = millis()};

    Serial.printf("Sending: Packet{%u} ... ", packet.timestamp);

    esp_err_t result = esp_now_send(
        target_mac.data(),
        reinterpret_cast<uint8_t *>(&packet),
        sizeof(packet)
    );

    Serial.println(ESP_FAIL == result ? "Fail" : "Ok");

    delay(5000);
}