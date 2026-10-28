#ifndef PADBRIDGE_SHARED_STATE_H
#define PADBRIDGE_SHARED_STATE_H

#include "padbridge/gamepad.h"
#include "padbridge/rumble.h"

void padbridge_shared_state_init(void);
void padbridge_shared_state_set(const padbridge_gamepad_state_t *state);
void padbridge_shared_state_get(padbridge_gamepad_state_t *state);
void padbridge_shared_state_clear(void);

void padbridge_shared_rumble_set(const padbridge_rumble_command_t *command);
bool padbridge_shared_rumble_get(
    padbridge_rumble_command_t *command,
    uint32_t *sequence);

#endif

