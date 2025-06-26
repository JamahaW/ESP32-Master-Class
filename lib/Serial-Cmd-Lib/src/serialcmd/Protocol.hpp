#pragma once

#include <functional>
#include <vector>

#include "serialcmd/Serializer.hpp"

#include "rs/primitives.hpp"


namespace serialcmd {
/// Протокол P2P общения
template<
    typename LocalInstructionCode, ///< Тип кода отправляемой инструкции
    typename RemoteInstructionCode ///< Тип кода принимаемой инструкции
> struct Protocol {

public:

    using onReceiceFunction = std::function<void(Serializer &)>;

private:

    /// Сериализатор
    Serializer serializer;
    /// Обработчики приёма данных
    std::vector<onReceiceFunction> receive_handlers{};

public:

    explicit Protocol(Stream &stream) :
        serializer{stream} {}

    void addReceiver(onReceiceFunction &&handler) {
        receive_handlers.push_back(std::move(handler));
    }

    /// Обновление
    void pull() {
        if (serializer.stream.available() < sizeof(RemoteInstructionCode)) { return; }

        RemoteInstructionCode code;
        serializer.read(code);

        if (code >= receive_handlers.size()) { return; }

        receive_handlers[code](serializer);
    }

public:

    Protocol() = delete;

};

}