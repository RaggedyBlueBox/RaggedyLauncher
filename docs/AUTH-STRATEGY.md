# Authentication Strategy

The launcher uses the official Microsoft device-code flow.

## Screen flow

1. Welcome screen explains sign-in.
2. Request device code.
3. Show code and verification URL.
4. User signs in on another device.
5. App receives token and verifies entitlement.
6. App enters the main menu.

## Rules

- No password collection.
- No copied login window.
- No copied icons, strings, or layout.
- No fake token, fake UUID, fake XUID, or offline session.
- Tokens stay in memory unless a documented secure storage design is added.
- Tokens never enter logs, crash reports, telemetry, or diagnostics exports.
- Game downloads and launch do not proceed before entitlement succeeds.

## Failure states

- Device code expired: offer a new code.
- Network failure: explain the retry.
- Entitlement missing: explain that Minecraft Java Edition is required.
- Account cancelled: return to welcome.
