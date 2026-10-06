# Optional Wireless DualShock 4 Backend

This directory vendors the GPL-3.0-or-later PoorDS4 implementation by ItsBlurf as an **optional wireless DS4 backend** for GhostControl Expanded.

## Why this backend exists

PoorDS4 takes a safer route than directly taking over the PS5 Bluetooth controller:

1. Pair the DualShock 4 normally from the PS5 Bluetooth Accessories menu.
2. Let Sony's Bluetooth stack own and maintain the wireless link.
3. Discover the live DS4 from the PS5 pad APIs.
4. Read its 120-byte `ScePadData` state through the system `SceRemotePlay` process.
5. Bridge the validated DS4 state into native PS5 games.

This avoids competing with the kernel Bluetooth driver for HCI packets and avoids custom pairing/link-key management.

## Relationship to the rest of GhostControl Expanded

This backend is deliberately separate from the main USB/GameCube payload.

- GameCube USB adapters remain the primary expansion target.
- Wired DS3/DS4 support remains in the USB controller path.
- The experimental direct-HCI Bluetooth research branches remain useful diagnostics, but this PoorDS4-derived backend is the preferred route for **wireless DS4**.
- Wireless DS3 is **not solved by PoorDS4** and remains a separate research task.

## Current upstream evidence

PoorDS4 reports:

- live-tested wireless DS4 operation on PS5 firmware 11.60;
- exact/structural evidence on several other firmwares;
- structural runtime admission for unlisted firmware;
- multi-controller support up to four DS4 sources as experimental;
- native DualSense passthrough for local multiplayer.

Firmware 13.60 is **not currently listed as live-tested** upstream. The bridge is designed to fail closed when its ABI/structure checks do not match.

## Build

Install the PS5 Payload SDK and set `PS5_PAYLOAD_SDK`.

From this directory:

```sh
make clean
make release
```

On Windows, if your SDK uses a wrapper:

```powershell
make CC=ps5-clang.cmd clean
make CC=ps5-clang.cmd release
```

Expected outputs:

- `PoorDS4rc51.elf` — automatic wireless DS4 bridge
- `PoorDS4-status.elf` — read-only status snapshot
- `PoorDS4-stop.elf` — cooperative stop payload
- `PoorDS4-evidence.elf` — read-only 13.x evidence bundle collector

## Pairing

Pair the DS4 using the PS5 UI rather than GhostControl:

1. Hold **SHARE + PS** until the DS4 lightbar rapidly double-blinks.
2. Open **Settings > Accessories > Bluetooth Accessories**.
3. Select **DUALSHOCK 4**.
4. Attach the DS4 to the user profile that should control the game.
5. Deploy `PoorDS4rc51.elf`.

## Firmware 13.60 testing order

If your game is launched from an `.ffpfsc` image, keep using your normal FFPFSC mount/launch workflow. PoorDS4 acts on the running game process after launch, so the compressed storage container does not need a separate controller path.

For the first 13.60 test:

1. Pair the DS4 normally in PS5 Settings.
2. Back up game saves before testing an unknown firmware/game combination.
3. Launch `PoorDS4rc51.elf`.
4. If no active notification appears, run `PoorDS4-status.elf`.
5. Run `PoorDS4-evidence.elf` after reproducing the test once.
6. Retrieve `/data/poords4/13x-evidence.txt` plus, if convenient, the full `/data/poords4/` directory.
7. Check `/data/poords4/fw-compat-last.txt` first. It gives a compact pass/fail breakdown of the structural firmware gate and source/game fingerprints without bypassing any check.
8. Use `PoorDS4-stop.elf` before replacing/reinjecting another bridge build.

Do not bypass structural rejection just to force 13.60 support. The rejection report is more useful than a blind hook.

## Upstream

Source project:

https://github.com/ItsBlurf/PoorDS4

The vendored source was imported from the upstream `main` branch and remains under GPL-3.0-or-later. See `LICENSE`, `NOTICE.md`, and the copied architecture/firmware documents in this directory.
