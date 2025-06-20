#include "Utils.hpp"
#include "EspNow.hpp"
#include <Arduino.h>
#include <WiFi.h>
#include <queue>
#include <map>

/// Логирование операций EspNow
#define log(__EspNow_api_Result_func) ({auto __r = __EspNow_api_Result_func; Serial.printf(#__EspNow_api_Result_func " -> %s\n", EspNow::toString(__r));})

/// Адрес сервера
constexpr EspNow::Mac server = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};

/// Тип пакета сообщения от сервера
using ServerMessage = std::array<char, 64>;

/// Тип пакета сообщения от игрока
using PlayerMessage = std::array<char, 16>;

/// Тип для определения позиции
using Position = uint8_t;

/// Тип пакета хода от клиента
struct [[gnu::packed]] PlayerMove {
    Position x, y;
};

/// Размер доски
const Position board_size = 8;

/// Отложенное сообщение
struct ServerReply {
    EspNow::Mac target;
    ServerMessage message;
};

/// Данные пользователя
struct Player {
    /// Адрес клиента игрока
    EspNow::Mac mac;
    /// Отображаемое имя пользователя
    PlayerMessage username;
    /// Номер команды
    uint8_t team;

    static Player create(const EspNow::Mac &mac, const PlayerMessage &username) {
        static uint8_t team = 0;

        team += 1;

        return {mac, username, team};
    }
};

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

    Result<MakeMove> makeMove(const Player &player, const PlayerMove &move) {


        return {MakeMove::Ok};
    }
}

/// Запуск сервера
[[noreturn]] void runServer(EspNow &esp_now) {

    /// Очередь отложенных сообщений
    static std::queue<ServerReply> sends;
    /// Сведения о клиентах
    static std::map<EspNow::Mac, Player> clients;
    /// Очередь ходов
    static std::queue<std::pair<std::reference_wrapper<const Player>, PlayerMove>> moves;

    // Доставка сообщений (К клиенту)
    auto on_delivery = [](const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
        Serial.printf("Delivery to client %s : %s", EspNow::toString(mac).data(), EspNow::toString(status));
    };

    // Приём сообщений (От клиента)
    auto on_receive = [](const EspNow::Mac &mac, const void *data, int size) {
        // Проверка типа сообщения через размер

        if (size == sizeof(PlayerMessage)) {
            // Пакет - это сообщение от клиента
            const auto &new_username = *static_cast<const PlayerMessage *>(data);
            const auto mac_str = EspNow::toString(mac);

            // Если клиента ещё не существует - регистрируем его, иначе - переименовываем
            if (clients.find(mac) == clients.end()) {
                clients.emplace(mac, Player::create(mac, new_username));
                sends.push({mac, formatted<sizeof(ServerMessage)>("Client %s registered with username: '%s'", mac_str.data(), new_username.data())});
            } else {
                auto &client = clients[mac];
                sends.push({mac, formatted<sizeof(ServerMessage)>("Client %s was renamed from %s to %s", mac_str.data(), client.username.data(), new_username.data())});
                client.username = new_username;
            }

            return;
        }

        if (size == sizeof(PlayerMove)) {
            // Пакет - это выполнение хода клиента

            const auto &move = *static_cast<const PlayerMove *>(data);

            if (clients.find(mac) == clients.end()) {
                sends.push({mac, {"Unregistered Client cannot use command"}});
            } else {
                auto &client = clients[mac];
                moves.emplace(std::ref(client), move);
            }

            return;
        }

        // Тип пакета не определен.
        sends.push({mac, formatted<sizeof(ServerMessage)>("Invalid Client Package size (%d)", size)});
    };

    log(esp_now.setDeliveryHandler(on_delivery));
    log(esp_now.setReceiveHandler(on_receive));

    // Синхронный цикл отправки запланированных сообщений и исполнения ходов
    while (true) {
        delay(50);

        if (not moves.empty()) {
            const auto &move = moves.front();
            const auto &player = move.first.get();

            auto result = game::makeMove(player, move.second);

            if (result.ok()) {
                sends.push({player.mac, {"Player move ok"}});
            } else {
                sends.push({player.mac, {"Move Fail"}});
            }
        }

        if (not sends.empty()) {
            const auto &reply = sends.front();
            auto result = EspNow::send(reply.target, reply.message);

            Serial.printf("Sending reply to %s ... ", EspNow::toString(reply.target).data());

            if (result.ok()) {
                sends.pop();
                Serial.printf("Ok (%s)", reply.message.data());
            } else {
                Serial.printf("Fail (%s)", EspNow::toString(result));
            }

            Serial.println();
        }
    }
}

/// Запуск клиента
[[noreturn]] void runClient(EspNow &esp_now) {
    // Доставка сообщений (К серверу)
    auto on_delivery = [](const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
        Serial.printf("Delivery to server %s : %s", EspNow::toString(mac).data(), EspNow::toString(status));
    };

    // Приём сообщений (От сервера)
    auto on_receive = [](const EspNow::Mac &mac, const void *data, int size) {

        // Проверка, что сообщение пришло от сервера
        if (mac != server) {
            Serial.printf("Message (%d Bytes) from %s (not server)\n", size, EspNow::toString(mac).data());
            return;
        }

        // Проверка, что размер пакета равен ожидаемому
        if (size != sizeof(ServerMessage)) {
            Serial.printf("Got message from %s (server) with incorrect size (%d) expected (%d)\n", EspNow::toString(mac).data(), size, sizeof(ServerMessage));
            return;
        }

        // Использование

        const auto &message = *static_cast<const ServerMessage *>(data);

        Serial.print("Server: ");
        Serial.println(message.data());
    };

    log(esp_now.setDeliveryHandler(on_delivery));
    log(esp_now.setReceiveHandler(on_receive));

    log(EspNow::addPeer(server));

    PlayerMove command = {
        .x = 123,
        .y = 69
    };

    PlayerMessage message = {"Cool_Client"};

    log(EspNow::send(server, message));

    while (true) {
        log(EspNow::send(server, command));

        delay(2000);
    }
}

void setup() {
    Serial.begin(115200);
    WiFiClass::mode(WIFI_STA);

    log(EspNow::init());

    auto &esp_now = EspNow::instance();
    const bool is_server = esp_now.mac == server;

    Serial.printf(
        "Self: %s (Role: %s)\n",
        EspNow::toString(esp_now.mac).data(),
        is_server ? "Server" : "Client"
    );

    if (is_server) {
        runServer(esp_now);
    } else {
        runClient(esp_now);
    }
}

void loop() {}