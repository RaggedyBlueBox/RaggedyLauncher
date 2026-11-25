# Graphics Strategy

The graphics layer will be written from public specifications and original design.

## Goal

Let Minecraft Java run inside the Xbox UWP sandbox by providing platform abstractions for windowing, input, OpenGL, and optional Vulkan paths.

## Allowed approach

- Study public OpenGL, Vulkan, DirectX, ANGLE, and Mesa documentation.
- Define our own platform interfaces.
- Implement translation behavior independently.
- Use third-party libraries only under compatible licenses.
- Record every dependency with version and license.

## Not allowed

- Copying a compatibility shim source tree.
- Copying patches, binary blobs, shader code, or config files from another launcher.
- Reusing generated files or package contents.
- Relicensing third-party components as MIT.

## Planned layers

- Window and input abstraction.
- OpenGL context abstraction.
- DirectX display backend.
- Optional Vulkan backend.
- File and resource path abstraction.
- Diagnostics for unsupported extensions.

## Validation

Each layer needs a narrow test target before the next layer starts. The first target is window creation and input alone. The second is a clear-color DirectX surface. The third is a minimal OpenGL draw call translated through our abstraction.
