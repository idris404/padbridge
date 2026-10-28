#ifndef PADBRIDGE_SWITCH_USB_DESCRIPTORS_H
#define PADBRIDGE_SWITCH_USB_DESCRIPTORS_H

#include <stddef.h>
#include <stdint.h>

extern const uint8_t padbridge_switch_device_descriptor[18];
extern const uint8_t padbridge_switch_hid_report_descriptor[203];
extern const uint8_t padbridge_switch_configuration_descriptor[41];

extern const size_t padbridge_switch_device_descriptor_size;
extern const size_t padbridge_switch_hid_report_descriptor_size;
extern const size_t padbridge_switch_configuration_descriptor_size;

#endif

