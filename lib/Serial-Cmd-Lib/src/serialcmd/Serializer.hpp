#pragma once

#include <Stream.h>


namespace serialcmd {

    /// Вспомогательный класс для чтения структур и примитивных типов из потока
    class Serializer {

    public:
        /// Используемый поток
        Stream &stream;

        explicit Serializer(Stream &stream) :
            stream(stream) {}

        /// Побайтово считать тип
        template<typename T> void read(T &destination) {
            this->stream.readBytes(reinterpret_cast<uint8_t *>(&destination), sizeof(T));
        }

        /// Побайтово записать тип
        template<typename T> void write(T &&source) {
            this->stream.write(reinterpret_cast<const uint8_t *>(&source), sizeof(T));
        }
    };
}