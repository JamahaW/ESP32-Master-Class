#pragma once

#include "Protocol.hpp"


namespace game {
    namespace core {
        /// Данные пользователя
        struct Player {
            /// Определение примитива для номера команды
            using Team = uint8_t;

            /// Отображаемое имя пользователя
            ClientMessage username;
            /// Номер команды
            Team team;
            /// Момент прошлой отправки
            uint32_t last_send;

            static Player create(const ClientMessage &username) {
                static uint8_t team = 0;
                team += 1;
                return {username, team, 0};
            }
        };
    }
}
