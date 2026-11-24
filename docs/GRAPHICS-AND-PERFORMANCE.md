# Graphics and performance renderers

This note records what the launcher does about graphics APIs and
Sodium-family performance mods, and what is still untested.

## Graphics API policy

- The launcher no longer offers a graphics-backend choice. The only supported
  path is OpenGL through the packaged Mesa/D3D12 bridge.
- For 26.3/Fabric, launch skips the legacy GLFW override and sets both
  `org.lwjgl.opengl.libname` and the `SDL_OPENGL_LIBRARY` hint to the packaged
  `graphics/<runtime>/opengl32.dll`. This is required because RenderPearl's SDL
  backend calls `SDL_GL_LoadLibrary()` before LWJGL uses the OpenGL library.
- Older targets keep the legacy GLFW and LWJGL OpenGL overrides. `java.library.path`
  and the extracted LWJGL native directory are unchanged.
- Vulkan is open work, not an opt-in toggle. There is no `runtime/renderer.txt`,
  renderer probe, or Vulkan setting. A Vulkan path needs Dozen ICD discovery,
  instance/device negotiation, an SDL3 surface that presents on Xbox UWP, and
  end-to-end shader/memory validation before it is exposed. `vulkan-1.dll` is
  not present in the installed runtime.
- Every launch logs the Minecraft version, loader, graphics runtime, selected
  OpenGL library, and any native-path fallback. No secrets are logged.
- Mojang's in-game Graphics API preference (including `Default`) is
  Minecraft-side configuration. The launcher never reads or rewrites the
  game's video options; pinning the 26.3 option explicitly is open work because
  the exact key and migration have not been verified on console.

## Performance renderers

- `config/performance-mods.json` names at most one candidate per exact
  Minecraft version and loader, chosen from publisher metadata
  (Sodium/Embeddium/Rubidium/Nothirium survey in late November 2026).
  Targets with no acceptable candidate carry an explicit `gap` message
  instead of a fake entry. The file is packaged as
  `runtime/performance-mods.json` and covered by
  `scripts/validate-modern-version-matrix.ps1`.
- Auto mode installs the candidate only when the matrix marks it `proven` on
  Xbox. No target is marked proven yet, so today Auto reports the candidate
  (or the gap) and installs nothing. Flipping a target to proven requires an
  on-console test: menus, movement, a save/load cycle, frame times and memory,
  with Legacy4J present where applicable, and exactly one renderer active.
- Managed files use `perf:<project>` ownership tokens in the generic managed-mod
  sidecar format: only exact launcher-installed versions are
  ever removed, colliding user jars are stashed before replacement and
  restored on removal, and shared dependencies survive through refcounts.
  Turning the setting off removes managed files and leaves user mods alone.
- Only one managed renderer is kept per profile. A user-installed renderer
  alongside a managed one is reported as a conflict and left in place.
- Nothirium (1.12.2 Cleanroom) is a Forge-compatible chunk renderer similar
  to Sodium, not a Sodium port. Its required dependencies (MixinBootstrap,
  RenderLib) resolve dynamically at install time. Cleanroom and Xbox
  graphics-stack compatibility are untested.
- Sodium's publisher notes that OpenGL translation layers are unsupported and
  may not work. Metadata matches are therefore install candidates only; the
  `proven` flag is the claim that matters, and it is set only by testing.
