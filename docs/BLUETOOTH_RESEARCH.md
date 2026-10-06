# Sony Bluetooth Research

## Status

Direct PS5 Bluetooth is an experimental secondary track. GameCube USB adapter support remains the primary project goal.

This branch deliberately stops before touching the PS5's internal Bluetooth controller. It implements and tests the layers that can be verified safely off-console:

- DualShock 4 Bluetooth report parsing.
- DualShock 3 / Sixaxis Bluetooth report parsing.
- HCI ACL -> L2CAP HID Interrupt -> HIDP DATA/INPUT extraction.
- Host-side regression tests and GitHub Actions CI.

None of these files are linked into the PS5 ELF yet.

## DualShock 4 report support

The parser accepts the two input shapes documented by the Linux PlayStation HID driver:

- Full Bluetooth report: ID `0x11`, 78 bytes.
- Minimal Bluetooth report: ID `0x01`, 10 bytes.

The full report has two transport bytes after the report ID; after those bytes the core stick/button/trigger state is laid out the same way as the USB report. The implementation uses one shared core-state decoder so USB and Bluetooth mappings cannot drift apart.

## DualShock 3 / Sixaxis report support

The initial Bluetooth parser accepts the standard 49-byte report ID `0x01` and reuses the wired DS3 state decoder.

It also rejects the known bogus Bluetooth frame where report byte 1 is `0xff`, matching the behavior documented by Linux `hid-sony`.

This is only report parsing. DS3 Bluetooth pairing is not implemented.

## HCI ACL / L2CAP / HIDP boundary

`sony_bt_transport.c` currently parses only a complete, already-received Bluetooth Classic HID interrupt frame:

```
HCI ACL header
  -> L2CAP header
     -> HID Interrupt CID 0x0013
        -> HIDP DATA/INPUT 0xA1
           -> controller HID report
```

ACL fragmentation and reassembly are intentionally left to the future HCI host layer.

## Reference implementation: AnyPad-PS5

AnyPad-PS5 is a GPL-3.0 project with a real-console verified DualShock 4 Bluetooth path on a PS5 fat running firmware 10.01. Its design gives us a much better starting point for the PS5 radio layer than raw experimentation.

Important safety/architecture lessons from that project:

- The PS5 Bluetooth USB device is shared with the system.
- Do not detach the system's Bluetooth driver; doing so can take the official DualSense path down.
- Read the Bluetooth interface/endpoints from USB descriptors rather than assuming fixed endpoint numbers.
- The system and payload may compete for incoming HCI packets, so the host logic must tolerate packet loss and retry.
- Never rely on a single HCI event arriving.
- Cleanly close only links created/owned by the payload.
- Stop/cleanup cleanly so stale ACL links do not survive a payload restart.
- Keep radio work bounded and recoverable; a Bluetooth failure must not break the USB controller path.

## Firmware 13.60 plan

AnyPad's verified Bluetooth result is on firmware 10.01, while this project targets firmware 13.60. We should therefore port the transport as an explicitly experimental backend rather than assume identical USB topology or Bluetooth coexistence behavior.

Proposed phases:

1. **Read-only radio discovery**
   - enumerate `/dev/ugen*.*`
   - inspect device/configuration descriptors
   - identify Bluetooth HCI interface(s)
   - log endpoints without detaching any kernel driver

2. **Passive HCI observation**
   - open safe read transfers alongside the system driver
   - confirm HCI event + ACL endpoint behavior on 13.60
   - no inquiry/pairing commands yet

3. **DS4 pairing experiment**
   - isolated experimental build/compile flag
   - inquiry/page/pair using a dedicated link
   - HID control + interrupt L2CAP channels
   - feed HID interrupt reports into `ds4_parse_bt_input`
   - translate to the existing virtual DualSense path

4. **Persistence/reconnect**
   - save link key + controller address
   - reconnect after controller power cycle
   - clean stale links on restart

5. **DS3 research**
   - determine safe PS3 host-address pairing workflow on PS5
   - establish Classic Bluetooth HID
   - feed 49-byte reports into `ds3_parse_bt_input`

## What is not claimed

- Direct DS4 Bluetooth is not yet validated by this GhostControl fork.
- Direct DS3 Bluetooth is not implemented.
- Firmware 13.60 radio topology/coexistence is not yet verified.
- Bluetooth support must not be merged into the stable GameCube/USB path until console testing confirms safe coexistence.
