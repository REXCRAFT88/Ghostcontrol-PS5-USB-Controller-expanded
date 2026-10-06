# GameCube Adapter PC Mode

This branch adds direct support for common Mayflash/EVORETRO/DragonRise GameCube adapters in PC/HID mode.

## Supported USB IDs

- `0079:1843`
- `0079:1844`
- `0079:1846`

These IDs are recognized as GameCube adapters by current SDL and Linux HID support.

`0079:1847` is intentionally not claimed yet because the report layout has not been verified against the same protocol.

## Report formats

### Legacy PC firmware

10-byte report:

- byte 0: physical controller slot, 1..4
- bytes 1..9: controller state

The parser preserves:

- A/B/X/Y
- D-pad
- Start
- Z
- digital L/R hard-click
- analog L/R trigger travel
- main stick
- C-stick

Legacy firmware exposes the C-stick with a different orientation from newer firmware; that quirk is handled in the parser.

### Newer PC firmware (v0x7+)

9-byte report:

- no explicit slot prefix
- controller state begins at byte 0
- C-stick orientation matches the newer SDL interpretation

The baseline branch treats this stream as physical port 1 until hardware confirms whether the PS5 exposes additional HID interfaces/nodes for the other ports.

## USB behavior

PC-mode devices are input-only in this implementation:

- interrupt IN endpoint: `0x81`
- no Nintendo `0x13` initialization
- no Nintendo OUT endpoint requirement

Nintendo/Wii-U protocol adapters (`057e:0337`) keep their existing initialization and four-port `0x21` report path.

## Validation

Host tests cover:

- detection of 1843/1844/1846
- rejection of unverified 1847
- legacy 10-byte slot-indexed reports
- newer 9-byte reports
- face buttons and D-pad
- digital + analog triggers
- main-stick orientation
- legacy/newer C-stick orientation
- invalid slot/report rejection

Real PS5 + Mayflash hardware validation is still required.
