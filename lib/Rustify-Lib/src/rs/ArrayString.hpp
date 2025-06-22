#pragma once

#include <array>
#include <cstdarg>
#include "rs/primitives.hpp"


namespace rs {

    template<rs::size N> using ArrayString = std::array<char, N>;

    /// Создать с форматированием
    template<rs::size N> ArrayString<N> formatted(const char *format, ...) {
        ArrayString < N > ret{};

        if (N == 0) { return ret; }

        va_list args;
        va_start(args, format);

        vsnprintf(ret.data(), N, format, args);
        ret[N - 1] = '\0';

        va_end(args);

        return ret;
    }

}

