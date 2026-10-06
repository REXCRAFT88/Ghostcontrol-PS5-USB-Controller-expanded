#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/ghostcontrol-bt-probe-main.o"

cc -std=c99 -Wall -Wextra -Werror \
  -I"$ROOT/SOURCE MODIFIEE PAYLOAD" \
  -c "$ROOT/SOURCE MODIFIEE PAYLOAD/sony_bt_probe_main.c" \
  -o "$OUT"

echo "Bluetooth probe entry point host compile passed"
