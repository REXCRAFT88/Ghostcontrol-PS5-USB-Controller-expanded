# GameCube USB Adapter Support

## Current status

The `feature/gamecube-adapter` branch adds the first GameCube-controller path to GhostControl Expanded.

### Initial target

- Official Nintendo Wii U / Switch GameCube Controller Adapter protocol
- USB VID:PID: `057e:0337`
- Input endpoint: `0x81`
- Output endpoint: `0x02`
- Adapter start command: `0x13`
- 37-byte input report: `0x21` plus four 9-byte controller-port records

Third-party adapters that truly emulate the Nintendo Wii U adapter protocol in their "Wii U" or "Switch" mode should use the same backend.

## Phase 1 limitation

The adapter report parser already understands all four physical controller ports, but the first PS5 integration intentionally exposes **adapter port 1 only**.

This keeps the first hardware test small and lets us validate:

1. PS5 USB detection of `057e:0337`.
2. Interface detach and endpoint ownership.
3. The `0x13` initialization command.
4. 37-byte input reports arriving on the PS5.
5. GameCube button/stick/trigger mapping into `ScePadData`.
6. Virtual DualSense injection on firmware 13.60.

After port 1 is confirmed, the next milestone is one independent VDA/PS5 virtual controller per occupied GameCube adapter port.

## Default mapping

| GameCube | Virtual PS5 input |
|---|---|
| A | Cross |
| B | Circle |
| X | Square |
| Y | Triangle |
| D-pad | D-pad |
| Start | Options |
| Z | R1 |
| L analog | L2 analog |
| R analog | R2 analog |
| L hard click | L2 digital |
| R hard click | R2 digital |
| Main stick | Left stick |
| C-stick | Right stick |

The GameCube trigger analog travel and hard-click state are preserved separately.

## Testing

For the first test, connect a GameCube controller to **port 1** of the USB adapter before or immediately after loading the payload.

Watch `/data/ghostpad/gc_status.log` and kernel logging for lines containing:

- `VID=0x057e PID=0x0337`
- `Nintendo GameCube Controller Adapter detected`
- `GameCube adapter IN ep=0x81`
- `GameCube adapter init ret=0`
- `streaming - controller active`

Please record the log if the adapter is detected but does not stream input.

## Next GameCube milestones

1. Four simultaneous GameCube controllers from one adapter.
2. Per-port hotplug without restarting the payload.
3. Rumble output using the adapter's `0x11` command.
4. Additional adapter VID/PID profiles, including Mayflash/clone PC modes.
5. Configurable button mappings.
6. WaveBird/wireless receiver validation.


## Adapter compatibility notes

### Mayflash / third-party adapters in Wii U or Switch mode

Dolphin's current setup guide explicitly notes that third-party adapters such as Mayflash should be switched to **Wii U** or **Switch** mode, where they identify as the standard Nintendo adapter `057e:0337`. Those modes should therefore use this backend directly.

The initialization path also sends the optional class/interface compatibility request used by Dolphin for some Nyko/off-brand adapters before sending the standard `0x13` adapter start command. Failure of that optional request is non-fatal because Mayflash adapters may reject it and still operate normally.

### Mayflash / DragonRise PC mode

Known PC-mode GameCube adapters may enumerate as DragonRise devices such as `0079:1843`, `0079:1844`, or `0079:1846`. They are **not** currently routed through the Wii U multi-port protocol parser because they expose a different HID interface/report format.

Support for those PC/HID modes belongs in the generic-HID/profile layer rather than pretending they use `057e:0337` reports.
