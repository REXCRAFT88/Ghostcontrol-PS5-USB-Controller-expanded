# Upstream provenance

Vendored project: ItsBlurf/PoorDS4

Upstream repository:
https://github.com/ItsBlurf/PoorDS4

Imported branch:
`main`

Imported commit:
`476b0b667ce82c6e51ffd9e96d7ef1f5bfc74d4b`

License:
GPL-3.0-or-later

The initial vendoring commit copied the PoorDS4 payload source, license, notice, architecture document, and firmware-support document without behavioral changes. GhostControl Expanded-specific changes are kept in later commits.

Local integration changes currently include:

- keeping the backend in `WIRELESS DS4 POORDS4/`;
- removing the hard-coded `ps5-clang.cmd` assignment so callers can supply their SDK/toolchain normally;
- adding GhostControl Expanded build/pairing/firmware-13.60 guidance.

When updating from upstream, preserve this file and record the new upstream commit SHA.
