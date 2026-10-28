#include "padbridge/switch_report.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint16_t unpack_x(const uint8_t packed[3])
{
    return (uint16_t)(packed[0] | ((packed[1] & 0x0fu) << 8));
}

static uint16_t unpack_y(const uint8_t packed[3])
{
    return (uint16_t)((packed[1] >> 4) | (packed[2] << 4));
}

static void test_axis_conversion(void)
{
    assert(padbridge_switch_axis_from_i16(INT16_MIN) == 0u);
    assert(padbridge_switch_axis_from_i16(0) == 2048u);
    assert(padbridge_switch_axis_from_i16(INT16_MAX) == 4095u);
}

static void test_centered_report(void)
{
    const padbridge_gamepad_state_t state = {
        .battery_percent = 100u,
    };
    padbridge_switch_report_30_t report;

    padbridge_switch_build_report(&state, 42u, &report);

    assert(report.timer == 42u);
    assert(report.connection_info == 0x81u);
    assert(unpack_x(report.left_stick) == 2048u);
    assert(unpack_y(report.left_stick) == 2048u);
    assert(unpack_x(report.right_stick) == 2048u);
    assert(unpack_y(report.right_stick) == 2048u);
}

static void test_buttons_and_dpad(void)
{
    const padbridge_gamepad_state_t state = {
        .buttons = PADBRIDGE_BUTTON_A | PADBRIDGE_BUTTON_ZR |
                   PADBRIDGE_BUTTON_PLUS | PADBRIDGE_BUTTON_HOME |
                   PADBRIDGE_BUTTON_L,
        .dpad_up = true,
        .dpad_left = true,
        .left_x = INT16_MAX,
        .left_y = INT16_MIN,
        .battery_percent = 30u,
    };
    padbridge_switch_report_30_t report;

    padbridge_switch_build_report(&state, 0u, &report);

    assert(report.buttons[0] == ((1u << 3) | (1u << 7)));
    assert(report.buttons[1] == ((1u << 1) | (1u << 4)));
    assert(report.buttons[2] == ((1u << 1) | (1u << 3) | (1u << 6)));
    assert(unpack_x(report.left_stick) == 4095u);
    assert(unpack_y(report.left_stick) == 0u);
    assert(report.connection_info == 0x41u);
}

int main(void)
{
    test_axis_conversion();
    test_centered_report();
    test_buttons_and_dpad();
    puts("switch_report: all tests passed");
    return 0;
}

