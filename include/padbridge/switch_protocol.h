#ifndef PADBRIDGE_SWITCH_PROTOCOL_H
#define PADBRIDGE_SWITCH_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "padbridge/gamepad.h"

#define PADBRIDGE_SWITCH_WIRE_REPORT_SIZE 64u

typedef struct {
    uint8_t request[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE];
    padbridge_gamepad_state_t gamepad;
    uint8_t address[6];
    size_t request_size;
    uint32_t timestamp_ms;
    uint8_t timer;
    uint8_t vibration_report;
    uint8_t player_number;
    bool has_request;
    bool imu_enabled;
    bool vibration_enabled;
} padbridge_switch_protocol_t;

void padbridge_switch_protocol_init(
    padbridge_switch_protocol_t *protocol,
    uint8_t controller_index);

void padbridge_switch_protocol_set_gamepad(
    padbridge_switch_protocol_t *protocol,
    const padbridge_gamepad_state_t *state);

void padbridge_switch_protocol_receive(
    padbridge_switch_protocol_t *protocol,
    const uint8_t *report,
    size_t report_size);

size_t padbridge_switch_protocol_next_report(
    padbridge_switch_protocol_t *protocol,
    uint32_t now_ms,
    uint8_t output[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE]);

#endif

