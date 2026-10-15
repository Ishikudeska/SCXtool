#!/usr/bin/env bash
# Builds and installs Smart Citizen++ on Arch Linux (and derivatives) from the
# PKGBUILD in this repository, the same way an AUR helper would:
#
#   curl -fsSL https://raw.githubusercontent.com/Ishikudeska/Smart-Citizen-plus-plus/main/packaging/aur/install.sh | bash
#
# Run it again to update to the latest commit. Remove with:
#   sudo pacman -R smart-citizen-plus-plus-git
set -euo pipefail

repo="https://github.com/Ishikudeska/Smart-Citizen-plus-plus"
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

echo ":: Fetching the PKGBUILD"
curl -fsSL "$repo/raw/$branch/packaging/aur/PKGBUILD" -o "$workdir/PKGBUILD"

echo ":: Building and installing (this compiles the app and runs its tests)"
cd "$workdir"
makepkg --syncdeps --install --needed --noconfirm

echo ":: Done. Launch Smart Citizen++ from your app menu, or run SmartCitizenPlusPlus"
