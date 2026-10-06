# GhostControl Expanded v0.1.5-beta

This beta is based on fresh PS5 firmware 13.60 hardware logs and specifically targets the remaining DualShock 3 bridge and cleanup failure.

## DualShock 3 / PS3 controller USB

The PS5 successfully enumerated the DS3 as 054C:0268 and SceShellUI opened the generated virtual pad, but the SceShellCore forwarding bridge remained unassigned and timed out with ready=-1.

v0.1.5-beta changes that flow:

- Captures the newly created virtual pad device ID from klog.
- Immediately binds that virtual pad to the current foreground PS5 user.
- Keeps ownership of the bridge while it is still becoming ready.
- Reduces the bridge failure wait from roughly one minute to 12 seconds.
- Makes the remote handle-resolution loop honor stop requests.
- Explicitly disconnects the virtual pad if bridge setup fails.
- Explicitly disconnects the virtual pad on physical-controller unplug.
- Cleans bridge virtual pads on payload reload and shutdown.

These changes are intended to prevent the PS5 from remaining stuck on the controller profile-selection screen after the physical controller is unplugged.

## Wired USB controllers

The 0810:0003 PS1/PS2 adapter and 0F0D:00C1 HORI/Switch-compatible profile support from v0.1.4 remains included.

## Validation

Host parser tests, the PoorDS4 audit, and the complete PS5 Payload SDK build must pass in CI before this prerelease is published. Physical PS5 confirmation is still required before the DS3 path can be called fully validated.
