#pragma once

#include <unordered_map>

#include "serialcmd/StreamSerializer.hpp"

#include "rs/Result.hpp"
#include "rs/macro.hpp"

#include "Player.hpp"
#include "Protocol.hpp"


namespace game {
    namespace core {

        /// Игровое окружение
        struct Environment {

            // Хеш-функция для ClientMove
            struct ClientMoveHash {
                size_t operator()(const ClientMove &move) const {
                    return (static_cast<size_t>(move.x) << sizeof(ClientMove::Value)) | move.y;
                }
            };

            // Функция сравнения для ClientMove
            struct ClientMoveEqual {
                bool operator()(const ClientMove &a, const ClientMove &b) const {
                    return a.x == b.x and a.y == b.y;
                }
            };

            ClientMove::Value win_length; ///< Длина полосы для выигрыша
            ClientMove field_size; ///< Размер доски
            std::unordered_map<ClientMove, Player::Team, ClientMoveHash, ClientMoveEqual> field_state; ///< Состояние доски
            uint32_t move_timeout; ///< Минимальный период отправки (ms)

            /// Результат действия игры
            enum class MakeMove {
                Ok = 0,            ///< Успешный ход ,
                WinnerFounded,     ///< Завершение игры - найден победитель
                FieldNotEmpty,     ///< Поле занято
                OutOfBounds,       ///< Ход вне игрового поля
            };

            rs::Result<MakeMove> makeMove(const Player &player, const ClientMove &move) {

                if (move.x >= field_size.x or move.y >= field_size.y) {
                    return {MakeMove::OutOfBounds};
                }

                // Проверка занятости поля
                const auto it = field_state.find(move);
                if (it != field_state.end()) {
                    return {MakeMove::FieldNotEmpty};
                }

                field_state[move] = player.team;

                const auto *winner = checkWin();

                if (winner == nullptr) {
                    return {MakeMove::Ok};
                } else {
                    return {MakeMove::WinnerFounded};
                }
            }

            /// Проверить состояние клетки
            const Player::Team *getFieldState(const ClientMove &move) const {
                auto it = field_state.find(move);

                if (it != field_state.end()) {
                    return &it->second;
                }

                return nullptr;
            }

            /// Очистить игровое поле
            void clearField() {
                field_state.clear();
            }

        private:

            const Player::Team *checkWin() const {
                // Проверяем только занятые клетки
                for (const auto &v: field_state) {
                    const auto &position = v.first;
                    const auto &team = v.second;

                    // Проверка горизонтальной линии (влево-вправо)
                    uint8_t horizontal_count = 1;

                    // Проверка вправо
                    for (uint8_t x = position.x + 1; x < field_size.x; x++) {
                        const auto *cell = getFieldState({x, position.y});
                        if (cell and *cell == team) { horizontal_count++; }
                        else { break; }
                    }

                    // Проверка влево
                    for (int8_t x = position.x - 1; x >= 0; --x) {
                        const auto *cell = getFieldState({static_cast<uint8_t>(x), position.y});
                        if (cell and *cell == team) { horizontal_count++; }
                        else { break; }
                    }

                    if (horizontal_count >= win_length) { return &team; }

                    // Проверка вертикальной линии (вверх-вниз)
                    uint8_t vertical_count = 1;

                    // Проверка вниз
                    for (uint8_t y = position.y + 1; y < field_size.y; y++) {
                        const auto *cell = getFieldState({position.x, y});
                        if (cell and *cell == team) { vertical_count++; }
                        else { break; }
                    }

                    // Проверка вверх
                    for (int8_t y = position.y - 1; y >= 0; --y) {
                        const auto *cell = getFieldState({position.x, static_cast<uint8_t>(y)});
                        if (cell and *cell == team) { vertical_count++; }
                        else { break; }
                    }

                    if (vertical_count >= win_length) { return &team; }
                }

                return nullptr; // Победитель не найден
            }

        public:

            Environment() = delete;
        };
    }
}

namespace rs {
    static str toString(game::core::Environment::MakeMove result) {
        switch (result) {
            return_case(game::core::Environment::MakeMove::Ok)
            return_case(game::core::Environment::MakeMove::WinnerFounded)
            return_case(game::core::Environment::MakeMove::FieldNotEmpty)
            return_case(game::core::Environment::MakeMove::OutOfBounds)
            return_default()
        }
    }
}