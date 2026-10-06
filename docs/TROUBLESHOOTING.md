# Troubleshooting

## GhostControl starts but no controller appears

Check:

```text
/data/ghostpad/gc_status.log
```

Look for detected VID/PID, endpoint-open failures, GameCube initialization results, assignment timeouts, and virtual-device creation/bind failures.

## GameCube adapter is not detected

The release GameCube backend targets the Nintendo Wii U/Switch protocol:

```text
057e:0337
```

If your adapter has a mode switch, use Wii U or Switch compatibility mode. PC-mode HID variants can expose a different protocol and are not handled by the GameCube backend yet.

## GameCube controller is connected but no input reaches the game

For the first test use adapter port 1 and press a button after virtual-controller assignment appears.

Confirm the log contains lines similar to:

```text
VID=0x057e PID=0x0337
GameCube adapter IN ep=0x81
GameCube adapter init ret=0
```

If the adapter initializes but input still fails, preserve the complete `gc_status.log`.

## Additional GameCube ports do not become players

Multi-port is beta.

Additional VDAs are created after the primary virtual pad has been confirmed and the extra GameCube controller generates activity.

The current user-binding path still needs real-console validation for true four-user local multiplayer.

## DS4 USB is ignored

Confirm the controller is actually connected over USB and matches a supported Sony/HORI ID.

Known Sony IDs include:

```text
054c:05c4
054c:09cc
```

## DS3 USB is detected but not streaming

DS3 requires a SET_REPORT initialization before normal input streaming. Check the log for the DS3 initialization and endpoint-open results.

## Wireless DS4 does not work on firmware 13.60

Do not bypass structural rejection.

Run:

1. `PoorDS4-status.elf`
2. `PoorDS4-evidence.elf`

Retrieve:

```text
/data/poords4/13x-evidence.txt
/data/poords4/fw-compat-last.txt
```

The compatibility digest shows whether failure came from wrapper shapes/targets, executable mappings, controller-info ABI evidence, or source/game fingerprint mismatch.

## FFPFSC game considerations

GhostControl does not parse the `.ffpfsc` container. Make sure ShadowMountPlus has successfully launched the title first, then run GhostControl/PoorDS4 against the live game process.

## Reloading leaves USB input broken

Use a clean restart:

1. send `GhostControl-Stop.elf`;
2. wait about two seconds;
3. send `GhostControl-Expanded.elf`.

The Windows launcher provides this as option 2.

## Reporting a useful bug

Include:

- PS5 firmware;
- controller/adapter model;
- adapter mode switch position if applicable;
- VID/PID if known;
- physical GameCube adapter port;
- `/data/ghostpad/gc_status.log`;
- for wireless DS4, `/data/poords4/13x-evidence.txt`.
