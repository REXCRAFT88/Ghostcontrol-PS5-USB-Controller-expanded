#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-bt-passive"

cc -std=c99 -Wall -Wextra -Werror -D__PROSPERO__ \
  -I"$ROOT/tests/ps5_stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_passive_main.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_passive.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_probe.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_stream.c" \
  -o "$OUT"

echo "Standalone passive Bluetooth observer Prospero compile passed"
