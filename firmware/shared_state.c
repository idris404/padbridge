#include "shared_state.h"

#include <string.h>

#include "pico/critical_section.h"

static critical_section_t state_lock;
static padbridge_gamepad_state_t shared_state;
static padbridge_rumble_command_t shared_rumble;
static uint32_t shared_rumble_sequence;

void padbridge_shared_state_init(void)
{
    critical_section_init(&state_lock);
    padbridge_shared_state_clear();
}

void padbridge_shared_state_set(const padbridge_gamepad_state_t *state)
{
    if (state == NULL) {
        return;
    }
    critical_section_enter_blocking(&state_lock);
    shared_state = *state;
    critical_section_exit(&state_lock);
}

void padbridge_shared_state_get(padbridge_gamepad_state_t *state)
{
    if (state == NULL) {
        return;
    }
    critical_section_enter_blocking(&state_lock);
    *state = shared_state;
    critical_section_exit(&state_lock);
}

void padbridge_shared_state_clear(void)
{
    critical_section_enter_blocking(&state_lock);
    memset(&shared_state, 0, sizeof(shared_state));
    shared_state.battery_percent = 100u;
    critical_section_exit(&state_lock);
}

void padbridge_shared_rumble_set(const padbridge_rumble_command_t *command)
{
    if (command == NULL) {
        return;
    }
    critical_section_enter_blocking(&state_lock);
    shared_rumble = *command;
    shared_rumble_sequence++;
    critical_section_exit(&state_lock);
}

bool padbridge_shared_rumble_get(
    padbridge_rumble_command_t *command,
    uint32_t *sequence)
{
    if (command == NULL || sequence == NULL) {
        return false;
    }
    critical_section_enter_blocking(&state_lock);
    *command = shared_rumble;
    *sequence = shared_rumble_sequence;
    critical_section_exit(&state_lock);
    return true;
}

