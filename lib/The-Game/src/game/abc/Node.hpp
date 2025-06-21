#pragma once

#include "EspNow.hpp"


namespace game {
    namespace abc {

        /// Узел сети
        struct Node {

        public:

            /// Инициализировать узел
            void init() {
                auto &esp_now = EspNow::instance();

                auto on_delivery = [this](const EspNow::Mac &mac, EspNow::DeliveryStatus status) {
                    this->onDelivery(mac, status);
                };

                auto on_receive = [this](const EspNow::Mac &mac, const void *data, int size) {
                    this->onReceive(mac, data, size);
                };

                esp_now.setDeliveryHandler(on_delivery);
                esp_now.setReceiveHandler(on_receive);
            }

        protected:

            /// Обработчик доставки сообщения
            virtual void onDelivery(const EspNow::Mac &mac, EspNow::DeliveryStatus status) = 0;

            /// Обработчик приёма сообщения
            virtual void onReceive(const EspNow::Mac &mac, const void *data, int size) = 0;
        };
    }
}