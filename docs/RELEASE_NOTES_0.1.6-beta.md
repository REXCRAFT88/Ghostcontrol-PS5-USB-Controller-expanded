# GhostControl Expanded v0.1.6-beta

This beta fixes two issues confirmed by fresh PS5 firmware 13.60 logs.

## DualShock 3 USB

The PS5 reports GhostControl's virtual controller through SceShellUI as Open Pad type=0. The ShellCore bridge was only searching the type=3 handle namespace after creating the virtual DualSense, so it never found a usable handle and timed out.

v0.1.6-beta now:
- probes the type=0 handle namespace first inside SceShellCore;
- keeps the existing type=3 recovery as a fallback;
- retains the v0.1.5 virtual-device binding and ghost-controller cleanup logic.

## Generic wired USB controllers

The observed IDs 0810:0003 and 0F0D:00C1 had parser code but were accidentally omitted from the discovery table and packet-dispatch path, so they were repeatedly logged as "generic HID backend not mapped yet" and never claimed.

v0.1.6-beta now:
- registers supported generic HID VID/PID pairs during discovery;
- labels them with their generic-HID controller names;
- opens their interrupt-IN endpoint;
- routes reports through generic_hid_parse();
- logs the first raw report for hardware-specific mapping correction.

## Validation

The Sony/generic parser tests, GameCube parser tests, PoorDS4 audit, and full PS5 Payload SDK build are run in CI. Physical PS5 confirmation is still required before these paths can be called fully validated.
