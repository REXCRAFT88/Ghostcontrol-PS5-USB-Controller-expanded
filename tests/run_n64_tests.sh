#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-n64-tests"

cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/tests/stubs" \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  "$ROOT/tests/test_n64_profile.c" \
  "$ROOT/SOURCE MODIFIEE PAYLOAD/controller_nintendo.c" \
  -o "$OUT"

"$OUT"
