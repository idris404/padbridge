# PadBridge

PadBridge is a Raspberry Pi Pico 2 W firmware that receives Bluetooth gamepads
and presents them to a Nintendo Switch as wired Pro Controllers.

## Target architecture

```text
Bluetooth controller
        |
        v
Bluepad32 / BTstack (core 1)
        |
        v
Normalized gamepad state
        |
        v
Switch Pro protocol / TinyUSB (core 0)
        |
        v
Nintendo Switch USB
```

The console firmware is not modified. PadBridge behaves only as a USB gamepad.
Macros, turbo and gameplay automation are intentionally outside the project
scope.

## Current state

Implemented and locally verified:

- normalized buttons and axes;
- Nintendo Switch button bitfields;
- signed 16-bit to 12-bit stick conversion;
- 64-byte standard input report `0x30` generation;
- USB device, HID and configuration descriptors;
- USB handshake and common Switch subcommand replies;
- Bluepad32 Bluetooth reception on core 1;
- controller-to-Nintendo button, stick and battery translation;
- TinyUSB and the Switch protocol on core 0;
- a successful Pico 2 W firmware build producing a `.uf2` image;
- host-side unit tests.

Still requiring physical hardware validation:

- USB enumeration and handshake against a real Switch;
- Bluetooth pairing and reconnection with real controllers;
- end-to-end button, stick and latency checks;
- rumble and IMU/gyro forwarding;
- persistent pairing controls and recovery behaviour.

## Run the core tests

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Build the Pico firmware

Install the Raspberry Pi Pico SDK and an ARM embedded toolchain, then:

```sh
export PICO_SDK_PATH=/absolute/path/to/pico-sdk
cmake -S firmware -B build-firmware -DPICO_BOARD=pico2_w
cmake --build build-firmware
```

The output `padbridge_switch.uf2` can be copied to a Pico 2 W in BOOTSEL mode.
Bluetooth input is handled by Bluepad32 on core 1 while TinyUSB and the Switch
protocol run on core 0. The current build targets one Bluetooth controller.

Initialize dependencies after cloning:

```sh
git submodule update --init --recursive
```

## Hardware

- Raspberry Pi Pico 2 W (preferred) or Pico W;
- Micro-USB data cable;
- Nintendo Switch dock, or a compatible USB-C OTG connection for testing.

## References

- Bluepad32: Bluetooth gamepad host
- TinyUSB: USB device stack
- PicoSwitch2: open implementation reference for the Switch Pro handshake
- Nintendo Switch reverse-engineering notes by dekuNukem

