#!/usr/bin/env bash
# setup-sdl2.sh
# Downloads/installs SDL2 for Windows and macOS.
# Linux is handled separately via a Docker container.

set -euo pipefail

VERSION="${1:?Usage: setup-sdl2.sh <version>}"

case "$RUNNER_OS" in
    Windows)
        ARCHIVE="SDL2-devel-${VERSION}-VC.zip"
        URL="https://github.com/libsdl-org/SDL/releases/download/release-${VERSION}/${ARCHIVE}"
        DEST="${RUNNER_TEMP}/SDL2"

        echo "Downloading SDL2 ${VERSION} (Windows/MSVC)..."
        curl -fsSL -o "${RUNNER_TEMP}/${ARCHIVE}" "${URL}"
        unzip -q "${RUNNER_TEMP}/${ARCHIVE}" -d "${DEST}"

        SDL2_ROOT="${DEST}/SDL2-${VERSION}"
        echo "SDL2_ROOT=${SDL2_ROOT}" >> "${GITHUB_ENV}"
        ;;

    macOS)
        ARCHIVE="SDL2-${VERSION}.dmg"
        URL="https://github.com/libsdl-org/SDL/releases/download/release-${VERSION}/${ARCHIVE}"

        echo "Downloading SDL2 ${VERSION} (macOS framework)..."
        curl -fsSL -o "${RUNNER_TEMP}/${ARCHIVE}" "${URL}"

        hdiutil attach "${RUNNER_TEMP}/${ARCHIVE}" -quiet -mountpoint "${RUNNER_TEMP}/sdl2-mnt"
        sudo cp -R "${RUNNER_TEMP}/sdl2-mnt/SDL2.framework" /Library/Frameworks/
        hdiutil detach "${RUNNER_TEMP}/sdl2-mnt" -quiet
        ;;

    Linux)
        echo "Skipping — SDL2 is provided by the Docker container"
        ;;

    *)
        echo "::error::Unsupported RUNNER_OS: ${RUNNER_OS}"
        exit 1
        ;;
esac

echo "SDL2 ${VERSION} setup complete for ${RUNNER_OS}"
