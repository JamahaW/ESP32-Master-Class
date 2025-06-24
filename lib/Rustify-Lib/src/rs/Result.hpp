#pragma once

#include "primitives.hpp"


namespace rs {

    /// Result тип на основе перечисления
    template<typename E> struct Result {

        /// значение
        const E value;

        /// Значение является успехом
        constexpr bool ok() const noexcept {
            return static_cast<int>(value) == 0;
        }

        /// Значение является ошибкой
        constexpr bool fail() const noexcept {
            return not ok();
        }

        Result() = delete;
    };
}

