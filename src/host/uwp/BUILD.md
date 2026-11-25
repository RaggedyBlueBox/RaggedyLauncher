# Host build

The UWP host files are an original C++/WinRT skeleton. They are kept separate from the portable core so the core can be built and tested on any platform.

## Prerequisites

- Visual Studio 2022 with the C++/WinRT and UWP workloads.
- Windows SDK 10.0.19041.0 or newer.
- Your own signing identity for the package manifest.

## Steps

1. Create a fresh UWP C++/WinRT project.
2. Add the files from `src/host/uwp` to the host project.
3. Add the portable core headers and sources under `include` and `src` to the same solution.
4. Replace the placeholder package identity and publisher in `Package.appxmanifest` with your own.
5. Build and deploy the host in Developer Mode.

The sign-in page uses the portable device-code view model and draws a QR matrix for the Microsoft link page. Authentication requests, polling, entitlement verification, downloads, and game launch are not implemented in this skeleton.
