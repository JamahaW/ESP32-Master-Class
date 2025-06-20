#include "EspNow.hpp"
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "array"


// Целевой MAC адрес
constexpr std::array<uint8_t, 6> target_mac = {0x00, 0x4B, 0x12, 0x38, 0x8D, 0x00};
//constexpr std::array<uint8_t, 6> target_mac = {0xFC, 0xE8, 0xC0, 0x74, 0xA6, 0x30};

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

// Функция для отображения MAC адреса
void printMac(const uint8_t *mac) {
    Serial.printf(
        "[%02X:%02X:%02X:%02X:%02X:%02X]",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );
}

// Обработчик приёма данных
void onReceive(const uint8_t *mac, const uint8_t *data, int size) {
    // Проверяем размер пакета
    if (size != sizeof(Packet)) {
        Serial.printf("Неверный размер пакета: %d\n", size);
        return;
    }

    // Интерпретируем сырые данные как пакет
    const auto &packet = *reinterpret_cast<const Packet *>(data);

    printMac(mac);
    Serial.printf(" : %u ms\n", packet.timestamp);
}

// Обработчик на отправку данных
void onSend(const uint8_t *mac, esp_now_send_status_t status) {
    printMac(mac);
    Serial.printf(" : %s ms\n", ESP_NOW_SEND_SUCCESS == status ? "Ok" : "Fail");
}

void setup() {
    Serial.begin(115200);

    delay(1000);

    WiFi.mode(WIFI_STA);
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());

    while (not initEspNow()) {
        Serial.println("ESP-NOW init failed!");
        delay(1000);
    }

    // Регистрируем обработчики событий (На отправку и на доставку)
    esp_now_register_recv_cb(onReceive);
    esp_now_register_send_cb(onSend);
}

void loop() {
    Packet packet = {.timestamp = millis()};

    Serial.printf("Packet{%u} : Sending: ... ", packet.timestamp);

    esp_err_t result = esp_now_send(
        target_mac.data(),
        reinterpret_cast<uint8_t *>(&packet),
        sizeof(packet)
    );

    Serial.println(ESP_FAIL == result ? "Fail" : "Ok");

    delay(1000);
}