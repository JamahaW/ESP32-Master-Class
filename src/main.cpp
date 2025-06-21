#include "Arduino.h"


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
    using Value = int;
    using TimeMs = uint32_t;

    struct JoystickAxis {
        static constexpr Value max_value = 4095;
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
                .filter = ExponentialFilter<float>(0.5f, 0),
            };
        }

        void updateCenter(Value new_center) {
            center = float(new_center);
            center_max = max_value - center;
        }

        void init() const { pinMode(pin, INPUT); }

        Value readRaw() const { return analogRead(pin); }

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

        Value sum_x = 0;
        Value sum_y = 0;

        for (int i = 0; i < iterations; i++) {
            sum_x += axis_x.readRaw();
            sum_y += axis_y.readRaw();
            delay(period);
        }

        axis_x.updateCenter(sum_x / iterations);
        axis_y.updateCenter(sum_y / iterations);
    }

    Vector2D read() {
        const auto x = axis_x.readNormalized();
        const auto y = axis_y.readNormalized();
        const auto h = std::hypot(x, y);

        if (h < 0.01) { return {0, 0}; }

        return {x / h, y / h};
    }

    Joystick() = delete;
};

auto joystick = Joystick::create(32, 33);

auto button = Button{15};

void setup() {
    joystick.init();

    Serial.begin(115200);

    joystick.calibrate(100);
}

void loop() {
    const auto a = joystick.read();
    bool b = button.read();

    Serial.printf("x: %03.2f\ty: %03.2f\tb: %s\n", a.x, a.y, b ? "HIGH" : "LOW");

    delay(100);
}