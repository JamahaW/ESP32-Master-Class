#include "game/impl/node/Client.hpp"
#include "game/impl/node/Host.hpp"
#include "game/core/Protocol.hpp"

#include <Arduino.h>
#include "WiFi.h"

#include "FixOled.h"


using game::core::ClientMessage;
using game::core::ClientMove;


/// Адрес сервера
constexpr EspNow::Mac server_address = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};

/// Адрес Рандом Трона 3000
constexpr EspNow::Mac random_tron_address = {0xFC, 0xE8, 0xC0, 0x74, 0xA6, 0x30};

/// Запуск сервера
[[noreturn]] void runServer() {
    game::core::Environment environment;

    game::impl::node::Host host(environment, Serial);

    host.init();

    while (true) {
        host.pull();
    }
}

/// Запуск клиента Рандом трона 3000
[[noreturn]] void runClientRandomTron3000() {

    struct OledAdapter : FixOled<SSD1306_128x64, OLED_NO_BUFFER> {
        size_t write(uint8_t data) override {
            if (isEnd()) {
                clear();
                home();
            }
            return FixOled::write(data);
        }
    };

    OledAdapter screen;
    screen.init();
    screen.autoPrintln(true);

    game::impl::node::Client client(server_address, screen);

    client.init();
    client.sendMessage(ClientMessage{"RandomTron-3000"});

    auto rand = []() {
        return ClientMove::Value(random() & 0b1111);
    };

    while (true) {
        client.sendMove(ClientMove{rand(), rand()});
        delay(rand() * 300);
    }
}

/// Запуск клиента пользователя
[[noreturn]] void runClientUser() {
    game::impl::node::Client client(server_address, Serial);

    client.init();
    client.sendMessage(ClientMessage{"User"});

    while (true) {
        client.sendMove(ClientMove{123, 69});
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
        "MAC: %s (Роль: %s)\n",
        EspNow::toString(esp_now.mac).data(),
        is_server ? "Сервер" : "Клиент"
    );

    if (is_server) {
        runServer();
    } else {
        if (esp_now.mac == random_tron_address) {
            runClientRandomTron3000();
        } else {
            runClientUser();
        }
    }
}

void loop() {}