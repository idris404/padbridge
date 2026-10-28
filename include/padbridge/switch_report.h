#ifndef PADBRIDGE_SWITCH_REPORT_H
#define PADBRIDGE_SWITCH_REPORT_H

#include <stddef.h>
#include <stdint.h>

#include "padbridge/gamepad.h"

#define PADBRIDGE_SWITCH_REPORT_ID 0x30u
#define PADBRIDGE_SWITCH_PAYLOAD_SIZE 63u

typedef struct {
    uint8_t timer;
    uint8_t connection_info;
    uint8_t buttons[3];
    uint8_t left_stick[3];
    uint8_t right_stick[3];
    uint8_t vibrator_report;
    uint8_t imu[36];
    uint8_t reserved[15];
} padbridge_switch_report_30_t;

_Static_assert(sizeof(padbridge_switch_report_30_t) == PADBRIDGE_SWITCH_PAYLOAD_SIZE,
               "Switch report 0x30 payload must be 63 bytes");

uint16_t padbridge_switch_axis_from_i16(int16_t value);

void padbridge_switch_pack_stick(uint16_t x, uint16_t y, uint8_t output[3]);

void padbridge_switch_build_report(
    const padbridge_gamepad_state_t *state,
    uint8_t timer,
    padbridge_switch_report_30_t *report);

#endif

