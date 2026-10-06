# Architecture

GhostControl Expanded has two release paths that are intentionally separated.

## 1. Unified USB controller payload

`GhostControl-Expanded.elf` owns supported USB controller devices and converts their input into virtual DualSense state.

```text
USB controller / adapter
        |
        v
device detection
        |
        v
controller-specific parser
        |
        v
ScePadData
        |
        v
virtual DualSense device
        |
        v
PS5 game
```

The main payload integrates the GameCube adapter protocol, DualShock 4 USB, DualShock 3 USB, and Manba V2 paths inherited from the original fork. Nintendo/Xbox parser code remains available for future release work.

## GameCube multi-port model

One Nintendo-style GameCube USB adapter carries four physical controller records in one 37-byte input report.

The physical USB device is read once. One controller port uses the normal GhostControl slot/VDA. Additional occupied ports can receive auxiliary virtual DualSense devices.

Auxiliary devices are created only after the primary virtual-device assignment is confirmed and an additional GameCube controller becomes active. This avoids creating four assignment dialogs at once.

Per-port disconnect destroys the corresponding auxiliary VDA. Adapter teardown destroys all auxiliary VDAs.

The current inherited user-binding model still uses a shared injection-user path. Four independent VDA input streams are implemented, but four fully independent PS5 user-profile assignments remain hardware-validation work.

## 2. Optional wireless DS4 backend

Wireless DS4 is deliberately separate from the USB payload.

The release uses the PoorDS4-derived architecture instead of taking over the PS5 Bluetooth HCI controller.

```text
DS4
 |
 | normal PS5 Bluetooth pairing
 v
Sony PS5 Bluetooth stack
 |
 v
PS5 pad / RemotePlay source
 |
 v
PoorDS4 structural validation
 |
 v
game bridge
 |
 v
native PS5 game
```

The bridge validates wrappers, target relationships, executable mappings, controller-information ABI evidence, and source/game library fingerprints before installing hooks.

Unknown firmware is not automatically enabled. It must pass structural admission.

## Experimental direct-HCI branches

Repository history contains direct-HCI Bluetooth research with descriptor probing, passive observation, L2CAP/HIDP extraction, and DS3/DS4 report parsing.

Those experiments are not part of v0.1.0-beta because the native-paired PoorDS4 approach is safer for DS4 and GameCube remains the primary goal. Direct-HCI research remains potentially useful for future DS3 Bluetooth support.

## Process lifecycle

The USB payload writes `/data/ghostpad/gc_main.pid`.

On startup, a new instance reads that PID and sends SIGTERM to an older instance. The signal handler attempts to stop/uninitialize USB transfers, close file descriptors, delete virtual devices, and exit.

`GhostControl-Stop.elf` performs the same cooperative stop request without starting a replacement payload.

## Logging

Main payload:

```text
/data/ghostpad/gc_status.log
```

Wireless DS4:

```text
/data/poords4/
```

Important wireless-DS4 diagnostics include `fw-compat-last.txt` and `13x-evidence.txt`.
