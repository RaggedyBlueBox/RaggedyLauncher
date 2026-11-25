# Raggedy Launcher

Raggedy Launcher is a controller-first Minecraft Java launcher for Xbox Series S|X in Developer Mode and Windows. This repository is a clean-room implementation: original code, original UI, original docs, and original assets under the MIT license.

This is an independently authored project with its own source, architecture, file layout, strings, scripts, schemas, UI, and assets.

This project would not have been possible in the way I imagined it without the Simply Legacy and Re-Console teams. Their work gave this launcher a purpose: giving 4J Studios' Legacy Console Edition work a new life on Xbox. See [CREDITS.md](CREDITS.md) for the full credits and thanks.

## Current status

Independent implementation phase:

- MIT license for original work.
- Provenance rules, dependency tracking, architecture, and behavior requirements.
- Portable C++ core for versions, profiles, catalog filtering, device-code auth, entitlement checks, downloads, launch planning, mod plans, controller navigation, diagnostics, and remote sessions.
- Official Microsoft device-code request and polling, followed by Xbox Live, XSTS, Minecraft sign-in, and Java Edition ownership verification.
- Original device-code view model with a scannable QR matrix and copyable user code.
- Original UWP host with a TV-friendly sign-in page, main menu, version screen, and generated app assets.
- UWP services for HTTP, local state files, SHA-256, a steady clock, and redacted debug logging.
- Target manifests with SHA-256 hashes drive downloads and launch arguments; no URLs or hashes are fabricated by the launcher.
- Game launch uses a configured loopback process bridge with an explicit request and response schema.
- Graphics translation planning layer for OpenGL or Vulkan concepts mapped to Direct3D.
- Core tests and CI for the portable code.

An approved Microsoft application ID, target manifests, and a loopback process bridge are required for a live Minecraft session. Diagnostics and telemetry stay off unless enabled by the user.

## Build and test the core

With CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Without CMake:

```sh
clang++ -std=c++20 -Iinclude tests/core/test_full.cpp $(find src -name '*.cpp' -not -path 'src/host/*' -print) -o test_full
./test_full
```

## Rules

- Microsoft sign-in and Minecraft Java entitlement checks stay intact.
- No offline, cracked, forged, or entitlement-free launch paths.
- All project source and assets are original to this repository.
- Third-party components keep their own licenses.

See `PROVENANCE.md`, `DEPENDENCIES.md`, and `docs/CLEAN_ROOM_ARCHITECTURE.md`.
