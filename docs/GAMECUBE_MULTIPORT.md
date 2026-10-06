# Experimental GameCube Multi-Port Virtualization

This branch is stacked on top of `feature/gamecube-adapter`.

## Goal

Use all four physical controller ports on one Nintendo-compatible GameCube USB adapter while keeping a single USB reader for the adapter.

The adapter produces one 37-byte report containing four independent 9-byte controller records. This branch routes those records to multiple virtual DualSense devices.

## Current design

- One normal GhostControl slot owns the physical USB adapter.
- That slot's existing VDA is the **primary GameCube port**.
- The primary port is chosen from whichever connected port is active; before assignment is confirmed, a real button press can move primary ownership away from an idle port.
- Up to three additional physical ports get **auxiliary VDAs**.
- Auxiliary VDAs are created only after the primary VDA has been confirmed and the player on that extra port presses a button.
- Per-port disconnect removes that port's auxiliary VDA.
- Adapter disconnect or payload shutdown removes all auxiliary VDAs.
- A second GameCube adapter and unrelated USB controllers are deferred while a four-port adapter is active, avoiding an uncontrolled virtual-pad count.

## Why delayed auxiliary creation?

Creating four virtual controllers immediately when an adapter is plugged in could stack multiple PS5 assignment events/dialogs. Requiring activity on each extra controller serializes creation and keeps the first hardware test safer.

## Important limitation

The inherited GhostControl code force-binds virtual devices through one `g_inject_uid`. This multi-port work creates independent VDA handles and independent input streams, but it does **not yet prove that PS5 games will treat all four as four separately assigned local users**.

Hardware validation on firmware 13.60 must determine:

1. Whether multiple VDA devices bound through the current path are accepted simultaneously.
2. Whether each can be assigned to a separate local player/user when required.
3. Whether ShellUI needs a per-VDA assignment queue rather than the current single-user force-bind behavior.

If separate user assignment is required, that should be solved as the next architecture step rather than hidden behind a "4-player supported" claim.

## Test status

- GameCube 37-byte report parser: host-tested.
- All four physical report records: host-tested independently.
- PS5 VDA multi-port creation: not hardware-tested.
- Four-player PS5 gameplay: not hardware-tested.
