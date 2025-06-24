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

                /// Очередь сообщений логов
                std::queue<LogMessage> logs{};

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

                explicit Host(core::Environment &environment, serialcmd::StreamSerializer &serializer) :
                    environment{environment}, serializer(serializer) {}

                void sendMac() {
                    serializer.write(MessageType::SendMac);
                    serializer.write(EspNow::instance().mac);
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
                    logs.push(message);
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
                            core::Environment::toString(result.value)
                        ));

                        if (result.fail()) {
                            if (result.value == core::Environment::MakeMove::WinnerFounded) {
                                sendLog(rs::formatted<sizeof(LogMessage)>(
                                    "Победитель: %s\n",
                                    record.player.username.data()
                                ));
                            }
                        } else {
                            sendFieldState();
                        }

                        moves.pop();
                    }

                    if (not sends.empty()) {
                        const auto &record = sends.front();

                        if (not EspNow::checkPeerExist(record.mac)) {
                            EspNow::addPeer(record.mac);
                        }

                        auto result = EspNow::send(record.mac, record.message);

                        sendLog(rs::formatted<sizeof(LogMessage)>(
                            "Отправка ответа %s .. ",
                            EspNow::toString(record.mac).data()
                        ));

                        if (result.ok()) {
                            sends.pop();

                            sendLog(rs::formatted<sizeof(LogMessage)>(
                                "Ok   -> %s\n",
                                record.message.data()
                            ));
                        } else {
                            sendLog(rs::formatted<sizeof(LogMessage)>(
                                "Fail -> %s\n",
                                EspNow::toString(result)
                            ));
                        }
                    }

                    if (not logs.empty()) {
                        const auto &message = logs.front();

                        serializer.write(MessageType::LogOutput);
                        serializer.write(message);

                        logs.pop();
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

                        sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s зарегистрирован как '%s' номер команды: %d",
                            EspNow::toString(mac).data(),
                            player.username.data(),
                            player.team
                        ));
                    } else {
                        // Игрок уже существует - изменяем имя

                        auto &player = it->second;

                        sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s переименован ('%s' -> '%s')",
                            EspNow::toString(mac).data(),
                            player.username.data(),
                            message.data()
                        ));

                        player.username = message;
                        player.last_send = millis();
                    }

                    sendPlayerListUpdate();
                }

                void onPlayerMove(const EspNow::Mac &mac, const core::ClientMove &move) {
                    const auto &it = clients.find(mac);

                    if (it == clients.end()) {
                        // Игрок не зарегистрирован - отказ в действии

                        sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                            "Клиент %s (не зарегистрирован) ход отклонён",
                            EspNow::toString(mac).data()
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
                            EspNow::toString(mac).data(),
                            player.username.data(),
                            secs
                        ));
                        return;
                    }

                    player.last_send = now;

                    moves.push(MoveRecord{mac, player, move});

                    sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>(
                        "Клиент %s (Игрок %s) ход отправлен в очередь",
                        EspNow::toString(mac).data(),
                        player.username.data()
                    ));
                }

                void onPlayerUnknown(const EspNow::Mac &mac, int size) {
                    sendPeer(mac, rs::formatted<sizeof(core::ServerMessage)>("Непредвиденный размер пакета (%d)", size));
                }

                // сервис

                /// Добавить сообщение в очередь на отправку
                void sendPeer(const EspNow::Mac &mac, core::ServerMessage message) {
                    sends.push(SendRecord{mac, message});
                }

            protected:

                // Обработчики событий

                void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) final {
                    sendLog(rs::formatted<sizeof(LogMessage)>(
                        "Отправка клиенту %s : %s\n",
                        EspNow::toString(mac).data(),
                        EspNow::toString(status)
                    ));
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
