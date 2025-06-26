#pragma once

#include "rs/ArrayString.hpp"
#include "lina/Vector2D.hpp"


namespace game {

/// Определение сообщения от хоста к серверу
using HostLogMessage = rs::ArrayString<128>;

/// Тип пакета сообщения к клиенту
using ServerMessage = rs::ArrayString<128>;

/// Тип пакета сообщения к клиенту
using ClientMessage = rs::ArrayString<32>;

/// Тип пакета хода от клиента
using ClientMove = lina::Vector2D<rs::u8>;

}