#!/bin/bash
# build-web-native.sh — Build the Wesnoth Emscripten web port natively (no Docker).
#
# This script is an alternative to make_emscripten_build for environments where
# Docker bind mounts don't work (e.g. devcontainers with DinD/virtiofs).  It
# installs the Emscripten SDK + system packages directly, then delegates to the
# existing build-web.sh with environment variables overridden for native paths.
#
# Usage:
#   bash utils/dockerbuilds/emscripten/build-web-native.sh
#
# Environment variables (all optional, sensible defaults provided):
#   EMSDK_DIR                    — emsdk install location (default: /tmp/emsdk)
#   EMSDK_VERSION                — emsdk version to install  (default: 5.0.1)
#   WESNOTH_NATIVE_SKIP_BOOTSTRAP — set 1 to skip apt/emsdk install
#   WESNOTH_SKIP_DATA_PACKAGE    — set 1 to skip file_packager.py step
#   WESNOTH_PACKAGE_DATA         — set 0 to skip data packaging (alias)
#   WESNOTH_DATA_LZ4             — 1/0/auto; auto detects from EM_FLAGS
#   All WESNOTH_* env vars honored by build-web.sh are passed through.

set -euo pipefail

build_start_seconds=$SECONDS

# ---------------------------------------------------------------------------
# 1. Resolve paths
# ---------------------------------------------------------------------------
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
source_dir="${WESNOTH_SOURCE_DIR:-$(cd -- "${script_dir}/../../.." && pwd)}"
output_dir="${WESNOTH_OUTPUT_DIR:-${script_dir}/../emscriptenbuild}"
build_dir="${WESNOTH_BUILD_DIR:-/tmp/wesnoth-web-build}"
emsdk_dir="${EMSDK_DIR:-/tmp/emsdk}"
emsdk_version="${EMSDK_VERSION:-5.0.1}"

# vcpkg paths — redirect Docker defaults to /tmp
vcpkg_root="${WESNOTH_VCPKG_ROOT:-/tmp/vcpkg-cache/vcpkg}"
vcpkg_install_root="${WESNOTH_VCPKG_INSTALL_ROOT:-/tmp/vcpkg-cache/installed}"
vcpkg_binary_cache="${WESNOTH_VCPKG_BINARY_CACHE:-/tmp/vcpkg-cache/archives}"
vcpkg_overlay_ports="${WESNOTH_VCPKG_OVERLAY_PORTS:-${source_dir}/utils/dockerbuilds/emscripten/vcpkg-overlay-ports}"
vcpkg_overlay_triplets="${WESNOTH_VCPKG_OVERLAY_TRIPLETS:-${source_dir}/utils/dockerbuilds/emscripten/vcpkg-overlay-triplets}"

# Build options
use_pthreads="${WESNOTH_USE_PTHREADS:-1}"
package_data="${WESNOTH_PACKAGE_DATA:-1}"
skip_data_package="${WESNOTH_SKIP_DATA_PACKAGE:-0}"
data_lz4="${WESNOTH_DATA_LZ4:-auto}"
extra_em_flags="${WESNOTH_EXTRA_EM_FLAGS:-}"
extra_em_link_flags="${WESNOTH_EXTRA_EM_LINK_FLAGS:-}"

echo "========================================"
echo " Wesnoth Emscripten — native build"
echo "========================================"
echo "  source_dir:   ${source_dir}"
echo "  build_dir:    ${build_dir}"
echo "  output_dir:   ${output_dir}"
echo "  emsdk_dir:    ${emsdk_dir}"
echo "  emsdk_version:${emsdk_version}"
echo "  pthreads:     ${use_pthreads}"
echo "  package_data: ${package_data}"
echo "  skip_data_pkg:${skip_data_package}"
echo ""

# ---------------------------------------------------------------------------
# 2. Bootstrap — idempotent: apt packages + emsdk
# ---------------------------------------------------------------------------
if [ "${WESNOTH_NATIVE_SKIP_BOOTSTRAP:-0}" != "1" ]; then
    echo "--- Bootstrap: system packages ---"
    apt_packages=(
        cmake ninja-build pkg-config
        autoconf autoconf-archive automake libtool
        bison flex rsync
        python3 git ca-certificates
        curl zip unzip tar
    )
    missing=()
    for pkg in "${apt_packages[@]}"; do
        if ! dpkg -s "$pkg" >/dev/null 2>&1; then
            missing+=("$pkg")
        fi
    done
    if [ ${#missing[@]} -gt 0 ]; then
        echo "Installing missing packages: ${missing[*]}"
        apt-get update -qq
        apt-get install -y --no-install-recommends "${missing[@]}"
        rm -rf /var/lib/apt/lists/*
    else
        echo "All system packages already installed."
    fi

    echo "--- Bootstrap: emsdk ${emsdk_version} ---"
    if [ ! -d "${emsdk_dir}/.git" ]; then
        echo "Cloning emsdk into ${emsdk_dir}"
        git clone --depth 1 https://github.com/emscripten-core/emsdk.git "${emsdk_dir}"
    fi
    if [ ! -f "${emsdk_dir}/upstream/emscripten/emcc" ] || \
       ! "${emsdk_dir}/upstream/emscripten/emcc" --version 2>/dev/null | grep -q "${emsdk_version}"; then
        echo "Installing emsdk ${emsdk_version}"
        "${emsdk_dir}/emsdk" install "${emsdk_version}"
        "${emsdk_dir}/emsdk" activate "${emsdk_version}"
    else
        echo "emsdk ${emsdk_version} already installed and active."
    fi
else
    echo "--- Bootstrap skipped (WESNOTH_NATIVE_SKIP_BOOTSTRAP=1) ---"
fi

# ---------------------------------------------------------------------------
# 3. Activate emsdk — puts emcc/emcmake on PATH, sets EMSDK
# ---------------------------------------------------------------------------
echo "--- Sourcing emsdk_env.sh ---"
# emsdk_env.sh uses unbound variables; temporarily relax
set +u
# shellcheck disable=SC1091
source "${emsdk_dir}/emsdk_env.sh"
set -u
echo "  EMSDK=${EMSDK}"
echo "  emcc=$(command -v emcc)"

# ---------------------------------------------------------------------------
# 4. Export environment variables for build-web.sh
# ---------------------------------------------------------------------------
export WESNOTH_SOURCE_DIR="${source_dir}"
export WESNOTH_BUILD_DIR="${build_dir}"
export WESNOTH_OUTPUT_DIR="${output_dir}"
export WESNOTH_VCPKG_ROOT="${vcpkg_root}"
export WESNOTH_VCPKG_INSTALL_ROOT="${vcpkg_install_root}"
export WESNOTH_VCPKG_BINARY_CACHE="${vcpkg_binary_cache}"
export WESNOTH_VCPKG_OVERLAY_PORTS="${vcpkg_overlay_ports}"
export WESNOTH_VCPKG_OVERLAY_TRIPLETS="${vcpkg_overlay_triplets}"
export EMSCRIPTEN="${EMSDK}/upstream/emscripten"

# ---------------------------------------------------------------------------
# 5. Handle triplet based on pthreads
# ---------------------------------------------------------------------------
if [ -z "${WESNOTH_VCPKG_TARGET_TRIPLET:-}" ]; then
    if [ "${use_pthreads}" = "1" ]; then
        export WESNOTH_VCPKG_TARGET_TRIPLET="wasm32-emscripten-pthreads"
    else
        export WESNOTH_VCPKG_TARGET_TRIPLET="wasm32-emscripten"
    fi
fi
echo "  vcpkg target triplet: ${WESNOTH_VCPKG_TARGET_TRIPLET}"

# Ensure output directory exists
mkdir -p "${output_dir}"

# ---------------------------------------------------------------------------
# 6. Call build-web.sh
# ---------------------------------------------------------------------------
echo ""
echo "========================================"
echo " Delegating to build-web.sh"
echo "========================================"
bash "${source_dir}/utils/dockerbuilds/emscripten/build-web.sh"

# ---------------------------------------------------------------------------
# 7. Package data (file_packager.py)
# ---------------------------------------------------------------------------
if [ "${skip_data_package}" = "1" ] || [ "${package_data}" = "0" ]; then
    echo ""
    echo "--- Data packaging skipped ---"
else
    echo ""
    echo "========================================"
    echo " Packaging game data (file_packager.py)"
    echo "========================================"

    file_packager="${EMSDK}/upstream/emscripten/tools/file_packager.py"
    if [ ! -f "${file_packager}" ]; then
        echo "ERROR: file_packager.py not found at ${file_packager}" >&2
        exit 1
    fi

    # Determine LZ4
    if [ "${data_lz4}" = "auto" ]; then
        case " ${extra_em_flags} ${extra_em_link_flags} " in
            *" -sLZ4=1 "*|*"-sLZ4=1 "*|*" -sLZ4=1"*) data_lz4="1" ;;
            *) data_lz4="0" ;;
        esac
    fi

    packager_args=(
        "${output_dir}/wesnoth.data"
        "--js-output=${output_dir}/wesnoth.data.js"
        "--use-preload-cache"
        "--preload" "${source_dir}/data@/data"
        "--preload" "${source_dir}/images@/images"
        "--preload" "${source_dir}/fonts@/fonts"
        "--preload" "${source_dir}/sounds@/sounds"
    )

    if [ "${data_lz4}" = "1" ]; then
        echo "  LZ4 compression: enabled"
        packager_args+=("--lz4")
    else
        echo "  LZ4 compression: disabled"
    fi

    python3 "${file_packager}" "${packager_args[@]}"
    echo "  data packaging complete"
fi

# ---------------------------------------------------------------------------
# 8. Copy web assets (index.html, serve_coi.py)
# ---------------------------------------------------------------------------
echo ""
echo "--- Copying web assets ---"
cp -f "${script_dir}/index.html" "${output_dir}/"
cp -f "${script_dir}/serve_coi.py" "${output_dir}/"

# ---------------------------------------------------------------------------
# 9. Summary
# ---------------------------------------------------------------------------
elapsed=$(( SECONDS - build_start_seconds ))
elapsed_min=$(( elapsed / 60 ))
elapsed_sec=$(( elapsed % 60 ))

echo ""
echo "========================================"
echo " Build complete  (${elapsed_min}m ${elapsed_sec}s)"
echo "========================================"
echo ""
echo "Artifacts in: ${output_dir}"
echo ""

for artifact in index.html serve_coi.py wesnoth.js wesnoth.wasm wesnoth.data wesnoth.data.js wesnoth.worker.js; do
    f="${output_dir}/${artifact}"
    if [ -f "$f" ]; then
        size=$(du -h "$f" | cut -f1)
        printf "  %-24s %s\n" "${artifact}" "${size}"
    else
        printf "  %-24s %s\n" "${artifact}" "(not found)"
    fi
done

echo ""
echo "To serve locally:"
echo "  cd ${output_dir} && python3 serve_coi.py --port 8040"
