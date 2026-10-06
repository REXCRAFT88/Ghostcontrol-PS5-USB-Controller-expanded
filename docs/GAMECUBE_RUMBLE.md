# Experimental GameCube Rumble

This branch adds **Nintendo/Wii-U protocol GameCube rumble** on top of the GameCube multi-port stack.

## VDA feedback source

PS5 virtual pad input is injected with `scePadVirtualDeviceInsertData()`, but games can also send controller output state back to the virtual device.

The branch weak-links:

```c
scePadVirtualDeviceGetRemoteSetting(handle, buffer)
```

Public PS5 VDA research from Control4Free documents the returned buffer as a DualShock-4-style output state. The relevant observed bytes are:

- byte 3: small/fast motor
- byte 4: large/slow motor

GhostControl only needs a binary GameCube motor state, so either non-zero DS4 motor requests **rumble on**.

If the symbol is unavailable on a firmware, the branch logs that feedback is unavailable and keeps normal controller input working.

## Nintendo adapter output

For `057e:0337` Nintendo/Wii-U protocol adapters the output packet is:

```text
11 P1 P2 P3 P4
```

where each port byte is normalized to:

- `0` = rumble off
- `1` = rumble on

The feedback loop:

- polls VDA remote settings at approximately 16 ms intervals;
- reads the primary VDA and each active auxiliary VDA independently;
- sends a USB rumble packet only when the four-port state changes;
- sends `11 00 00 00 00` before adapter teardown.

WaveBird receivers do not physically support rumble; sending an on state to such a port does not create a motor that is not present.

## PC-mode adapters

Mayflash/DragonRise PC-mode rumble is **not enabled** in this branch.

SDL documents PC-mode rumble as a 3-byte `00 value value` report that must be refreshed while active, but the raw PS5 USB transport for that HID output has not been hardware-verified. The current PC-mode path therefore remains input-only.

## Validation

Host tests cover:

- VDA feedback motor-byte interpretation;
- short/invalid feedback buffers;
- exact Nintendo `0x11` five-byte output packet;
- normalization of arbitrary non-zero port states to `1`.

Still requires PS5 hardware validation:

- `scePadVirtualDeviceGetRemoteSetting` behavior on firmware 13.60;
- actual adapter rumble;
- multi-port independent rumble;
- WaveBird/no-rumble behavior;
- teardown motor stop.
