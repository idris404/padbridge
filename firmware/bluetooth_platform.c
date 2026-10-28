#include "bluetooth_platform.h"

#include <limits.h>
#include <string.h>

#include "btstack_run_loop.h"
#include "bt/uni_bt.h"
#include "pico/cyw43_arch.h"
#include "uni.h"

#include "padbridge/gamepad.h"
#include "padbridge/runtime.h"
#include "sdkconfig.h"
#include "shared_state.h"

#define PADBRIDGE_LED_TICK_MS 50u
#define PADBRIDGE_PAIRING_RESET_HOLD_MS 5000u
#define PADBRIDGE_PAIRING_RESET_LED_MS 1000u

static padbridge_runtime_t runtime;
static padbridge_pairing_reset_detector_t pairing_reset_detector;
static btstack_timer_source_t led_timer;
static bool led_timer_started;

static uint32_t runtime_now_ms(void)
{
    return btstack_run_loop_get_time_ms();
}

static void set_runtime_state(padbridge_runtime_state_t state)
{
    padbridge_runtime_set_state(&runtime, state, runtime_now_ms());
    cyw43_arch_gpio_put(
        CYW43_WL_GPIO_LED_PIN,
        padbridge_runtime_led_is_on(&runtime, runtime_now_ms()));
}

static void led_timer_handler(btstack_timer_source_t *timer)
{
    const uint32_t now_ms = runtime_now_ms();
    if (runtime.state == PADBRIDGE_RUNTIME_RESETTING_PAIRING &&
        (now_ms - runtime.state_since_ms) >= PADBRIDGE_PAIRING_RESET_LED_MS) {
        padbridge_runtime_set_state(&runtime, PADBRIDGE_RUNTIME_SCANNING, now_ms);
    }
    cyw43_arch_gpio_put(
        CYW43_WL_GPIO_LED_PIN,
        padbridge_runtime_led_is_on(&runtime, now_ms));
    btstack_run_loop_set_timer(timer, PADBRIDGE_LED_TICK_MS);
    btstack_run_loop_add_timer(timer);
}

static void start_led_timer(void)
{
    if (led_timer_started) {
        return;
    }
    led_timer_started = true;
    btstack_run_loop_set_timer_handler(&led_timer, led_timer_handler);
    btstack_run_loop_set_timer(&led_timer, PADBRIDGE_LED_TICK_MS);
    btstack_run_loop_add_timer(&led_timer);
}

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
    set_runtime_state(PADBRIDGE_RUNTIME_SCANNING);
    start_led_timer();
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
    runtime.connection_attempts++;
    set_runtime_state(PADBRIDGE_RUNTIME_CONNECTING);
}

static void device_disconnected(uni_hid_device_t *device)
{
    (void)device;
    runtime.disconnections++;
    padbridge_shared_state_clear();
    padbridge_pairing_reset_detector_init(&pairing_reset_detector);
    if (runtime.state != PADBRIDGE_RUNTIME_RESETTING_PAIRING) {
        set_runtime_state(PADBRIDGE_RUNTIME_SCANNING);
    }
    uni_bt_start_scanning_and_autoconnect_unsafe();
}

static uni_error_t device_ready(uni_hid_device_t *device)
{
    (void)device;
    runtime.successful_connections++;
    set_runtime_state(PADBRIDGE_RUNTIME_READY);
    return UNI_ERROR_SUCCESS;
}

static bool pairing_reset_combo_pressed(const uni_gamepad_t *gamepad)
{
    const uint16_t required_buttons =
        BUTTON_SHOULDER_L | BUTTON_SHOULDER_R |
        BUTTON_TRIGGER_L | BUTTON_TRIGGER_R;
    const uint8_t required_misc = MISC_BUTTON_SELECT | MISC_BUTTON_START;
    const bool triggers_pressed =
        ((gamepad->buttons & required_buttons) == required_buttons) ||
        (((gamepad->buttons & (BUTTON_SHOULDER_L | BUTTON_SHOULDER_R)) ==
          (BUTTON_SHOULDER_L | BUTTON_SHOULDER_R)) &&
         gamepad->brake > 64 && gamepad->throttle > 64);

    return triggers_pressed &&
           (gamepad->misc_buttons & required_misc) == required_misc;
}

static void controller_data(uni_hid_device_t *device, uni_controller_t *controller)
{
    if (controller->klass == UNI_CONTROLLER_CLASS_GAMEPAD) {
        runtime.input_reports++;
        map_gamepad(controller);

        if (padbridge_pairing_reset_detector_update(
                &pairing_reset_detector,
                pairing_reset_combo_pressed(&controller->gamepad),
                runtime_now_ms(),
                PADBRIDGE_PAIRING_RESET_HOLD_MS)) {
            runtime.pairing_resets++;
            set_runtime_state(PADBRIDGE_RUNTIME_RESETTING_PAIRING);
            padbridge_shared_state_clear();
            uni_bt_del_keys_unsafe();
            uni_hid_device_disconnect(device);
            uni_bt_start_scanning_and_autoconnect_unsafe();
        }
    }
}

static const uni_property_t *get_property(uni_property_idx_t index)
{
    (void)index;
    return NULL;
}

static void oob_event(uni_platform_oob_event_t event, void *data)
{
    if (event == UNI_PLATFORM_OOB_BLUETOOTH_ENABLED &&
        runtime.state != PADBRIDGE_RUNTIME_READY &&
        runtime.state != PADBRIDGE_RUNTIME_CONNECTING &&
        runtime.state != PADBRIDGE_RUNTIME_RESETTING_PAIRING) {
        set_runtime_state((bool)(uintptr_t)data
                              ? PADBRIDGE_RUNTIME_SCANNING
                              : PADBRIDGE_RUNTIME_BOOTING);
    }
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
    padbridge_runtime_init(&runtime, 0u);
    padbridge_pairing_reset_detector_init(&pairing_reset_detector);

    if (cyw43_arch_init() != 0) {
        runtime.state = PADBRIDGE_RUNTIME_ERROR;
        return;
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    uni_platform_set_custom(padbridge_platform());
    uni_init(0, NULL);
    btstack_run_loop_execute();
}

