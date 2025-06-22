#pragma once


namespace lina {

    /// Вектор на плоскости
    template<typename T> struct Vector2D {
        /// Тип значения компонента вектора
        using Value = T;

        T x, y;
    };
}