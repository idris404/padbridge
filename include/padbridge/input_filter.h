#ifndef PADBRIDGE_INPUT_FILTER_H
#define PADBRIDGE_INPUT_FILTER_H

#include <stdint.h>

// Removes an axial deadzone and rescales the remaining travel to the full
// signed 16-bit range. A deadzone of 0 leaves the value unchanged.
int16_t padbridge_axis_apply_deadzone(int16_t value, uint16_t deadzone);

#endif
