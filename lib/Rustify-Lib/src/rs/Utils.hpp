#pragma once

#include <array>
#include <cstdarg>


namespace rs {

    /// Получить форматированную строку
    template<size_t N> std::array<char, N> formatted(const char *format, ...) {
        std::array<char, N> buffer{};

        va_list args;
        va_start(args, format);

        vsnprintf(buffer.data(), N, format, args);

        va_end(args);

        if (N > 0) { buffer[N - 1] = '\0'; }

        return buffer;
    }
}

