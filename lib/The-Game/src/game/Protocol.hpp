#pragma once

#include <array>


namespace game {
    /// Тип пакета сообщения от сервера
    using ServerMessage = std::array<char, 128>;

    /// Тип пакета сообщения от клиента
    using ClientMessage = std::array<char, 16>;

    /// Тип для определения позиции
    using Position = uint8_t;

    /// Тип пакета хода от клиента
    struct [[gnu::packed]] ClientMove {
        Position x, y;
    };
}