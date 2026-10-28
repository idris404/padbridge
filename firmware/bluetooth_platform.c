#include "bluetooth_platform.h"

#include <limits.h>
#include <string.h>

#include "btstack_run_loop.h"
#include "pico/cyw43_arch.h"
#include "uni.h"

#include "padbridge/gamepad.h"
#include "sdkconfig.h"
#include "shared_state.h"

static int16_t axis_to_i16(int32_t value, bool invert)
{
    int32_t scaled = value * 64;
    if (invert) {
        scaled = -scaled;
    }
    if (scaled < INT16_MIN) {
        scaled = INT16_MIN;
    }
    if (scaled > INT16_MAX) {
        scaled = INT16_MAX;
    }
    return (int16_t)scaled;
}

static void map_gamepad(const uni_controller_t *controller)
{
    const uni_gamepad_t *gamepad = &controller->gamepad;
    padbridge_gamepad_state_t state;
    memset(&state, 0, sizeof(state));

    // Bluepad32 normalizes face buttons by physical position. Route those
    // positions to Nintendo's east=A, south=B, north=X, west=Y layout.
    if ((gamepad->buttons & BUTTON_A) != 0u) state.buttons |= PADBRIDGE_BUTTON_B;
    if ((gamepad->buttons & BUTTON_B) != 0u) state.buttons |= PADBRIDGE_BUTTON_A;
    if ((gamepad->buttons & BUTTON_X) != 0u) state.buttons |= PADBRIDGE_BUTTON_Y;
    if ((gamepad->buttons & BUTTON_Y) != 0u) state.buttons |= PADBRIDGE_BUTTON_X;
    if ((gamepad->buttons & BUTTON_SHOULDER_L) != 0u) state.buttons |= PADBRIDGE_BUTTON_L;
    if ((gamepad->buttons & BUTTON_SHOULDER_R) != 0u) state.buttons |= PADBRIDGE_BUTTON_R;
    if ((gamepad->buttons & BUTTON_TRIGGER_L) != 0u || gamepad->brake > 64) {
        state.buttons |= PADBRIDGE_BUTTON_ZL;
    }
    if ((gamepad->buttons & BUTTON_TRIGGER_R) != 0u || gamepad->throttle > 64) {
        state.buttons |= PADBRIDGE_BUTTON_ZR;
    }
    if ((gamepad->buttons & BUTTON_THUMB_L) != 0u) state.buttons |= PADBRIDGE_BUTTON_L3;
    if ((gamepad->buttons & BUTTON_THUMB_R) != 0u) state.buttons |= PADBRIDGE_BUTTON_R3;
    if ((gamepad->misc_buttons & MISC_BUTTON_SELECT) != 0u) state.buttons |= PADBRIDGE_BUTTON_MINUS;
    if ((gamepad->misc_buttons & MISC_BUTTON_START) != 0u) state.buttons |= PADBRIDGE_BUTTON_PLUS;
    if ((gamepad->misc_buttons & MISC_BUTTON_SYSTEM) != 0u) state.buttons |= PADBRIDGE_BUTTON_HOME;
    if ((gamepad->misc_buttons & MISC_BUTTON_CAPTURE) != 0u) state.buttons |= PADBRIDGE_BUTTON_CAPTURE;

    state.dpad_up = (gamepad->dpad & DPAD_UP) != 0u;
    state.dpad_down = (gamepad->dpad & DPAD_DOWN) != 0u;
    state.dpad_left = (gamepad->dpad & DPAD_LEFT) != 0u;
    state.dpad_right = (gamepad->dpad & DPAD_RIGHT) != 0u;

    state.left_x = axis_to_i16(gamepad->axis_x, false);
    state.left_y = axis_to_i16(gamepad->axis_y, true);
    state.right_x = axis_to_i16(gamepad->axis_rx, false);
    state.right_y = axis_to_i16(gamepad->axis_ry, true);
    state.battery_percent = controller->battery == 0u
                          ? 100u
                          : (uint8_t)(((uint16_t)controller->battery * 100u) / 255u);

    padbridge_shared_state_set(&state);
}

static void platform_init(int argc, const char **argv)
{
    (void)argc;
    (void)argv;
}

static void platform_ready(void)
{
    uni_bt_start_scanning_and_autoconnect_unsafe();
}

static uni_error_t device_discovered(
    bd_addr_t address,
    const char *name,
    uint16_t class_of_device,
    uint8_t rssi)
{
    (void)address;
    (void)name;
    (void)class_of_device;
    (void)rssi;
    return UNI_ERROR_SUCCESS;
}

static void device_connected(uni_hid_device_t *device)
{
    (void)device;
}

static void device_disconnected(uni_hid_device_t *device)
{
    (void)device;
    padbridge_shared_state_clear();
}

static uni_error_t device_ready(uni_hid_device_t *device)
{
    (void)device;
    return UNI_ERROR_SUCCESS;
}

static void controller_data(uni_hid_device_t *device, uni_controller_t *controller)
{
    (void)device;
    if (controller->klass == UNI_CONTROLLER_CLASS_GAMEPAD) {
        map_gamepad(controller);
    }
}

static const uni_property_t *get_property(uni_property_idx_t index)
{
    (void)index;
    return NULL;
}

static void oob_event(uni_platform_oob_event_t event, void *data)
{
    (void)event;
    (void)data;
}

static struct uni_platform *padbridge_platform(void)
{
    static struct uni_platform platform = {
        .name = "PadBridge",
        .init = platform_init,
        .on_init_complete = platform_ready,
        .on_device_discovered = device_discovered,
        .on_device_connected = device_connected,
        .on_device_disconnected = device_disconnected,
        .on_device_ready = device_ready,
        .on_controller_data = controller_data,
        .on_oob_event = oob_event,
        .get_property = get_property,
    };
    return &platform;
}

void padbridge_bluetooth_task(void)
{
    if (cyw43_arch_init() != 0) {
        return;
    }

    uni_platform_set_custom(padbridge_platform());
    uni_init(0, NULL);
    btstack_run_loop_execute();
}

