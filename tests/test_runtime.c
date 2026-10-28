#include "padbridge/runtime.h"

#include <assert.h>
#include <stdio.h>

static void test_led_patterns(void)
{
    padbridge_runtime_t runtime;
    padbridge_runtime_init(&runtime, 1000u);

    assert(padbridge_runtime_led_is_on(&runtime, 1000u));
    assert(!padbridge_runtime_led_is_on(&runtime, 1100u));

    padbridge_runtime_set_state(&runtime, PADBRIDGE_RUNTIME_SCANNING, 2000u);
    assert(padbridge_runtime_led_is_on(&runtime, 2000u));
    assert(!padbridge_runtime_led_is_on(&runtime, 2100u));
    assert(padbridge_runtime_led_is_on(&runtime, 2500u));

    padbridge_runtime_set_state(&runtime, PADBRIDGE_RUNTIME_CONNECTING, 3000u);
    assert(padbridge_runtime_led_is_on(&runtime, 3000u));
    assert(!padbridge_runtime_led_is_on(&runtime, 3100u));
    assert(padbridge_runtime_led_is_on(&runtime, 3200u));

    padbridge_runtime_set_state(&runtime, PADBRIDGE_RUNTIME_READY, 4000u);
    assert(padbridge_runtime_led_is_on(&runtime, 4000u));
    assert(padbridge_runtime_led_is_on(&runtime, 9000u));

    padbridge_runtime_set_state(&runtime, PADBRIDGE_RUNTIME_ERROR, 10000u);
    assert(padbridge_runtime_led_is_on(&runtime, 10000u));
    assert(!padbridge_runtime_led_is_on(&runtime, 10100u));
    assert(padbridge_runtime_led_is_on(&runtime, 10200u));
    assert(!padbridge_runtime_led_is_on(&runtime, 10300u));
}

static void test_pairing_reset_requires_hold_and_release(void)
{
    padbridge_pairing_reset_detector_t detector;
    padbridge_pairing_reset_detector_init(&detector);

    assert(!padbridge_pairing_reset_detector_update(&detector, true, 100u, 5000u));
    assert(!padbridge_pairing_reset_detector_update(&detector, true, 5099u, 5000u));
    assert(padbridge_pairing_reset_detector_update(&detector, true, 5100u, 5000u));
    assert(!padbridge_pairing_reset_detector_update(&detector, true, 9000u, 5000u));

    assert(!padbridge_pairing_reset_detector_update(&detector, false, 9100u, 5000u));
    assert(!padbridge_pairing_reset_detector_update(&detector, true, 9200u, 5000u));
    assert(padbridge_pairing_reset_detector_update(&detector, true, 14200u, 5000u));
}

static void test_pairing_reset_timer_wraparound(void)
{
    padbridge_pairing_reset_detector_t detector;
    padbridge_pairing_reset_detector_init(&detector);

    assert(!padbridge_pairing_reset_detector_update(&detector, true, UINT32_MAX - 100u, 200u));
    assert(!padbridge_pairing_reset_detector_update(&detector, true, 50u, 200u));
    assert(padbridge_pairing_reset_detector_update(&detector, true, 100u, 200u));
}

static void test_immediate_pairing_reset_is_latched(void)
{
    padbridge_pairing_reset_detector_t detector;
    padbridge_pairing_reset_detector_init(&detector);

    assert(padbridge_pairing_reset_detector_update(&detector, true, 10u, 0u));
    assert(!padbridge_pairing_reset_detector_update(&detector, true, 11u, 0u));
}

int main(void)
{
    test_led_patterns();
    test_pairing_reset_requires_hold_and_release();
    test_pairing_reset_timer_wraparound();
    test_immediate_pairing_reset_is_latched();
    puts("runtime tests passed");
    return 0;
}
