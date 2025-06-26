#pragma once

#include "espnow/Protocol.hpp"


namespace espnow {
namespace abc {

/// Узел сети (Больше не нужен)
struct Node {

public:

    /// Инициализировать узел
    void init() {
        auto &esp_now = espnow::Protocol::instance();

        auto on_delivery = [this](const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) {
            this->onDelivery(mac, status);
        };

        auto on_receive = [this](const espnow::Mac &mac, const void *data, rs::u8 size) {
            this->onReceive(mac, data, size);
        };

        esp_now.setDeliveryHandler(on_delivery);
        esp_now.setReceiveHandler(on_receive);
    }

protected:

    /// Обработчик доставки сообщения
    virtual void onDelivery(const espnow::Mac &mac, espnow::Protocol::DeliveryStatus status) = 0;

    /// Обработчик приёма сообщения
    virtual void onReceive(const espnow::Mac &mac, const void *data, rs::u8 size) = 0;
};

}
}