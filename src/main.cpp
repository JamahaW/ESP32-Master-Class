#include "game/impl/Client.hpp"
#include "game/impl/Host.hpp"
#include "game/core/Protocol.hpp"

#include <Arduino.h>

#include <esp_wifi.h>
#include <nvs.h>
#include <nvs_flash.h>


/// Адрес сервера
const EspNow::Mac server_address = {0x78, 0x1C, 0x3C, 0xA4, 0x9E, 0x7C};


void initWiFiSTA();

/// Запуск сервера
[[noreturn]] void runServer() {
    game::impl::Host host;

    host.init();

    while (true) {
        host.pull();
    }
}


/// Запуск клиента
[[noreturn]] void runClient() {
    game::impl::Client client(server_address);

    client.init();
    client.sendMessage(game::core::ClientMessage{"DarkWoldX17"});

    while (true) {
        client.pull();
    }
}

void setup() {
    Serial.begin(115200);

    // Инициализация WiFi в режиме станции
    initWiFiSTA();

    EspNow::init();

    auto &esp_now = EspNow::instance();
    const bool is_server = esp_now.mac == server_address;

    Serial.printf(
        "Self: %s (Role: %s)\n",
        EspNow::toString(esp_now.mac).data(),
        is_server ? "Server" : "Client"
    );

    if (is_server) {
        runServer();
    } else {
        runClient();
    }
}

void initWiFiSTA() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void loop() {}