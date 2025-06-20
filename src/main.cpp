#include "EspNow.hpp"
#include <Arduino.h>
#include <WiFi.h>

/// Логирование операций EspNow
#define log(__EspNow_api_Result_func) ({auto result = __EspNow_api_Result_func; Serial.printf(#__EspNow_api_Result_func " -> %s\n", EspNow::toString(result));})

/// Адрес сервера
static constexpr EspNow::Mac server = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};

/// Тип пакета сообщения от сервера
using ServerMessage = std::array<char, 64>;

/// Тип пакета сообщения от клиента
using ClientMessage = std::array<char, 16>;

/// Тип пакета команды от клиента
struct [[gnu::packed]] ClientCommand {
    uint8_t x, y;
};

/// Запуск сервера
void runServer(EspNow &esp_now) {
    auto on_delivery = [](const EspNow::Mac &mac, EspNow::DeliveryStatus status) {

    };

    auto on_receive = [](const EspNow::Mac &mac, const void *data, int size) {

    };

    log(esp_now.setDeliveryHandler(on_delivery));
    log(esp_now.setReceiveHandler(on_receive));
}

/// Запуск клиента
void runClient(EspNow &esp_now) {
    auto on_delivery = [](const EspNow::Mac &mac, EspNow::DeliveryStatus status) {

    };

    auto on_receive = [](const EspNow::Mac &mac, const void *data, int size) {

    };

    log(esp_now.setDeliveryHandler(on_delivery));
    log(esp_now.setReceiveHandler(on_receive));

    log(EspNow::addPeer(server));
}

void setup() {
    Serial.begin(115200);
    WiFiClass::mode(WIFI_STA);

    log(EspNow::init());

    auto &esp_now = EspNow::instance();
    const bool is_server = esp_now.mac == server;

    Serial.printf(
        "Self: %s (Role: %s)\n",
        EspNow::toString(esp_now.mac).data(),
        is_server ? "Server" : "Client"
    );

    if (is_server) {
        runServer(esp_now);
    } else {
        runClient(esp_now);
    }
}

void loop() {}