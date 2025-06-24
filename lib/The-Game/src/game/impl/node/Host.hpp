#pragma once

#include "game/abc/Node.hpp"
#include "game/core/Player.hpp"
#include "game/core/Protocol.hpp"
#include "game/core/Environment.hpp"

#include "Arduino.h"
#include <map>
#include <queue>

#include "rs/ArrayString.hpp"


namespace game {
    namespace impl {
        namespace node {

            /// Хост игры
            struct Host : abc::Node {

            private:

                /// Игровое окружение
                core::Environment &environment;

                /// Поток отображения сообщений
                Print &out;

                /// Сведения о клиентах
                std::map<EspNow::Mac, core::Player> clients{};

                struct SendRecord {
                    EspNow::Mac mac;
                    core::ServerMessage message;
                };

                /// Очередь отложенных сообщений
                std::queue<SendRecord> sends{};

                struct MoveRecord {
                    EspNow::Mac mac;
                    const core::Player &player;
                    const core::ClientMove move;
                };

                /// Очередь отложенных ходов
                std::queue<MoveRecord> moves{};

            public:

                explicit Host(core::Environment &environment, Print &out) :
                    environment{environment}, out{out} {}

                void pull() {
                    delay(50);

                    if (not moves.empty()) {
                        auto record = moves.front();

                        auto result = environment.makeMove(record.player, record.move);

                        if (result.ok()) {
                            send(record.mac, core::ServerMessage{"Move Ok"});
                        } else {
                            send(record.mac, core::ServerMessage{"Move error"});
                        }

                        moves.pop();
                    }

                    if (not sends.empty()) {
                        auto record = sends.front();

                        if (not EspNow::checkPeerExist(record.mac)) {
                            EspNow::addPeer(record.mac);
                        }

                        auto result = EspNow::send(record.mac, record.message);

                        out.printf("Отправка ответа %s .. ", EspNow::toString(record.mac).data());

                        if (result.ok()) {
                            sends.pop();
                            out.printf("Ok   -> %s\n", record.message.data());
                        } else {
                            out.printf("Fail -> %s\n", EspNow::toString(result));
                        }
                    }
                }

            private:

                // Обработчики сообщений

                void onPlayerMessage(const EspNow::Mac &mac, const core::ClientMessage &message) {
                    const auto &it = clients.find(mac);

                    if (it == clients.end()) {
                        // Игрок ещё не существует - регистрируем

                        const auto &player = core::Player::create(message);
                        clients.emplace(mac, player);

                        send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s зарегистрирован как '%s' номер команды: %d",
                            EspNow::toString(mac).data(),
                            player.username.data(),
                            player.team
                        ));
                    } else {
                        // Игрок уже существует - изменяем имя

                        auto &player = it->second;

                        send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s переименован ('%s' -> '%s')",
                            EspNow::toString(mac).data(),
                            player.username.data(),
                            message.data()
                        ));

                        player.username = message;
                    }
                }

                void onPlayerMove(const EspNow::Mac &mac, const core::ClientMove &move) {
                    const auto &it = clients.find(mac);

                    if (it == clients.end()) {
                        // Игрок не зарегистрирован - отказ в действии

                        send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s (не зарегистрирован) ход отклонён",
                            EspNow::toString(mac).data()
                        ));

                    } else {
                        // Игрок зарегистрирован - отправляем запись действия в очередь

                        auto &player = it->second;

                        moves.push(MoveRecord{mac, player, move});

                        send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s (Игрок %s) ход отправлен в очередь",
                            EspNow::toString(mac).data(),
                            player.username.data()
                        ));
                    }
                }

                void onPlayerUnknown(const EspNow::Mac &mac, int size) {
                    send(mac, rs::formatted<sizeof(core::ServerMessage)>("Непредвиденный размер пакета (%d)", size));
                }

                // сервис

                /// Добавить сообщение в очередь на отправку
                void send(const EspNow::Mac &mac, core::ServerMessage message) {
                    sends.push(SendRecord{mac, message});
                }

                // Обработчики событий

            protected:

                void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) final {
                    out.printf(
                        "Отправка клиенту %s : %s\n",
                        EspNow::toString(mac).data(),
                        EspNow::toString(status)
                    );
                }

                void onReceive(const EspNow::Mac &mac, const void *data, int size) final {
                    switch (size) {
                        case sizeof(core::ClientMessage):
                            onPlayerMessage(mac, *static_cast<const core::ClientMessage *>(data));
                            return;

                        case sizeof(core::ClientMove):
                            onPlayerMove(mac, *static_cast<const core::ClientMove *>(data));
                            return;

                        default:
                            onPlayerUnknown(mac, size);
                    }
                }
            };
        }
    }
}
