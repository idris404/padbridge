#include "padbridge/runtime.h"

#include <stddef.h>
#include <string.h>

static uint32_t elapsed_since(uint32_t now_ms, uint32_t since_ms)
{
    return now_ms - since_ms;
}

void padbridge_runtime_init(padbridge_runtime_t *runtime, uint32_t now_ms)
{
    if (runtime == NULL) {
        return;
    }
    memset(runtime, 0, sizeof(*runtime));
    runtime->state = PADBRIDGE_RUNTIME_BOOTING;
    runtime->state_since_ms = now_ms;
}

void padbridge_runtime_set_state(
    padbridge_runtime_t *runtime,
    padbridge_runtime_state_t state,
    uint32_t now_ms)
{
    if (runtime == NULL || runtime->state == state) {
        return;
    }
    runtime->state = state;
    runtime->state_since_ms = now_ms;
}

bool padbridge_runtime_led_is_on(const padbridge_runtime_t *runtime, uint32_t now_ms)
{
    if (runtime == NULL) {
        return false;
    }

    const uint32_t elapsed = elapsed_since(now_ms, runtime->state_since_ms);
    switch (runtime->state) {
        case PADBRIDGE_RUNTIME_BOOTING:
            return (elapsed % 1000u) < 100u;
        case PADBRIDGE_RUNTIME_SCANNING:
            return (elapsed % 500u) < 100u;
        case PADBRIDGE_RUNTIME_CONNECTING:
            return (elapsed % 200u) < 100u;
        case PADBRIDGE_RUNTIME_READY:
            return true;
        case PADBRIDGE_RUNTIME_RESETTING_PAIRING:
            return (elapsed % 100u) < 50u;
        case PADBRIDGE_RUNTIME_ERROR: {
            const uint32_t phase = elapsed % 1000u;
            return phase < 100u || (phase >= 200u && phase < 300u);
        }
        default:
            return false;
    }
}

void padbridge_pairing_reset_detector_init(padbridge_pairing_reset_detector_t *detector)
{
    if (detector == NULL) {
        return;
    }
    memset(detector, 0, sizeof(*detector));
}

bool padbridge_pairing_reset_detector_update(
    padbridge_pairing_reset_detector_t *detector,
    bool combo_pressed,
    uint32_t now_ms,
    uint32_t hold_duration_ms)
{
    if (detector == NULL) {
        return false;
    }

    if (!combo_pressed) {
        detector->tracking = false;
        detector->latched = false;
        return false;
    }

    if (detector->latched) {
        return false;
    }

    if (!detector->tracking) {
        detector->tracking = true;
        detector->held_since_ms = now_ms;
        if (hold_duration_ms == 0u) {
            detector->latched = true;
            return true;
        }
        return false;
    }

    if (elapsed_since(now_ms, detector->held_since_ms) < hold_duration_ms) {
        return false;
    }

    detector->latched = true;
    return true;
}
