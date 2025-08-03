#include "serialcmd/impl/protocol/GameProtocol.hpp"

#include "Arduino.h"
#include "WiFi.h"


#include "espnow/Protocol.hpp"
#include "game/Packets.hpp"


/// Адрес сервера
constexpr espnow::Mac server_address = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};

/// Запуск сервера
[[noreturn]] void runServer() {
    using serialcmd::Serializer;
    using serialcmd::impl::protocol::GameProtocol;

    /// Протокол связи между компьютером и игровой средой
    auto bridge = GameProtocol(Serial);

    auto on_delivery = [&bridge](const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) {
        auto &s = bridge.send_espnow_delivery_status.begin();

        // mac
        s.write(mac);

        // status
        s.write(status);
    };

    auto on_receive = [&bridge](const espnow::Mac &mac, const void *data, rs::u8 size) {
        auto &s = bridge.send_espnow_client_packet.begin();

        // mac
        s.write(mac);

        // vec
        s.write(size);
        s.stream.write(static_cast<const rs::u8 *>(data), size);
    };

    auto &now = espnow::Protocol::instance();
    now.setReceiveHandler(on_receive);
    now.setDeliveryHandler(on_delivery);

    /// { [6]u8, [u8]u8 }
    auto on_espnow_send = [&bridge](Serializer &serializer) {
        espnow::Mac mac;
        serializer.read(mac);

        rs::u8 size;
        serializer.read(size);

        static rs::u8 data[256];
        serializer.stream.readBytes(data, size);

        if (not espnow::Peer::exist(mac)) {
            auto result = espnow::Peer::add(mac);

            auto &s = bridge.send_log.begin();
            s.write(rs::formatted<sizeof(game::HostLogMessage)>(
                "peer %s add -> %s",
                rs::toArrayString(mac).data(),
                rs::toString(result.value)
            ));
        }

        auto result = espnow::Protocol::send(mac, data, size);

        auto &s = bridge.send_log.begin();
        s.write(rs::formatted<sizeof(game::HostLogMessage)>(
            "sending to %s -> %s",
            rs::toArrayString(mac).data(),
            rs::toString(result.value)
        ));
    };

    /// ()
    auto on_get_mac = [&bridge](Serializer &serializer) {
        auto &s = bridge.send_mac.begin();
        s.write(espnow::Protocol::instance().mac);
    };

    bridge.registerReceiver(on_get_mac);
    bridge.registerReceiver(on_espnow_send);

    while (true) {
        delay(10);
        bridge.pull();
    }
}

/// Запуск клиента пользователя
[[noreturn]] void runClientUser() {

    auto onDelivery = [](const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) {
        Serial.printf(
            "Отправка %s : %s\n",
            rs::toArrayString(mac).data(),
            rs::toString(status)
        );
    };

    auto onReceive = [](const espnow::Mac &mac, const void *data, rs::u8 size) {
        // Проверка, что сообщение пришло от сервера
        if (mac != server_address) {
            Serial.printf(
                "Пакет (%d) от %s (не сервер)\n",
                size,
                rs::toArrayString(mac).data()
            );
            return;
        }

        // Проверка размера пакета
        if (size != sizeof(game::ServerMessage)) {
            Serial.printf(
                "Получен пакет от %s (сервер) с неожиданной длиной сообщения (%d) ожидалось: %d\n",
                rs::toArrayString(mac).data(),
                size,
                sizeof(game::ServerMessage)
            );
            return;
        }

        const auto &message = *static_cast<const game::ServerMessage *>(data);

        Serial.print("Сервер: ");
        Serial.println(message.data());
    };

    auto &now = espnow::Protocol::instance();
    now.setReceiveHandler(onReceive);
    now.setDeliveryHandler(onDelivery);

    espnow::Peer::add(server_address);

    espnow::Protocol::send(server_address, rs::formatted<sizeof(game::ClientMessage)>(
        "User %s",
        rs::toArrayString(espnow::Protocol::instance().mac).data()
    ));

    unsigned int x, y;

    while (true) {
        delay(10);

        if (Serial.available() < 1) { continue; }

        String input = Serial.readStringUntil('\n');
        if (sscanf(input.c_str(), "%d %d", &x, &y) != 2) { continue; }

        auto result = espnow::Protocol::send(server_address, game::ClientMove{
            .x = static_cast<rs::u8>(x),
            .y = static_cast<rs::u8>(y)
        });

        Serial.printf("Send: %s\n", rs::toString(result.value));
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