#ifndef PADBRIDGE_INPUT_FILTER_H
#define PADBRIDGE_INPUT_FILTER_H

#include <stdint.h>

// Removes an axial deadzone and rescales the remaining travel to the full
// signed 16-bit range. A deadzone of 0 leaves the value unchanged.
int16_t padbridge_axis_apply_deadzone(int16_t value, uint16_t deadzone);

// Bluepad32 uses 0..254 for empty..full and 255 when battery information is
// unavailable. Unknown values use the supplied fallback percentage.
uint8_t padbridge_battery_to_percent(uint8_t raw_battery, uint8_t fallback_percent);

#endif
