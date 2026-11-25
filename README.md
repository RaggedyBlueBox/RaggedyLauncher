# Raggedy Launcher

![Test build](https://github.com/RaggedyBlueBox/RaggedyLauncher/actions/workflows/test-build.yml/badge.svg)
![Xbox Series X|S](https://img.shields.io/badge/platform-Xbox%20Series%20X%7CS-107c10)
![Minecraft Java Edition](https://img.shields.io/badge/Minecraft-Java%20Edition-62b47a)
![vibe-coded](https://img.shields.io/badge/vibe--coded-100%25-7c3aed)

Raggedy Launcher by **RaggedyBlueBox** — a controller-first Minecraft Java
launcher for Xbox Series X|S (Dev Mode) and Windows.

> **FULLY VIBE CODED.** This entire project is designed and built with AI
> assistance — no hand-written code. On-console testing happens on a real
> Series S devkit.

Raggedy Launcher is a derivative project: its code was taken from the upstream
JavaUWP/BanditVault launcher ([veroxsity/JavaUWP](https://github.com/veroxsity/JavaUWP))
and completely readapted into a new controller-first launcher. It is branded
only as Raggedy Launcher, not as a fork, mod, reskin, or separate install of
another launcher. The goal is a lightweight everyday launcher with clear
settings, sane defaults, honest messages when something is not available on a
version, and no silent surprises with your mods, worlds, or login.

> Experimental and under active development. Test builds come from the
> `Test build (no publish)` workflow; nothing here auto-updates any console.
> Back up your worlds, saves, and profiles before changing builds or removing
> the app.

What is in this build: a **Version** page in place of Mods, a version list
limited to the Minecraft versions supported by Simply Legacy and Re-Console,
selection-only version changes, and persistent built-in profile selection.
Downloads start when you press **Play**. A performance overwrite applies
lightweight in-game options and the matched renderer, the launcher UI is capped
at 30 FPS, and menus use a single-image backdrop. See
[docs/GRAPHICS-AND-PERFORMANCE.md](docs/GRAPHICS-AND-PERFORMANCE.md)
and [CHANGELOG.md](CHANGELOG.md).

---

![Pre-release](https://img.shields.io/badge/status-pre%20release-2ea44f)
![Xbox Series X|S](https://img.shields.io/badge/platform-Xbox%20Series%20X%7CS-107c10)
![Minecraft Java Edition](https://img.shields.io/badge/Minecraft-Java%20Edition-62b47a)

Raggedy Launcher runs **Minecraft Java Edition directly on Xbox Series X|S in Developer Mode**, with Fabric. It signs in through Microsoft, verifies Java Edition ownership and downloads the official game files.

> **Xbox One, Xbox One S and Xbox One X are not supported.** The launcher is for Xbox Series S and Xbox Series X only.

[Download](#download) · [Getting started](#getting-started) · [Minecraft versions](#minecraft-versions)

## Requirements

- **Xbox Series S or Xbox Series X** with Developer Mode enabled.
- A Microsoft account with a valid Minecraft Java Edition entitlement.
- An internet connection for sign-in and downloads.
- Free console storage for game files, profiles, mods and worlds.

Java is included in the launcher package for supported targets. No separate Java installation is needed on the console. Raggedy Launcher requires Microsoft authentication and ownership verification. It does not support cracked or offline accounts.

## Download

| Build | Download | Mouse input |
| --- | --- | --- |
| Test | [Test build workflow artifacts](https://github.com/RaggedyBlueBox/RaggedyLauncher/actions/workflows/test-build.yml) | Direct USB mouse, or optional mouse relay from a phone or PC on the same network. |

Download the `.appx` from a successful test-build run and check it against the published `.sha256`. Sideload it through Xbox Device Portal while the console is in Developer Mode, trusting the certificate that ships with the build.

These are pre-release testing builds. They are experimental and do not come with support.

APPX packages keep their own local data. Export important worlds through Remote Files before changing builds or removing an app.

## Getting started

1. Install the signed APPX through Xbox Device Portal while the console is in Developer Mode.
2. Open **Raggedy Launcher**.
3. When the Microsoft device code appears, open [microsoft.com/link](https://www.microsoft.com/link) on another device. Enter the code and sign in with the account that has Java Edition.
4. Wait for the launcher to verify the account and Minecraft entitlement.
5. Open **Version** on the main menu. Simply Legacy is the initial built-in profile; use left/right to switch to Re-Console if you want it. Up/down picks a supported Minecraft version, and **A** selects it and returns to the menu. Your selection is kept for next time.
6. Press **Play**. The launcher downloads the game, libraries, assets, loader files and newest compatible modpack for that target. Allow time for this to finish.

The **Version** page only saves your choice. Use **Remote Files** to upload worlds, edit configs, import modpacks or download logs from another device.

## Features

- Built-in Simply Legacy and Re-Console profiles whose active selection persists across restarts.
- A Version page filtered to the Minecraft versions each built-in modpack supports, with horizontal modpack tabs and a separate version list.
- Dynamic stick navigation: holding a direction repeats after a short delay, with keyboard, D-pad and left stick handled the same way.
- Newest compatible modpack pulled when you press **Play**, not while browsing versions.
- Custom profiles for separate mod setups on supported catalog targets.
- Remote Files for managing profiles, worlds and modpack imports through a browser on the local network.
- Profile backups and restore options, including undoing the last deleted profile.
- Download repair, current and previous run logs, and crash report ZIPs.
- Optional crash and usage reporting, and a Credits page, in launcher Settings.
- Lightweight always-on defaults: 30 FPS launcher UI, single-image menu backdrop, performance options overwrite and an always-on renderer.

## Minecraft versions

Only Fabric 0.20.1 versions supported by the two built-in modpacks are cataloged. The Version page shows the rows for the profile you select.

| Built-in profile | Minecraft versions |
| --- | --- |
| Simply Legacy | `26.3`, `26.1.2`, `1.21.10` |
| Re-Console | `26.3`, `26.2`, `26.1.2`, `1.21.10`, `1.21.8`, `1.21.5`, `1.21.4`, `1.21.3`, `1.21.1`, `1.21`, `1.20.6`, `1.20.4`, `1.20.1` |

`26.x` rows use the packaged Java 25 runtime and the Mesa/D3D12 OpenGL bridge; newer 1.21 rows use Java 21, and `1.20.4`/`1.20.1` use Java 17 with their existing OpenGL path. Experimental targets still need testing, and available targets can differ between builds. Check the installed launcher's Version page and the release notes.

## Mods, modpacks and other content

Choose the built-in profile and launch target on the **Version** page before installing content. Pressing **Play** replaces that profile's managed mods with the newest compatible modpack. Mods must match both the Minecraft version and loader, and must work inside the Xbox UWP sandbox.

The launcher applies lightweight performance defaults: matched renderers are installed in overwrite mode and a small set of in-game video options is overwritten for predictable frame times. Metadata matches without on-console testing are labelled as overwrite candidates. Custom profiles are still available for your own mod setups. Remote Files accepts Modrinth `.mrpack` and CurseForge `.zip` modpacks.

**CurseForge needs a personal API key.** Start Remote Files and add it under **Settings**, then select CurseForge in the launcher. Modrinth does not need a key.

Some CurseForge authors block downloads by third-party launchers. If a pack needs those files, Remote Files shows a **Manual downloads** panel with links and an upload option. Finish those downloads before trying to play the pack.

Shader packs need a compatible shader loader such as Iris or Oculus in the same profile. Installing a shader pack does not guarantee it will render correctly or run well on Xbox.

## Controller and mouse

Simply Legacy is seeded as the initial built-in profile on first run and gets its controller support from the newest compatible modpack, so play is ready without a keyboard. After that, your selection persists: Re-Console stays active if you choose it. The launcher no longer offers a controller bundle or look sensitivity setting; controller sensitivity stays at the mod and game defaults and can be changed in the in-game controller settings where available.

Raggedy Launcher accepts a USB mouse directly and also offers an optional browser mouse relay. A relay page is available from the launcher, with companion apps for Windows, Android and iOS. The launcher UI is capped at 30 FPS and uses one backdrop image at a time; in-game framerate is not capped by the launcher itself, but the performance overwrite writes a 60 FPS in-game cap. Settings has no menu or game mouse sensitivity rows.

## Remote Files

Start **Remote Files** in the launcher, then open the displayed URL and PIN on a phone or computer on the same network. Pick the profile to manage before uploading or changing files.

Remote Files can:

- Browse, upload, download, rename and delete profile files and create folders.
- Edit text files such as configs in the browser.
- Import and export world saves as `.zip` files.
- Upload mods, resource packs and world datapacks.
- Import Modrinth `.mrpack` and CurseForge `.zip` modpacks.
- Export a mod profile as a Modrinth-compatible `.mrpack` for use on PC.
- Download logs and crash report ZIPs.
- Set the CurseForge API key and complete manual pack downloads.

Remote Files uses plain HTTP and a PIN on the local network. Keep the URL private, use a trusted network and stop the server when finished. Do not expose it to the internet. See [Privacy](docs/PRIVACY.md) for details.

## Saves and updates

Each profile has its own worlds, mods, resource packs, configs and logs. Downloaded game libraries and assets are shared between profiles within the same installed app.

Updating the same app package keeps its normal profile data and your selected built-in profile. **Back up worlds before uninstalling or moving to another build.** Removing the app can remove its local data, and APPX packages do not share storage.

Use Remote Files to export worlds. The Profiles page also has profile backup and restore options.

## Known limitations and troubleshooting

- **Xbox One consoles are not supported.**
- The `26.x` Fabric targets are experimental. Mod and shader compatibility varies by version and build.
- Large modpacks can exceed the console app's available memory. A successful download does not mean the pack will run.
- First launch and large pack imports can take several minutes.
- Some desktop mods depend on native libraries or platform features that the Xbox sandbox does not provide.

If sign-in fails, check that the Microsoft account has Java Edition and a Minecraft Java profile. If required game files are missing, use the launcher's download repair option.

For a game or mod crash, download the crash report ZIP and the profile's `latest.log` through Remote Files. Include the launcher build, Minecraft version, loader and modpack or mod names when describing the problem. Remove private information before sharing files.

Crash and usage reporting is off until enabled. It can be changed in launcher Settings and is not required to use the launcher. See [Privacy](docs/PRIVACY.md) for what is sent and how it is stored.

## Project docs

- [Building](docs/BUILDING.md)
- [Compatible mods](docs/COMPATIBLE-MODS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Patching notes](docs/PATCHING.md)
- [Privacy](docs/PRIVACY.md)
- [Contributing](CONTRIBUTING.md)
- [Translating](docs/TRANSLATING.md)
- [Third-party components](docs/THIRD-PARTY.md)

## License and creator content

The code in Raggedy Launcher was taken from the upstream JavaUWP/BanditVault launcher by veroxsity / BanditVault and completely readapted into this controller-first launcher. Raggedy Launcher is a derivative, not a fork. Original project code uses the custom terms in [LICENSE](LICENSE). Private use and private modifications are allowed under those terms. Public forks, mirrors, source redistribution and packaged builds require prior written permission from veroxsity / BanditVault.

Videos, streams, screenshots, reviews, benchmarks and tutorials are allowed when they credit veroxsity / BanditVault and visibly link to the official project or another official BanditVault page. They must follow the license's redistribution, authentication and endorsement rules.

Redistributing APPX packages, including nightly builds, requires prior written permission. Removing or bypassing Microsoft/Xbox authentication or Minecraft ownership checks is not permitted.

Raggedy Launcher does not bundle Minecraft game files. Minecraft and all third-party components remain under their own licenses and terms. See [Legal notes](docs/LEGAL.md) for more detail.
