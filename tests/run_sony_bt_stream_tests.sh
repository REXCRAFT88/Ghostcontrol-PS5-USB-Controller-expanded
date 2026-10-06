#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-sony-bt-stream-tests"

cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/tests/test_sony_bt_stream.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_stream.c" \
  -o "$OUT"

"$OUT"
