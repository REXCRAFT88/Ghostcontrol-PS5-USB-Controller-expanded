#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-sony-parser-tests"

cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/tests/stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/tests/test_sony_parsers.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/controller_ds4.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/controller_ds3.c" \
  -o "$OUT"

"$OUT"


GENERIC_OUT="${TMPDIR:-/tmp}/ghostcontrol-generic-hid-tests"
cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/tests/stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/tests/test_generic_hid.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/controller_generic_hid.c" \
  -o "$GENERIC_OUT"

"$GENERIC_OUT"
