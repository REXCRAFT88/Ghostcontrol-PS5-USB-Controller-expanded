#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-sony-bt-transport-tests"

cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/tests/stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/tests/test_sony_bt_transport.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_transport.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/controller_ds4.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/controller_ds3.c" \
  -o "$OUT"

"$OUT"
