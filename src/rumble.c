#include "padbridge/rumble.h"

#include <stddef.h>

#define PADBRIDGE_RUMBLE_KEEPALIVE_MS 120u

static uint8_t decode_motor_magnitude(const uint8_t encoded[4])
{
    // Nintendo's amplitude table advances byte 3 from 0x40 to 0x72;
    // bit 7 of byte 2 selects the half-step. 0x00,0x01,0x40,0x40 is neutral.
    if (encoded[3] < 0x40u) {
        return 0u;
    }

    uint16_t amplitude_index = (uint16_t)(encoded[3] - 0x40u) * 2u;
    if ((encoded[2] & 0x80u) != 0u) {
        amplitude_index++;
    }
    if (amplitude_index > 100u) {
        amplitude_index = 100u;
    }
    return (uint8_t)((amplitude_index * 255u) / 100u);
}

void padbridge_switch_decode_rumble(
    const uint8_t encoded[8],
    padbridge_rumble_command_t *command)
{
    if (encoded == NULL || command == NULL) {
        return;
    }

    command->weak_magnitude = decode_motor_magnitude(encoded);
    command->strong_magnitude = decode_motor_magnitude(encoded + 4);
    command->duration_ms =
        command->weak_magnitude == 0u && command->strong_magnitude == 0u
            ? 0u
            : PADBRIDGE_RUMBLE_KEEPALIVE_MS;
}
