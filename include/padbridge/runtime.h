#ifndef PADBRIDGE_RUNTIME_H
#define PADBRIDGE_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PADBRIDGE_RUNTIME_BOOTING = 0,
    PADBRIDGE_RUNTIME_SCANNING,
    PADBRIDGE_RUNTIME_CONNECTING,
    PADBRIDGE_RUNTIME_READY,
    PADBRIDGE_RUNTIME_RESETTING_PAIRING,
    PADBRIDGE_RUNTIME_ERROR,
} padbridge_runtime_state_t;

typedef struct {
    padbridge_runtime_state_t state;
    uint32_t state_since_ms;
    uint32_t connection_attempts;
    uint32_t successful_connections;
    uint32_t disconnections;
    uint32_t input_reports;
    uint32_t pairing_resets;
} padbridge_runtime_t;

typedef struct {
    uint32_t held_since_ms;
    bool tracking;
    bool latched;
} padbridge_pairing_reset_detector_t;

void padbridge_runtime_init(padbridge_runtime_t *runtime, uint32_t now_ms);
void padbridge_runtime_set_state(
    padbridge_runtime_t *runtime,
    padbridge_runtime_state_t state,
    uint32_t now_ms);
bool padbridge_runtime_led_is_on(const padbridge_runtime_t *runtime, uint32_t now_ms);

void padbridge_pairing_reset_detector_init(padbridge_pairing_reset_detector_t *detector);
bool padbridge_pairing_reset_detector_update(
    padbridge_pairing_reset_detector_t *detector,
    bool combo_pressed,
    uint32_t now_ms,
    uint32_t hold_duration_ms);

#endif
