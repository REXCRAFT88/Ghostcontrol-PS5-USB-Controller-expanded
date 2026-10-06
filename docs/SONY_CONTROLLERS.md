# Wired Sony Controller Support

This branch restores wired DualShock 4 discovery and adds an initial wired DualShock 3 / Sixaxis backend.

## DualShock 4

Supported USB IDs:

- Sony CUH-ZCT1: `054c:05c4`
- Sony CUH-ZCT2: `054c:09cc`
- Known HORI DS4-compatible IDs already documented in the inherited backend

The existing DS4 parser maps:

- sticks
- D-pad
- face buttons
- L1/R1
- analog + digital L2/R2
- Share -> Create
- Options
- L3/R3
- PS
- touchpad click

## DualShock 3 / Sixaxis

Initial USB ID:

- Sony DS3/Sixaxis: `054c:0268`

The backend sends the known USB SET_REPORT initialization:

- request type: `0x21`
- request: `0x09`
- value: `0x03f4`
- payload: `42 03 00 00`

The initial parser supports the four sticks axes, D-pad, face buttons, Start/Select, L1/R1/L2/R2, L3/R3 and PS.

DS3 pressure-sensitive button values are intentionally not exposed yet. The digital state is preserved first; pressure parsing can be added after real report validation.

## Status

Parser behavior is covered by host-side C tests in `tests/test_sony_parsers.c`.

PS5-specific USB endpoint ownership, DS3 initialization and actual controller input remain hardware-unverified.
