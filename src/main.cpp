#include "game/impl/node/Client.hpp"
#include "game/impl/node/Host.hpp"
#include "game/core/Protocol.hpp"

#include <Arduino.h>
#include "WiFi.h"


/// Адрес сервера
constexpr espnow::Mac server_address = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};

/// Запуск сервера
[[noreturn]] void runServer() {
    game::core::Environment environment = {
        .win_length = 14,
        .field_size = {16, 16},
        .field_state = {},
        .move_timeout = 5000,
    };

    serialcmd::StreamSerializer serializer(Serial);

    game::impl::node::Host host(environment, serializer);

    host.init();
    host.sendMac();

    while (true) {
        host.pull();
    }
}


/// Запуск клиента пользователя
[[noreturn]] void runClientUser() {
    game::impl::node::Client client(server_address, Serial);

    client.init();

    client.send(rs::formatted<sizeof(game::core::ClientMessage)>(
        "User %s",
        rs::toArrayString(espnow::Protocol::instance().mac).data()
    ));

    unsigned int x, y;

    while (true) {
        delay(10);

        if (Serial.available() < 1) { continue; }

        String input = Serial.readStringUntil('\n');
        if (sscanf(input.c_str(), "%d %d", &x, &y) != 2) { continue; }

        client.send(game::core::ClientMove{
            .x = rs::u8(x),
            .y = rs::u8(y)
        });
    }
}


void setup() {
    Serial.begin(115200);

    WiFiClass::mode(WIFI_MODE_STA);

    espnow::Protocol::init();

    auto &esp_now = espnow::Protocol::instance();

    if (esp_now.mac == server_address) {
        runServer();
    } else {
        runClientUser();
    }
}

void loop() {}