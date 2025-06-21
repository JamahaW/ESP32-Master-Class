#include "Arduino.h"
#include "GyverOLED.h"


using u8 = uint8_t;

struct Button {
    const u8 pin;

    void init() const { pinMode(pin, INPUT_PULLUP); }

    bool read() const { return not digitalRead(pin); }
};

struct Vector2D { float x, y; };

template<typename T> struct ExponentialFilter {
    const float k;
    T filtered;

    constexpr explicit ExponentialFilter(float k, T &&init_value) :
        k{k}, filtered{init_value} {}

    const T &calc(T value) {
        filtered += (value - filtered) * k;
        return filtered;
    }
};

struct Joystick {
    using TimeMs = uint32_t;

    struct JoystickAxis {
        static constexpr int max_value = 4095;
        static constexpr auto default_center = max_value / 2;

        const u8 pin;
        float center;
        float center_max;
        ExponentialFilter<float> filter;

        static constexpr JoystickAxis create(u8 pin) {
            return {
                .pin = pin,
                .center = default_center,
                .center_max = default_center,
                .filter = ExponentialFilter<float>(0.3f, default_center),
            };
        }

        void updateCenter(int new_center) {
            center = float(new_center);
            center_max = max_value - center;
        }

        void init() const { pinMode(pin, INPUT); }

        int readRaw() const { return analogRead(pin); }

        float readNormalized() {
            const auto value = filter.calc(float(readRaw()) - center);

            if (value < 0) {
                return value / center;
            } else {
                return value / center_max;
            }
        }

        JoystickAxis() = delete;
    };

    struct Value {
        Vector2D axis;
        float power;
    };

    JoystickAxis axis_x, axis_y;

    static constexpr Joystick create(u8 pin_x, u8 pin_y) {
        return {
            .axis_x = JoystickAxis::create(pin_x),
            .axis_y = JoystickAxis::create(pin_y),
        };
    }

    void init() const {
        axis_x.init();
        axis_y.init();
    }

    void calibrate(int iterations) {
        constexpr TimeMs period = 10;

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

    Value read() {
        const auto x = axis_x.readNormalized();
        const auto y = axis_y.readNormalized();
        const auto h = std::hypot(x, y);

        if (h < 0.01) { return {{0, 0}, 0}; }

        if (h > 1) { return {{x / h, y / h}, 1}; }

        return {{x, y}, h};
    }

    Joystick() = delete;
};

auto joystick = Joystick::create(32, 33);

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
        screen.printf("p %.3f", v.power);

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