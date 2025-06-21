#pragma once

#include <Print.h>
#include "game/core/Protocol.hpp"
#include "game/abc/Node.hpp"


namespace game {
    namespace impl {

        /// Клиент игры
        struct Client : abc::Node {

            /// Адрес сервера
            const EspNow::Mac &server;
            /// Поток отображения сообщений
            Print &out;

            explicit Client(const EspNow::Mac &server, Print &out) :
                server{server}, out{out} {
                EspNow::addPeer(server);
            }

            /// Отправить сообщение
            void sendMessage(core::ClientMessage &&message) const {
                EspNow::send(server, message);
            }

            /// Отправить ход
            void sendMove(core::ClientMove &&move) const {
                EspNow::send(server, move);
            }

        protected:

            void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) final {
                out.printf(
                    "Delivery to server %s : %s\n",
                    EspNow::toString(mac).data(),
                    EspNow::toString(status)
                );
            }

            void onReceive(const EspNow::Mac &mac, const void *data, int size) final {
                // Проверка, что сообщение пришло от сервера
                if (mac != server) {
                    out.printf(
                        "Message (%d Bytes) from %s (not server)\n",
                        size,
                        EspNow::toString(mac).data());
                    return;
                }

                // Проверка размера пакета
                if (size != sizeof(core::ServerMessage)) {
                    out.printf(
                        "Got message from %s (server) with incorrect size (%d) expected (%d)\n",
                        EspNow::toString(mac).data(),
                        size,
                        sizeof(core::ServerMessage));
                    return;
                }

                const auto &message = *static_cast<const core::ServerMessage *>(data);
                out.print("Server: ");
                out.println(message.data());
            }
        };
    }
}
