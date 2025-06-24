#pragma once

#include "game/abc/Node.hpp"
#include "game/core/Player.hpp"
#include "game/core/Protocol.hpp"
#include "game/core/Environment.hpp"

#include "Arduino.h"

#include <map>
#include <queue>


namespace game {
    namespace impl {
        namespace node {

            /// Хост игры
            struct Host : abc::Node {

                using LogMessage = rs::ArrayString<128>;

                enum MessageType : rs::u8 {
                    SendMac = 0x01,
                    LogOutput = 0x02,
                    BoardStateUpdate = 0x03,
                    PlayerListUpdate = 0x04,
                };

            private:

                /// Игровое окружение
                core::Environment &environment;

                /// Поток вывода
                serialcmd::StreamSerializer &serializer;

                /// Сведения о клиентах
                std::map<espnow::Mac, core::Player> clients{};

                struct SendRecord {
                    espnow::Mac mac;
                    core::ServerMessage message;
                };

                /// Очередь отложенных сообщений
                std::queue<SendRecord> sends{};

                struct MoveRecord {
                    espnow::Mac mac;
                    const core::Player &player;
                    const core::ClientMove move;
                };

                /// Очередь отложенных ходов
                std::queue<MoveRecord> moves{};

            public:

                explicit Host(core::Environment &environment, serialcmd::StreamSerializer &serializer) :
                    environment{environment}, serializer(serializer) {}

                void sendMac() {
                    serializer.write(MessageType::SendMac);
                    serializer.write(espnow::Protocol::instance().mac);
                }

                void sendPlayerListUpdate() {
                    serializer.write(MessageType::PlayerListUpdate);

                    const rs::u16 len = clients.size();
                    serializer.write(len);

                    for (const auto &c: clients) {
                        const auto &mac = c.first;
                        const auto &player = c.second;

                        serializer.write(mac);
                        serializer.write(player.username);
                        serializer.write(player.team);
                    }
                }

                void sendFieldState() {
                    serializer.write(MessageType::BoardStateUpdate);

                    const rs::u16 len = environment.field_state.size();
                    serializer.write(len);

                    for (const auto &kv: environment.field_state) {
                        const auto &pos = kv.first;
                        const auto team = kv.second;

                        serializer.write(pos);
                        serializer.write(team);
                    }
                }

                void sendLog(LogMessage &&message) {
                    serializer.write(MessageType::LogOutput);
                    serializer.write(message);
                }

            public:

                void pull() {
                    delay(50);

                    if (not moves.empty()) {
                        auto record = moves.front();

                        auto result = environment.makeMove(record.player, record.move);

                        sendPeer(record.mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Ход (%d, %d) : %s",
                            record.move.x, record.move.y,
                            rs::toString(result.value)
                        ));

                        if (result.ok()) { sendFieldState(); }

                        moves.pop();
                    }

                    if (not sends.empty()) {
                        const auto &record = sends.front();

                        if (not espnow::Peer::exist(record.mac)) {
                            espnow::Peer::add(record.mac);
                        }

                        auto result = espnow::Protocol::send(record.mac, record.message);

                        sendLog(rs::formatted<sizeof(LogMessage)>(
                            "Отправка ответа %s .. ",
                            rs::toArrayString(record.mac).data()
                        ));

                        if (result.ok()) {
                            sends.pop();

                            sendLog(rs::formatted<sizeof(LogMessage)>(
                                "Ok   -> %s",
                                record.message.data()
                            ));
                        } else {
                            sendLog(rs::formatted<sizeof(LogMessage)>(
                                "Fail -> %s",
                                rs::toString(result.value)
                            ));
                        }
                    }
                }

            private:

                // Обработчики сообщений

                void onPlayerMessage(const espnow::Mac &mac, const core::ClientMessage &message) {
                    const auto &it = clients.find(mac);

                    if (it == clients.end()) {
                        // Игрок ещё не существует - регистрируем

                        const auto &player = core::Player::create(message);
                        clients.emplace(mac, player);

                        sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s зарегистрирован как '%s' номер команды: %d",
                            rs::toArrayString(mac).data(),
                            player.username.data(),
                            player.team
                        ));
                    } else {
                        // Игрок уже существует - изменяем имя

                        auto &player = it->second;

                        sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s переименован ('%s' -> '%s')",
                            rs::toArrayString(mac).data(),
                            player.username.data(),
                            message.data()
                        ));

                        player.username = message;
                        player.last_send = millis();
                    }

                    sendPlayerListUpdate();
                }

                void onPlayerMove(const espnow::Mac &mac, const core::ClientMove &move) {
                    const auto &it = clients.find(mac);

                    if (it == clients.end()) {
                        // Игрок не зарегистрирован - отказ в действии

                        sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s (не зарегистрирован) ход отклонён",
                            rs::toArrayString(mac).data()
                        ));

                        return;
                    }

                    auto &player = it->second;
                    const auto now = millis();

                    const auto time_since_last = now - player.last_send;

                    if (time_since_last < environment.move_timeout) {
                        const auto secs = float(environment.move_timeout - time_since_last) * 1e-3f;

                        sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s (Игрок %s) подождите %.3f с",
                            rs::toArrayString(mac).data(),
                            player.username.data(),
                            secs
                        ));
                        return;
                    }

                    player.last_send = now;

                    moves.push(MoveRecord{mac, player, move});

                    sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                        "Клиент %s (Игрок %s) ход отправлен в очередь",
                        rs::toArrayString(mac).data(),
                        player.username.data()
                    ));
                }

                void onPlayerUnknown(const espnow::Mac &mac, int size) {
                    sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>("Непредвиденный размер пакета (%d)", size));
                }

                // сервис

                /// Добавить сообщение в очередь на отправку
                void sendPeer(const espnow::Mac &mac, core::ServerMessage message) {
                    sends.push(SendRecord{mac, message});
                }

            protected:

                // Обработчики событий

                void onDelivery(const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) final {
                    sendLog(rs::formatted<sizeof(LogMessage)>(
                        "Отправка клиенту %s : %s",
                        rs::toArrayString(mac).data(),
                        rs::toString(status)
                    ));
                }

                void onReceive(const espnow::Mac &mac, const void *data, int size) final {
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
