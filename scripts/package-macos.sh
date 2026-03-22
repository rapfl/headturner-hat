#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-"$ROOT_DIR/build"}"
OUT_DIR="${2:-"$ROOT_DIR/out/macos"}"
VERSION="${3:-0.1.0}"

STAGE_DIR="$OUT_DIR/stage"
PKG_PATH="$OUT_DIR/HeadturnerHat-macOS-installer.pkg"

find_artifact() {
    local candidate
    for candidate in "$@"; do
        if [[ -e "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

VST3_SOURCE="$(find_artifact \
    "$BUILD_DIR/HeadturnerHat_artefacts/Release/VST3/Headturner Hat.vst3" \
    "$BUILD_DIR/HeadturnerHat_artefacts/VST3/Headturner Hat.vst3")"

AU_SOURCE="$(find_artifact \
    "$BUILD_DIR/HeadturnerHat_artefacts/Release/AU/Headturner Hat.component" \
    "$BUILD_DIR/HeadturnerHat_artefacts/AU/Headturner Hat.component")"

STANDALONE_SOURCE="$(find_artifact \
    "$BUILD_DIR/HeadturnerHat_artefacts/Release/Standalone/Headturner Hat.app" \
    "$BUILD_DIR/HeadturnerHat_artefacts/Standalone/Headturner Hat.app")"

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/Library/Audio/Plug-Ins/VST3" \
         "$STAGE_DIR/Library/Audio/Plug-Ins/Components" \
         "$STAGE_DIR/Applications"

cp -R "$VST3_SOURCE" "$STAGE_DIR/Library/Audio/Plug-Ins/VST3/"
cp -R "$AU_SOURCE" "$STAGE_DIR/Library/Audio/Plug-Ins/Components/"
cp -R "$STANDALONE_SOURCE" "$STAGE_DIR/Applications/"

mkdir -p "$OUT_DIR"
pkgbuild --root "$STAGE_DIR" \
    --identifier com.codexaudiolab.headturnerhat \
    --version "$VERSION" \
    --install-location / \
    "$PKG_PATH"

echo "$PKG_PATH"
