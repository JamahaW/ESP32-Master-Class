#pragma once

#include "game/abc/Node.hpp"
#include "game/core/Protocol.hpp"

#include <Print.h>


namespace game {
    namespace impl {
        namespace node {

            /// Клиент игры
            struct Client : abc::Node {

                /// Адрес сервера
                const espnow::Mac &server;
                /// Поток отображения сообщений
                Print &out;

                explicit Client(const espnow::Mac &server, Print &out) :
                    server{server}, out{out} {
                    espnow::Peer::add(server);
                }

                /// Отправить данные на сервер
                template<typename T> rs::Result<espnow::Protocol::Send> send(T &&value) const {
                    return espnow::Protocol::send(server, value);
                }


            protected:

                void onDelivery(const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) final {
                    out.printf(
                        "Отправка %s : %s\n",
                        rs::toArrayString(mac).data(),
                        rs::toString(status)
                    );
                }

                void onReceive(const espnow::Mac &mac, const void *data, int size) final {
                    // Проверка, что сообщение пришло от сервера
                    if (mac != server) {
                        out.printf(
                            "Пакет (%d) от %s (не сервер)\n",
                            size,
                            rs::toArrayString(mac).data()
                        );
                        return;
                    }

                    // Проверка размера пакета
                    if (size != sizeof(core::ServerMessage)) {
                        out.printf(
                            "Получен пакет от %s (сервер) с неожиданной длиной сообщения (%d) ожидалось: %d\n",
                            rs::toArrayString(mac).data(),
                            size,
                            sizeof(core::ServerMessage)
                        );
                        return;
                    }

                    const auto &message = *static_cast<const core::ServerMessage *>(data);

                    out.print("Сервер: ");
                    out.println(message.data());
                }
            };
        }
    }
}
