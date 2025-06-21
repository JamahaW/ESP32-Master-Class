#pragma once

#include <map>
#include <queue>

#include "EspNow.hpp"
#include "rs/Utils.hpp"

#include "game/Player.hpp"
#include "game/Protocol.hpp"
#include "game/Environment.hpp"


namespace game {

    /// Хост игры
    struct Host {

        /// Сведения о клиентах
        std::map<EspNow::Mac, Player> _clients;

        struct SendRecord {
            EspNow::Mac mac;
            ServerMessage message;
        };

        /// Очередь отложенных сообщений
        std::queue<SendRecord> _sends;

        struct MoveRecord {
            EspNow::Mac mac;
            const Player &player;
            const ClientMove move;
        };

        /// Очередь отложенных ходов
        std::queue<MoveRecord> _moves;

        void init() {
            auto &esp_now = EspNow::instance();

            auto on_delivery = [](const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
                game::Host::onDelivery(mac, status);
            };

            auto on_receive = [this](const EspNow::Mac &mac, const void *data, int size) {
                this->onReceive(mac, data, size);
            };

            esp_now.setDeliveryHandler(on_delivery);
            esp_now.setReceiveHandler(on_receive);
        }

        void pull() {
            delay(50);

            if (not _moves.empty()) {
                auto record = _moves.front();

                auto result = game::makeMove(record.player, record.move); // todo заменить на игру

                if (result.ok()) {
                    send(record.mac, ServerMessage{"Move Ok"});
                } else {
                    send(record.mac, ServerMessage{"Move error"});
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

        void onPlayerMessage(const EspNow::Mac &mac, const ClientMessage &message) {
            const auto &it = _clients.find(mac);

            if (it == _clients.end()) {
                // Игрок ещё не существует - регистрируем

                const auto &player = Player::create(message);
                _clients.emplace(mac, player);

                send(mac, rs::formatted<sizeof(ServerMessage)>(
                    "Client %s registered as '%s' team: %d",
                    EspNow::toString(mac).data(),
                    player.username.data(),
                    player.team
                ));
            } else {
                // Игрок уже существует - изменяем имя

                auto &player = it->second;

                send(mac, rs::formatted<sizeof(ServerMessage)>(
                    "Client %s renamed from '%s' to '%s'",
                    EspNow::toString(mac).data(),
                    player.username.data(),
                    message.data()
                ));

                player.username = message;
            }
        }

        void onPlayerMove(const EspNow::Mac &mac, const ClientMove &move) {
            const auto &it = _clients.find(mac);

            if (it == _clients.end()) {
                // Игрок не зарегистрирован - отказ в действии

                send(mac, rs::formatted<sizeof(ServerMessage)>(
                    "Client %s (Not registered) move denied",
                    EspNow::toString(mac).data()
                ));

            } else {
                // Игрок зарегистрирован - отправляем запись действия в очередь

                auto &player = it->second;

                _moves.push(MoveRecord{mac, player, move});

                send(mac, rs::formatted<sizeof(ServerMessage)>(
                    "Client %s (Player %s) move send to queue",
                    EspNow::toString(mac).data(),
                    player.username.data()
                ));
            }
        }

        void onPlayerUnknown(const EspNow::Mac &mac, int size) {
            send(mac, rs::formatted<sizeof(ServerMessage)>("Invalid Packed (%d Bytes)", size));
        }

        // сервис

        /// Добавить сообщение в очередь на отправку
        void send(const EspNow::Mac &mac, ServerMessage message) {
            _sends.push(SendRecord{mac, message});
        }

        // Обработчики событий

        static void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
            Serial.printf(
                "Delivery to client %s : %s\n",
                EspNow::toString(mac).data(),
                EspNow::toString(status)
            );
        }

        void onReceive(const EspNow::Mac &mac, const void *data, int size) {
            switch (size) {
                case sizeof(ClientMessage):
                    onPlayerMessage(mac, *static_cast<const ClientMessage *>(data));
                    return;

                case sizeof(ClientMove):
                    onPlayerMove(mac, *static_cast<const ClientMove *>(data));
                    return;

                default:
                    onPlayerUnknown(mac, size);
            }
        }
    };
}
