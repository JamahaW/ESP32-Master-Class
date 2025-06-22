#pragma once

#include <array>
#include <algorithm>


namespace filter {
    /// Медианный фильтр с буфером произвольного нечётного размера (2*N+1)
    template<typename T, size_t N> struct MedianFilter {
        static_assert(N % 2 == 1, "MedianFilter requires odd buffer size (2*K+1)");

    private:

        std::array<T, N> buffer{};
        size_t next_index{0};

    public:

        explicit MedianFilter(T init_value = 0) {
            buffer.fill(init_value);
        }

        /// Рассчитать медиану
        const T &calc(T &&value) {
            buffer[next_index] = value;
            next_index = (next_index + 1) % N;

            auto sorted = buffer;
            auto middle = sorted.begin() + N / 2;

            std::nth_element(sorted.begin(), middle, sorted.end());

            return *middle;
        }
    };
}