#include "padbridge/switch_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_initial_usb_identification(void)
{
    padbridge_switch_protocol_t protocol;
    uint8_t output[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE];

    padbridge_switch_protocol_init(&protocol, 2u);
    assert(padbridge_switch_protocol_next_report(&protocol, 1u, output) == 64u);
    assert(output[0] == 0x81u);
    assert(output[1] == 0x01u);
    assert(output[3] == 0x03u);
    assert(output[4] == 0x02u);
    assert(output[9] == 0x7cu);
}

static void test_device_info_subcommand(void)
{
    padbridge_switch_protocol_t protocol;
    uint8_t request[64] = {0};
    uint8_t output[64];

    padbridge_switch_protocol_init(&protocol, 0u);
    (void)padbridge_switch_protocol_next_report(&protocol, 1u, output);

    request[0] = 0x01u;
    request[10] = 0x02u;
    padbridge_switch_protocol_receive(&protocol, request, sizeof(request));
    (void)padbridge_switch_protocol_next_report(&protocol, 2u, output);

    assert(output[0] == 0x21u);
    assert(output[13] == 0x82u);
    assert(output[14] == 0x02u);
    assert(output[17] == 0x03u);
}

static void test_standard_input_report(void)
{
    padbridge_switch_protocol_t protocol;
    const padbridge_gamepad_state_t state = {
        .buttons = PADBRIDGE_BUTTON_A | PADBRIDGE_BUTTON_PLUS,
        .dpad_right = true,
        .battery_percent = 90u,
    };
    uint8_t output[64];

    padbridge_switch_protocol_init(&protocol, 0u);
    (void)padbridge_switch_protocol_next_report(&protocol, 1u, output);
    padbridge_switch_protocol_set_gamepad(&protocol, &state);
    (void)padbridge_switch_protocol_next_report(&protocol, 2u, output);

    assert(output[0] == 0x30u);
    assert(output[2] == 0x81u);
    assert(output[3] == (1u << 3));
    assert(output[4] == (1u << 1));
    assert(output[5] == (1u << 2));
}

static void test_spi_read_is_bounded(void)
{
    padbridge_switch_protocol_t protocol;
    uint8_t request[64] = {0};
    uint8_t output[64];

    padbridge_switch_protocol_init(&protocol, 0u);
    (void)padbridge_switch_protocol_next_report(&protocol, 1u, output);

    request[0] = 0x01u;
    request[10] = 0x10u;
    request[11] = 0x20u;
    request[12] = 0x60u;
    request[15] = 0xffu;
    padbridge_switch_protocol_receive(&protocol, request, sizeof(request));
    (void)padbridge_switch_protocol_next_report(&protocol, 2u, output);

    assert(output[13] == 0x90u);
    assert(output[14] == 0x10u);
    assert(output[19] == 44u);
}

static void test_rumble_is_forwarded_only_when_enabled(void)
{
    padbridge_switch_protocol_t protocol;
    padbridge_rumble_command_t rumble;
    uint8_t request[64] = {0};
    uint8_t output[64];

    padbridge_switch_protocol_init(&protocol, 0u);
    (void)padbridge_switch_protocol_next_report(&protocol, 1u, output);

    request[0] = 0x10u;
    request[2] = 0x00u;
    request[3] = 0x01u;
    request[4] = 0x80u;
    request[5] = 0x48u;
    request[6] = 0x00u;
    request[7] = 0x01u;
    request[8] = 0x00u;
    request[9] = 0x60u;
    padbridge_switch_protocol_receive(&protocol, request, sizeof(request));
    assert(padbridge_switch_protocol_take_rumble(&protocol, &rumble));
    assert(rumble.duration_ms == 0u);

    memset(request, 0, sizeof(request));
    request[0] = 0x01u;
    request[2] = 0x00u;
    request[3] = 0x01u;
    request[4] = 0x40u;
    request[5] = 0x40u;
    request[6] = 0x00u;
    request[7] = 0x01u;
    request[8] = 0x40u;
    request[9] = 0x40u;
    request[10] = 0x48u;
    request[11] = 0x01u;
    padbridge_switch_protocol_receive(&protocol, request, sizeof(request));
    (void)padbridge_switch_protocol_next_report(&protocol, 2u, output);
    assert(protocol.vibration_enabled);

    memset(request, 0, sizeof(request));
    request[0] = 0x10u;
    request[2] = 0x00u;
    request[3] = 0x01u;
    request[4] = 0x80u;
    request[5] = 0x48u;
    request[6] = 0x00u;
    request[7] = 0x01u;
    request[8] = 0x00u;
    request[9] = 0x60u;
    padbridge_switch_protocol_receive(&protocol, request, sizeof(request));
    assert(padbridge_switch_protocol_take_rumble(&protocol, &rumble));
    assert(rumble.weak_magnitude > 0u);
    assert(rumble.strong_magnitude > rumble.weak_magnitude);
    assert(rumble.duration_ms == 120u);
    assert(!padbridge_switch_protocol_take_rumble(&protocol, &rumble));
}

int main(void)
{
    test_initial_usb_identification();
    test_device_info_subcommand();
    test_standard_input_report();
    test_spi_read_is_bounded();
    test_rumble_is_forwarded_only_when_enabled();
    puts("switch_protocol: all tests passed");
    return 0;
}
