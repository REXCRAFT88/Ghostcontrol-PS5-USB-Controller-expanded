# GhostControl Expanded v0.1.3-beta

## Sony USB input architecture fix

Real PS5 firmware 13.60 testing confirmed that DualShock 3 discovery and user binding worked, but payload-process VDI failed with error `0x803b0003`, so no controller input reached the PS5.

### Changed

- DualShock 3 and DualShock 4 USB now use the existing SceShellCore forwarding bridge.
- The virtual DualSense is created inside SceShellCore.
- Controller frames are delivered through `shellui_pad_update()` and VDI executes in the process context that owns the virtual-device handle.
- Added clean bridge shutdown on controller disconnect, payload reload, and normal teardown.
- Manba remains on its established path.
- GameCube remains on its existing VDA path for now.

### Why

On firmware 13.60, the SceShellUI Open Pad handle is process-local. Calling `scePadVirtualDeviceInsertData()` from the payload process with that handle returned `0x803b0003`. Moving creation and injection into SceShellCore removes that cross-process handle misuse.

### Generic HID status

The tested PS2 USB adapter was identified as `0810:0003`, HID class `03/00/00`. It is now known and ready for the generic HID backend, but v0.1.3-beta does not yet claim full PS2/N64 generic-HID input support.

### Validation

This release passed:

- GameCube host tests;
- wired Sony parser tests;
- PoorDS4 static audit;
- full PS5 Payload SDK compilation;
- release packaging.

### Test order

1. Reboot the PS5.
2. Load `GhostControl-Expanded.elf` from v0.1.3-beta.
3. Connect the DS3 by mini-USB.
4. Press PS and test D-pad, face buttons, sticks, and PS.
5. If anything fails, collect `/data/ghostpad/gc_status.log`.
