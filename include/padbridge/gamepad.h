#ifndef PADBRIDGE_GAMEPAD_H
#define PADBRIDGE_GAMEPAD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PADBRIDGE_BUTTON_A       = 1u << 0,
    PADBRIDGE_BUTTON_B       = 1u << 1,
    PADBRIDGE_BUTTON_X       = 1u << 2,
    PADBRIDGE_BUTTON_Y       = 1u << 3,
    PADBRIDGE_BUTTON_L       = 1u << 4,
    PADBRIDGE_BUTTON_R       = 1u << 5,
    PADBRIDGE_BUTTON_ZL      = 1u << 6,
    PADBRIDGE_BUTTON_ZR      = 1u << 7,
    PADBRIDGE_BUTTON_MINUS   = 1u << 8,
    PADBRIDGE_BUTTON_PLUS    = 1u << 9,
    PADBRIDGE_BUTTON_L3      = 1u << 10,
    PADBRIDGE_BUTTON_R3      = 1u << 11,
    PADBRIDGE_BUTTON_HOME    = 1u << 12,
    PADBRIDGE_BUTTON_CAPTURE = 1u << 13,
} padbridge_button_t;

typedef struct {
    uint16_t buttons;
    bool dpad_up;
    bool dpad_down;
    bool dpad_left;
    bool dpad_right;
    int16_t left_x;
    int16_t left_y;
    int16_t right_x;
    int16_t right_y;
    uint8_t battery_percent;
} padbridge_gamepad_state_t;

#endif

