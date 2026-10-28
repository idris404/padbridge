#include "usb_callbacks.h"

#include <string.h>

#include "padbridge/switch_usb_descriptors.h"
#include "tusb.h"

static padbridge_switch_protocol_t *active_protocol;
static uint16_t string_descriptor[32];

static const char *const usb_strings[] = {
    (const char[]){0x09, 0x04},
    "Nintendo Co., Ltd.",
    "Pro Controller",
    "000000000001",
};

void padbridge_usb_set_protocol(padbridge_switch_protocol_t *protocol)
{
    active_protocol = protocol;
}

const uint8_t *tud_descriptor_device_cb(void)
{
    return padbridge_switch_device_descriptor;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return padbridge_switch_configuration_descriptor;
}

const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return padbridge_switch_hid_report_descriptor;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t language_id)
{
    (void)language_id;

    if (index >= sizeof(usb_strings) / sizeof(usb_strings[0])) {
        return NULL;
    }

    uint8_t character_count;
    if (index == 0u) {
        memcpy(&string_descriptor[1], usb_strings[0], 2u);
        character_count = 1u;
    } else {
        const char *text = usb_strings[index];
        character_count = (uint8_t)strlen(text);
        if (character_count > 31u) {
            character_count = 31u;
        }
        for (uint8_t i = 0; i < character_count; ++i) {
            string_descriptor[1u + i] = (uint8_t)text[i];
        }
    }

    string_descriptor[0] =
        (uint16_t)((TUSB_DESC_STRING << 8) | (2u * character_count + 2u));
    return string_descriptor;
}

uint16_t tud_hid_get_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t *buffer,
    uint16_t requested_size)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)requested_size;
    return 0u;
}

void tud_hid_set_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    const uint8_t *buffer,
    uint16_t buffer_size)
{
    (void)instance;
    (void)report_id;
    (void)report_type;

    if (active_protocol != NULL) {
        padbridge_switch_protocol_receive(active_protocol, buffer, buffer_size);
    }
}

