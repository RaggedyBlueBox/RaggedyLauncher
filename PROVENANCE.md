# Provenance

Original author: Yaro.

Original work in this repository is licensed under MIT. The copyright notice in `LICENSE` applies to original code, docs, UI copy, icons, and design work created here.

## Clean-room rule

Implementation must come from the written requirements, public official documentation, and original design work. Do not open or copy from unrelated launcher source trees, build outputs, binaries, package contents, caches, screenshots of source, or git history.

Ideas and general behavior are allowed. Copied expression is not.

## Before each commit

- No copied source files.
- No copied comments.
- No copied identifiers.
- No copied UI strings.
- No copied folder layout.
- No copied JSON schemas.
- No copied scripts or CI workflows.
- No copied icons, art, or fonts.
- No copied manifests or package identities.
- Third-party dependencies recorded in `DEPENDENCIES.md`.
- Auth and entitlement checks untouched or strengthened.

## Log

- 2026-11-25: repository initialized with MIT license, architecture, requirements, design docs, and portable core foundation.
- 2026-11-25: added independent version, profile, catalog, device-code, UI view, QR, and graphics translation modules.
- 2026-11-25: added original UWP host skeleton, TV-friendly sign-in page, generated app assets, and portable-core CI.
- 2026-11-25: completed the portable graphics layer with window, context, resource, capability, extension, and translation validation.
- 2026-11-25: added mod source sanitization, `.mrpack` validation, and reversible performance options.
- 2026-11-25: set personal Microsoft accounts to the `consumers` tenant and documented app registration setup.
