#pragma once

#include <HardwareSerial.h>
#include "game/core/Protocol.hpp"
#include "game/abc/Node.hpp"


namespace game {
    namespace impl {

        struct Client : abc::Node {

            /// Адрес сервера
            const EspNow::Mac &server;

            explicit Client(const EspNow::Mac &server) :
                server{server} {
                EspNow::addPeer(server);
            }

            void sendMessage(core::ClientMessage &&message) const {
                EspNow::send(server, message);
            }

            void pull() const {
                EspNow::send(server, core::ClientMove{123, 69});

                delay(5000);
            }

        protected:

            void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) final {
                Serial.printf(
                    "Delivery to server %s : %s\n",
                    EspNow::toString(mac).data(),
                    EspNow::toString(status)
                );
            }

            void onReceive(const EspNow::Mac &mac, const void *data, int size) final {
                // Проверка, что сообщение пришло от сервера
                if (mac != server) {
                    Serial.printf(
                        "Message (%d Bytes) from %s (not server)\n",
                        size,
                        EspNow::toString(mac).data());
                    return;
                }

                // Проверка размера пакета
                if (size != sizeof(core::ServerMessage)) {
                    Serial.printf(
                        "Got message from %s (server) with incorrect size (%d) expected (%d)\n",
                        EspNow::toString(mac).data(),
                        size,
                        sizeof(core::ServerMessage));
                    return;
                }

                const auto &message = *static_cast<const core::ServerMessage *>(data);
                Serial.print("Server: ");
                Serial.println(message.data());
            }
        };
    }
}
