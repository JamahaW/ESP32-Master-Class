#include "Arduino.h"
#include "GyverOLED.h"


using u8 = uint8_t;

struct Button {
    const u8 pin;

    void init() const { pinMode(pin, INPUT_PULLUP); }

    bool read() const { return not digitalRead(pin); }
};

/// Вектор на плоскости
struct Vector2D {
    float x, y;
};

/// Экспоненциальный фильтр
template<typename T> struct ExponentialFilter {
    float k;
    T filtered;

    constexpr explicit ExponentialFilter(const float &k, T &&init_value = 0) :
        k{k}, filtered{init_value} {}

    const T &calc(T value) {
        filtered += (value - filtered) * k;
        return filtered;
    }
};

#include <array>
#include <algorithm>


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

/// Джойстик с одной осью
struct JoystickAxis {

private:

    /// Максимальное аналоговое значение
    static constexpr auto max_analog_value = 4095;
    /// Аналоговый центр по умолчанию (Среднее значение)
    static constexpr auto default_analog_center = max_analog_value / 2;

    /// Пин подключения джойстика
    const u8 pin;
    /// Внешний фильтр значений
    ExponentialFilter<float> outer_filter;
    /// Внутренний фильтр аналоговых значений
    MedianFilter<int, 5> inner_filter{default_analog_center};
    /// Граница от центра на уменьшение
    float generic_edge{default_analog_center};
    /// Граница от центра на возрастание
    float positive_edge{default_analog_center};

public:

    explicit JoystickAxis(u8 pin, const float &k) :
        pin{pin}, outer_filter{k} {}

    /// Инициализировать джойстик
    inline void init() const { pinMode(pin, INPUT); }

    /// Обновить значение аналогового цента
    void updateCenter(int new_center) {
        generic_edge = float(new_center);
        positive_edge = max_analog_value - generic_edge;
    }

    /// Считать (сырое) аналоговое значение
    inline int readRaw() const { return analogRead(pin); }

    /// Считать нормализованное значение
    float read() {
        const auto raw = inner_filter.calc(readRaw());
        const auto value = outer_filter.calc(float(raw) - generic_edge);

        if (value < 0) {
            return value / generic_edge;
        } else {
            return value / positive_edge;
        }
    }
};


/// Джойстик с двумя осями
struct Joystick {

private:

    /// Ось джойстика X
    JoystickAxis axis_x;
    /// Ось джойстика Y
    JoystickAxis axis_y;

public:

    explicit Joystick(u8 pin_x, u8 pin_y, float &&filter_k) :
        axis_x{pin_x, filter_k},
        axis_y{pin_y, filter_k} {}

    /// Инициализировать джойстик
    void init() const {
        axis_x.init();
        axis_y.init();
    }

    /// выполнить калибровку центра джойстика
    void calibrate(int iterations) {
        constexpr auto period = 10;

        int sum_x = 0;
        int sum_y = 0;

        for (int i = 0; i < iterations; i++) {
            sum_x += axis_x.readRaw();
            sum_y += axis_y.readRaw();
            delay(period);
        }

        axis_x.updateCenter(sum_x / iterations);
        axis_y.updateCenter(sum_y / iterations);
    }

    /// Возвращаемое значение джойстика
    struct Value {
        /// Нормализованное значение по двум осям
        Vector2D axis;
        /// Вычисленная магнитуда по двум осям
        float magnitude;
    };

    /// Считать значение джойстика
    Value read() {
        const auto x = axis_x.read();
        const auto y = axis_y.read();
        const auto h = std::hypot(x, y);

        if (h < 0.01) { return {{0, 0}, 0}; }
        if (h > 1) { return {{x / h, y / h}, 1}; }
        return {{x, y}, h};
    }
};

auto joystick = Joystick(32, 33, 0.6);

auto button = Button{15};

auto screen = GyverOLED<SSD1306_128x64, OLED_BUFFER>();

void setup() {
    screen.init();
    joystick.init();

    Wire.setClock(1000000UL);

    joystick.calibrate(100);
}

void loop() {
    const auto v = joystick.read();
    bool b = button.read();

    screen.clear();

    {
        screen.setCursor(0, 0);
        screen.printf("x %.3f", v.axis.x);

        screen.setCursor(0, 1);
        screen.printf("y %.3f", v.axis.y);

        screen.setCursor(0, 2);
        screen.printf("p %.3f", v.magnitude);

        screen.setCursor(0, 3);
        screen.printf("b %s", b ? "high" : "low");
    }

    {
        constexpr int
            height = 63,
            width = 127,
            inner_circle_r = height / 6,
            outer_circle_r = int(double(height) / 2),
            pos_x = width - outer_circle_r,
            pos_y = height / 2;

        constexpr auto scale = float(outer_circle_r - inner_circle_r);

        auto joy_pos_x = int(v.axis.x * scale) + pos_x;
        auto joy_pos_y = int(v.axis.y * scale) + pos_y;

        screen.circle(pos_x, pos_y, outer_circle_r, OLED_STROKE);
        screen.circle(joy_pos_x, joy_pos_y, inner_circle_r, b ? OLED_FILL : OLED_STROKE);
        screen.line(pos_x, pos_y, joy_pos_x, joy_pos_y);
    }

    screen.update();

    const auto d = 1000 / 30;
    delay(d);
}