#ifndef PADBRIDGE_SHARED_STATE_H
#define PADBRIDGE_SHARED_STATE_H

#include "padbridge/gamepad.h"

void padbridge_shared_state_init(void);
void padbridge_shared_state_set(const padbridge_gamepad_state_t *state);
void padbridge_shared_state_get(padbridge_gamepad_state_t *state);
void padbridge_shared_state_clear(void);

#endif

