# Credits

## Projet D'origine

- Projet/tool original : StonedModder,
  `Ghostcontrol-PS5-USB-Controller-Patcher`
  - https://github.com/StonedModder/Ghostcontrol-PS5-USB-Controller-Patcher

Les credits Ghost-Control originaux, les noms et les notifications existantes
ont ete gardes dans les sources autant que possible.

## Patch Manba V2 NBJr Et Tests

- Tests Manba V2 NBJr USB, validation sur PS5, verification des axes, tests de
  changement user, tests Manba/officielle .



## Bluetooth Research References

- AnyPad-PS5 by sinfiltros and contributors (GPL-3.0-or-later):
  https://github.com/sinfiltros/AnyPad-PS5
  - Reference architecture for safe coexistence with the PS5 Bluetooth USB
    device, HCI endpoint discovery from configuration descriptors, pairing,
    L2CAP/HID transport, and virtual-pad integration.
  - GhostControl Expanded keeps its implementation separate and ports only
    compatible concepts/code with attribution, with firmware 13.60 treated as
    a new validation target.

- Linux HID drivers (`hid-playstation` and `hid-sony`) are used as protocol
  references for DualShock 4 and DualShock 3 report formats and quirks.
