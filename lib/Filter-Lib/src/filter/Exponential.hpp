#pragma once

namespace filter {

    /// Экспоненциальный фильтр
    template<typename T> struct Exponential {
        float k;
        T filtered;

        constexpr explicit Exponential(const float &k, T &&init_value = 0) :
            k{k}, filtered{init_value} {}

        const T &calc(T value) {
            filtered += (value - filtered) * k;
            return filtered;
        }
    };
}