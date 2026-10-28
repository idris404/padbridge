#include "padbridge/switch_protocol.h"

#include <string.h>

#include "padbridge/switch_report.h"

enum {
    REPORT_SUBCOMMAND_REPLY = 0x21,
    REPORT_FULL_INPUT = 0x30,
    REPORT_USB_REPLY = 0x81,

    REQUEST_RUMBLE_SUBCOMMAND = 0x01,
    REQUEST_RUMBLE_ONLY = 0x10,
    REQUEST_USB_COMMAND = 0x80,

    SUBCOMMAND_PAIR = 0x01,
    SUBCOMMAND_DEVICE_INFO = 0x02,
    SUBCOMMAND_SET_MODE = 0x03,
    SUBCOMMAND_TRIGGER_BUTTONS = 0x04,
    SUBCOMMAND_SET_SHIPMENT = 0x08,
    SUBCOMMAND_SPI_READ = 0x10,
    SUBCOMMAND_SET_PLAYER = 0x30,
    SUBCOMMAND_TOGGLE_IMU = 0x40,
    SUBCOMMAND_IMU_SENSITIVITY = 0x41,
    SUBCOMMAND_ENABLE_VIBRATION = 0x48,
};

static uint8_t advance_timer(padbridge_switch_protocol_t *protocol, uint32_t now_ms)
{
    if (protocol->timestamp_ms == 0u) {
        protocol->timestamp_ms = now_ms;
        return protocol->timer;
    }

    const uint32_t delta = now_ms - protocol->timestamp_ms;
    protocol->timer = (uint8_t)(protocol->timer + delta * 4u);
    protocol->timestamp_ms = now_ms;
    return protocol->timer;
}

static void build_input_core(
    padbridge_switch_protocol_t *protocol,
    uint32_t now_ms,
    uint8_t report_id,
    uint8_t output[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE])
{
    padbridge_switch_report_30_t payload;
    padbridge_switch_build_report(
        &protocol->gamepad,
        advance_timer(protocol, now_ms),
        &payload);
    payload.vibrator_report = protocol->vibration_report;

    output[0] = report_id;
    memcpy(output + 1, &payload, sizeof(payload));
}

static void build_spi_read(
    const padbridge_switch_protocol_t *protocol,
    uint8_t output[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE])
{
    const uint8_t address_low = protocol->request[11];
    const uint8_t address_high = protocol->request[12];
    uint8_t read_size = protocol->request[15];
    if (read_size > 44u) {
        read_size = 44u;
    }

    output[13] = 0x90;
    output[14] = SUBCOMMAND_SPI_READ;
    output[15] = address_low;
    output[16] = address_high;
    output[19] = read_size;
    memset(output + 20, 0xff, read_size);

    if (address_high == 0x60u && address_low == 0x20u) {
        static const uint8_t imu_calibration[24] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x40, 0x00, 0x40, 0x00, 0x40,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x3b, 0x34, 0x3b, 0x34, 0x3b, 0x34,
        };
        const size_t copy_size = read_size < sizeof(imu_calibration)
                               ? read_size
                               : sizeof(imu_calibration);
        memcpy(output + 20, imu_calibration, copy_size);
    }
}

static bool build_subcommand_reply(
    padbridge_switch_protocol_t *protocol,
    uint32_t now_ms,
    uint8_t output[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE])
{
    const uint8_t subcommand = protocol->request[10];
    build_input_core(protocol, now_ms, REPORT_SUBCOMMAND_REPLY, output);

    switch (subcommand) {
    case SUBCOMMAND_PAIR:
        output[13] = 0x81;
        output[14] = SUBCOMMAND_PAIR;
        output[15] = 0x03;
        break;
    case SUBCOMMAND_DEVICE_INFO:
        output[13] = 0x82;
        output[14] = SUBCOMMAND_DEVICE_INFO;
        output[15] = 0x03;
        output[16] = 0x48;
        output[17] = 0x03;
        output[18] = 0x02;
        memcpy(output + 19, protocol->address, sizeof(protocol->address));
        output[25] = 0x01;
        output[26] = 0x01;
        break;
    case SUBCOMMAND_SET_MODE:
    case SUBCOMMAND_TRIGGER_BUTTONS:
    case SUBCOMMAND_SET_SHIPMENT:
    case SUBCOMMAND_IMU_SENSITIVITY:
        output[13] = 0x80;
        output[14] = subcommand;
        break;
    case SUBCOMMAND_SPI_READ:
        build_spi_read(protocol, output);
        break;
    case SUBCOMMAND_SET_PLAYER:
        protocol->player_number = protocol->request[11];
        output[13] = 0x80;
        output[14] = SUBCOMMAND_SET_PLAYER;
        break;
    case SUBCOMMAND_TOGGLE_IMU:
        protocol->imu_enabled = protocol->request[11] == 0x01u;
        output[13] = 0x80;
        output[14] = SUBCOMMAND_TOGGLE_IMU;
        break;
    case SUBCOMMAND_ENABLE_VIBRATION:
        protocol->vibration_enabled = protocol->request[11] == 0x01u;
        protocol->vibration_report = protocol->vibration_enabled ? 0x0au : 0x00u;
        output[13] = 0x80;
        output[14] = SUBCOMMAND_ENABLE_VIBRATION;
        break;
    default:
        return false;
    }

    return true;
}

static void build_usb_reply(
    padbridge_switch_protocol_t *protocol,
    uint32_t now_ms,
    uint8_t output[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE])
{
    output[0] = REPORT_USB_REPLY;
    output[1] = protocol->request[1];

    if (protocol->request[1] == 0x01u) {
        output[3] = 0x03;
        for (size_t i = 0; i < sizeof(protocol->address); ++i) {
            output[4 + i] = protocol->address[sizeof(protocol->address) - 1u - i];
        }
    } else if (protocol->request[1] != 0x02u &&
               protocol->request[1] != 0x03u) {
        build_input_core(protocol, now_ms, REPORT_FULL_INPUT, output);
    }
}

void padbridge_switch_protocol_init(
    padbridge_switch_protocol_t *protocol,
    uint8_t controller_index)
{
    memset(protocol, 0, sizeof(*protocol));
    protocol->address[0] = 0x7c;
    protocol->address[1] = 0xbb;
    protocol->address[2] = 0x8a;
    protocol->address[5] = controller_index;
    protocol->gamepad.battery_percent = 100u;
    protocol->request[0] = REQUEST_USB_COMMAND;
    protocol->request[1] = 0x01;
    protocol->request_size = 2u;
    protocol->has_request = true;
}

void padbridge_switch_protocol_set_gamepad(
    padbridge_switch_protocol_t *protocol,
    const padbridge_gamepad_state_t *state)
{
    if (protocol == NULL || state == NULL) {
        return;
    }
    protocol->gamepad = *state;
}

void padbridge_switch_protocol_receive(
    padbridge_switch_protocol_t *protocol,
    const uint8_t *report,
    size_t report_size)
{
    if (protocol == NULL || report == NULL || report_size == 0u) {
        return;
    }

    if (report_size > sizeof(protocol->request)) {
        report_size = sizeof(protocol->request);
    }
    memset(protocol->request, 0, sizeof(protocol->request));
    memcpy(protocol->request, report, report_size);
    protocol->request_size = report_size;
    protocol->has_request = true;
}

size_t padbridge_switch_protocol_next_report(
    padbridge_switch_protocol_t *protocol,
    uint32_t now_ms,
    uint8_t output[PADBRIDGE_SWITCH_WIRE_REPORT_SIZE])
{
    if (protocol == NULL || output == NULL) {
        return 0u;
    }

    memset(output, 0, PADBRIDGE_SWITCH_WIRE_REPORT_SIZE);

    if (protocol->has_request && protocol->request[0] == REQUEST_USB_COMMAND) {
        build_usb_reply(protocol, now_ms, output);
    } else if (protocol->has_request &&
               protocol->request[0] == REQUEST_RUMBLE_SUBCOMMAND) {
        if (!build_subcommand_reply(protocol, now_ms, output)) {
            build_input_core(protocol, now_ms, REPORT_FULL_INPUT, output);
        }
    } else {
        build_input_core(protocol, now_ms, REPORT_FULL_INPUT, output);
    }

    protocol->has_request = false;
    return PADBRIDGE_SWITCH_WIRE_REPORT_SIZE;
}

