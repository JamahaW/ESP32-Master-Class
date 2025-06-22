#include "joystick/Joystick.hpp"

#include "Arduino.h"
#include "GyverOLED.h"


using u8 = uint8_t;

struct Button {
    const u8 pin;

    void init() const { pinMode(pin, INPUT_PULLUP); }

    bool read() const { return not digitalRead(pin); }
};


auto j = joystick::Joystick(32, 33, 0.6);

auto button = Button{15};

auto screen = GyverOLED<SSD1306_128x64, OLED_BUFFER>();

void setup() {
    screen.init();
    j.init();

    Wire.setClock(1000000UL);

    j.calibrate(100);
}

void loop() {
    const auto v = j.read();
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
            outer_circle_r = height / 2,
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