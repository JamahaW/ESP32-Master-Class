#pragma once

#include "game/abc/Node.hpp"

namespace game {
namespace impl {
namespace node {

/// Мост между сервером и клиентом
struct Bridge : abc::Node {



protected:

    void onDelivery(const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) final {
        // Передавать статус доставки
    }

    void onReceive(const espnow::Mac &mac, const void *data, int size) final {
        // Передавать любые пакеты на сервер
    }
};

}
}
}
