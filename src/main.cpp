// Занятие 5.2 Примитивный мессенджер

#include <Arduino.h>

#include <WiFi.h>
#include <esp_now.h>


// определяем широковещательный пир (Для отправки сообщения всех в сети)
esp_now_peer_info_t peer = {};

// Определяем вид пакета для сообщения и создаём экземпляр
struct [[gnu::packed]] Message {
    std::array<char, 8> username;
    std::array<char, 64> content;
};

Message packet = {
    .username = {"Alpha"},  // Имя пользователя
    .content = {}           // Содержание сообщения пока оставим пустым (заполнено '\0')
};

void onReceive(const uint8_t *, const uint8_t *data, int size) {
    // Игнорируем данные, не являющиеся пакетом (Проверка размера)
    if (size != sizeof(Message)) { return; }

    const auto &message = reinterpret_cast<const Message *>(data); // Интерпретируем пакет

    Serial.printf("<%s> %s\n", message->username.data(), message->content.data());
}

void setup() {
    Serial.begin(115200);

    // Инициализируем протокол
    WiFi.mode(WIFI_STA);
    esp_now_init();
    esp_now_register_recv_cb(onReceive);

    // Добавляем широковещательный пир
    std::array<uint8_t, 6> broadcast_address = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    std::copy(broadcast_address.begin(), broadcast_address.end(), peer.peer_addr);
    esp_now_add_peer(&peer);
}

void loop() {
    delay(1);

    // Ждём пока в порте появится сообщение
    if (Serial.available() < 1) { return; }

    // Заполняем пакет
    Serial.readBytesUntil('\n', packet.content.data(), packet.content.size());

    // Отправляем сообщение
    esp_err_t result = esp_now_send(peer.peer_addr, reinterpret_cast<uint8_t *>(&packet), sizeof(Message));
    if (result != ESP_OK) { Serial.println(esp_err_to_name(result)); }
}