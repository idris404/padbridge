#ifndef PADBRIDGE_RUMBLE_H
#define PADBRIDGE_RUMBLE_H

#include <stdint.h>

typedef struct {
    uint8_t weak_magnitude;
    uint8_t strong_magnitude;
    uint16_t duration_ms;
} padbridge_rumble_command_t;

// Convert Switch HD-rumble blocks to the simple dual-motor magnitudes exposed
// by Bluepad32. Frequency is discarded because most controllers only expose
// motor strength.
void padbridge_switch_decode_rumble(
    const uint8_t encoded[8],
    padbridge_rumble_command_t *command);

#endif
