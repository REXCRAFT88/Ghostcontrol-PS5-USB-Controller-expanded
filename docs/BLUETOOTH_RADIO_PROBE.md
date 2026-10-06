# Experimental Sony Bluetooth Radio Probe

This branch is stacked on top of the Sony Bluetooth parser/transport PR.

## Purpose

Before sending any Bluetooth command on firmware 13.60, determine how the console exposes its internal Bluetooth controller on this hardware/firmware combination.

The module has two pieces:

1. A host-testable USB configuration-descriptor parser.
2. A Prospero-only **read-only** `/dev/ugen*.*` scanner.

## HCI function signature

A candidate Bluetooth HCI USB function must be alternate setting 0 with:

- interface class `0xe0`
- subclass `0x01`
- protocol `0x01`
- one interrupt-IN endpoint for HCI events
- one bulk-IN endpoint for incoming ACL
- one bulk-OUT endpoint for outgoing ACL

Endpoint numbers are read from descriptors. They are not hard-coded.

## What the PS5 scanner does

- enumerates `/dev/ugen*.*`
- skips root-hub-style `.1` nodes
- opens a node with `O_RDONLY | O_NONBLOCK`
- reads the USB device descriptor
- reads the current USB configuration descriptor
- parses any complete HCI functions
- closes the node

## What it explicitly does NOT do

- no `USB_IFACE_DRIVER_DETACH`
- no `USB_FS_INIT`
- no endpoint opens/claims
- no HCI Reset
- no Inquiry
- no pairing/page request
- no ACL writes
- no changes to the PS5 Bluetooth driver's state

The scanner is not linked into the normal payload yet.

## Why this is separate

AnyPad-PS5 reports a working direct DS4 Bluetooth implementation on a PS5 fat running firmware 10.01. Its public source shows that the console Bluetooth device is shared with the operating system and that endpoint layout should be read from the configuration descriptor.

GhostControl Expanded targets firmware 13.60, so the first console experiment should be observation only. Once the 13.60 descriptor/topology is captured, a later experimental branch can open passive HCI reads alongside the system driver.


## Standalone diagnostic ELF

The branch also adds a separate Makefile target:

```sh
cd "SOURCE MODIFIEE PAYLOAD"
make -f Makefile.bt-probe
```

This builds:

```text
ghostcontrol-bt-probe.elf
```

It is deliberately separate from the normal GhostControl ELF. The normal `Makefile` is intentionally unchanged; the diagnostic uses `Makefile.bt-probe` so it cannot be pulled into the standard payload build by accident. When run on the PS5 it:

1. creates/truncates `/data/ghostpad/bt_probe.log`;
2. elevates only its own short-lived payload process using the same credential model already used by GhostControl;
3. performs the read-only `/dev/ugen*.*` descriptor scan;
4. logs every complete Bluetooth HCI USB function it finds;
5. exits.

Expected log format resembles:

```text
radio[0]: /dev/ugen0.2 VID=0x0e8d PID=0x3603 HCI functions=2
  hci[0]: iface=0 events=0x81 mps=16 acl_in=0x82 mps=1024 acl_out=0x01 mps=1024
```

The exact node, endpoints and function count must be taken from the firmware 13.60 result rather than assumed from firmware 10.01.

Deployment can be done with:

```sh
make -f Makefile.bt-probe deploy PS5_HOST=<PS5_IP>
```

The probe exits after writing the log. It does not remain resident and does not enable controller pairing.
