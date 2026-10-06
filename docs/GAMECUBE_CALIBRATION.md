# GameCube Axis Calibration

This branch adds independent per-port GameCube stick and trigger calibration.

## Why

GameCube controllers do not necessarily use the full USB byte range for their physical stick travel.

Current SDL GameCube support starts each axis with an expected physical range of approximately:

```text
128 - 88 = 40
128 + 88 = 216
```

and then expands the learned minimum/maximum when real hardware exceeds those assumptions.

Passing the raw adapter bytes directly into a virtual PS5 controller can therefore produce reduced stick travel in games.

## Behavior

Each physical GameCube port gets its own `GameCubeCalibration`.

Initial range:

- sticks: 40..216
- analog L trigger: 40..216
- analog R trigger: 40..216

For every valid report:

1. observed minima/maxima expand the learned range;
2. the raw value is remapped to the full virtual-pad 0..255 range;
3. center remains near 128;
4. digital buttons and L/R hard-click bits are unchanged.

Nintendo-mode controller reconnect resets calibration for that physical port.

Mayflash/DragonRise PC-mode calibration starts with the first valid report for each indexed port.

## What this improves

- full main-stick deflection in PS5 games;
- full C-stick deflection;
- normalized analog trigger travel;
- support for controllers whose actual extrema exceed the default assumptions;
- independent behavior for four controllers on one adapter.

## What it does not do

This is range calibration, not a configurable deadzone system.

A future hardware-tested configuration layer could add:

- center deadzones;
- anti-deadzones;
- user calibration capture;
- saved per-controller profiles.

## Validation

Host tests cover:

- minimum -> 0;
- center -> approximately 128;
- maximum -> 255;
- trigger normalization;
- expansion beyond initial min/max;
- persistence of learned extrema.

Real controller validation is still required.
