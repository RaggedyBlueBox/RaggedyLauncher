# FAQ
This doc answers Frequently Asked Questions in case you happen to be stuck.

Raggedy Launcher is a controller-first derivative of the upstream JavaUWP/BanditVault launcher by veroxsity / BanditVault, completely readapted into a new launcher. See [README.md](../README.md) and [LEGAL.md](LEGAL.md) for the required credits.

## Will this hack my XBOX?
Raggedy Launcher does not provide any way to hack the XBOX. All code runs natively on the console.

## What data does the launcher collect?
Signing in gives the launcher your Minecraft username, account UUID and an access token, which it needs to start the game. Those stay on the console. The only thing kept between sessions is your Microsoft refresh token, stored in the Windows Credential Locker so you don't have to sign in every time.

Crash and usage reporting is off until you switch it on. When it is on it sends version numbers, a mod set hash, a crash fingerprint and a stack trace, never your account details and never your log files. The full breakdown, including what the Remote Files and mouse relay servers expose on your network, is in [PRIVACY.md](PRIVACY.md).

## How do I turn crash reporting off?
Answer Never when the launcher asks, or change it later in the launcher. You can also reset your install ID from there, which generates a new random one and clears anything still waiting to send.

## Do I need a Java account?
Raggedy Launcher requires a valid copy of Minecraft: Java Edition. The launcher does not condone piracy or redistribution of the game, but rather only makes it playable to legal users.

## My Minecraft account won't sign in!
If you have Minecraft through Game Pass, you need to create an account on minecraft.net and set a player name.

## How do I use my mouse?
Raggedy Launcher accepts a direct USB mouse. You can also start the optional browser mouse relay, with Windows, Android and iOS companions. Check the main README for the relay setup.

## Is the mouse relay laggy?
The mouse relay does not lag unless you have a bad computer or *really* bad wifi. In other words, if it does lag, look into better wifi or a better device to use the relay.

## How does multiplayer work?
You don't need a Game Pass subscription or anything else to play multiplayer. You can hop on any server you wish (ideally with mouse relay and a keyboard).

## Where do I choose a version?
Open **Version** from the main menu; it replaces the old Mods entry. Left/right switches between Simply Legacy and Re-Console, up/down picks a version that modpack supports, and **A** selects it and returns. **B** returns without changing anything. The page never downloads; pressing **Play** downloads the newest compatible modpack for the selected target.

## Why does the launcher open on Simply Legacy?
Simply Legacy is seeded as the initial built-in profile on first run only. After that, your built-in profile selection is kept across restarts, so Re-Console stays active once you choose it.

## Can I change right-stick sensitivity?
The launcher no longer forces a look-speed value. Controller sensitivity stays at the mod and game defaults; change it in the in-game controller settings where available, or use a custom profile if you need your own controller setup.

## Can I turn the performance renderer off?
No. It is always on in lightweight `auto` mode and Settings reports **Always on**. Matched candidates are installed in overwrite mode, including metadata-only matches that are labelled untested; unsupported targets are reported instead of getting fake entries.

## How can I change the RAM allocation?
Unfortunately, you can't allocate more than 3-4 GB of RAM, as this is the max achievable by an XBOX in Developer Mode.

## How does stick navigation work?
Holding a direction on the D-pad or left stick repeats after a short delay, then keeps moving at a steady rate. A and B still act once per press.

## Why does the launcher menu look so still?
The launcher UI is capped at 30 FPS and shows a single backdrop image at a time with no cross-fade. That keeps menu drawing light on console; the launcher itself does not cap Minecraft's framerate, but the performance overwrite writes a 60 FPS in-game cap.

## How do I upload worlds?
Open Raggedy Launcher, start Remote Files, and go to that link and upload your world to the portal.

Remote Files is plain HTTP on your local network, and the PIN sits in the URL, so treat it as open to anyone on the same wifi. Stop it when you're done, and never forward that port through your router.

## How do I check crash reports?
Open Raggedy Launcher, start Remote Files, and go to the crash reports directory.

## Is this AI-assisted?
Yes. Raggedy Launcher is built with AI assistance. The agent files exist to keep Microsoft/Xbox authentication and Minecraft entitlement checks intact; nothing here supports offline or fake accounts.

## Other launchers?
Use other launchers at your own risk. Raggedy Launcher requires legitimate Microsoft sign-in and Minecraft Java Edition ownership, and any alternative should be judged on the same basis.