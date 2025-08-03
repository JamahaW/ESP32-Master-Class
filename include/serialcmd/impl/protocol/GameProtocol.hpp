#pragma once

#include "serialcmd/abc/Protocol.hpp"


namespace serialcmd {
namespace impl {
namespace protocol {

struct GameProtocol : abc::Protocol<rs::u8, rs::u8> {

    ProtocolInstruction send_mac; ///< Отправить MAC адрес
    ProtocolInstruction send_log; ///< Отправить лог
    ProtocolInstruction send_espnow_client_packet; ///< Отправить пакет от клиента
    ProtocolInstruction send_espnow_delivery_status; ///< Отправить статус доставки сообщения

    explicit GameProtocol(Stream &stream) :
        abc::Protocol<rs::u8, rs::u8>(stream),
        send_mac(createSender()),
        send_log(createSender()),
        send_espnow_client_packet(createSender()),
        send_espnow_delivery_status(createSender()) {}
};

}
}
}