# Authentication Strategy

The launcher uses the official Microsoft device-code flow.

## Application setup

- Use your own Microsoft application registration.
- The client ID must be approved for Minecraft services; unapproved IDs receive an explicit error from Minecraft services.
- Store the public client ID in `state/settings.json` as `microsoft_client_id`.
- Never store access tokens, refresh tokens, XUIDs, or account identifiers in settings, logs, telemetry, or diagnostics exports.

## Screen flow

1. Welcome screen explains sign-in.
2. Request a device code from the Microsoft identity platform.
3. Show the code and verification URL.
4. The user signs in on another device.
5. Poll the Microsoft token endpoint until success, denial, expiry, or error.
6. Exchange the Microsoft token through Xbox Live and XSTS.
7. Sign in to Minecraft services and verify Java Edition ownership.
8. Load the real Minecraft profile name and ID.
9. Enter the main menu only after entitlement succeeds.

## Rules

- No password collection.
- No copied login window.
- No copied icons, strings, or layout.
- No fake token, fake UUID, fake XUID, or offline session.
- Tokens stay in memory and are cleared on sign-out, cancel, suspend, or failure.
- Game downloads and target manifests are not touched before entitlement succeeds.

## Failure states

- Device code expired: offer a new code.
- Network failure: explain the retry.
- Sign-in declined: return to welcome.
- Xbox account setup required: explain that an Xbox account must be ready.
- Minecraft app ID rejected: explain that the client ID needs Minecraft approval.
- Entitlement missing: explain that Minecraft Java Edition is required.
