#pragma once

#include "core/Result.hpp"
#include "game/Player.hpp"


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

    // Environment

    /// todo Сделать методом окружения
    Result <MakeMove> makeMove(const Player &player, const ClientMove &move) {
        return {MakeMove::Ok};
    }
}
