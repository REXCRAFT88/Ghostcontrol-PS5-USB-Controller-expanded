# GhostControl Expanded v0.1.4-beta

This beta packages the latest controller fixes from real PS5 firmware 13.60 testing.

## PS3 / DualShock 3 USB

- Keeps Sony VID:PID 054C:0268 detection and DS3 USB SET_REPORT initialization.
- Creates the virtual DualSense through the SceShellCore bridge instead of relying on a payload-process VDI handle.
- Waits for the remote virtual pad to report a usable handle before starting controller streaming.
- Retains ptrace-backed bridge updates when direct mdbg_copyin is denied.
- Cleans up the remote bridge on disconnect/reload.

This directly addresses the observed failure where the DS3 initialized and SceShellUI opened the virtual pad, but payload-process VDI returned 0x803b0003.

## Additional wired USB controllers

- Adds an initial USB HID profile for 0810:0003 PS1/PS2-style USB adapters.
- Adds an initial USB HID profile for 0F0D:00C1 HORI/Switch-compatible controllers, including the N64-style USB controller observed during testing.
- Logs initial raw HID reports to make clone-specific button-layout corrections possible.

## Existing support retained

- DualShock 4 USB
- Nintendo GameCube adapter support
- Manba V2 paths
- Optional PoorDS4 wireless DS4 helper package

## Validation status

Host parser tests and the full PS5 Payload SDK build run in CI. The DS3 path is based on real PS5 13.60 logs, but this release still requires a fresh physical-controller confirmation after installing the new ELF. Generic HID button ordering may vary between clones and may need refinement from the next hardware log.
