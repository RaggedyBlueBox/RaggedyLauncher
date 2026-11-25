# Repository Instructions

Raggedy Launcher is an independent project owned by Yaro. All project code, docs, UI, icons, schemas, and build scripts must be authored for this repository.

Use only official public documentation, written requirements, original design work, and standard engineering knowledge when implementing features.

Do not open or use source trees, binaries, packages, manifests, schemas, icons, scripts, strings, or UI files from unrelated launcher projects.

Authentication and entitlement checks must remain intact:

- Official Microsoft device-code sign-in.
- Real Minecraft Java entitlement verification.
- No offline, cracked, forged, or entitlement-free launch paths.
- No fake tokens, UUIDs, XUIDs, or account profiles.
- No hidden switches that weaken account or ownership checks.
- No tokens, refresh tokens, account IDs, passwords, or secrets in logs, diagnostics, or telemetry.

Keep user-facing status explicit. Selection pages do not download or install anything. Downloads and installs happen only when the user starts Play.

Remote tools are LAN-only, PIN-protected, session-scoped, and stopped when the app closes.

Diagnostics and telemetry are off by default and require clear, revocable consent. Exports must be redacted.

Third-party dependencies keep their own licenses and must be recorded in `DEPENDENCIES.md` before vendoring.
