#include "padbridge/rumble.h"

#include <assert.h>
#include <stdio.h>

static void test_neutral_packet_stops_rumble(void)
{
    const uint8_t neutral[8] = {
        0x00, 0x01, 0x40, 0x40,
        0x00, 0x01, 0x40, 0x40,
    };
    padbridge_rumble_command_t command = {255u, 255u, 999u};
    padbridge_switch_decode_rumble(neutral, &command);
    assert(command.weak_magnitude == 0u);
    assert(command.strong_magnitude == 0u);
    assert(command.duration_ms == 0u);
}

static void test_motors_decode_independently(void)
{
    const uint8_t encoded[8] = {
        0x00, 0x01, 0x80, 0x48,
        0x00, 0x01, 0x00, 0x60,
    };
    padbridge_rumble_command_t command = {0};
    padbridge_switch_decode_rumble(encoded, &command);
    assert(command.weak_magnitude > 0u);
    assert(command.strong_magnitude > command.weak_magnitude);
    assert(command.duration_ms == 120u);
}

static void test_amplitude_is_clamped(void)
{
    const uint8_t encoded[8] = {
        0x00, 0x00, 0x80, 0xff,
        0x00, 0x00, 0x80, 0xff,
    };
    padbridge_rumble_command_t command = {0};
    padbridge_switch_decode_rumble(encoded, &command);
    assert(command.weak_magnitude == 255u);
    assert(command.strong_magnitude == 255u);
}

int main(void)
{
    test_neutral_packet_stops_rumble();
    test_motors_decode_independently();
    test_amplitude_is_clamped();
    puts("rumble tests passed");
    return 0;
}
