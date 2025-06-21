#pragma once

#include <HardwareSerial.h>
#include "EspNow.hpp"
#include "game/Protocol.hpp"


namespace game {
    struct Client {

        EspNow::Mac server;

        void init() {
            auto &esp_now = EspNow::instance();

            auto on_delivery = [this](const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
                this->onDelivery(mac, status);
            };

            auto on_receive = [this](const EspNow::Mac &mac, const void *data, int size) {
                this->onReceive(mac, data, size);
            };

            esp_now.setDeliveryHandler(on_delivery);
            esp_now.setReceiveHandler(on_receive);


            EspNow::addPeer(server);

            ClientMessage message = {"DarkWoldX17"};

            EspNow::send(server, message);
        }

        void pull() {
            EspNow::send(server, ClientMove{123, 69});

            delay(5000);
        }

    protected:

        void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
            Serial.printf(
                "Delivery to server %s : %s\n",
                EspNow::toString(mac).data(),
                EspNow::toString(status)
            );
        }

        void onReceive(const EspNow::Mac &mac, const void *data, int size) {
            // Проверка, что сообщение пришло от сервера
            if (mac != server) {
                Serial.printf(
                    "Message (%d Bytes) from %s (not server)\n",
                    size,
                    EspNow::toString(mac).data());
                return;
            }

            // Проверка размера пакета
            if (size != sizeof(ServerMessage)) {
                Serial.printf(
                    "Got message from %s (server) with incorrect size (%d) expected (%d)\n",
                    EspNow::toString(mac).data(),
                    size,
                    sizeof(ServerMessage));
                return;
            }

            const auto &message = *static_cast<const ServerMessage *>(data);
            Serial.print("Server: ");
            Serial.println(message.data());
        }
    };
}
