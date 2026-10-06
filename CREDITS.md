# Credits

GhostControl Expanded is built on work from several PS5 homebrew and controller projects.

## Core GhostControl lineage

- **StonedModder** — original Ghost-Control multi-controller / virtual-pad work that this project descends from.
- **NikoBellikJR31** — Manba V2 USB patch/fork used as the direct starting point for this repository.
- **REXCRAFT88** — GhostControl Expanded integration, GameCube focus, controller expansion, release packaging, and project maintenance.

## Wireless DualShock 4

- **ItsBlurf / PoorDS4** — native-paired wireless DS4 bridge architecture and implementation. The vendored backend under `WIRELESS DS4 POORDS4/` retains PoorDS4's GPL-3.0-or-later license, NOTICE, architecture documentation, firmware-support documentation, and upstream provenance.

## Protocol / interoperability references

- **Dolphin Emulator** — GameCube USB adapter protocol behavior, initialization, and third-party adapter compatibility references.
- **Linux `hid-playstation` and `hid-sony`** — DualShock 4 and DualShock 3 report/protocol references.
- **AnyPad-PS5 / OmniPad-PS5** — useful public PS5 controller/Bluetooth research references. Experimental direct-HCI work remains outside the v0.1.0-beta release path.
- **ps5-payload-dev / PS5 Payload SDK** — payload build toolchain.

## License notes

Each upstream component retains its original licensing requirements. The PoorDS4-derived directory contains its own license and provenance files. This project does not claim authorship of upstream controller protocols or PS5 SDK work.
