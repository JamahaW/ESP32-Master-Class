#pragma once

#include <array>


namespace game {
    namespace core {
        /// Тип пакета сообщения от сервера
        using ServerMessage = std::array<char, 128>;

        /// Тип пакета сообщения от клиента
        using ClientMessage = std::array<char, 16>;

        /// Тип пакета хода от клиента
        struct [[gnu::packed]] ClientMove {
            /// Тип для определения позиции
            using Position = uint8_t;

            Position x, y;
        };
    }
}