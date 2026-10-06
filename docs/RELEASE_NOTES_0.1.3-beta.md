# v0.1.3-beta candidate

Hardware-driven changes for PS5 firmware 13.60:

- Gate DS3/DS4 USB streaming until the SceShellCore virtual-pad bridge reports ready with a valid handle.
- Keep ptrace-backed bridge writes available when mdbg_copyin is denied.
- Add USB HID support for the observed PS1/PS2 adapter ID 0810:0003.
- Add USB HID support for the observed 0F0D:00C1 HORI/Switch-compatible protocol used by the tested N64-style USB controller.
- Log the first raw generic-HID reports so clone-specific mapping differences can be corrected from real hardware.
- Cleanly stop the remote virtual-pad bridge on disconnect/reload.

The generic mappings are initial hardware-informed profiles and may need button-order refinement after the first console test.
