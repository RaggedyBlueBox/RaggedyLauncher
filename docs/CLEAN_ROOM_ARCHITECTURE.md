# Raggedy Launcher Clean-Room Architecture

Status: active rewrite in progress. Portable core code and tests are included.

## Goal

Build Raggedy Launcher as an independently authored Minecraft Java launcher for Xbox Series S|X in Developer Mode and Windows.

The project has its own architecture, names, file layout, UI copy, data schema, package identity, build scripts, and source history.

Legal ownership cannot be guaranteed by a document alone. This architecture is written to support a real clean-room independent creation. A lawyer should review the final product, branding, third-party licenses, and public claims.

This project applies MIT only to original code and original assets written here. Third-party material stays under its own license and must be handled under that license.

## Clean-room rules

### Allowed inputs

- User requirements and product decisions.
- Public official documentation for Microsoft identity, Xbox Developer Mode, UWP packaging, Minecraft Java launching, Modrinth, CurseForge, Fabric, Forge, NeoForge, Java, LWJGL, Mesa, and controller APIs.
- Public behavioral descriptions of what the launcher needs to do.
- Observed outputs from running software, when limited to behavior and not source code.
- Standard algorithms and general engineering knowledge.

### Blocked inputs during implementation

- Unrelated launcher source trees at any commit.
- Files, snippets, manifests, scripts, strings, comments, identifiers, JSON keys, icons, layout files, certificates, or generated outputs from unrelated launcher projects.
- Source obtained from unrelated launcher git history, cache folders, build outputs, package contents, or decompiled binaries.
- Implementation notes that closely track an unrelated launcher's source structure.

### Process

1. A behavior specification is written first.
2. Implementation is done only from the specification and public documentation.
3. Implementers do not open unrelated launcher source trees once implementation starts.
4. Reviews check for copied structure, naming, strings, comments, and file organization.
5. A provenance log records major design decisions and third-party dependencies.
6. Original branding, icons, copy, and package identity are created for this project.

## Product definition

Raggedy Launcher is a controller-first launcher that runs Minecraft Java Edition on Xbox Series S|X in Developer Mode. It signs the user in through the official Microsoft device-code flow, verifies Minecraft Java ownership, downloads official game files, manages profiles and mods, and starts the game with the correct Java runtime and loader.

The project provides:

- Official Microsoft sign-in and entitlement verification.
- Profiles with separate Minecraft version, loader, mods, worlds, and settings.
- A local version catalog.
- Downloads on Play instead of during version selection.
- Fabric, Forge, NeoForge, and vanilla targets where supported.
- Modrinth browsing and optional CurseForge support.
- Controller-first launcher navigation.
- In-game controller support as an installable component.
- Performance options and renderer management.
- Optional local remote file tools.
- Diagnostics and crash reports.
- Optional telemetry that is off by default.

Screens, wording, data layout, and implementation details are original to this project.

## Non-goals

- No auth bypass, offline accounts, cracked accounts, or fake entitlement checks.
- No bundled Minecraft game files.
- No redistribution of third-party APPX packages.
- No reuse of unrelated project package identities, publisher identities, certificates, or branding assets.
- No code from unrelated launcher projects.
- No reproduction of protected expression from any third-party UI.

## Ownership and branding

- Project name: Raggedy Launcher.
- Original author/owner: Yaro.
- Copyright notice for original work: `Copyright (c) 2026 Yaro`.
- License: MIT for original code and original assets in this repository.
- GitHub description must present Raggedy Launcher as an independent project owned by Yaro.
- The name, logo, icons, UI copy, documentation, and example configuration must be created for this project.
- Do not claim endorsement by Microsoft, Xbox, Mojang, Minecraft, or any third-party project.

### License policy

- The project is MIT licensed for original work written in this repository.
- MIT applies only to new code, docs, UI copy, icons, and design work created here.
- Third-party components keep their own licenses.
- General ideas and behavior do not become licensed material by themselves.
- If any third-party code, text, asset, script, or schema is added, it keeps its own license and required notices.
- Public contributors keep their own copyright in their contributions and grant the project MIT rights to those contributions.

## High-level architecture

The system is split into a thin UWP host, a platform abstraction layer, and independent feature modules.

```text
Raggedy.Launcher.Host
    UWP application entry point, lifecycle, package services, window handling.

Raggedy.Launcher.Platform
    Filesystem, HTTP, process launch, settings storage, cryptography, logging,
    device APIs, and sandbox-specific abstractions.

Raggedy.Launcher.Core
    Pure launcher logic: profiles, targets, launch planning, version matching,
    managed-file manifests, and state machines.

Raggedy.Launcher.Auth
    Microsoft device-code sign-in, token storage in memory, and entitlement checks.

Raggedy.Launcher.Catalog
    Local version catalog, loader metadata, Java runtime requirements, and target filtering.

Raggedy.Launcher.Storage
    Settings, profile documents, cache layout, atomic writes, backups, and migration helpers.

Raggedy.Launcher.Download
    Download queue, checksums, retries, progress reporting, cancellation, and extraction.

Raggedy.Launcher.Runtime
    Java runtime selection, classpath construction, JVM argument planning, and launch execution.

Raggedy.Launcher.Mods
    Mod source browsing, modpack import, dependency resolution, and profile mod state.

Raggedy.Launcher.Controller
    Launcher menu navigation and controller configuration.

Raggedy.Launcher.Performance
    Game options overwrite, renderer file management, backups, and conflict detection.

Raggedy.Launcher.RemoteTools
    Optional LAN file tools and browser-based management.

Raggedy.Launcher.Diagnostics
    Logs, crash collection, crash summaries, report export, and opt-in telemetry.

Raggedy.Launcher.UI
    Screens, widgets, theme system, localization hooks, and input routing.
```

## Dependency direction

- `Host` depends on `UI`, `Platform`, and `Core`.
- `UI` depends on `Core` interfaces and view models only.
- `Core` depends on abstractions, not UWP-specific APIs.
- `Platform` implements abstractions used by `Core`.
- Feature modules depend on narrow interfaces from `Core` and `Platform`.
- No feature module reads another feature module's private storage.

## Data model

All schemas, field names, document layouts, and storage paths are authored for this project.

### Settings document

- Preferred theme.
- Preferred language.
- Telemetry consent state.
- Remote tools PIN settings.
- Controller navigation tuning.
- Last selected profile ID.
- Advanced launch options.

### Profile document

- Stable profile ID.
- Display name.
- Target ID.
- Loader ID.
- Mod list and enabled state.
- Resource pack state.
- World storage location.
- Per-profile game options.
- Per-profile controller configuration.
- Per-profile renderer selection.

### Target document

- Target ID.
- Minecraft version.
- Loader ID and version.
- Java runtime requirement.
- Asset index.
- Required native libraries.
- Support status.
- Controller provider status.
- Known compatibility notes.

### Download task record

- Source URL.
- Destination path.
- Expected hash.
- Size.
- Retry count.
- State.
- Cancellation token.
- Progress callback.

### Managed file record

- Absolute path inside profile.
- Owner feature.
- Source package or project.
- SHA-256 hash.
- Install timestamp.
- Backup path.

### Crash report record

- Launch ID.
- Target ID.
- Profile ID.
- Exit code.
- Process duration.
- Relevant log excerpts.
- Redacted environment details.
- Repeat count.

## Storage layout

Use a new LocalState layout owned by this project.

```text
LocalState/
    settings/
        app-settings.json
    profiles/
        {profile-id}/
            profile.json
            mods/
            worlds/
            config/
            logs/
    catalog/
        targets.json
        compatibility.json
    cache/
        java/
        libraries/
        assets/
        loaders/
        downloads/
    diagnostics/
        logs/
        crash-reports/
    backups/
        settings/
        profiles/
    remote/
        session.json
```

Atomic write rule: write to a temporary file, flush, then replace the destination. Keep a backup before overwriting user data.

## Identity and authentication flow

1. App starts with no token in persistent storage.
2. User selects Sign In.
3. App requests a Microsoft device code.
4. App shows the code and verification URL.
5. User completes sign-in on another device.
6. App receives token and keeps it in memory only.
7. App checks Xbox Live and Minecraft Services entitlement.
8. App shows the account display name and continues only after entitlement succeeds.
9. Tokens are discarded on sign-out and app close.

Rules:

- No password collection.
- No cached refresh token on disk unless there is a documented security design and user consent.
- No entitlement bypass.
- No download of official game files before entitlement succeeds.
- No tokens in logs, crash reports, telemetry, or diagnostics exports.

## Version selection flow

1. User opens the version screen.
2. App filters targets from the local catalog.
3. User selects a profile and target.
4. App saves the selection locally.
5. App returns to the main menu.
6. No download or install happens on the version screen.
7. Downloads start only when the user presses Play.

The version screen must handle keyboard, mouse, D-pad, and left stick. Holding a direction repeats after a short delay, then moves at a steady rate. A and B act once per press. B returns.

## Launch flow

1. User presses Play.
2. App verifies sign-in and entitlement.
3. App resolves the selected target and loader.
4. App downloads missing official files, libraries, assets, and runtime files.
5. App verifies hashes where hashes are available.
6. App builds the classpath and JVM arguments.
7. App applies the selected loader bootstrap.
8. App installs or updates managed profile components.
9. App starts Java inside the UWP sandbox.
10. App monitors stdout, stderr, and exit code.
11. App writes a launch summary and opens crash details if the game exits abnormally.

The launch planner must be pure and testable. It receives resolved paths and settings and returns a plan. The platform layer executes the plan.

## Profile flow

Profiles are independent containers. Switching profiles must not mix mods, worlds, configs, or renderer files.

Profile actions:

- Create.
- Rename.
- Duplicate.
- Delete with confirmation.
- Export.
- Import.
- Backup.
- Restore.
- Undo last delete.

Worlds and configs must have a visible export path before deletion or package changes.

## Mods and modpacks

Supported mod operations:

- Browse Modrinth.
- Search by Minecraft version and loader.
- Install a mod to the selected profile.
- Enable and disable installed mods.
- Detect duplicate and conflicting mod files.
- Import `.mrpack`.
- Export a profile as a modpack.
- Show manual download requirements when a source blocks automated download.

CurseForge support is optional and uses a user-provided API key stored locally and never logged.

Mod source results are untrusted data. Names, summaries, image URLs, and file names must be sanitized before display or filesystem use.

## Controller architecture

Controller handling has two separate parts.

### Launcher navigation

- Input events are normalized into Select, Back, Up, Down, Left, Right, and Context.
- D-pad and left stick produce directional navigation.
- A confirms.
- B returns.
- Direction repeat uses independent state per direction.
- Menus call a key-release gate on entry so a held A from the previous screen does not trigger immediately.

### In-game controller support

- Controller support is a managed profile component.
- It is installed only for compatible targets.
- Profiles can choose between supported controller components.
- Turning one component off removes only its managed files.
- Controller settings are per profile.
- The launcher never changes in-game sensitivity defaults without explicit user action.

## Performance architecture

Performance work is explicit and reversible.

- Game options are written only when the user enables a performance profile.
- The app writes standard Minecraft options values such as frame cap, VSync, render distance, simulation distance, entity distance, graphics mode, fancy graphics, ambient occlusion, clouds, and GUI scale.
- Renderer files are tracked in a managed-file manifest.
- Conflicts between renderer components are shown to the user.
- Backups are created before overwrite.
- A Restore Defaults action removes only files owned by this feature.

The launcher must not present untested targets as verified. Status messages must separate metadata match from Xbox compatibility confirmation.

## Remote tools

Remote tools are optional and start only when the user starts them.

- LAN only by default.
- PIN required.
- No internet exposure.
- Rate-limited PIN attempts.
- No secrets in logs.
- Browser UI has separate sessions.
- File operations are scoped to the selected profile.
- Remote tools stop when the user closes the session or app.

Capabilities:

- Browse, upload, download, rename, and delete profile files.
- Edit text configs.
- Import and export worlds.
- Import and export modpacks.
- Download logs and crash reports.
- Add manual downloads.
- Set optional CurseForge API key.

## Diagnostics and telemetry

- Crash and usage reporting are off by default.
- Consent is explicit and revocable.
- Clearing consent removes pending reports.
- Reports contain launch events, versions, mod list, crash details, and random install ID.
- Reports do not contain worlds, full logs, tokens, passwords, account identifiers, or remote PINs.
- Local logs remain available even with telemetry off.
- Crash exports must be redacted by default.

## UI design

The UI is original.

Screens:

- Welcome and sign-in.
- Main menu.
- Profiles.
- Version selection.
- Mods.
- Settings.
- Controller.
- Performance.
- Diagnostics.
- Remote tools.
- Credits.
- Crash details.

UI rules:

- Controller-first layout.
- Large readable text.
- Visible focus.
- B returns from every subpage.
- A activates once per press.
- Downloads show progress only when they actually start.
- Selection screens do not perform installations.
- Status messages say what happened and what happens next.

## Packaging

- New package family name.
- New publisher identity.
- New certificate and signing process.
- New app icons and splash assets.
- Separate LocalState from all previous builds.
- No shared package identity with any unrelated launcher build.
- No redistribution of third-party APPX files.
- Artifacts include APPX, checksum, and build metadata.

## Build and CI

Fresh repository with fresh workflows.

Required CI checks:

- Native host build.
- Unit tests for pure logic.
- Validation of catalog schema.
- Validation of data schemas.
- Secret scan.
- Dependency license check.
- Package build.
- Checksum generation.
- Provenance checklist.

Optional CI checks:

- Localization validation.
- Static analysis.
- Linker checks.
- Smoke tests with mocked services.

No CI file, build script, or PowerShell module may be copied from the earlier tree.

## Testing plan

### Unit tests

- Target filtering.
- Version matching.
- Classpath planning.
- JVM argument planning.
- Managed file conflict detection.
- Options overwrite and restore.
- Controller repeat timing.
- Crash redaction.
- Modpack validation.
- Profile import and export.

### Integration tests with mocks

- Sign-in state machine.
- Entitlement failure paths.
- Download retry and cancellation.
- Play flow with missing files.
- Profile switching.
- Mod install and removal.
- Renderer install and conflict detection.

### Manual Xbox tests

- Install from Device Portal.
- Launch once from Xbox Home.
- Sign in with a real entitlement.
- Navigate every menu with controller only.
- Select a version and confirm no download starts.
- Press Play and confirm downloads start.
- Launch a game target.
- Open and exit every page with B.
- Verify crash report export.
- Verify telemetry is off by default.

## Migration policy

- No automatic migration from the earlier app.
- Separate package identity means separate LocalState.
- Provide manual export and import tools only.
- Do not import earlier schema files by name or shape unless those files are user data formats with documented public structure.
- Back up before import.
- Never copy user tokens or credentials between apps.

## Milestones

### Milestone 0: architecture and provenance

- Approve this document.
- Create fresh repository.
- Choose package identity and publisher.
- Create provenance log and dependency manifest.
- Write clean-room review checklist.

### Milestone 1: host shell

- UWP host.
- Empty navigation shell.
- Settings storage.
- Theme system.
- Controller navigation.
- CI package build.

### Milestone 2: identity

- Microsoft device-code sign-in.
- Entitlement check.
- Sign-out.
- Token lifetime handling.
- Security tests.

### Milestone 3: catalog and downloads

- Target catalog schema.
- Download queue.
- Checksums.
- Cache layout.
- Progress UI.

### Milestone 4: profiles and launch

- Profile documents.
- Version selection.
- Launch planner.
- Process execution.
- Logs and exit handling.

### Milestone 5: mods

- Modrinth browsing.
- Mod install flow.
- `.mrpack` import and export.
- Conflict detection.

### Milestone 6: controller and performance

- In-game controller component management.
- Options overwrite.
- Renderer management.
- Restore defaults.

### Milestone 7: remote tools and diagnostics

- LAN file tools.
- Crash reports.
- Opt-in telemetry.
- Export and redaction.

### Milestone 8: release readiness

- Manual Xbox test pass.
- License review.
- Branding review.
- Public README.
- Release checklist.

## Clean-room review checklist

Before each commit:

- No copied source files.
- No copied comments.
- No copied identifiers.
- No copied strings or UI copy.
- No copied folder layout.
- No copied JSON schema.
- No copied scripts or CI YAML.
- No copied icons or assets.
- No unrelated launcher source tree opened during implementation.
- Third-party dependency recorded with license.
- User data paths handled safely.
- No auth bypass introduced.

## Open decisions

- Final app package identity.
- Final publisher name.
- UWP UI technology choice.
- Whether to support Windows desktop outside Xbox.
- Which loaders and versions ship in the first public build.
- Whether CurseForge ships in the first release.
- Which telemetry provider, if any.
- Whether to add a contributor license agreement, or rely on the MIT license in each contribution.

## Current implementation state

- Portable catalog, profile, version, launch-plan, download, mod-plan, controller, diagnostics, and remote modules are implemented.
- Official Microsoft device-code request and polling are implemented with Xbox Live, XSTS, Minecraft sign-in, and Java Edition ownership checks.
- Downloads use SHA-256 verification and binary writes; target manifests supply every URL, destination, hash, and launch argument.
- The launcher coordinator gates manifest reads, downloads, and process launch on verified entitlement.
- The original UWP host has a device-code page, main menu, version screen, and platform services for HTTP, local state files, SHA-256, clock, and redacted logging.
- Game launch uses a configured loopback process bridge; no process escape or hidden launch path exists.
- The portable graphics layer validates window, context, resource, capability, extension, and translation plans without claiming native backend support.
- Mod source results are sanitized, filtered by game version and loader, and converted into managed selections; unsafe file names and URLs are rejected.
- `.mrpack` import and export use relative paths, HTTPS URLs, and SHA-256 hashes with optional files kept separate.
- Performance options use explicit overwrite, report foreign-owner conflicts, and restore only files owned by this feature.
- Focused tests and fresh CI cover the portable codebase.
