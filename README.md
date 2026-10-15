# Smart Citizen++

A complete rewrite of Smart Citizen in C++.

Smart Citizen++ is a localization toolkit for Star Citizen. It changes the text the game shows you, so ship, item and mission information you would normally look up on a wiki shows up in the game itself. It reads your installed `Data.p4k` directly, generates that text from the game's own data, merges it with any edits you make, and writes the result to the game with a backup first.

It is a native C++ and Qt application. There is no Python runtime to install and no helper programs to run.

# ON AI USAGE:
 NOTICE, this repository was ***ORIGINALLY*** a fork of https://github.com/Osiris-DevWorks/smart-citizen. I have, after consideration, taken it OFF of that fork network. I use AI, to help me write commit messages. I'm sorry, but writing is NOT my goddamn strong suit and I need help with it.

## What it does

- **Generates enhanced game text** from the game's DataForge records: name tags and stat blocks on ships, components, weapons and missiles; mission details, rewards and hauling routes on contracts; mining and crafting information on commodities.
- **Lets you edit any string in the game** in a searchable table with a live preview of how it will look in-game.
- **Tracks the blueprints you own** by scanning your game logs, and marks them in mission blueprint lists.
- **Browses and extracts `Data.p4k`**, including DataForge records as XML and a `game_data.json` export.
- **Applies everything safely**: a backup before every write, a check of the written file, and automatic rollback if something goes wrong.

## Documentation

How each feature works is documented on the [wiki](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki):

- [How it works](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/How-It-Works): extract, generate, merge and apply.
- [Enhancements](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/Enhancements): name tags, stat blocks, mission details, hauling routes, mining and crafting.
- [String Editor](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/String-Editor)
- [Blueprint Tracker](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/Blueprint-Tracker)
- [P4K Explorer](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/P4K-Explorer)
- [Languages](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/Languages)
- [Simple and Advanced mode](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/Simple-and-Advanced-Mode)
- [Safety and undo](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/Safety-and-Undo)
- [Other features](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/wiki/Other-Features)

## Download

Windows builds are published on the [Releases](https://github.com/Ishikudeska/Smart-Citizen-plus-plus/releases) page in two forms:

- `SmartCitizenPlusPlus-<version>-Setup.exe`: an installer. Settings are stored in `%APPDATA%\Smart Citizen++\`, and data (edits, generated files, backups) in `Documents\Smart Citizen++\`.
- `SmartCitizenPlusPlus-Portable-<version>.zip`: a portable build that keeps its settings and data next to the executable.

The app is not code-signed yet, so Windows SmartScreen will warn that it is unrecognized. Choose **More info → Run anyway**, and only download builds from this repository's Releases page.

## Building from source

Requirements:

- Qt 6.8 or newer (Core, Gui, Network, Concurrent, Qml, Quick, QuickControls2, Test, QuickTest)
- CMake 3.25 or newer and Ninja
- A C++23 compiler

zstd, zlib and pugixml are downloaded and built automatically by CMake, pinned by SHA-256.

The presets in `CMakePresets.json` expect Qt 6.12 with MinGW 13.1 installed under `C:\Qt`:

```sh
cmake --preset mingw-debug
cmake --build --preset mingw-debug
ctest --preset mingw-debug -LE parity
```

The parity tests are excluded above because they need the original Python app as a reference.

To build the release installer and portable zip into `dist/`:

```powershell
powershell -ExecutionPolicy Bypass -File packaging\package.ps1
```

The installer step needs [Inno Setup 6](https://jrsoftware.org/isinfo.php) and is skipped if it is not installed.

On other platforms, configure with CMake and point `CMAKE_PREFIX_PATH` at your Qt install. See the next sections for the state of Linux support.

## Why?

Well, for a few reasons:

> 1. Python, while not terrible, is going to be slower at everything this application is trying to accomplish.
> 2. C++ (IMO) can be modified and added to with minimal work, whereas the Python side takes quite some time for a new version.
> 3. Cross-platform compatibility. This application was built using the C++ standard library (`std::`), meaning it can easily be run on any flavor of Linux you so choose!

## Linux and non-Windows platforms

Should work "out of the box". The application uses Qt as its backend for the UI, so your mileage will vary. If something doesn't work, please let me know by creating an issue.

### Installing on Arch Linux

Run this as your normal user (not root):

```sh
curl -fsSL https://raw.githubusercontent.com/Ishikudeska/Smart-Citizen-plus-plus/main/packaging/aur/install.sh | bash
```

It builds a pacman package from [`packaging/aur/PKGBUILD`](packaging/aur/PKGBUILD) and installs it, asking for your password when it needs `sudo`. The app then shows up in your app menu as Smart Citizen++. Run the same command again to update to the latest version, and `sudo pacman -R smart-citizen-plus-plus-git` to remove it.

If you would rather not pipe a script into your shell, do the same thing by hand:

```sh
git clone https://github.com/Ishikudeska/Smart-Citizen-plus-plus.git
cd Smart-Citizen-plus-plus/packaging/aur
makepkg -si
```

### Other distributions

Install Qt 6.8 or newer (including Qt Declarative), CMake, Ninja and a C++23 compiler from your package manager, then:

```sh
git clone https://github.com/Ishikudeska/Smart-Citizen-plus-plus.git
cd Smart-Citizen-plus-plus
cmake --preset linux-release -DCMAKE_INSTALL_PREFIX=/usr/local -DAPP_UPDATE_REPO=
cmake --build --preset linux-release
sudo cmake --install build/linux-release
```

## Acknowledgements

Smart Citizen++ stands on the shoulders of these projects and people.

### Smart Citizen

This is a port of [**Smart Citizen**](https://github.com/Osiris-DevWorks/smart-citizen) by [**Osiris DevWorks**](https://github.com/Osiris-DevWorks). Its features, merge logic, settings formats and translations all come from the original Python app. Thank you to the Smart Citizen developers and contributors:

- [**Osiris DevWorks**](https://github.com/Osiris-DevWorks)
- [**Stealrull**](https://github.com/Stealrull)
- **jonigirl**
- [**Coerwyn**](https://github.com/Coerwyn)
- [**denis-coach**](https://github.com/denis-coach) (also [h0use](https://github.com/h0useRus))
- [**scubamount**](https://github.com/scubamount)
- **hkstrongside**
- [**odw-okano**](https://github.com/odw-okano)

Smart Citizen in turn credits [**ExoAE's ScCompLangPack**](https://github.com/ExoAE/ScCompLangPack) for the original concept and merge logic, and [**MrKraken**](https://github.com/MrKraken/StarStrings) for the ASOP terminal enhancements and mission contract localization work.

### unp4k

The `Data.p4k` reader and DataForge → XML converter in `src/engine/` are a C++ port of [**unp4k / unforge**](https://github.com/dolkensp/unp4k) by **Peter Dolkens** and contributors (MIT License), and of Osiris DevWorks' parallelized fork, [**odw-fast-unp4k**](https://github.com/Osiris-DevWorks/odw-fast-unp4k). Without their work on reverse-engineering the p4k and DataForge formats, none of this would be possible.

### Translations

- **Akwa**: French interface translation
- **Nxzzin**: Brazilian Portuguese interface translation
- [**Thord82**](https://github.com/Thord82): Spanish interface translation and the [Spanish `global.ini`](https://github.com/Thord82/Star_citizen_ES)
- [**Dymerz/StarCitizen-Localization**](https://github.com/Dymerz/StarCitizen-Localization): French, Brazilian Portuguese and Italian `global.ini`
- [**stdblue/StarCitizenJapaneseResources**](https://github.com/stdblue/StarCitizenJapaneseResources): Japanese `global.ini`
- [**42Kit**](https://ini.42kit.com/): Chinese `global.ini`
- [**rjcncpt**](https://github.com/rjcncpt): [German `global.ini`](https://github.com/rjcncpt/StarCitizen-Deutsch-INI)

And the testers and the wider **Star Citizen community**, whose feedback shaped Smart Citizen.

## License

Smart Citizen++ is licensed under the **Apache License, Version 2.0**, the same as Smart Citizen. See [LICENSE](LICENSE), and [NOTICE](NOTICE) for the Smart Citizen and unp4k attributions and the Star Citizen / CIG trademark notice. Smart Citizen++ is a fan project and is not affiliated with Cloud Imperium Games, Roberts Space Industries, or Osiris DevWorks.
