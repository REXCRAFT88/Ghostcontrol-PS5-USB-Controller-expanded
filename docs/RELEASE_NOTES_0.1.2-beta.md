# GhostControl Expanded v0.1.2-beta

## DS3 input delivery fix

Real-console testing on PS5 firmware 13.60 showed that the DualShock 3 could be detected and reach the PS5 user-assignment screen, but no controller input reached the virtual pad afterward.

The issue was traced to non-Manba controllers inheriting a Manba-specific virtual-device handle assumption.

### Fixed

- DS3, DS4, and GameCube now prefer the actual opened virtual pad handle observed after VDA creation.
- If no opened pad handle is observed, non-Manba controllers retain the direct VDA-return handle.
- Manba retains its original deviceId-low32 token behavior.
- Existing DS3 parser offsets and USB initialization remain unchanged.

## Generic HID discovery

Unknown USB controllers are no longer completely silent.

The payload now logs unsupported HID candidates with:

- VID/PID;
- interface class;
- subclass;
- protocol.

This is specifically intended to identify the user's PS2 USB adapter and USB N64 controller so proper generic HID profiles can be added safely.

### Validation

This release passed:

- GameCube host tests;
- wired Sony parser tests;
- PoorDS4 static audit;
- full PS5 Payload SDK compilation;
- release packaging.

### Testing

For DS3:

1. reboot the PS5;
2. load v0.1.2-beta;
3. connect DS3 over mini-USB;
4. press PS;
5. assign the user;
6. test D-pad, face buttons, sticks, and PS.

For PS2/N64 adapters:

1. keep GhostControl running;
2. connect each adapter/controller;
3. collect `/data/ghostpad/gc_status.log`;
4. look for `unsupported USB` / `HID gamepad candidate` lines.
