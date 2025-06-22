#include <Arduino.h>

#include <WiFi.h>
#include <esp_now.h>


const int pin_led = 12, pin_button = 32;

// Определяем целевой MAC адрес
std::array<uint8_t, 6> target_mac = {0xfc, 0xe8, 0xc0, 0x74, 0xa6, 0x30};

// Определяем структуру нашего пакета, используем атрибут gnu::packed (без выравнивания)
struct [[gnu::packed]] Packet {
    uint32_t send_time_ms;
    bool led_state;
};

// Обработчик приёма данных
void onReceive(const uint8_t *mac, const uint8_t *data, int size) {
    // Интерпретация данных как пакета
    const auto &packet = *reinterpret_cast<const Packet *>(data);

    digitalWrite(pin_led, packet.led_state);
    Serial.printf("%u -> %s", packet.send_time_ms, packet.led_state ? "HIGH" : "LOW");
}

// Обработчик на доставку данных
void onSend(const uint8_t *mac, esp_now_send_status_t status) {
    // esp_now_send_status_t - перечисление из двух значений
    // - ESP_NOW_SEND_SUCCESS (= 0)
    // - ESP_NOW_SEND_FAIL (= 1)

    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.println("ESP_NOW_SEND_SUCCESS");
    } else {
        Serial.println("ESP_NOW_SEND_FAIL");
    }
}

void setup() {
    pinMode(12, OUTPUT);
    pinMode(32, INPUT_PULLUP);
    Serial.begin(9600);

    WiFi.mode(WIFI_STA); // Устанавливаем режим работы NOLINT(*-static-accessed-through-instance)
    ESP_ERROR_CHECK(esp_now_init()); // Инициализируем протокол
    // макрос ESP_ERROR_CHECK(x) реализует проверку, а в случае ошибки завершает исполнение

    // Зарегистрирует функцию при получении пакета
    esp_now_register_recv_cb(onReceive);

    // Зарегистрирует функцию, которая будет вызвана при доставке сообщения (получатель получил наше сообщение)
    // мы отправили - дошло ли до получателя, проверить статус
    esp_now_register_send_cb(onSend);

    // Добавляем пир - структура, важно peer_addr,
    esp_now_peer_info_t peer = {};
    std::copy(target_mac.begin(), target_mac.end(), peer.peer_addr);
//    memcpy(peer.peer_addr, target_mac.data(), sizeof(peer.peer_addr)); // Побайтовое копирование

    ESP_ERROR_CHECK(esp_now_add_peer(&peer)); // Добавляем peer
}

void loop() {
    // Заполняем пакет
    Packet packet = {
        .send_time_ms = millis(),
        .led_state = not digitalRead(pin_button)
    };

    // Отправляем пакет и получаем статус отправки в очередь сообщений
    esp_err_t result = esp_now_send(
        // целевой MAC (передаём сырой указатель)
        target_mac.data(),
        // Приводим данные к сырому указателю (реинтерпретация указателя)
        reinterpret_cast<uint8_t *>(&packet),
        // Размер пакета определяем как размер структуры
        sizeof(Packet)
    );

    if (result != ESP_OK) {
        // Переводим код ошибки в её наименование
        Serial.println(esp_err_to_name(result));
    }

    delay(100);
}