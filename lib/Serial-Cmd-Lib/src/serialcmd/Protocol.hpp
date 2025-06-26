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

    /// Инструкция
    struct Instruction {

    private:

        /// Код инструкции
        const LocalInstructionCode code;
        /// Сериализатор
        Serializer &ser;

    public:

        Instruction(LocalInstructionCode code, Serializer &s) :
            code{code}, ser{s} {}

        /// Отправить инструкцию
        template<typename T> void send(T &&value) {
            ser.write(code);
            ser.write(value);
        }

        Instruction() = delete;
    };

private:

    /// Сериализатор
    Serializer serializer;
    /// Обработчики приёма данных
    std::vector<onReceiceFunction> receive_handlers{};
    /// Обработчики отправки данных
    LocalInstructionCode senders{0};

public:

    explicit Protocol(Stream &stream) :
        serializer{stream} {}

    void addReceiver(onReceiceFunction &&handler) {
        receive_handlers.push_back(std::move(handler));
    }

    const Instruction &addSender() {
        auto instruction = Instruction(LocalInstructionCode(senders), serializer);
        senders += 1;
        return instruction;
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