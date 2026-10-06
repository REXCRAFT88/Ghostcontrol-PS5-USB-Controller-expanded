# Build and Install

## Requirements

- A jailbroken PS5 capable of receiving ELF payloads.
- PS5 Payload SDK.
- A payload loader, commonly on TCP port `9021`.
- For Windows launcher use: PowerShell 5+ or PowerShell 7.
- For `.ffpfsc` games: your normal ShadowMountPlus/FFPFSC setup.

The current public PS5 Payload SDK publishes a prebuilt SDK archive that can be installed under `/opt/ps5-payload-sdk`.

## Build the unified USB payload

```sh
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
make -C "SOURCE MODIFIEE PAYLOAD" clean
make -C "SOURCE MODIFIEE PAYLOAD"
```

Outputs:

```text
SOURCE MODIFIEE PAYLOAD/GhostControl-Expanded.elf
SOURCE MODIFIEE PAYLOAD/GhostControl-Stop.elf
```

## Build the optional wireless DS4 backend

```sh
cd "WIRELESS DS4 POORDS4"
make clean
make release
```

Expected outputs include:

```text
PoorDS4rc51.elf
PoorDS4-status.elf
PoorDS4-stop.elf
PoorDS4-evidence.elf
```

## Send manually

Main payload:

```sh
nc -w 8 <PS5_IP> 9021 < "SOURCE MODIFIEE PAYLOAD/GhostControl-Expanded.elf"
```

Stop payload:

```sh
nc -w 8 <PS5_IP> 9021 < "SOURCE MODIFIEE PAYLOAD/GhostControl-Stop.elf"
```

The main payload also detects an older GhostControl PID and attempts a cooperative stop before starting.

## Windows release launcher

Extract the release ZIP and run:

```text
Launch-GhostControl-Expanded.bat
```

The launcher remembers the PS5 IP/port locally and can:

1. Start / reload GhostControl.
2. Clean restart (stop, wait, start).
3. Stop GhostControl.

## GameCube adapter setup

For the first hardware test:

1. connect a GameCube controller to adapter port 1;
2. connect the adapter to the PS5;
3. use Wii U / Switch compatibility mode if your third-party adapter provides a mode switch;
4. send `GhostControl-Expanded.elf`;
5. watch the PS5 assignment flow;
6. press a GameCube button to confirm assignment;
7. inspect `/data/ghostpad/gc_status.log` if input does not appear.

The official Nintendo-compatible protocol is `057e:0337`.

## Wireless DS4 setup

1. Put the DS4 into pairing mode with **SHARE + PS**.
2. Pair it in **Settings > Accessories > Bluetooth Accessories**.
3. Attach it to the intended user.
4. Launch the target PS5 game.
5. Send `PoorDS4rc51.elf`.
6. If the bridge does not activate, send `PoorDS4-status.elf`.
7. Send `PoorDS4-evidence.elf`.
8. Retrieve `/data/poords4/13x-evidence.txt`.

Firmware 13.60 is a project test target, but it must pass PoorDS4's structural checks rather than being force-enabled.

## FFPFSC titles

If the game is stored as `.ffpfsc`, launch it normally through ShadowMountPlus first. Controller translation happens against the running PS5 processes, not the compressed container format.

## Release CI

The release branch downloads the public PS5 Payload SDK, runs parser/static tests, compiles all release ELFs, packages them into one ZIP, and uploads that ZIP as a GitHub Actions artifact.
