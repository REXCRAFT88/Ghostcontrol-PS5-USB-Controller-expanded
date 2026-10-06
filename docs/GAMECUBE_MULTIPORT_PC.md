# GameCube Multi-Port + PC Mode

This branch stacks the experimental multi-VDA GameCube architecture on top of the Mayflash/DragonRise PC-mode support.

## Nintendo / Wii U protocol

For `057e:0337` adapters, behavior is unchanged from the earlier multi-port work:

- one 37-byte `0x21` report contains all four physical ports;
- one existing GhostControl VDA becomes the primary GameCube controller;
- up to three auxiliary VDAs are created as additional players press buttons;
- per-port presence bits allow controller insertion/removal tracking.

## Legacy Mayflash/DragonRise PC mode

For 10-byte PC-mode reports:

- byte 0 identifies physical adapter port 1..4;
- each packet updates exactly one physical port;
- the explicit port index is used to select the primary or auxiliary VDA;
- an active extra port creates its auxiliary VDA only after the primary assignment is confirmed.

Unlike Nintendo-mode reports, a legacy PC packet is **not** treated as a snapshot of every port. Missing ports in one packet are not disconnected.

### Presence limitation

The PC report format has no Nintendo-style wired/wireless presence byte. Therefore this branch does not synthesize a per-port disconnect merely because another port produced the latest report.

Adapter-level disconnect still tears down all VDAs. Fine-grained controller-unplug behavior in PC mode requires real hardware captures.

## Newer 9-byte PC firmware

Newer PC firmware exposes a 9-byte controller report without a physical port prefix.

Until hardware testing shows how the PS5 enumerates the adapter's other HID interfaces/nodes, this form is deliberately treated as **one controller stream / port 1**.

We should not guess that one 9-byte stream represents all four ports.

## Assignment behavior

The same experimental limitation as the original multi-port PR still applies:

- auxiliary VDAs are independent input handles;
- the inherited force-bind path uses one `g_inject_uid`;
- four distinct PS5 local-user assignments have not been hardware-proven.

## Validation status

Host-tested:

- Nintendo 37-byte four-port parsing;
- Mayflash 10-byte port-indexed parsing;
- Mayflash 9-byte parsing;
- mappings, triggers and C-stick variants.

Not hardware-tested:

- PS5 VDA multi-port behavior;
- Mayflash PC-mode endpoint behavior on PS5;
- physical controller unplug/replug in PC mode;
- four distinct local player assignments.
