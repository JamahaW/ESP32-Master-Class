#pragma once

#include "game/abc/Node.hpp"
#include "game/core/Player.hpp"
#include "game/core/Protocol.hpp"
#include "game/core/Environment.hpp"

#include "Arduino.h"
#include <map>
#include <queue>

#include "rs/Utils.hpp"


namespace game {
    namespace impl {

        /// Хост игры
        struct Host : abc::Node {

            /// Сведения о клиентах
            std::map<EspNow::Mac, core::Player> _clients;

            struct SendRecord {
                EspNow::Mac mac;
                core::ServerMessage message;
            };

            /// Очередь отложенных сообщений
            std::queue<SendRecord> _sends;

            struct MoveRecord {
                EspNow::Mac mac;
                const core::Player &player;
                const core::ClientMove move;
            };

            /// Очередь отложенных ходов
            std::queue<MoveRecord> _moves;

            void pull() {
                delay(50);

                if (not _moves.empty()) {
                    auto record = _moves.front();

                    auto result = core::makeMove(record.player, record.move); // todo заменить на игру

                    if (result.ok()) {
                        send(record.mac, core::ServerMessage{"Move Ok"});
                    } else {
                        send(record.mac, core::ServerMessage{"Move error"});
                    }

                    _moves.pop();
                }

                if (not _sends.empty()) {
                    auto record = _sends.front();

                    if (not EspNow::checkPeerExist(record.mac)) {
                        EspNow::addPeer(record.mac);
                    }

                    auto result = EspNow::send(record.mac, record.message);

                    Serial.printf("Sending reply to %s .. ", EspNow::toString(record.mac).data());

                    if (result.ok()) {
                        _sends.pop();
                        Serial.printf("Ok -> %s\n", record.message.data());
                    } else {
                        Serial.printf("Fail -> %s\n", EspNow::toString(result));
                    }
                }
            }

        private:

            // Обработчики сообщений

            void onPlayerMessage(const EspNow::Mac &mac, const core::ClientMessage &message) {
                const auto &it = _clients.find(mac);

                if (it == _clients.end()) {
                    // Игрок ещё не существует - регистрируем

                    const auto &player = core::Player::create(message);
                    _clients.emplace(mac, player);

                    send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                        "Client %s registered as '%s' team: %d",
                        EspNow::toString(mac).data(),
                        player.username.data(),
                        player.team
                    ));
                } else {
                    // Игрок уже существует - изменяем имя

                    auto &player = it->second;

                    send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                        "Client %s renamed from '%s' to '%s'",
                        EspNow::toString(mac).data(),
                        player.username.data(),
                        message.data()
                    ));

                    player.username = message;
                }
            }

            void onPlayerMove(const EspNow::Mac &mac, const core::ClientMove &move) {
                const auto &it = _clients.find(mac);

                if (it == _clients.end()) {
                    // Игрок не зарегистрирован - отказ в действии

                    send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                        "Client %s (Not registered) move denied",
                        EspNow::toString(mac).data()
                    ));

                } else {
                    // Игрок зарегистрирован - отправляем запись действия в очередь

                    auto &player = it->second;

                    _moves.push(MoveRecord{mac, player, move});

                    send(mac, rs::formatted<sizeof(core::ServerMessage)>(
                        "Client %s (Player %s) move send to queue",
                        EspNow::toString(mac).data(),
                        player.username.data()
                    ));
                }
            }

            void onPlayerUnknown(const EspNow::Mac &mac, int size) {
                send(mac, rs::formatted<sizeof(core::ServerMessage)>("Invalid Packed (%d Bytes)", size));
            }

            // сервис

            /// Добавить сообщение в очередь на отправку
            void send(const EspNow::Mac &mac, core::ServerMessage message) {
                _sends.push(SendRecord{mac, message});
            }

            // Обработчики событий

        protected:

            void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) final {
                Serial.printf(
                    "Delivery to client %s : %s\n",
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
