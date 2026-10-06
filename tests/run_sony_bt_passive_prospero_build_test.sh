#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-bt-passive-prospero.o"

cc -std=c99 -Wall -Wextra -Werror -D__PROSPERO__ \
  -I"$ROOT/tests/ps5_stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  -c "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_passive.c" \
  -o "$OUT"

echo "Bluetooth passive observer Prospero compile passed"
