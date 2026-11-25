# Privacy

What Raggedy Launcher does with your data, where it goes, and how long it stays there.
This covers the launcher itself. Minecraft, Mojang's services, Modrinth, GitHub and
anything else the launcher talks to have their own policies and are not covered here.

Accurate against `main` as of 23 November 2026.

## Who runs this

Raggedy Launcher is made by RaggedyBlueBox. It is a derivative of the upstream
JavaUWP/BanditVault launcher by veroxsity / BanditVault, completely readapted into a new
controller-first launcher and not a fork (see LICENSE). There is no Raggedy telemetry server. The only place launcher
crash reports can go is a private GitHub repository owned by the same account,
`RaggedyBlueBox/RaggedyLauncher-telemetry`, and only as issues you opt into with a token
you save yourself. Nothing is sold, shared with advertisers, or passed to a third party
beyond GitHub hosting those private issues under its own policy.

## Signing in

Sign-in uses the Microsoft device code flow with the scopes `XboxLive.signin offline_access`.
You enter a code on a Microsoft page in your own browser, so the launcher never sees your
Microsoft password.

The launcher then exchanges tokens through Xbox Live, XSTS and Minecraft services, checks the
account owns Minecraft: Java Edition, and fetches the Java profile.

Held in memory for the session and passed to the Java process as launch arguments:

- your Minecraft username
- your Minecraft account UUID
- a Minecraft access token

Written to disk:

- the Microsoft refresh token, stored in the Windows Credential Locker under the resource
  `MinecraftJavaUWP.MicrosoftRefreshToken`. It is not written to LocalState and the launcher
  does not keep a plain text copy anywhere.

Because the username and UUID are passed to Minecraft as launch arguments, Minecraft's own
files and logs under your profile directory contain them. That is the Mojang client's
behaviour rather than the launcher's, but it is why those values appear if you read your own
logs.

None of this leaves the console except to Microsoft and Mojang. No account data is ever
filed to the crash inbox: reports carry only the random install ID, never your Microsoft
account, Minecraft username, UUID, gamertag, or any token.

Signing out removes the stored refresh token.

### Known issue: the application registration

The Microsoft application ID the launcher currently uses is Prism Launcher's public client ID
rather than one owned by RaggedyBlueBox. The consent screen you see during sign-in therefore
names an application that is not this one. Replacing it with a Raggedy-owned registration is
tracked work and this section will be removed when that lands.

## Telemetry

Off by default. The launcher asks once and sends nothing until you allow it.

Your answer is stored at `LocalState\telemetry\consent.txt` as `always` or `never`. Until you
answer, nothing is sent.

### Install ID

When telemetry is enabled the launcher generates a random GUID and stores it at
`LocalState\telemetry\install_id.txt`. It is not derived from your Microsoft account, your
gamertag, your console or any hardware identifier. It exists so that fourteen reports of the
same crash can be counted as fourteen installs rather than fourteen unknowns.

You can reset it from the launcher. Resetting generates a new GUID and clears anything queued
for sending. Rows already on the server keep the old ID, which no longer maps to your install.

### What is sent

Crash reports, filed as private GitHub issues when a launch fails or the game crashes:

- install ID (a random GUID, not your account)
- a crash fingerprint, which is a sixteen character hash
- launcher build, Minecraft version, loader, loader version, mod set hash
- which phase the crash happened in
- maximum heap size and heap in use at the crash
- the Java exception class and its message
- up to sixteen stack frames
- for mixin failures, the target class, method, descriptor, owning mod and symbol

Session beacons, mod set uploads, and the compatibility feed below speak a different
protocol that needs a custom `endpoint.txt`; with no endpoint configured they stay
dormant and nothing is queued for them. Only crash issues use the GitHub path.

The crash screen shows you the exact payload before it is sent.

### What is never sent

- Crash report files, `hs_err_pid*.log`, `latest.log` or any other log file. Crash parsing
  happens on the console, so the server receives the result and never the input.
- Worlds, saves, screenshots, configs or any other file from your game directory.
- Your Microsoft account, Minecraft username, UUID, gamertag, or any token.
- Any of the above, while telemetry is off. The launcher still downloads the compatibility
  feed when telemetry is off, which is covered in its own section below, but it sends nothing.

### If a send fails

GitHub issues are filed immediately; if the network is down at that moment the report is
lost and a log line says so. The crash screen's send-once option can re-file the last
crash later. (The `LocalState\telemetry\queue` retry store only applies to the dormant
custom-endpoint protocol, is capped at twenty files, and is cleared by resetting the
install ID.)

### Where it goes and how long it stays

Crash issues go to the private repository `RaggedyBlueBox/RaggedyLauncher-telemetry`,
visible only to the repo owner, plus nothing else anywhere: no session events, no mod set
uploads, no third-party analytics. You can still point the dormant custom protocol
somewhere else with `endpoint.txt` in the telemetry folder or the
`RAGGEDY_TELEMETRY_ENDPOINT` environment variable, but nothing in this build does that.

Issues stay in the private repo until the repo owner deletes them. Your GitHub token
stays in `LocalState\telemetry\github-token.txt` on the console until you clear it in
Remote Files; only its last four characters are ever shown back to you, and it is sent
solely to `api.github.com` over HTTPS.

To remove your data: delete the issue in the private repo, clear the token, reset the
install ID, and delete `LocalState\telemetry` to clear consent, install ID and queue
together.

## The compatibility feed

The compatibility feed is dormant in this build: with no custom endpoint configured the
launcher makes no daily request anywhere. A previously cached feed at
`LocalState\telemetry\compat.json` is still honoured if one exists.

## Remote Files and the mouse relay

Both open HTTP servers on your local network. They are built for a home network you trust.

Remote Files listens on port 27632 on every network interface and serves the launcher's own
data under LocalState: profiles, worlds, mods, logs and crash reports. Access is gated by a
PIN, and that PIN appears in the URL. It is plain HTTP with no TLS, so anyone on the same
network who has the URL can read and write that data, and anyone able to watch traffic on that
network can read it in transit. It stays off unless you start it.

The mouse relay page listens on port 6090 on every network interface with no authentication,
and forwards pointer input to the launcher. It carries cursor movement and button presses, not
files and not account data. Anyone on your network can open it and move your cursor.

The companion relay apps for Windows, Android and iOS send pointer data over UDP on the local
network. They do not touch your account.

Do not expose either port to the internet.

## What the launcher downloads

- Minecraft client files, libraries and assets from Mojang's official endpoints, after
  ownership is verified. No game files are bundled in the package.
- Fabric, Forge and NeoForge loaders and their libraries, from their own distribution points.
- Mods and modpacks from Modrinth when you browse or install them. Those requests go to
  Modrinth and fall under Modrinth's own privacy policy.
- The newest version number and the changelog, from GitHub, when the launcher opens. These are
  plain requests for public pages. They carry no install ID, account details or anything else
  from your console, though GitHub can see the console's IP address like any other website.
  The answer is kept in `LocalState\updates` and asked for again after about six hours.

## Your choices

- Answer Never at the telemetry prompt, or change it later in the launcher. Declining stops
  all reporting.
- Reset your install ID at any time to break the link between your console and rows already
  sent.
- Sign out to remove the stored refresh token.
- Leave Remote Files and the mouse relay stopped.
- Turn off Check for new versions in Settings to stop the launcher contacting GitHub on start.
- Delete `LocalState\telemetry` to clear consent, install ID, queue and cached feed together.
- Delete the corresponding issue in the private telemetry repo to remove a filed crash.

## Contact

There is no support mailbox. For privacy questions, manage the data yourself: delete the
issue in the private telemetry repo, clear the token in Remote Files, reset the install ID
in Settings, or delete `LocalState\telemetry` outright. Upstream license and attribution
questions fall under [LICENSE](LICENSE).

## Changes

This document is versioned in the repository. Material changes are noted in the release notes
for the build that carries them.
