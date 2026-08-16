#!/usr/bin/env bash
# setup-sdl.sh
# Provisions SDL for Companion builds.
#
#   Windows: upstream SDL2 MSVC development binaries.
#   macOS:   sdl2-compat + SDL3 built from source.
#   Linux:   provided by the Docker container.
#
# macOS builds against sdl2-compat rather than classic SDL2 because that is what users get
# from Homebrew, and it is the configuration that exposed the SDL3 bundling bug - CI needs to
# exercise the same code path Companion ships through.
#
# Homebrew itself cannot be used here: its bottles are single-architecture and target the
# runner's macOS version, while Companion ships a universal binary for macOS 11.0+. Building
# from source is what keeps both properties.

set -euo pipefail

SDL2_VERSION="${SDL2_VERSION:?SDL2_VERSION must be set}"

build_cmake_project() {
    local name="$1" url="$2" srcdir="$3"
    shift 3

    echo "Building ${name} (universal, macOS ${MACOS_DEPLOYMENT_TARGET}+)..."
    curl -fsSL -o "${RUNNER_TEMP}/${name}.tar.gz" "${url}"
    mkdir -p "${RUNNER_TEMP}/src"
    tar -xzf "${RUNNER_TEMP}/${name}.tar.gz" -C "${RUNNER_TEMP}/src"

    cmake -S "${RUNNER_TEMP}/src/${srcdir}" -B "${RUNNER_TEMP}/build-${name}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="${MACOS_DEPLOYMENT_TARGET}" \
        -DCMAKE_INSTALL_PREFIX="${SDL_ROOT}" \
        -DCMAKE_PREFIX_PATH="${SDL_ROOT}" \
        "$@"
    cmake --build "${RUNNER_TEMP}/build-${name}" --parallel
    cmake --install "${RUNNER_TEMP}/build-${name}"
}

case "$RUNNER_OS" in
    Windows)
        ARCHIVE="SDL2-devel-${SDL2_VERSION}-VC.zip"
        URL="https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${ARCHIVE}"
        DEST="${RUNNER_TEMP}/SDL2"

        echo "Downloading SDL2 ${SDL2_VERSION} (Windows/MSVC)..."
        curl -fsSL -o "${RUNNER_TEMP}/${ARCHIVE}" "${URL}"
        unzip -q "${RUNNER_TEMP}/${ARCHIVE}" -d "${DEST}"

        SDL2_ROOT="${DEST}/SDL2-${SDL2_VERSION}"
        echo "SDL2_ROOT=${SDL2_ROOT}" >> "${GITHUB_ENV}"
        ;;

    macOS)
        SDL3_VERSION="${SDL3_VERSION:?SDL3_VERSION must be set}"
        SDL2_COMPAT_VERSION="${SDL2_COMPAT_VERSION:?SDL2_COMPAT_VERSION must be set}"
        MACOS_DEPLOYMENT_TARGET="${MACOS_DEPLOYMENT_TARGET:-11.0}"
        SDL_ROOT="${RUNNER_TEMP}/sdl"
        STAMP="${SDL_ROOT}/.stamp-${SDL3_VERSION}-${SDL2_COMPAT_VERSION}-${MACOS_DEPLOYMENT_TARGET}"

        if [[ -f "${STAMP}" ]]; then
            echo "Reusing cached SDL build in ${SDL_ROOT}"
        else
            rm -rf "${SDL_ROOT}"

            # SDL3 first - sdl2-compat needs its headers and CMake package to configure
            build_cmake_project "SDL3" \
                "https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VERSION}/SDL3-${SDL3_VERSION}.tar.gz" \
                "SDL3-${SDL3_VERSION}" \
                -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF

            build_cmake_project "sdl2-compat" \
                "https://github.com/libsdl-org/sdl2-compat/releases/download/release-${SDL2_COMPAT_VERSION}/sdl2-compat-${SDL2_COMPAT_VERSION}.tar.gz" \
                "sdl2-compat-${SDL2_COMPAT_VERSION}" \
                -DSDL2COMPAT_STATIC=OFF -DSDL2COMPAT_TESTS=OFF

            touch "${STAMP}"
        fi

        # Both live under one prefix, so a single entry on CMAKE_PREFIX_PATH finds
        # SDL2 (sdl2-compat) and SDL3 alike.
        echo "SDL2_ROOT=${SDL_ROOT}" >> "${GITHUB_ENV}"

        echo "Verifying universal slices and deployment target..."
        for lib in "${SDL_ROOT}"/lib/libSDL2-2.0.0.dylib "${SDL_ROOT}"/lib/libSDL3.0.dylib; do
            [[ -f "${lib}" ]] || { echo "::error::Expected ${lib} to exist"; exit 1; }
            archs="$(lipo -archs "${lib}")"
            case "${archs}" in
                *x86_64*arm64*|*arm64*x86_64*) ;;
                *) echo "::error::${lib} is not universal (got: ${archs})"; exit 1 ;;
            esac
            echo "  $(basename "${lib}"): ${archs}"
            vtool -show-build "${lib}" | grep -E 'minos|platform' || true
        done
        ;;

    Linux)
        echo "Skipping — SDL is provided by the Docker container"
        ;;

    *)
        echo "::error::Unsupported RUNNER_OS: ${RUNNER_OS}"
        exit 1
        ;;
esac

echo "SDL setup complete for ${RUNNER_OS}"
