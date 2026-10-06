# Supported Controllers

This file describes what v0.1.0-beta actually claims.

## Release-supported code paths

### GameCube USB adapter — primary target

Protocol:

```text
VID:PID 057e:0337
IN      0x81
OUT     0x02
START   0x13
REPORT  0x21 / 37 bytes
PORTS   4
```

Implemented:

- all four 9-byte controller records;
- wired and wireless-present status bits;
- A/B/X/Y;
- D-pad;
- Start;
- Z;
- main stick;
- C-stick;
- analog L/R trigger travel;
- L/R hard click;
- multi-port virtual-pad routing;
- per-port disconnect cleanup.

Status: parser/CI verified; real GameCube hardware validation still required.

Third-party adapters in Wii U/Switch mode are expected to work if they genuinely emulate `057e:0337`. PC-mode HID variants are not yet supported by the GameCube protocol backend.

### DualShock 4 USB

Recognized Sony IDs include:

```text
054c:05c4
054c:09cc
```

Known HORI-compatible IDs from the inherited backend are also matched.

Input support includes sticks, D-pad, face buttons, shoulders, L2/R2, Share, Options, L3/R3, PS, and touchpad click.

Status: parser-tested; PS5 hardware validation still desired.

### DualShock 3 / Sixaxis USB

Primary ID:

```text
054c:0268
```

The backend sends the DS3 SET_REPORT initialization and parses sticks/buttons/triggers/PS.

Status: parser-tested; pressure-sensitive face-button values are not yet exposed.

### Manba V2

The original fork's Manba support is retained for its XInput/Switch-style USB modes.

## Optional wireless DS4

Wireless DS4 uses the separate PoorDS4-derived backend.

This is intentionally not implemented as raw HCI takeover in the release. The PS5 pairs the controller normally and retains Bluetooth ownership.

Status: upstream has working evidence on other supported firmware; firmware 13.60 is pending project validation.

## Not claimed as release-ready

- DualShock 3 Bluetooth.
- Native GameCube rumble.
- Generic USB HID auto-mapping.
- Generic N64 USB adapters.
- Switch Online N64 controller.
- Xbox One/Series USB in this integrated beta.
- Generic Switch Pro USB in this integrated beta.
- Mayflash PC/DragonRise HID mode.

These remain roadmap items even when partial inherited code exists.
