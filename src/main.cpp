#include "EspNow.hpp"
#include <Arduino.h>
#include <WiFi.h>


// Целевой MAC адрес
//constexpr EspNow::Mac target_mac = {0x00, 0x4B, 0x12, 0x38, 0x8D, 0x00};
constexpr EspNow::Mac target_mac = {0xFC, 0xE8, 0xC0, 0x74, 0xA6, 0x30};

// Структура пакета
struct __attribute__((packed)) Packet {
    uint32_t timestamp;
};


// Обработчик приёма данных
void onReceive(const EspNow::Mac &mac, const void *data, int size) {
    Serial.printf("(onReceive) [%s]: ", EspNow::toString(mac).data());

    // Проверяем размер пакета
    if (size != sizeof(Packet)) {
        Serial.printf("Неверный размер пакета: %d\n", size);
        return;
    }

    // Интерпретируем сырые данные как пакет
    const auto &packet = *static_cast<const Packet *>(data);

    Serial.printf(" : %u ms\n", packet.timestamp);
}

// Обработчик на отправку данных
void onSend(const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
    printf("(onDelivery) [%s]: %s\n", EspNow::toString(mac).data(), EspNow::toString(status));
}

void setup() {
    Serial.begin(115200);

    delay(1000);

    WiFiClass::mode(WIFI_STA);

    auto &e = EspNow::instance();

    e._on_receive = onReceive;
    e._on_delivery = onSend;

    while (true) {
        auto result = e.init();
        if (result == EspNow::InitResult::Ok) { break; }
        Serial.printf("Error: %s\n", EspNow::toString(result));
        delay(500);
    }

    auto self_mac = EspNow::mac();
    Serial.printf("Self: [%s]\n", EspNow::toString(self_mac).data());

    auto result = EspNow::addPeer(target_mac);
    Serial.println(EspNow::toString(result));
}

void loop() {
    Packet packet = {.timestamp = millis()};

    Serial.printf("Packet{%u} : Sending: ... ", packet.timestamp);

    auto result = EspNow::send(target_mac, packet);
    Serial.println(EspNow::toString(result));

    delay(5000);
}