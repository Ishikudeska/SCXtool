#!/usr/bin/env bash
set -euo pipefail

repo="https://github.com/Ishikudeska/SCXtool"
branch="${SC_BRANCH:-main}"

if [[ $EUID -eq 0 ]]; then
    echo "Run this as your normal user, not root; makepkg asks for sudo when it installs." >&2
    exit 1
fi
if ! command -v pacman >/dev/null; then
    echo "This installer needs pacman (Arch Linux or a derivative)." >&2
    exit 1
fi

echo ":: Installing the build tools (base-devel, git) if they are missing"
sudo pacman -S --needed --noconfirm base-devel git

workdir=$(mktemp -d)
trap 'rm -rf "$workdir"' EXIT

if pacman -Q smart-citizen-plus-plus-git >/dev/null 2>&1; then
    echo ":: Removing the old smart-citizen-plus-plus-git package"
    sudo pacman -R --noconfirm smart-citizen-plus-plus-git
fi

echo ":: Fetching the PKGBUILD"
curl -fsSL "$repo/raw/$branch/packaging/aur/PKGBUILD" -o "$workdir/PKGBUILD"

echo ":: Building and installing (this compiles the app and runs its tests)"
cd "$workdir"
makepkg --syncdeps --install --needed --noconfirm

echo ":: Done. Launch SCXtool from your app menu, or run SCXtool"
