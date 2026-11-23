# Graphics and performance renderers

This note records what the launcher does about graphics APIs and
Sodium-family performance mods, and what is still untested.

## Graphics API policy

- The default is OpenGL through the packaged Mesa/D3D12 path
  (`MC.Xbox/launch/renderer_policy.cpp`). The choice is stored per install in
  `runtime/renderer.txt` (`opengl` or `vulkan-experimental`) and is independent
  of controller bundles and Xbox compatibility fixes.
- Vulkan is an explicit opt-in only. `ProbeVulkanBridge` checks the packaged
  artifacts (`natives/vulkan_dzn.dll`, `natives/vulkan-1.dll`,
  `natives/SDL3.dll`) and stays fail-closed until a real bridge passes
  end-to-end validation on Xbox: loader plus Dozen ICD discovery, instance and
  device negotiation, an SDL3 surface with presentation on Xbox UWP, the shader
  stack and memory behavior. A packaged DLL is not proof of a working path,
  and the probe does not claim otherwise.
- Requesting Vulkan while the probe fails leaves OpenGL active, logs the
  reason and tells the user in Settings. A launch that finds a stored Vulkan
  request with a failing probe falls back to OpenGL and logs the fallback.
- Every launch logs requested backend, actual backend, Minecraft version and
  the detected graphics runtime. No secrets are logged.
- Recovery: pick OpenGL again in Settings. No separate safe mode is needed
  because Vulkan can never become the launch backend without a passing probe.
- 26.x note: Mojang's own Graphics API preference (including `Default`) is
  Minecraft-side configuration. The launcher never reads or rewrites the
  game's video options; it only keeps its native path on OpenGL and logs that
  a 26.x target may still offer its own choice in game. Pinning the in-game
  option to OpenGL explicitly is open work: the exact option key and migration
  for 26.3 have not been verified on console, and guessing at config keys
  would risk wiping the player's other video settings.

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
- Managed files use `perf:<project>` ownership tokens in the same sidecar
  format as controller bundles: only exact launcher-installed versions are
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
