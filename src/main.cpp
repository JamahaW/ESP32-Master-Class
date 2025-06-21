#include "game/impl/Client.hpp"
#include "game/impl/Host.hpp"
#include "game/core/Protocol.hpp"

#include <Arduino.h>

#include "WiFi.h"


/// Адрес сервера
const EspNow::Mac server_address = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};


/// Запуск сервера
[[noreturn]] void runServer() {
    game::core::Environment environment;

    game::impl::Host host(environment, Serial);

    host.init();

    while (true) {
        host.pull();
    }
}

/// Запуск клиента
[[noreturn]] void runClient() {
    game::impl::Client client(server_address, Serial);

    client.init();

    client.sendMessage(game::core::ClientMessage{"OriginalName"});
    delay(2000);
    client.sendMessage(game::core::ClientMessage{"OtherName"});

    while (true) {
        client.sendMove(game::core::ClientMove{123, 69});
        delay(5000);
    }
}

void setup() {
    Serial.begin(115200);

    WiFiClass::mode(WIFI_MODE_STA);

    EspNow::init();

    auto &esp_now = EspNow::instance();
    const bool is_server = esp_now.mac == server_address;

    Serial.printf(
        "Self: %s (Role: %s)\n",
        EspNow::toString(esp_now.mac).data(),
        is_server ? "Server" : "Client"
    );

    if (is_server) {
        runServer();
    } else {
        runClient();
    }
}

void loop() {}