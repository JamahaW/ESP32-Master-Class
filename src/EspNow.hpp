#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <esp_err.h>
#include <esp_now.h>
#include <cstring>


/// Обёртка над ESP-NOW API с использованием С++
struct EspNow {
    using u8 = uint8_t;
    /// Безопасный тип для MAC адреса
    using Mac = std::array<u8, ESP_NOW_ETH_ALEN>;

    /// Статус доставки
    enum class SendStatus {
        Ok, /// Пакет дошел до получателя
        Fail, /// Не удалось доставить пакет
    };

    /// Получить экземпляр протокола для настройки
    static EspNow &instance() {
        static EspNow instance = {
            .on_send = nullptr,
            .on_receive = nullptr
        };
        return instance;
    }

    /// Обработчик доставки сообщения
    std::function<void(Mac, SendStatus)> on_send;
    /// Обработчик получения сообщения
    std::function<void(Mac, const void *, u8 len)> on_receive;

    /// Результат инициализации
    enum class InitResult {
        Ok, /// Инициализация прошла успешно
        InitFail, /// Не удалось инициализировать протокол ESP-NOW
        ReceiveHandlerRegisterFail, /// Не удалось зарегистрировать обработчик поступающего сообщения
        SendHandlerRegisterFail, /// Не удалось зарегистрировать обработчик проверки доставки
    };

    /// Инициализировать протокол ESP-NOW
    static InitResult init() {
        if (esp_now_init() == ESP_ERR_ESPNOW_INTERNAL) { return InitResult::InitFail; }

        auto _on_receive = [](const u8 *mac, const u8 *data, int size) {
            auto &self = EspNow::instance();
            if (self.on_receive == nullptr) { return; }
            self.on_receive(
                castMac(mac),
                static_cast<const void *>(data),
                u8(size)
            );
        };

        if (esp_now_register_recv_cb(_on_receive) != ESP_OK) { return InitResult::ReceiveHandlerRegisterFail; }

        auto _on_send = [](const u8 *mac, esp_now_send_status_t status) {
            auto &self = EspNow::instance();
            if (self.on_send == nullptr) { return; }
            self.on_send(
                castMac(mac),
                (status == ESP_NOW_SEND_SUCCESS) ? SendStatus::Ok : SendStatus::Fail
            );
        };

        if (esp_now_register_send_cb(_on_send) != ESP_OK) { return InitResult::SendHandlerRegisterFail; }

        return InitResult::Ok;
    }

    /// Завершить работу протокола
    static void quit() { esp_now_deinit(); }

    /// Результат добавления пира
    enum class AddPeerResult {
        Ok, /// Пир успешно добавлен
        NotInit, /// Протокол ESP-NOW не был инициализирован
        InvalidArg, /// Неверный аргумент
        Full, /// Список пиров полон
        NoMemory, /// Не хватает памяти для добавления пира
        Exists, /// Пир уже добавлен
        UnknownError, /// Неизвестная ошибка ESP API
    };

    /// Добавить пир
    static AddPeerResult addPeer(const Mac &mac) {
        esp_now_peer_info_t peer = {};
        memcpy(peer.peer_addr, mac.data(), 6);

        esp_err_t result = esp_now_add_peer(&peer);

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

    /// Удалить пир
    enum class DeletePeerResult {
        Ok, /// Пир успешно удален
        NotInit, /// Протокол ESP-NOW не был инициализирован
        InvalidArg,/// Неверный аргумент
        NotFound, /// Пир не найден в списке добавленных
        UnknownError, /// Неизвестная ошибка ESP API
    };

    /// Удалить пир
    static DeletePeerResult deletePeer(const Mac &mac) {
        esp_err_t result = esp_now_del_peer(mac.data());

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

    /// Проверить существование пира
    static bool checkPeerExist(const Mac &mac) {
        return esp_now_is_peer_exist(mac.data());
    }

    /// Результат отправки сообщения
    enum class SendResult {
        Ok, /// Сообщение успешно отправлено
        NotInit, /// Протокол ESP-NOW не был инициализирован
        InvalidArg,/// Неверный аргумент
        InternalError, /// Внутренняя ошибка ESP-NOW API
        NoMemory, /// Не хватает памяти для отправки сообщения
        PeerNotFound, /// Целевой пир не найден
        IncorrectWiFiMode, /// Установлен неверный режим интерфейса WiFi
        UnknownError,/// Неизвестная ошибка ESP API
    };

    /// Отправить сообщение
    template<typename T> static SendResult send(const Mac &mac, const T &value) {
        static_assert(sizeof(T) < 250, "Message is too big!");

        esp_err_t result = esp_now_send(
            mac.data(),
            reinterpret_cast<const u8 *>(&value),
            sizeof(T)
        );

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

    EspNow() = delete;

    EspNow(const EspNow &) = delete;

    EspNow &operator=(const EspNow &) = delete;

private:
    inline static const Mac &castMac(const u8 *mac) {
        return *reinterpret_cast<const Mac *>(mac);
    }
};