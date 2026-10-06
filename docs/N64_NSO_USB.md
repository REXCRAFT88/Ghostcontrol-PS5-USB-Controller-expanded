# Nintendo Switch Online N64 Controller — USB

This branch adds an explicit Nintendo Switch Online N64 controller profile.

## Device

- Vendor: Nintendo `0x057e`
- Product: N64 Controller `0x2019`

The controller is part of Nintendo's Switch HID family and uses the same general USB transport/report family as Switch controllers, so GhostControl can reuse the existing Nintendo USB handshake and report state machine while applying an N64-specific input map.

## Default PS5 mapping

| N64 control | Virtual PS5 input |
|---|---|
| A | Cross |
| B | Circle |
| Z | L2 |
| L | L1 |
| R | R1 |
| ZR | R2 |
| Start | Options |
| Home | PS |
| Capture | Create |
| D-pad | D-pad |
| Analog stick | Left stick |
| C-left | Right stick left |
| C-right | Right stick right |
| C-up | Right stick up |
| C-down | Right stick down |

The C-buttons are represented as digital right-stick directions. This preserves all four C-buttons without consuming the PS5 face buttons that already represent N64 A/B.

If opposite C-buttons are pressed simultaneously on one axis, that virtual right-stick axis returns to center.

## Nintendo report bits

The profile follows the current Linux `hid-nintendo` N64 mapping:

- A = Nintendo A bit
- B = Nintendo B bit
- Z = ZL bit
- L = L bit
- R = R bit
- ZR = left-stick-click bit in the underlying Switch report
- Start = Plus
- C-up = Y
- C-down = ZR
- C-left = X
- C-right = Minus

Home, Capture and D-pad retain their normal Nintendo report locations.

## Reports

Supported parser paths:

- full Nintendo report `0x30`
- simple report `0x3f`
- streaming `0x21` subcommand replies through the same N64 profile parser

The normal Switch/Manba profile remains separate and unchanged.

## USB discovery

Unlike the old Manba-focused build, `057e:2019` is now explicitly accepted during the safe USB descriptor scan. It then enters the existing Nintendo USB transport rather than relying on endpoint heuristics.

## Validation

Automated host tests cover:

- A/B/Z/L/R/ZR/Start/Home mapping
- D-pad mapping
- N64 analog stick decoding
- all four C-button directions
- opposite C-buttons centering the virtual right stick
- simple `0x3f` reports
- profile selection
- regression coverage for the original standard Nintendo profile

The host CI is green.

## Hardware status

Still hardware-unverified on PS5.

The inherited Nintendo transport performs the existing Switch-style USB initialization and subcommands. The N64 controller is known to use the Nintendo HID family, but its exact behavior with this PS5 userspace USB sequence must be validated with a real NSO N64 controller before this PR is considered stable.

Bluetooth N64 support is not part of this branch.
