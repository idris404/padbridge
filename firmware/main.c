#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"

#include "padbridge/switch_protocol.h"
#include "bluetooth_platform.h"
#include "shared_state.h"
#include "usb_callbacks.h"

int main(void)
{
    padbridge_switch_protocol_t protocol;
    padbridge_gamepad_state_t gamepad;
    padbridge_rumble_command_t rumble;
    uint8_t player_leds;
    uint8_t wire_report[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE];

    padbridge_shared_state_init();
    padbridge_switch_protocol_init(&protocol, 0u);
    padbridge_usb_set_protocol(&protocol);
    multicore_launch_core1(padbridge_bluetooth_task);
    tusb_init();

    while (true) {
        tud_task();

        if (tud_hid_n_ready(0u)) {
            padbridge_shared_state_get(&gamepad);
            padbridge_switch_protocol_set_gamepad(&protocol, &gamepad);
            const uint32_t now_ms = to_ms_since_boot(get_absolute_time());
            const size_t size = padbridge_switch_protocol_next_report(
                &protocol,
                now_ms,
                wire_report);
            tud_hid_n_report(0u, 0u, wire_report, (uint16_t)size);
        }

        if (padbridge_switch_protocol_take_rumble(&protocol, &rumble)) {
            padbridge_shared_rumble_set(&rumble);
        }
        if (padbridge_switch_protocol_take_player(&protocol, &player_leds)) {
            padbridge_shared_player_set(player_leds);
        }
    }
}

