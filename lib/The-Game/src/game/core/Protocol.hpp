#pragma once

#include <array>
#include "rs/primitives.hpp"
#include "rs/ArrayString.hpp"
#include "lina/Vector2D.hpp"


namespace game {
    namespace core {
        /// Тип пакета сообщения от сервера
        using ServerMessage = rs::ArrayString<128>;

        /// Тип пакета сообщения от клиента
        using ClientMessage = rs::ArrayString<32>;

        /// Тип пакета хода от клиента
        using ClientMove = lina::Vector2D<rs::u8>;
    }
}