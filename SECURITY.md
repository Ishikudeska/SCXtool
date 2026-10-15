# Security Policy

## Supported versions

Security fixes go into the latest release only. If you're on an older version, update from the [Releases](../../releases) page before reporting.

## Reporting a vulnerability

**Please don't report security problems in public issues, pull requests or discussions.**

Report them privately through GitHub instead:

1. Open this repository's **Security** tab.
2. Click **Report a vulnerability**.
3. Fill in the form.

Please include:

- The SCXtool version and your OS.
- What an attacker could do, and what they would need first (for example, control of a download server, or a crafted file the user opens).
- Steps to reproduce it, or a proof of concept.
- Any relevant part of the app log, from the Log tab.

### What happens next

- You should get an acknowledgement within 7 days.
- SCXtool is maintained by volunteers, so fix times vary. We'll keep you updated on the private report as the fix progresses.
- Once a fix is released, we'll publish an advisory. You'll be credited unless you'd rather stay anonymous.

Please give us a reasonable chance to release a fix before you disclose the problem publicly.

## Scope

In scope: problems in this repository's code, and in the installer and portable zip published on its Releases page. Examples:

- Writing files outside the game's localization folder, the app's data folder or the paths the user chose.
- Running code, or overwriting files, through a crafted `Data.p4k`, INI, JSON, settings export or blueprint import.
- Getting the in-app updater to install something that isn't a genuine release.
- Leaking files or personal information off the user's PC.

Out of scope:

- Bugs in Star Citizen itself, or in CIG's services. Report those to Cloud Imperium Games.
- Bugs in the community translation files that the app downloads. Report those to the project that maintains the file (see the acknowledgements in the README).
- Windows SmartScreen warnings. The app isn't code-signed yet, and the README says so.
- Builds downloaded from anywhere other than this repository's Releases page.

## How the app touches your system

This section is here so reporters and users know what the app is expected to do. Anything it does beyond this is worth reporting.

**Files it writes:**

- The game's `global.ini` for the selected channel and language, and the `g_language` line in that channel's `user.cfg`. It takes a backup of the existing `global.ini` before every write.
- Its own settings (`%APPDATA%\SCXtool\` for the installer, next to the executable for the portable build).
- Its own data, such as edits, generated files and backups (`Documents\SCXtool\` for the installer, next to the executable for the portable build).
- Files you explicitly export or extract, written where you choose.

It reads `Data.p4k` and the game's logs, and never modifies them. It doesn't touch game logic or connect to CIG's servers.

**Network connections:**

| When | Where | What |
|---|---|---|
| Every launch, and when you click **Check for Updates** | `api.github.com` | Reads the latest release's version number. Nothing about you or your PC is sent except a `SCXtool/<version>` user agent. |
| When you choose to install an update | GitHub release downloads | Downloads the installer. It's verified against the SHA-256 digest GitHub publishes for the release, and discarded if the size or the hash doesn't match. |
| When you pick a non-English language | The URL for that language in `resources/languages/sources.json`, or your own override | Downloads that language's `global.ini`. It only re-downloads when the file has changed. |
| When you import an INI from a web address | The address you entered | Downloads that file. |

There is no telemetry, analytics or crash reporting.

Downloaded INI files are text that gets shown in the game. They're never executed. Treat a translation file from an untrusted address the same way you would any other file from an untrusted source.
