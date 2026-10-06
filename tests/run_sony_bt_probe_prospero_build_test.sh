#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-bt-probe-prospero"

cc -std=c99 -Wall -Wextra -Werror -D__PROSPERO__ \
  -I"$ROOT/tests/ps5_stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_probe_main.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_probe.c" \
  -o "$OUT"

echo "Bluetooth probe Prospero path host compile passed"
