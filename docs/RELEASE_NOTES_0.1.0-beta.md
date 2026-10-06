# GhostControl Expanded v0.1.0-beta

## Summary

This is the first integrated GhostControl Expanded release candidate.

The project has moved beyond a Manba-specific patch and now focuses on broader controller compatibility, with GameCube USB adapters as the main development target.

## Included

### GameCube USB adapter support

- detects the standard Nintendo adapter protocol `057e:0337`;
- opens IN `0x81` and OUT `0x02`;
- sends the `0x13` adapter-start command;
- parses the full 37-byte report;
- parses all four physical controller ports;
- preserves main stick and C-stick;
- preserves analog L/R trigger travel;
- preserves L/R hard-click state;
- maps GameCube buttons into PS5 virtual-pad input;
- includes a compatibility control request used by Dolphin for some third-party adapters.

### Experimental four-port virtualization

- one physical USB adapter reader;
- one primary GameCube virtual controller;
- up to three auxiliary virtual controllers;
- per-port connect/disconnect tracking;
- delayed auxiliary creation to avoid stacked assignment dialogs;
- teardown cleanup for auxiliary VDAs.

Four independent PS5 user assignments are not yet claimed as validated.

### Wired Sony support

- DualShock 4 USB discovery and parsing;
- Sony DS4 v1/v2 IDs;
- existing HORI-compatible IDs;
- DualShock 3 / Sixaxis `054c:0268`;
- DS3 SET_REPORT initialization;
- DS3 buttons/sticks/triggers/PS parsing.

### Wireless DS4

The release includes a separate optional PoorDS4-derived backend.

The DS4 is paired normally through the PS5 UI. The bridge uses Sony's existing controller path and validates the target game/firmware before bridging input.

Firmware 13.60 is not force-enabled. It must pass structural admission.

Project-specific diagnostics include:

- `/data/poords4/fw-compat-last.txt`;
- `PoorDS4-evidence.elf`;
- `/data/poords4/13x-evidence.txt`.

### Release tooling

- renamed primary ELF to `GhostControl-Expanded.elf`;
- added `GhostControl-Stop.elf`;
- added an English Windows launcher;
- replaced inherited French project documentation with English project-specific documentation;
- added combined CI and release packaging.

## Validation

Automated tests cover:

- GameCube adapter identity;
- GameCube initialization command;
- all four GameCube report records;
- GameCube mapping;
- DS3 parser;
- DS4 parser;
- Sony device matching;
- PoorDS4 safety/static invariants.

The release workflow also compiles actual PS5 ELFs using the public PS5 Payload SDK.

## Known beta limitations

- GameCube hardware has not yet been tested during this development cycle.
- Four-player GameCube user assignment needs real PS5 validation.
- GameCube rumble is not implemented.
- Generic HID adapters are not implemented.
- N64 support is not implemented yet.
- DS3 Bluetooth is not release-ready.
- Firmware 13.60 wireless DS4 support depends on structural compatibility and is pending real-console evidence.

## FFPFSC

Games stored in `.ffpfsc` format can be used normally. Launch the title through ShadowMountPlus first, then start GhostControl/PoorDS4.
