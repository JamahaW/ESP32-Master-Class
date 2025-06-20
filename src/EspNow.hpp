#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <esp_err.h>
#include <esp_now.h>
#include <cstring>
#include <esp_mac.h>


#define return_case(__v) case __v: return #__v;
#define return_default() default: return "Invalid";


/// Обёртка над ESP-NOW API с использованием С++
struct EspNow {
    static constexpr char mac_format_string[] = "%02X:%02X:%02X:%02X:%02X:%02X";
    using MacString = std::array<char, sizeof(mac_format_string)>;

    using u8 = uint8_t;
    using str = const char *;

    /// Безопасный тип для MAC адреса
    using Mac = std::array<u8, ESP_NOW_ETH_ALEN>;

    /// Статус доставки
    enum class DeliveryStatus {
        /// Пакет дошел до получателя
        Ok,
        /// Не удалось доставить пакет
        Fail,
    };

    using OnDeliveryFunction = std::function<void(const Mac &, DeliveryStatus)>;
    using OnReceiveFunction = std::function<void(const Mac &, const void *, int)>;

    /// Обработчик доставки сообщения
    OnDeliveryFunction _on_delivery;
    /// Обработчик получения сообщения
    OnReceiveFunction _on_receive;

    /// Получить экземпляр протокола для настройки
    static EspNow &instance() {
        static EspNow instance = {
            ._on_delivery = nullptr,
            ._on_receive = nullptr
        };
        return instance;
    }

    /// Результат инициализации
    enum class InitResult {
        /// Инициализация прошла успешно
        Ok,
        /// Внутренняя ошибка ESP-NOW API
        InternalError,
        /// Неизвестная ошибка ESP API
        UnknownError,
    };

    /// Инициализировать протокол ESP-NOW
    static InitResult init() {
        return translateInitResult(esp_now_init());
    }

    enum class SetHandlerResult {
        /// Обработчик успешно подключен
        Ok,
        /// Протокол ESP-NOW не был инициализирован
        NotInit,
        /// Внутренняя ошибка ESP-NOW API
        InternalError,
        /// Неизвестная ошибка ESP API
        UnknownError,
    };

    /// Установить обработчик входящих сообщений
    SetHandlerResult setReceiveHandler(OnReceiveFunction &&handler) {
        _on_receive = handler;

        return translateSetHandler(
            (
                (handler == nullptr) ?
                esp_now_register_recv_cb(onReceive) : esp_now_unregister_recv_cb()
            )
        );
    }

    /// Установить обработчик при доставке сообщений
    SetHandlerResult setDeliveryHandler(OnDeliveryFunction &&handler) {
        _on_delivery = handler;

        return translateSetHandler(
            (
                (handler == nullptr) ?
                esp_now_register_send_cb(onDelivery) : esp_now_unregister_send_cb()
            )
        );
    }

    /// Завершить работу протокола
    static void quit() { esp_now_deinit(); }

    /// Получить свой MAC адрес
    static Mac mac() {
        Mac ret = {};
        esp_read_mac(ret.data(), ESP_MAC_WIFI_STA);
        return ret;
    }

    /// Результат добавления пира
    enum class AddPeerResult {
        /// Пир успешно добавлен
        Ok,
        /// Протокол ESP-NOW не был инициализирован
        NotInit,
        /// Неверный аргумент
        InvalidArg,
        /// Список пиров полон
        Full,
        /// Не хватает памяти для добавления пира
        NoMemory,
        /// Пир уже добавлен
        Exists,
        /// Неизвестная ошибка ESP API
        UnknownError,
    };

    /// Добавить пир
    static AddPeerResult addPeer(const Mac &mac) {
        esp_now_peer_info_t peer = {};
        memcpy(peer.peer_addr, mac.data(), 6);

        return translateAddPeerResult(esp_now_add_peer(&peer));
    }

    /// Удалить пир
    enum class DeletePeerResult {
        /// Пир успешно удален
        Ok,
        /// Протокол ESP-NOW не был инициализирован
        NotInit,
        /// Неверный аргумент
        InvalidArg,
        /// Пир не найден в списке добавленных
        NotFound,
        /// Неизвестная ошибка ESP API
        UnknownError,
    };

    /// Удалить пир
    static DeletePeerResult deletePeer(const Mac &mac) {
        return translateDeletePeerResult(esp_now_del_peer(mac.data()));
    }

    /// Проверить существование пира
    static bool checkPeerExist(const Mac &mac) {
        return esp_now_is_peer_exist(mac.data());
    }

    /// Результат отправки сообщения
    enum class SendResult {
        /// Сообщение успешно отправлено
        Ok,
        /// Протокол ESP-NOW не был инициализирован
        NotInit,
        /// Неверный аргумент
        InvalidArg,
        /// Внутренняя ошибка ESP-NOW API
        InternalError,
        /// Не хватает памяти для отправки сообщения
        NoMemory,
        /// Целевой пир не найден
        PeerNotFound,
        /// Установлен неверный режим интерфейса WiFi
        IncorrectWiFiMode,
        /// Неизвестная ошибка ESP API
        UnknownError,
    };

    /// Отправить сообщение
    template<typename T> static SendResult send(const Mac &mac, const T &value) {
        static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");

        return translateSendResult(esp_now_send(
            mac.data(),
            reinterpret_cast<const u8 *>(&value),
            sizeof(T)
        ));
    }

private:

    static void onReceive(const u8 *mac, const u8 *data, int size) {
        instance()._on_receive(
            castMac(mac),
            static_cast<const void *>(data),
            size
        );
    }

    static void onDelivery(const u8 *mac, esp_now_send_status_t status) {
        instance()._on_delivery(
            castMac(mac),
            translateDeliveryStatus(status)
        );
    }

    inline static const Mac &castMac(const u8 *mac) {
        return *reinterpret_cast<const Mac *>(mac);
    }

private:

    // translate from esp

    static DeliveryStatus translateDeliveryStatus(esp_now_send_status_t status) {
        return (status == ESP_NOW_SEND_SUCCESS) ? DeliveryStatus::Ok : DeliveryStatus::Fail;
    }

    static InitResult translateInitResult(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return InitResult::Ok;
            case ESP_ERR_ESPNOW_INTERNAL:
                return InitResult::InternalError;
            default:
                return InitResult::UnknownError;
        }
    }

    static SetHandlerResult translateSetHandler(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return SetHandlerResult::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return SetHandlerResult::NotInit;
            case ESP_ERR_ESPNOW_INTERNAL:
                return SetHandlerResult::InternalError;
            default:
                return SetHandlerResult::UnknownError;
        }
    }

    static SendResult translateSendResult(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return SendResult::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return SendResult::NotInit;
            case ESP_ERR_ESPNOW_ARG:
                return SendResult::InvalidArg;
            case ESP_ERR_ESPNOW_INTERNAL:
                return SendResult::InternalError;
            case ESP_ERR_ESPNOW_NO_MEM:
                return SendResult::NoMemory;
            case ESP_ERR_ESPNOW_NOT_FOUND:
                return SendResult::PeerNotFound;
            case ESP_ERR_ESPNOW_IF:
                return SendResult::IncorrectWiFiMode;
            default:
                return SendResult::UnknownError;
        }
    }

    static DeletePeerResult translateDeletePeerResult(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return DeletePeerResult::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return DeletePeerResult::NotInit;
            case ESP_ERR_ESPNOW_ARG:
                return DeletePeerResult::InvalidArg;
            case ESP_ERR_ESPNOW_NOT_FOUND:
                return DeletePeerResult::NotFound;
            default:
                return DeletePeerResult::UnknownError;
        }
    }

    static AddPeerResult translateAddPeerResult(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return AddPeerResult::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return AddPeerResult::NotInit;
            case ESP_ERR_ESPNOW_ARG:
                return AddPeerResult::InvalidArg;
            case ESP_ERR_ESPNOW_FULL:
                return AddPeerResult::Full;
            case ESP_ERR_ESPNOW_NO_MEM:
                return AddPeerResult::NoMemory;
            case ESP_ERR_ESPNOW_EXIST:
                return AddPeerResult::Exists;
            default:
                return AddPeerResult::UnknownError;
        }
    }

public:

    EspNow() = delete;

    EspNow(const EspNow &) = delete;

    EspNow &operator=(const EspNow &) = delete;

public:

    // toString

    static MacString toString(const Mac &mac) {
        MacString buff;

        auto raw = mac.data();
        sprintf(buff.data(), mac_format_string, raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);

        return buff;
    }

    static str toString(SetHandlerResult result) {
        switch (result) {
            return_case(SetHandlerResult::Ok)
            return_case(SetHandlerResult::NotInit)
            return_case(SetHandlerResult::InternalError)
            return_case(SetHandlerResult::UnknownError)
            return_default()
        }
    }

    static str toString(DeliveryStatus status) {
        switch (status) {
            return_case(DeliveryStatus::Ok)
            return_case(DeliveryStatus::Fail)
            return_default()
        }
    }

    static str toString(InitResult result) {
        switch (result) {
            return_case(InitResult::Ok)
            return_case(InitResult::InternalError)
            return_case(InitResult::UnknownError)
            return_default()
        }
    }

    static str toString(AddPeerResult result) {
        switch (result) {
            return_case(AddPeerResult::Ok)
            return_case(AddPeerResult::NotInit)
            return_case(AddPeerResult::InvalidArg)
            return_case(AddPeerResult::Full)
            return_case(AddPeerResult::NoMemory)
            return_case(AddPeerResult::Exists)
            return_case(AddPeerResult::UnknownError)
            return_default()
        }
    }

    static str toString(DeletePeerResult result) {
        switch (result) {
            return_case(DeletePeerResult::Ok)
            return_case(DeletePeerResult::NotInit)
            return_case(DeletePeerResult::InvalidArg)
            return_case(DeletePeerResult::NotFound)
            return_case(DeletePeerResult::UnknownError)
            return_default()
        }
    }

    static str toString(SendResult result) {
        switch (result) {
            return_case(SendResult::Ok)
            return_case(SendResult::NotInit)
            return_case(SendResult::InvalidArg)
            return_case(SendResult::InternalError)
            return_case(SendResult::NoMemory)
            return_case(SendResult::PeerNotFound)
            return_case(SendResult::IncorrectWiFiMode)
            return_case(SendResult::UnknownError)
            return_default()
        }
    }
};
