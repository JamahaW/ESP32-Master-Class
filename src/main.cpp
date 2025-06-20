#include "EspNow.hpp"
#include <Arduino.h>
#include <WiFi.h>


// Структура пакета
struct __attribute__((packed)) Packet {
    uint32_t timestamp;
};

/// Широковещательный адрес
//static constexpr EspNow::Mac target = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static constexpr EspNow::Mac target = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};
//static constexpr EspNow::Mac target = {0xFC, 0xE8, 0xC0, 0x74, 0xA6, 0x30};

// Обработчик приёма данных
void onReceive(const EspNow::Mac &mac, const void *data, int size) {
    Serial.printf(
        "Received %d bytes from %s\n",
        size,
        EspNow::toString(mac).data()
    );
}

// Обработчик на отправку данных
void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
    Serial.printf("[%s] -> %s\n", EspNow::toString(mac).data(), EspNow::toString(status));
}

void setup() {
    Serial.begin(115200);

    // Инициализация
    {
        WiFiClass::mode(WIFI_STA);

        auto result = EspNow::init();
        Serial.printf("init: %s\n", EspNow::toString(result));

        auto &e = EspNow::instance();
        Serial.println(EspNow::toString(e.setDeliveryHandler(onDelivery)));
        Serial.println(EspNow::toString(e.setReceiveHandler(onReceive)));
    }

    Serial.printf("Self: [%s]\n", EspNow::toString(EspNow::mac()).data());

    {
        auto result = EspNow::addPeer(target);
        Serial.printf("add peer: %s\n", EspNow::toString(result));
    }
}

void loop() {
    Packet packet = {
        .timestamp = millis()
    };

    auto result = EspNow::send(target, packet);
    Serial.println(EspNow::toString(result));

    delay(1000);
}