#include "padbridge/switch_usb_descriptors.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    assert(padbridge_switch_device_descriptor_size == 18u);
    assert(padbridge_switch_hid_report_descriptor_size == 203u);
    assert(padbridge_switch_configuration_descriptor_size == 41u);

    assert(padbridge_switch_device_descriptor[8] == 0x7eu);
    assert(padbridge_switch_device_descriptor[9] == 0x05u);
    assert(padbridge_switch_device_descriptor[10] == 0x09u);
    assert(padbridge_switch_device_descriptor[11] == 0x20u);

    assert(padbridge_switch_configuration_descriptor[2] == 41u);
    assert(padbridge_switch_configuration_descriptor[4] == 1u);
    assert(padbridge_switch_configuration_descriptor[27] == 0x07u);
    assert(padbridge_switch_configuration_descriptor[29] == 0x81u);
    assert(padbridge_switch_configuration_descriptor[34] == 0x07u);
    assert(padbridge_switch_configuration_descriptor[36] == 0x01u);

    puts("switch_usb_descriptors: all tests passed");
    return 0;
}

