#pragma once

#include "game/abc/Node.hpp"

namespace game {
namespace impl {
namespace node {

/// Хост игры
struct Host : abc::Node {

protected:

    void onDelivery(const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) final {}

    void onReceive(const espnow::Mac &mac, const void *data, int size) final {}
};

}
}
}
