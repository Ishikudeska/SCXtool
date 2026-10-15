# Contributing to SCXtool

Thanks for your interest in helping. Bug reports, fixes, new enhancements and translations are all welcome. This page covers how to get set up, what a change needs before it can be merged, and where things live in the code.

## Reporting bugs and requesting features

Open an issue using the **Bug report** or **Feature request** template.

For bugs, please include:

- Your OS, the SCXtool version, and the game channel (LIVE, PTU, EPTU, HOTFIX or TECH-PREVIEW).
- Steps to reproduce the problem.
- The app log. Open the **Log** tab, filter it if you like, and export it.

If a string in the game looks wrong, include its localization key (shown in the String Editor) and what you expected it to say.

## Setting up a build

You need:

- Qt 6.8 or newer (Core, Gui, Network, Concurrent, Qml, Quick, QuickControls2, Test, QuickTest)
- CMake 3.25 or newer and Ninja
- A C++23 compiler

CMake downloads and builds zstd, zlib and pugixml itself.

The presets in `CMakePresets.json` expect Qt 6.12 with MinGW 13.1 under `C:\Qt`, which is also what CI uses:

```sh
cmake --preset mingw-debug
cmake --build --preset mingw-debug
ctest --preset mingw-debug -LE parity
```

On other platforms, configure with CMake and point `CMAKE_PREFIX_PATH` at your Qt install. To speed up builds, put LLVM's `ld.lld` on your PATH and Debug builds link with it automatically. You can also use ccache by setting `CMAKE_CXX_COMPILER_LAUNCHER=ccache` and `CMAKE_C_COMPILER_LAUNCHER=ccache` in your environment before configuring.

## Before you open a pull request

CI runs these checks on every push. Running them locally first saves a round trip.

1. **Formatting.** C++ is formatted with the repo's `.clang-format`, using **clang-format 22.1.8** (the version that ships with Qt Creator). Other versions lay some code out differently and will fail the check.

   ```sh
   pipx run --spec clang-format==22.1.8 clang-format -i <files>
   ```

2. **No warnings.** The code builds warning-free in both Debug and Release, and CI treats warnings as errors:

   ```sh
   cmake --preset mingw-debug -DSC_WARNINGS_AS_ERRORS=ON
   cmake --build --preset mingw-debug
   ```

   Release (`mingw-release`) is checked too, because `-O2` turns on warnings that a Debug build doesn't report.

3. **QML lint.**

   ```sh
   cmake --build --preset mingw-debug --target all_qmllint
   ```

4. **Tests.**

   ```sh
   ctest --preset mingw-debug -LE parity
   ```

5. **UI text keys**, if you touched any UI text (see [Translations](#translations)):

   ```sh
   python tools/i18n/check_keys.py
   ```

## Pull requests

- Branch from `main` and open your pull request against `main`.
- Keep each pull request to one change. A new feature and an unrelated cleanup should be two pull requests.
- Describe what changed and why. For UI changes, add a screenshot.
- Commit messages are a short, capitalized summary in the imperative, for example `Show reputation and blueprint rewards on missions`. Add a body when the reason for the change isn't obvious from the summary.
- Don't mix reformatting with real changes. If a commit only reformats code, add its hash to `.git-blame-ignore-revs`. To have local `git blame` skip those commits, run this once:

  ```sh
  git config blame.ignoreRevsFile .git-blame-ignore-revs
  ```

## Where things live

| Path | What it holds |
|---|---|
| `src/engine/` | Reading `Data.p4k`, CryXML and DataForge (a port of unp4k / unforge). |
| `src/core/` | Everything that isn't UI: enhancement generation, merging, applying to the game, blueprints, missions, loadouts, settings. |
| `src/app/` | Qt controllers and models, plus the QML interface in `src/app/qml/`. |
| `tests/engine`, `tests/core`, `tests/app` | Qt Test unit tests for each layer. |
| `tests/parity` | Checks that generated output matches the original Python Smart Citizen. |
| `resources/languages/` | UI translations, and `sources.json` for the community `global.ini` downloads. |
| `resources/patches/` | Workarounds for known bugs in CIG's game data. |
| `tools/` | Command-line tools for development, plus the i18n and parity helper scripts. |

The [wiki](../../wiki) explains how the app works and has notes for each source folder.

## Writing code

- Keep `engine` free of app and UI code, and keep `core` free of UI code. Logic in `core` should be testable without the interface.
- Follow the style of the code around you. clang-format takes care of layout.
- Write comments to explain *why* the code does something, not what it does.
- Add or update tests for what you change. Tests use Qt Test and are registered in the folder's `CMakeLists.txt` with `sc_add_test`. Small sample inputs go in `tests/fixtures/`.

### Keeping output identical to Smart Citizen

The generated text has to match the original Python Smart Citizen byte for byte. If your change touches enhancement generation, tags or merging, run the parity tests. They need the Python app and a `Data.p4k`; setup is described in `tests/CMakeLists.txt` and `tests/parity`:

```sh
ctest --preset mingw-debug -L parity
```

If your change intentionally makes the output different from Smart Citizen, say so in the pull request description.

### Game data patches

When CIG's data has a bug that breaks generated text, such as a contract that points at the wrong description, fix it with a patch file under `resources/patches/`. Don't special-case it in C++. Patches are reapplied on every regeneration and can be deleted once CIG fixes the data.

## Translations

UI text lives in `resources/languages/<language>/ui.json`. English (`resources/languages/english/ui.json`) is the source.

- **Adding UI text in code:** use a dotted key (`qsTr("section.key")` in QML, `tr("section.key")` in C++). Add the English text with `python tools/i18n/add_keys.py strings.json`, then run `python tools/i18n/check_keys.py` to check that no keys are missing.
- **Translating:** every entry has an `ht` field (human translation) and an `at` field (machine fallback). Fill in `ht`. To find strings that still need a human translator, search the language file for `"ht": ""`. Leave `at` as it is.

[`resources/languages/TRANSLATIONS.md`](resources/languages/TRANSLATIONS.md) has the full details.

## License

SCXtool is licensed under the Apache License 2.0. By submitting a contribution, you agree that it is licensed under the same terms, as described in section 5 of the [LICENSE](LICENSE).
