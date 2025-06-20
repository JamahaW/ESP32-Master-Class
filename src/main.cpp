#include "EspNow.hpp"
#include <Arduino.h>
#include <WiFi.h>


// Структура пакета
struct __attribute__((packed)) Packet {
    uint32_t timestamp;
};

// Обработчик приёма данных
void onReceive(const EspNow::Mac &mac, const void *data, int size) {
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
        if (result.fail()) { Serial.printf("error: %s\n", EspNow::toString(result)); }

        auto &e = EspNow::instance();
        e.setDeliveryHandler(onDelivery);
        e.setReceiveHandler(onReceive);
    }

    Serial.printf("Self: [%s]\n", EspNow::toString(EspNow::mac()).data());

    // Отправка широковещательного
    {
        Packet packet = {.timestamp = millis()};
        auto result = EspNow::send(EspNow::broadcast, packet);
        Serial.println(EspNow::toString(result));
    }
}

void loop() {}