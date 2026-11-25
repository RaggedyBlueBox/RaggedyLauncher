# Dependencies

This project has no vendored third-party runtime dependencies in the portable core.

## Current build dependencies

- C++20 compiler.
- CMake, optional.
- Pillow, development-time only, used to generate original PNG app assets.
- QR code matrix generated from the official Microsoft verification URL and stored as data in `include/raggedy/qr/microsoft_link.hpp`.

## Planned third-party components

These components are not yet vendored. Each needs a license review before inclusion.

- Java runtime.
- Minecraft official files, downloaded only after entitlement succeeds.
- Fabric, Forge, and NeoForge launch components.
- LWJGL.
- OpenGL or Vulkan compatibility layer.
- DirectX translation layer.
- HTTP and TLS provider.
- JSON library.
- Mod source clients.

No dependency may be copied from unrelated launcher source trees. Each dependency must be recorded with its source, version, license, and why it is needed.
