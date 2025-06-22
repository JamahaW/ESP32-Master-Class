#pragma once

#include <array>
#include "rs/primitives.hpp"
#include "lina/Vector2D.hpp"


namespace game {
    namespace core {
        /// Тип пакета сообщения от сервера
        using ServerMessage = std::array<char, 128>;

        /// Тип пакета сообщения от клиента
        using ClientMessage = std::array<char, 16>;

        /// Тип пакета хода от клиента
        using ClientMove = lina::Vector2D<rs::u8>;
    }
}