#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <cstring>
#include <esp_err.h>
#include <esp_now.h>
#include <esp_mac.h>
#include <esp_wifi.h>

#include "Result.hpp"


#define return_case(__v) case __v: return #__v;
#define return_default() default: return "Invalid";

static constexpr char mac_format_string[] = "%02X:%02X:%02X:%02X:%02X:%02X";

/// Обёртка над ESP-NOW API с использованием С++
struct EspNow {
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
    enum class Init {
        /// Инициализация прошла успешно
        Ok,
        /// Внутренняя ошибка ESP-NOW API
        InternalError,
        /// Неизвестная ошибка ESP API
        UnknownError,
    };

    /// Инициализировать протокол ESP-NOW
    static Result<Init> init() {
        return {translateInit(esp_now_init())};
    }

    enum class SetHandler {
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
    Result<SetHandler> setReceiveHandler(OnReceiveFunction &&handler) {
        _on_receive = std::move(handler);

        esp_err_t result;

        if (_on_receive == nullptr) {
            result = esp_now_unregister_recv_cb();
        } else {
            result = esp_now_register_recv_cb(onReceive);
        }

        return {translateSetHandler(result)};
    }

    /// Установить обработчик при доставке сообщений
    Result<SetHandler> setDeliveryHandler(OnDeliveryFunction &&handler) {
        _on_delivery = std::move(handler);

        esp_err_t result;

        if (_on_delivery == nullptr) {
            result = esp_now_unregister_send_cb();
        } else {
            result = esp_now_register_send_cb(onDelivery);
        }

        return {translateSetHandler(result)};
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
    enum class PeerAdd {
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
    static Result<PeerAdd> addPeer(const Mac &mac) {
        esp_now_peer_info_t peer = {
            .channel = 0,
            .ifidx = WIFI_IF_STA,
            .encrypt = false,
        };

        std::copy(mac.begin(), mac.end(), peer.peer_addr);

        return {translatePeerAdd(esp_now_add_peer(&peer))};
    }

    /// Удалить пир
    enum class PeerDelete {
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
    static Result<PeerDelete> deletePeer(const Mac &mac) {
        return {translatePeerDelete(esp_now_del_peer(mac.data()))};
    }

    /// Проверить существование пира
    static bool checkPeerExist(const Mac &mac) {
        return esp_now_is_peer_exist(mac.data());
    }

    /// Результат отправки сообщения
    enum class Send {
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
    template<typename T> static Result<Send> send(const Mac &mac, const T &value) {
        static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");

        return {
            translateSend(esp_now_send(
                mac.data(),
                reinterpret_cast<const u8 *>(&value),
                sizeof(T)
            ))
        };
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

    static Init translateInit(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return Init::Ok;
            case ESP_ERR_ESPNOW_INTERNAL:
                return Init::InternalError;
            default:
                return Init::UnknownError;
        }
    }

    static SetHandler translateSetHandler(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return SetHandler::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return SetHandler::NotInit;
            case ESP_ERR_ESPNOW_INTERNAL:
                return SetHandler::InternalError;
            default:
                return SetHandler::UnknownError;
        }
    }

    static Send translateSend(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return Send::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return Send::NotInit;
            case ESP_ERR_ESPNOW_ARG:
                return Send::InvalidArg;
            case ESP_ERR_ESPNOW_INTERNAL:
                return Send::InternalError;
            case ESP_ERR_ESPNOW_NO_MEM:
                return Send::NoMemory;
            case ESP_ERR_ESPNOW_NOT_FOUND:
                return Send::PeerNotFound;
            case ESP_ERR_ESPNOW_IF:
                return Send::IncorrectWiFiMode;
            default:
                return Send::UnknownError;
        }
    }

    static PeerDelete translatePeerDelete(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return PeerDelete::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return PeerDelete::NotInit;
            case ESP_ERR_ESPNOW_ARG:
                return PeerDelete::InvalidArg;
            case ESP_ERR_ESPNOW_NOT_FOUND:
                return PeerDelete::NotFound;
            default:
                return PeerDelete::UnknownError;
        }
    }

    static PeerAdd translatePeerAdd(esp_err_t result) {
        switch (result) {
            case ESP_OK:
                return PeerAdd::Ok;
            case ESP_ERR_ESPNOW_NOT_INIT:
                return PeerAdd::NotInit;
            case ESP_ERR_ESPNOW_ARG:
                return PeerAdd::InvalidArg;
            case ESP_ERR_ESPNOW_FULL:
                return PeerAdd::Full;
            case ESP_ERR_ESPNOW_NO_MEM:
                return PeerAdd::NoMemory;
            case ESP_ERR_ESPNOW_EXIST:
                return PeerAdd::Exists;
            default:
                return PeerAdd::UnknownError;
        }
    }

public:

    EspNow() = delete;

    EspNow(const EspNow &) = delete;

    EspNow &operator=(const EspNow &) = delete;

public:

    // toString

    template<typename E> static str toString(const Result<E> &result) {
        return toString(result.value);
    }

    static MacString toString(const Mac &mac) {
        MacString buff;

        auto raw = mac.data();
        sprintf(buff.data(), mac_format_string, raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);

        return buff;
    }

    static str toString(SetHandler value) {
        switch (value) {
            return_case(SetHandler::Ok)
            return_case(SetHandler::NotInit)
            return_case(SetHandler::InternalError)
            return_case(SetHandler::UnknownError)
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

    static str toString(Init value) {
        switch (value) {
            return_case(Init::Ok)
            return_case(Init::InternalError)
            return_case(Init::UnknownError)
            return_default()
        }
    }

    static str toString(PeerAdd value) {
        switch (value) {
            return_case(PeerAdd::Ok)
            return_case(PeerAdd::NotInit)
            return_case(PeerAdd::InvalidArg)
            return_case(PeerAdd::Full)
            return_case(PeerAdd::NoMemory)
            return_case(PeerAdd::Exists)
            return_case(PeerAdd::UnknownError)
            return_default()
        }
    }

    static str toString(PeerDelete value) {
        switch (value) {
            return_case(PeerDelete::Ok)
            return_case(PeerDelete::NotInit)
            return_case(PeerDelete::InvalidArg)
            return_case(PeerDelete::NotFound)
            return_case(PeerDelete::UnknownError)
            return_default()
        }
    }

    static str toString(Send value) {
        switch (value) {
            return_case(Send::Ok)
            return_case(Send::NotInit)
            return_case(Send::InvalidArg)
            return_case(Send::InternalError)
            return_case(Send::NoMemory)
            return_case(Send::PeerNotFound)
            return_case(Send::IncorrectWiFiMode)
            return_case(Send::UnknownError)
            return_default()
        }
    }
};
