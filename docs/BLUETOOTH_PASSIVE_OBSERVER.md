# Passive Bluetooth HCI Observer

This branch is stacked on top of the read-only Bluetooth radio-probe work.

## Purpose

Capture a small sample of **naturally occurring** Bluetooth HCI traffic on firmware 13.60 before attempting pairing, inquiry, or controller-specific radio commands.

The diagnostic uses the descriptor-discovered Bluetooth HCI function rather than hard-coded endpoint numbers.

## Safety boundary

This observer is more invasive than the descriptor-only probe because it calls `USB_FS_INIT` and opens two input endpoints:

- HCI event **IN**
- ACL **IN**

It does **not**:

- detach the PS5 kernel Bluetooth driver;
- open any ACL OUT endpoint;
- send a USB control request;
- send HCI Reset;
- start Inquiry/Page;
- send ACL/L2CAP packets;
- pair or modify controller keys.

The observation window is hard-limited to 5 seconds. The observer then stops both transfers, uninitializes `usb_fs`, closes the device, writes a summary, and exits.

Because input reads coexist with the system Bluetooth driver, they may temporarily win packets that the system would otherwise read. For that reason this remains a separate experimental ELF and must not be merged into the normal GhostControl payload.

## Build

```sh
cd "SOURCE MODIFIEE PAYLOAD"
make -f Makefile.bt-passive
```

Output:

```text
ghostcontrol-bt-passive.elf
```

Deploy:

```sh
make -f Makefile.bt-passive deploy PS5_HOST=<PS5_IP>
```

## Output

The observer writes:

```text
/data/ghostpad/bt_passive.log
```

It logs the selected radio/interface/endpoints, up to 16 packet samples (first 32 bytes each), and total HCI event/ACL packet counts.

A useful firmware-13.60 result tells us whether the descriptor-selected function actually carries natural traffic and whether the event/ACL stream framing matches the tested reassembler.

## Next step after a successful passive capture

Only after passive traffic is confirmed should we create a separate DS4 pairing experiment. That later branch would add outbound HCI/L2CAP operations with bounded retries and explicit cleanup. Direct DS3 Bluetooth remains a later research item.
