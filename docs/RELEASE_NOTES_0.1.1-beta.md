# GhostControl Expanded v0.1.1-beta

## DS3 assignment freeze hotfix

This hotfix is based on the first real DualShock 3 USB test on PS5 firmware 13.60.

The DS3 was correctly detected and the PS5 displayed the user-assignment screen, but the console froze immediately after user assignment.

The cause was traced to Manba-specific physical-controller management code being applied to all controller types.

### Fixed

- Physical-pad eviction now runs only for Manba controllers.
- Ordinary physical pads are no longer queued for Manba recovery when Manba is inactive.
- Physical recovery / ShellUI force-bind is paused while DS3, DS4, GameCube, or another non-Manba controller is active.
- DS3 USB initialization, report parsing, VDA creation, and input injection remain unchanged.

### Validation

The hotfix passed:

- GameCube host parser tests;
- wired Sony parser tests;
- PoorDS4 static audit;
- full PS5 Payload SDK compilation;
- release packaging.

### Retest target

For the next DS3 test:

1. replace v0.1.0-beta with the v0.1.1-beta `GhostControl-Expanded.elf`;
2. reboot the PS5 first because the previous test froze the system;
3. start GhostControl with no DS3 attached;
4. connect the DS3 over a data-capable mini-USB cable;
5. press PS;
6. assign the controller if prompted;
7. test basic navigation before launching a game;
8. collect `/data/ghostpad/gc_status.log` whether it works or fails.

Do not reuse the v0.1.0-beta ELF for DS3 testing.
