# Graphics and performance renderers

This note records what the launcher does about graphics APIs and
Sodium-family performance mods, and what is still untested.

## Graphics API policy

- The launcher no longer offers a graphics-backend choice. The only supported
  path is OpenGL through the packaged Mesa/D3D12 bridge.
- For 26.3/Fabric, launch skips the legacy GLFW override, sets
  `org.lwjgl.opengl.libname` to the absolute packaged `opengl32.dll`, sets
  the `SDL_OPENGL_LIBRARY` hint to the package-relative
  `graphics/<runtime>/opengl32.dll`, forces the WinRT SDL video driver, and
  preloads the Mesa/D3D12 support stack before the JVM. The pinned SDL fork is
  built with WGL enabled, EGL disabled and the WinRT-aware loader; the base
  patch normalizes absolute packaged Mesa paths for `LoadPackagedLibrary`;
  V7/V8/V9/V12 handle WGL lifecycle, LWJGL provider parity, stability and
  RenderPearl's hidden logical windows; and V13/V14/V15/V16/V17 handle the
  WinRT keyboard/mouse/WGI contract and hidden cursor path. This is required
  because RenderPearl's SDL backend calls `SDL_GL_LoadLibrary()` before LWJGL
  uses the OpenGL library.
- Older targets keep the legacy GLFW and LWJGL OpenGL overrides. `java.library.path`
  and the extracted LWJGL native directory are unchanged. Minecraft 26.3 also
  passes `org.lwjgl.system.SharedLibraryExtractPath` so native extraction uses
  the same writable directory as LWJGL's other temporary native paths.
  Minecraft 26.3 also gets `-XX:StackShadowPages=32` because the official
  launcher includes it and 26.3's OpenGL resource reload otherwise exits with
  a JVM access violation immediately after unifont loading.
- Vulkan is open work, not an opt-in toggle. Raggedy's 26.3 path is OpenGL/WGL;
  its known-mod list blocks VulkanMod because the UWP GLFW path lacks
  `glfwInitVulkanLoader`, and this package has `vulkan_dzn.dll` but no
  `vulkan-1.dll`. A Vulkan bridge needs Dozen ICD discovery, instance/device
  negotiation, an SDL3 surface that presents on Xbox UWP, and end-to-end
  shader/memory validation before it is exposed.
- Every launch logs the Minecraft version, loader, graphics runtime, selected
  OpenGL library, and any native-path fallback. No secrets are logged.
- Mojang's in-game Graphics API preference (including `Default`) is still
  Minecraft-side configuration and is not pinned by the launcher. The launcher
  does overwrite a narrow set of in-game performance options such as frame
  rate, render distance, graphics mode, clouds and ambient occlusion.

## Performance renderers

The performance renderer is always on in lightweight `auto` mode. Settings
shows **Always on** and has no off switch; the launcher applies the policy on
every launch, before Minecraft starts.

- `config/performance-mods.json` names at most one candidate per exact
  Minecraft version and loader, chosen from publisher metadata
  (Sodium/Embeddium/Rubidium/Nothirium survey in late November 2026).
  Targets with no acceptable candidate carry an explicit `gap` message
  instead of a fake entry. The file is packaged as
  `runtime/performance-mods.json` and covered by
  `scripts/validate-modern-version-matrix.ps1`.
- `auto` installs the matched candidate in overwrite mode, including entries
  that are not marked `proven` on Xbox. Unproven entries are metadata-only
  matches and are labelled that way; they still need an on-console test with
  menus, movement, a save/load cycle, frame times and memory, with Legacy4J
  present where applicable, and exactly one renderer active.
- Managed files use `perf:<project>` ownership tokens in the generic managed-mod
  sidecar format: only exact launcher-installed versions are
  ever removed, colliding user jars are stashed before replacement and
  restored on removal, and shared dependencies survive through refcounts.
  Replacing a built-in profile's modpack clears its managed mods and installs
  the newest compatible pack; unrelated user mods are left alone.
- Only one managed renderer is kept per profile. A user-installed renderer
  alongside a managed one is reported as a conflict and left in place.
- Nothirium (1.12.2 Cleanroom) is a Forge-compatible chunk renderer similar
  to Sodium, not a Sodium port. Its required dependencies (MixinBootstrap,
  RenderLib) resolve dynamically at install time. Cleanroom and Xbox
  graphics-stack compatibility are untested.
- Sodium's publisher notes that OpenGL translation layers are unsupported and
  may not work. Metadata matches are applied in overwrite mode but are not
  claimed as Xbox-tested; the `proven` flag remains the tested-only claim.

## Launcher UI cost

The launcher UI is capped at 30 FPS and draws one backdrop image at a time
with no cross-fade. These limits apply only to menus and setup screens; they
do not cap Minecraft's framerate.
