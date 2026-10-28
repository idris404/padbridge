#include "padbridge/input_filter.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

static void test_zero_deadzone_preserves_value(void)
{
    assert(padbridge_axis_apply_deadzone(1234, 0u) == 1234);
    assert(padbridge_axis_apply_deadzone(-1234, 0u) == -1234);
}

static void test_values_inside_deadzone_are_centered(void)
{
    assert(padbridge_axis_apply_deadzone(0, 2500u) == 0);
    assert(padbridge_axis_apply_deadzone(2500, 2500u) == 0);
    assert(padbridge_axis_apply_deadzone(-2500, 2500u) == 0);
}

static void test_values_outside_deadzone_are_rescaled(void)
{
    const int16_t positive = padbridge_axis_apply_deadzone(10000, 2500u);
    const int16_t negative = padbridge_axis_apply_deadzone(-10000, 2500u);
    assert(positive > 0 && positive < 10000);
    assert(negative < 0 && negative > -10000);
    assert(padbridge_axis_apply_deadzone(INT16_MAX, 2500u) == INT16_MAX);
    assert(padbridge_axis_apply_deadzone(INT16_MIN, 2500u) == INT16_MIN);
}

static void test_invalid_full_deadzone_centers_axis(void)
{
    assert(padbridge_axis_apply_deadzone(30000, INT16_MAX) == 0);
    assert(padbridge_axis_apply_deadzone(-30000, UINT16_MAX) == 0);
}

int main(void)
{
    test_zero_deadzone_preserves_value();
    test_values_inside_deadzone_are_centered();
    test_values_outside_deadzone_are_rescaled();
    test_invalid_full_deadzone_centers_axis();
    puts("input filter tests passed");
    return 0;
}
