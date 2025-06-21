#include "Utils.hpp"
#include "EspNow.hpp"
#include <Arduino.h>
#include <queue>
#include <esp_wifi.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <map>


namespace game {
    /// Тип пакета сообщения от сервера
    using ServerMessage = std::array<char, 128>;

    /// Тип пакета сообщения от игрока
    using PlayerMessage = std::array<char, 16>;

    /// Тип для определения позиции
    using Position = uint8_t;

    /// Тип пакета хода от клиента
    struct [[gnu::packed]] PlayerMove {
        Position x, y;
    };
}

namespace game {
    /// Данные пользователя
    struct Player {
        /// Отображаемое имя пользователя
        PlayerMessage username;
        /// Номер команды
        uint8_t team;

        static Player create(const PlayerMessage &username) {
            static uint8_t team = 0;
            team += 1;
            return {username, team};
        }
    };
}

namespace game {
    /// Результат действия игры
    enum class MakeMove {
        /// Успешный ход
        Ok = 0,
        /// Неверное значение хода
        InvalidArg,
        /// Поле занято
        FieldNotEmpty,
    };

    /// todo Сделать методом окружения
    Result<MakeMove> makeMove(const Player &player, const PlayerMove &move) {
        return {MakeMove::Ok};
    }
}

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
            const PlayerMove move;
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

        void onPlayerMessage(const EspNow::Mac &mac, const PlayerMessage &message) {
            const auto &it = _clients.find(mac);

            if (it == _clients.end()) {
                // Игрок ещё не существует - регистрируем

                const auto &player = Player::create(message);
                _clients.emplace(mac, player);

                send(mac, formatted<sizeof(ServerMessage)>(
                    "Client %s registered as '%s' team: %d",
                    EspNow::toString(mac).data(),
                    player.username.data(),
                    player.team
                ));
            } else {
                // Игрок уже существует - изменяем имя

                auto &player = it->second;

                send(mac, formatted<sizeof(ServerMessage)>(
                    "Client %s renamed from '%s' to '%s'",
                    EspNow::toString(mac).data(),
                    player.username.data(),
                    message.data()
                ));

                player.username = message;
            }
        }

        void onPlayerMove(const EspNow::Mac &mac, const PlayerMove &move) {
            const auto &it = _clients.find(mac);

            if (it == _clients.end()) {
                // Игрок не зарегистрирован - отказ в действии

                send(mac, formatted<sizeof(ServerMessage)>(
                    "Client %s (Not registered) move denied",
                    EspNow::toString(mac).data()
                ));

            } else {
                // Игрок зарегистрирован - отправляем запись действия в очередь

                auto &player = it->second;

                _moves.push(MoveRecord{mac, player, move});

                send(mac, formatted<sizeof(ServerMessage)>(
                    "Client %s (Player %s) move send to queue",
                    EspNow::toString(mac).data(),
                    player.username.data()
                ));
            }
        }

        void onPlayerUnknown(const EspNow::Mac &mac, int size) {
            send(mac, formatted<sizeof(ServerMessage)>("Invalid Packed (%d Bytes)", size));
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
                case sizeof(PlayerMessage):
                    onPlayerMessage(mac, *static_cast<const PlayerMessage *>(data));
                    return;

                case sizeof(PlayerMove):
                    onPlayerMove(mac, *static_cast<const PlayerMove *>(data));
                    return;

                default:
                    onPlayerUnknown(mac, size);
            }
        }
    };
}

/// Адрес сервера
constexpr EspNow::Mac server = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};


/// Запуск сервера
[[noreturn]] void runServer() {
    game::Host host;

    host.init();

    while (true) {
        host.pull();

        delay(50);
    }
}

namespace game {
    struct Client {
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

            PlayerMessage message = {"DarkWoldX17"};

            EspNow::send(server, message);
        }

        void pull() {
            EspNow::send(server, PlayerMove{123, 69});
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

/// Запуск клиента
[[noreturn]] void runClient() {
    game::Client client;

    client.init();

    while (true) {
        client.pull();
        delay(5000);
    }
}

void setup() {
    Serial.begin(115200);

    // Инициализация WiFi в режиме станции
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    EspNow::init();

    auto &esp_now = EspNow::instance();
    const bool is_server = esp_now.mac == server;

    Serial.printf(
        "Self: %s (Role: %s)\n",
        EspNow::toString(esp_now.mac).data(),
        is_server ? "Server" : "Client"
    );

    if (is_server) {
        runServer();
    } else {
        runClient();
    }
}

void loop() {}