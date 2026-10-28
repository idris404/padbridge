#include "padbridge/switch_report.h"

#include <string.h>

enum {
    SWITCH_Y = 1u << 0,
    SWITCH_X = 1u << 1,
    SWITCH_B = 1u << 2,
    SWITCH_A = 1u << 3,
    SWITCH_R = 1u << 6,
    SWITCH_ZR = 1u << 7,

    SWITCH_MINUS = 1u << 0,
    SWITCH_PLUS = 1u << 1,
    SWITCH_R3 = 1u << 2,
    SWITCH_L3 = 1u << 3,
    SWITCH_HOME = 1u << 4,
    SWITCH_CAPTURE = 1u << 5,

    SWITCH_DPAD_DOWN = 1u << 0,
    SWITCH_DPAD_UP = 1u << 1,
    SWITCH_DPAD_RIGHT = 1u << 2,
    SWITCH_DPAD_LEFT = 1u << 3,
    SWITCH_L = 1u << 6,
    SWITCH_ZL = 1u << 7,
};

static void set_if_pressed(
    uint16_t source,
    uint16_t source_mask,
    uint8_t *destination,
    uint8_t destination_mask)
{
    if ((source & source_mask) != 0u) {
        *destination |= destination_mask;
    }
}

uint16_t padbridge_switch_axis_from_i16(int16_t value)
{
    const int32_t shifted = (int32_t)value + 32768;
    return (uint16_t)((shifted * 4095 + 32767) / 65535);
}

void padbridge_switch_pack_stick(uint16_t x, uint16_t y, uint8_t output[3])
{
    x &= 0x0fffu;
    y &= 0x0fffu;

    output[0] = (uint8_t)(x & 0xffu);
    output[1] = (uint8_t)(((x >> 8) & 0x0fu) | ((y & 0x0fu) << 4));
    output[2] = (uint8_t)((y >> 4) & 0xffu);
}

void padbridge_switch_build_report(
    const padbridge_gamepad_state_t *state,
    uint8_t timer,
    padbridge_switch_report_30_t *report)
{
    memset(report, 0, sizeof(*report));

    report->timer = timer;

    // Wired connection with a coarse four-level battery indication.
    const uint8_t battery_level = state->battery_percent >= 75u ? 8u
                                : state->battery_percent >= 50u ? 6u
                                : state->battery_percent >= 25u ? 4u
                                : 2u;
    report->connection_info = (uint8_t)((battery_level << 4) | 0x01u);

    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_Y, &report->buttons[0], SWITCH_Y);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_X, &report->buttons[0], SWITCH_X);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_B, &report->buttons[0], SWITCH_B);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_A, &report->buttons[0], SWITCH_A);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_R, &report->buttons[0], SWITCH_R);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_ZR, &report->buttons[0], SWITCH_ZR);

    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_MINUS, &report->buttons[1], SWITCH_MINUS);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_PLUS, &report->buttons[1], SWITCH_PLUS);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_R3, &report->buttons[1], SWITCH_R3);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_L3, &report->buttons[1], SWITCH_L3);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_HOME, &report->buttons[1], SWITCH_HOME);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_CAPTURE, &report->buttons[1], SWITCH_CAPTURE);

    if (state->dpad_down) {
        report->buttons[2] |= SWITCH_DPAD_DOWN;
    }
    if (state->dpad_up) {
        report->buttons[2] |= SWITCH_DPAD_UP;
    }
    if (state->dpad_right) {
        report->buttons[2] |= SWITCH_DPAD_RIGHT;
    }
    if (state->dpad_left) {
        report->buttons[2] |= SWITCH_DPAD_LEFT;
    }
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_L, &report->buttons[2], SWITCH_L);
    set_if_pressed(state->buttons, PADBRIDGE_BUTTON_ZL, &report->buttons[2], SWITCH_ZL);

    padbridge_switch_pack_stick(
        padbridge_switch_axis_from_i16(state->left_x),
        padbridge_switch_axis_from_i16(state->left_y),
        report->left_stick);
    padbridge_switch_pack_stick(
        padbridge_switch_axis_from_i16(state->right_x),
        padbridge_switch_axis_from_i16(state->right_y),
        report->right_stick);
}

