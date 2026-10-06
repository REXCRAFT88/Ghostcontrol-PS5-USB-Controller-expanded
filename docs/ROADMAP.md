# GhostControl Expanded Roadmap

## Priority 1 — GameCube USB adapters

GameCube is the primary development target.

### Milestone GC-1 — official adapter protocol
- [x] Add Nintendo GameCube adapter protocol parser.
- [x] Detect `057e:0337`.
- [x] Send adapter `0x13` initialization command.
- [x] Parse all four 9-byte port records.
- [x] Map port 1 into a virtual DualSense for first hardware validation.
- [ ] Validate on PS5 firmware 13.60.

### Milestone GC-2 — four-port adapter virtualization
- [ ] Allocate one independent virtual DualSense slot per occupied adapter port.
- [ ] Detect controller insertion/removal per port.
- [ ] Do not require unplugging the USB adapter when changing controllers.
- [ ] Preserve independent assignment state for each GameCube controller.
- [ ] Validate four-player local input.

### Milestone GC-3 — adapter compatibility
- [ ] Official Nintendo Wii U adapter.
- [ ] Official Nintendo Switch-era adapter.
- [ ] Mayflash four-port adapter in Wii U/Switch compatibility mode.
- [ ] Mayflash/clone PC HID modes where practical.
- [ ] WaveBird receiver.
- [ ] Other common third-party adapters by submitted VID/PID and report captures.

### Milestone GC-4 — output/features
- [ ] GameCube rumble through adapter output command `0x11`.
- [ ] Configurable mappings.
- [ ] Deadzone/calibration options.
- [ ] Per-port status logging.

## Priority 2 — broaden wired USB controller support

The current inherited tree already contains partial DS4, Xbox and Nintendo backends, but the Manba fork restricted discovery.

- [ ] Restore and verify DualShock 4 USB discovery.
- [ ] Add/verify DualShock 3 USB initialization and report parsing.
- [ ] Restore and verify Xbox One/Series USB.
- [ ] General Switch Pro support separate from Manba-specific quirks.
- [ ] Generic HID profile layer for common retro USB adapters.
- [ ] N64 USB adapters and Switch Online N64 controller.

## Priority 3 — direct Bluetooth DS4 / DS3

Direct Bluetooth is a separate transport problem from USB and should not block GameCube work.

### Research track BT-1 — PS5 Bluetooth transport
- [ ] Dynamically locate the PS5 internal Bluetooth USB/HCI interface.
- [ ] Confirm safe HCI event + ACL I/O on firmware 13.60.
- [ ] Avoid detaching or disrupting the console's native Bluetooth stack unless a safe coexistence method is proven.
- [ ] Investigate `libSceBluetoothHid`, MBus, and existing AnyPad/OmniPad research.

### BT-2 — DualShock 4
- [ ] Classic Bluetooth pairing.
- [ ] HID control/interrupt L2CAP channels.
- [ ] Parse DS4 Bluetooth report `0x11`.
- [ ] Translate DS4 state into virtual DualSense input.
- [ ] Reconnect/pairing persistence.

### BT-3 — DualShock 3
- [ ] DS3 host-address/pairing workflow.
- [ ] Classic Bluetooth HID connection.
- [ ] DS3 Bluetooth report parser.
- [ ] Translate into virtual DualSense input.
- [ ] Reconnect/pairing persistence.

## Bluetooth feasibility note

Direct DS3/DS4 Bluetooth appears technically possible, and public PS5 homebrew contains useful HCI/L2CAP research. It is still substantially riskier than USB because the internal Bluetooth radio is owned by the PS5 operating system and direct raw access can interfere with native devices.

For that reason, Bluetooth work should initially live behind an experimental build/feature flag and should not modify the stable GameCube USB path.
