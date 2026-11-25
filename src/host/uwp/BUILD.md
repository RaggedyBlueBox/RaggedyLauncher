# Host build

The UWP host is an original C++/WinRT implementation. It is kept separate from the portable core so the core can be built and tested on any platform.

## Prerequisites

- Visual Studio 2022 with the C++/WinRT and UWP workloads.
- Windows SDK 10.0.19041.0 or newer.
- Your own signing identity for the package manifest.
- A Microsoft application ID approved for Minecraft services.
- A loopback process bridge for starting Java.

## Steps

1. Create a fresh UWP C++/WinRT project.
2. Add the files from `src/host/uwp` to the host project.
3. Add the portable core headers and sources under `include` and `src` to the same solution.
4. Replace the placeholder package identity and publisher in `Package.appxmanifest` with your own.
5. Build and deploy the host in Developer Mode.
6. Save `state/settings.json` in local app data with your approved client ID and bridge URL.
7. Save one manifest per target under `state/manifests/<target-id>.json`.

## Settings fields

- `microsoft_client_id`: the approved public application ID.
- `process_bridge_url`: a loopback URL such as `http://127.0.0.1:4137/launch`.
- `telemetry_enabled`: stays `false` unless the user opts in.
- `remote_files_enabled`: stays `false` unless the user opts in.

## Target manifest fields

- `id`: target identifier from the built-in catalog.
- `java_executable`, `class_path`, `jvm_args`, `main_class`, `game_args`, `native_directory`.
- `required_directories`: directories created before downloads.
- `downloads`: HTTPS URL, destination, and lowercase or uppercase 64-character SHA-256 hash for every required file.

The launcher verifies ownership before any target manifest is read or download is queued. The process bridge receives JSON with an `arguments` array and must return JSON with `exit_code` and `output` fields. Tokens and account identifiers are never logged.

The sign-in page uses the portable device-code view model and draws a QR matrix for the Microsoft link page.
