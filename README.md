# GhostControl Expanded

GhostControl Expanded is a PS5 controller-compatibility payload for jailbroken consoles. The primary goal is **GameCube controller support through USB GameCube adapters**, with expanded wired Sony-controller support and an optional wireless DualShock 4 backend.

**Release:** v0.1.3-beta  
**Primary test target:** PS5 firmware 13.60  
**Primary hardware target:** Nintendo-compatible GameCube USB adapters

## Release components

### GhostControl-Expanded.elf
The main payload detects supported USB controllers, translates their reports into PS5 `ScePadData`, creates virtual DualSense devices, handles hotplug/reconnect, and logs to:

```text
/data/ghostpad/gc_status.log
/data/ghostpad/gc_main.pid
```

### GhostControl-Stop.elf
Stops a running GhostControl instance through its PID file so the main payload can release USB endpoints and virtual pads cleanly.

### Optional wireless DS4 backend
`WIRELESS DS4 POORDS4/` contains an isolated PoorDS4-derived backend. The PS5 pairs and owns the DS4 normally; the bridge validates the running firmware/game layout and forwards DS4 state into native PS5 games. It fails closed if structural checks do not pass.

For firmware 13.60 it also includes `PoorDS4-evidence.elf`, which creates:

```text
/data/poords4/13x-evidence.txt
```

## Controller support

| Controller / adapter | Status |
|---|---|
| Nintendo GameCube adapter protocol (`057e:0337`) | Implemented; host-tested; PS5 hardware validation pending |
| GameCube ports 1-4 | Multi-port routing implemented; experimental until real hardware validation |
| GameCube analog L/R triggers | Preserved as analog L2/R2 |
| WaveBird through standard adapter | Expected; hardware validation pending |
| Mayflash/clone in Wii U/Switch mode | Expected when it enumerates as `057e:0337` |
| DualShock 4 USB | Implemented; parser-tested |
| DualShock 3 / Sixaxis USB | Implemented; parser-tested |
| DualShock 4 Bluetooth | Optional PoorDS4 backend; 13.60 validation pending |
| Manba V2 USB/XInput/Switch mode | Retained from the original fork |
| Generic N64 USB adapters | Planned |
| Switch Online N64 controller | Planned |
| Generic USB HID gamepads | Planned |
| DualShock 3 Bluetooth | Research / not release-ready |
| Xbox One / Series USB | Inherited code exists; not claimed as validated in this beta |
| Generic Switch Pro | Inherited parser exists; not claimed as validated in this beta |

See [Supported Controllers](docs/SUPPORTED_CONTROLLERS.md).

## GameCube mapping

| GameCube | PS5 virtual input |
|---|---|
| A | Cross |
| B | Circle |
| X | Square |
| Y | Triangle |
| Start | Options |
| Z | R1 |
| L analog | L2 analog |
| R analog | R2 analog |
| L hard click | L2 digital |
| R hard click | R2 digital |
| Main stick | Left stick |
| C-stick | Right stick |
| D-pad | D-pad |

The standard adapter exposes four controller ports through one USB device. GhostControl Expanded parses the full 37-byte report and can route occupied ports to separate virtual PS5 controllers. Four-player user assignment remains a beta feature pending real-console testing.

## Build

Install the current PS5 Payload SDK and set `PS5_PAYLOAD_SDK`:

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

The inherited source-directory name is retained for repository history; all project documentation is English.

For the wireless DS4 backend:

```sh
cd "WIRELESS DS4 POORDS4"
make clean
make release
```

See [Build and Install](docs/BUILD_AND_INSTALL.md).

## Windows launcher

Release packages contain:

```text
Launch-GhostControl-Expanded.bat
Launch-GhostControl-Expanded.ps1
ELF/
  GhostControl-Expanded.elf
  GhostControl-Stop.elf
```

The launcher uses payload port `9021` by default and offers Start/Reload, Clean Restart, and Stop.

## FFPFSC / ShadowMountPlus

Games launched from `.ffpfsc` images do not need a special controller build. Launch the game through your normal ShadowMountPlus workflow first. GhostControl and the optional PoorDS4 bridge operate on the running controller/game processes after launch.

For wireless DS4 testing on firmware 13.60:

1. launch the `.ffpfsc` game normally;
2. pair the DS4 under **Settings > Accessories > Bluetooth Accessories**;
3. attach it to the intended user;
4. run the PoorDS4 bridge;
5. if it does not activate, run `PoorDS4-status.elf`;
6. run `PoorDS4-evidence.elf`;
7. retrieve `/data/poords4/13x-evidence.txt`.

Do not bypass a structural firmware/game rejection.

## Automated validation

CI covers GameCube adapter matching and initialization, all four GameCube port records, GameCube buttons/sticks/triggers, wired DS3/DS4 parsing, Sony VID/PID matching, PoorDS4 safety invariants, and release compilation with the public PS5 Payload SDK.

## Repository layout

```text
SOURCE MODIFIEE PAYLOAD/   Unified USB controller payload
WIRELESS DS4 POORDS4/      Optional native-paired wireless DS4 backend
docs/                      Project documentation
tests/                     Host-side parser tests
.github/workflows/         CI and release packaging
```

## Documentation

- [Build and Install](docs/BUILD_AND_INSTALL.md)
- [Supported Controllers](docs/SUPPORTED_CONTROLLERS.md)
- [GameCube Adapter](docs/GAMECUBE_ADAPTER.md)
- [GameCube Multi-Port](docs/GAMECUBE_MULTIPORT.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [Roadmap](docs/ROADMAP.md)
- [v0.1.0-beta Release Notes](docs/RELEASE_NOTES_0.1.0-beta.md)
- [Credits](CREDITS.md)

## Beta limitations

This release is intentionally labeled beta. The parsers and build are automated-tested, but broader PS5 hardware validation is still needed for four simultaneous GameCube pads, third-party adapter variants, hotplug edge cases, and firmware 13.60 wireless DS4 compatibility.

When reporting a problem, include `/data/ghostpad/gc_status.log` or the PoorDS4 evidence bundle.

## License and credits

GhostControl Expanded retains the licensing requirements of its upstream components. The optional PoorDS4 backend is GPL-3.0-or-later and includes its upstream license, notice, provenance, and documentation.

See [CREDITS.md](CREDITS.md).
