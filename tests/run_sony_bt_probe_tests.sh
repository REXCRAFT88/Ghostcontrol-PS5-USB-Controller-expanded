#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-sony-bt-probe-tests"

cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/tests/test_sony_bt_probe.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_probe.c" \
  -o "$OUT"

"$OUT"
