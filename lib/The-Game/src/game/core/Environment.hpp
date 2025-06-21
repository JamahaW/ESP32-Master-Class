#pragma once

#include "rs/Result.hpp"
#include "Player.hpp"
#include "Protocol.hpp"


namespace game {
    namespace core {

        /// Игровое окружение
        struct Environment {

            /// Результат действия игры
            enum class MakeMove {
                /// Успешный ход
                Ok = 0,
                /// Неверное значение хода
                InvalidArg,
                /// Поле занято
                FieldNotEmpty,
            };

            rs::Result<MakeMove> makeMove(const Player &player, const ClientMove &move) {
                return {MakeMove::Ok};
            }
        };
    }
}
