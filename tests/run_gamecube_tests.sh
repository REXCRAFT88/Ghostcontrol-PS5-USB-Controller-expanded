#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-gamecube-parser-tests"

cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/tests/stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/tests/test_gamecube_parser.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/controller_gamecube.c" \
  -o "$OUT"

"$OUT"
