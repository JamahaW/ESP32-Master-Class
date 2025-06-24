#pragma once

#include "serialcmd/StreamSerializer.hpp"


namespace serialcmd {

    /// Протокол общения с мастер-устройством
    template<
        /// Тип индекса команды
        typename CommandIndex,
        /// Startup пакет данных
        typename Startup
    >
    class Protocol {
    public:
        /// Функция обработки команды
        typedef void(*CommandFunc)(StreamSerializer &);

    private:

        /// Таблица команд
        CommandFunc *commands;

        /// Кол-во команд
        const CommandIndex command_count;

        StreamSerializer serializer;

    public:

        explicit Protocol(
            CommandFunc *commands,
            const CommandIndex command_count,
            Stream &stream
        ) :
            commands(commands), command_count(command_count), serializer(stream) {}

        /// Отправить начальный пакет
        void begin(Startup &&startup) {
            serializer.write(startup);
        }

        /// Обновление
        void pull() {
            if (serializer.stream.available() < int(sizeof(CommandIndex))) { return; }

            CommandIndex cmd_index;
            serializer.read(cmd_index);

            if (cmd_index >= command_count) { return; }

            commands[cmd_index](serializer);
        }
    };
}