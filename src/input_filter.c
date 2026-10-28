#include "padbridge/input_filter.h"

#include <limits.h>

int16_t padbridge_axis_apply_deadzone(int16_t value, uint16_t deadzone)
{
    if (deadzone == 0u) {
        return value;
    }
    if (deadzone >= (uint16_t)INT16_MAX) {
        return 0;
    }

    const int32_t signed_value = value;
    const int32_t magnitude = signed_value < 0 ? -signed_value : signed_value;
    if (magnitude <= (int32_t)deadzone) {
        return 0;
    }

    const int32_t output_limit = signed_value < 0 ? 32768 : INT16_MAX;
    const int32_t input_limit = signed_value < 0 ? 32768 : INT16_MAX;
    const int32_t adjusted = magnitude - (int32_t)deadzone;
    const int32_t available = input_limit - (int32_t)deadzone;
    int32_t scaled = (adjusted * output_limit) / available;
    if (scaled > output_limit) {
        scaled = output_limit;
    }

    return (int16_t)(signed_value < 0 ? -scaled : scaled);
}
