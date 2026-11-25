# Raggedy Launcher

Raggedy Launcher is a controller-first Minecraft Java launcher for Xbox Series S|X in Developer Mode and Windows. This repository is a clean-room implementation: original code, original UI, original docs, and original assets under the MIT license.

This is an independently authored project with its own source, architecture, file layout, strings, scripts, schemas, UI, and assets.

This project would not have been possible in the way I imagined it without the Simply Legacy and Re-Console teams. Their work gave this launcher a purpose: giving 4J Studios' Legacy Console Edition work a new life on Xbox. See [CREDITS.md](CREDITS.md) for the full credits and thanks.

## Current status

Independent implementation phase:

- MIT license for original work.
- Provenance rules, dependency tracking, architecture, and behavior requirements.
- Portable C++ core for versions, profiles, catalog filtering, device-code state, and launch planning.
- Original device-code view model with a scannable QR matrix and copyable user code.
- Original UWP host skeleton with a TV-friendly sign-in page and generated app assets.
- Graphics translation planning layer for OpenGL or Vulkan concepts mapped to Direct3D.
- Core tests and CI for the portable code.

The UWP host is a skeleton. Full authentication requests, entitlement verification, downloads, game launch, mod management, and graphics runtime work are not implemented yet.

## Build and test the core

With CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Without CMake:

```sh
g++ -std=c++20 -Iinclude tests/core/test_core.cpp src/core/target.cpp src/core/launch_plan.cpp -o test_core
./test_core
```

## Rules

- Microsoft sign-in and Minecraft Java entitlement checks stay intact.
- No offline, cracked, forged, or entitlement-free launch paths.
- All project source and assets are original to this repository.
- Third-party components keep their own licenses.

See `PROVENANCE.md`, `DEPENDENCIES.md`, and `docs/CLEAN_ROOM_ARCHITECTURE.md`.
