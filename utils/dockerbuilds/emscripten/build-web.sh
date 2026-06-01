#!/usr/bin/env bash

set -euo pipefail

source_dir="${WESNOTH_SOURCE_DIR:-/wesnoth}"
build_dir="${WESNOTH_BUILD_DIR:-/build/wesnoth-web}"
output_dir="${WESNOTH_OUTPUT_DIR:-/output}"
build_type="${WESNOTH_CMAKE_BUILD_TYPE:-RelWithDebInfo}"
jobs="${WESNOTH_PARALLEL_JOBS:-$(nproc)}"
use_pthreads="${WESNOTH_USE_PTHREADS:-1}"
proxy_to_pthread="${WESNOTH_PROXY_TO_PTHREAD:-1}"
pthread_pool_size="${WESNOTH_PTHREAD_POOL_SIZE:-4}"
package_data="${WESNOTH_PACKAGE_DATA:-1}"
data_lz4="${WESNOTH_DATA_LZ4:-auto}"
use_vcpkg="${WESNOTH_USE_VCPKG:-1}"
vcpkg_root="${WESNOTH_VCPKG_ROOT:-/vcpkg-cache/vcpkg}"
vcpkg_install_root="${WESNOTH_VCPKG_INSTALL_ROOT:-/vcpkg-cache/installed}"
vcpkg_target_triplet="${WESNOTH_VCPKG_TARGET_TRIPLET:-wasm32-emscripten}"
vcpkg_allow_unsupported="${WESNOTH_VCPKG_ALLOW_UNSUPPORTED:-0}"
vcpkg_overlay_ports="${WESNOTH_VCPKG_OVERLAY_PORTS:-/wesnoth/utils/dockerbuilds/emscripten/vcpkg-overlay-ports}"
vcpkg_overlay_triplets="${WESNOTH_VCPKG_OVERLAY_TRIPLETS:-/wesnoth/utils/dockerbuilds/emscripten/vcpkg-overlay-triplets}"
vcpkg_binary_cache="${WESNOTH_VCPKG_BINARY_CACHE:-/vcpkg-cache/archives}"
emscripten_root="${EMSCRIPTEN:-/emsdk/upstream/emscripten}"
emscripten_toolchain="${WESNOTH_EMSCRIPTEN_TOOLCHAIN_FILE:-${emscripten_root}/cmake/Modules/Platform/Emscripten.cmake}"

case "$(uname -m)" in
    aarch64|arm64)
        default_vcpkg_host_triplet="arm64-linux"
        ;;
    x86_64|amd64)
        default_vcpkg_host_triplet="x64-linux"
        ;;
    *)
        default_vcpkg_host_triplet="x64-linux"
        ;;
esac
vcpkg_host_triplet="${WESNOTH_VCPKG_HOST_TRIPLET:-$default_vcpkg_host_triplet}"

if [[ ! -d "$source_dir" ]]; then
    echo "Source directory not found: $source_dir" >&2
    exit 1
fi

mkdir -p "$build_dir" "$output_dir"

em_flags=(
    "-fwasm-exceptions"
)
em_link_flags=()
pthreads_cmake=OFF

if [[ "$use_pthreads" == "1" ]]; then
    pthreads_cmake=ON
    em_flags+=(
        "-pthread"
        "-sUSE_PTHREADS=1"
    )
    if [[ "$pthread_pool_size" != "0" ]]; then
        em_flags+=("-sPTHREAD_POOL_SIZE=${pthread_pool_size}")
    fi
    if [[ "$proxy_to_pthread" == "1" ]]; then
        em_flags+=("-sPROXY_TO_PTHREAD=1")
    fi
fi

if [[ -n "${WESNOTH_EXTRA_EM_FLAGS:-}" ]]; then
    read -r -a extra_em_flags <<< "${WESNOTH_EXTRA_EM_FLAGS}"
    em_flags+=("${extra_em_flags[@]}")
fi

if [[ -n "${WESNOTH_EXTRA_EM_LINK_FLAGS:-}" ]]; then
    read -r -a extra_em_link_flags <<< "${WESNOTH_EXTRA_EM_LINK_FLAGS}"
    em_link_flags+=("${extra_em_link_flags[@]}")
fi

cmake_args=(
    "-DCMAKE_BUILD_TYPE=${build_type}"
    "-DCMAKE_EXECUTABLE_SUFFIX=.html"
    "-DENABLE_SERVER=OFF"
    "-DENABLE_CAMPAIGN_SERVER=OFF"
    "-DENABLE_TESTS=OFF"
    "-DENABLE_NLS=OFF"
    "-DENABLE_POT_UPDATE_TARGET=OFF"
    "-DENABLE_DISPLAY_REVISION=OFF"
    "-DENABLE_NOTIFICATIONS=OFF"
    "-DENABLE_DESKTOP_ENTRY=OFF"
    "-DENABLE_APPDATA_FILE=OFF"
    "-DWESNOTH_EMSCRIPTEN_PTHREADS=${pthreads_cmake}"
    "-DCMAKE_C_FLAGS=${em_flags[*]}"
    "-DCMAKE_CXX_FLAGS=${em_flags[*]}"
    "-DCMAKE_EXE_LINKER_FLAGS=${em_flags[*]} ${em_link_flags[*]}"
)

if [[ -n "${WESNOTH_BOOST_DIR:-}" ]]; then
    cmake_args+=("-DBoost_DIR=${WESNOTH_BOOST_DIR}")
fi

if [[ -n "${WESNOTH_CMAKE_PREFIX_PATH:-}" ]]; then
    cmake_args+=("-DCMAKE_PREFIX_PATH=${WESNOTH_CMAKE_PREFIX_PATH}")
fi

if [[ -n "${WESNOTH_EXTRA_CMAKE_ARGS:-}" ]]; then
    read -r -a extra_cmake_args <<< "${WESNOTH_EXTRA_CMAKE_ARGS}"
    cmake_args+=("${extra_cmake_args[@]}")
fi

echo "Configuring wesnoth web build"
echo "  source: $source_dir"
echo "  build: $build_dir"
echo "  output: $output_dir"
echo "  pthreads: $use_pthreads"
echo "  package data: $package_data"
echo "  vcpkg: $use_vcpkg"
echo "  vcpkg target triplet: $vcpkg_target_triplet"

if [[ "$use_vcpkg" == "1" ]]; then
    mkdir -p "$(dirname "$vcpkg_root")" "$vcpkg_install_root" "$vcpkg_binary_cache"
    export VCPKG_DEFAULT_BINARY_CACHE="$vcpkg_binary_cache"
    export VCPKG_BINARY_SOURCES="${WESNOTH_VCPKG_BINARY_SOURCES:-clear;files,${vcpkg_binary_cache},readwrite}"

    if [[ ! -d "$vcpkg_root/.git" ]]; then
        echo "Cloning vcpkg into $vcpkg_root"
        git clone --depth 1 https://github.com/microsoft/vcpkg "$vcpkg_root"
    fi
    git config --global --add safe.directory "$vcpkg_root" >/dev/null 2>&1 || true

    # The project pins a vcpkg baseline commit; fetch it when using a shallow clone.
    if [[ -f "${source_dir}/vcpkg.json" ]]; then
        baseline="$(grep -Eo '"builtin-baseline"[[:space:]]*:[[:space:]]*"[^"]+"' "${source_dir}/vcpkg.json" | head -n 1 | sed -E 's/.*"([^"]+)"/\1/' || true)"
        if [[ -n "$baseline" ]] && ! git -C "$vcpkg_root" cat-file -e "${baseline}^{commit}" 2>/dev/null; then
            git -C "$vcpkg_root" fetch --depth 1 origin "$baseline" || true
        fi
    fi

    if [[ ! -x "$vcpkg_root/vcpkg" ]]; then
        "$vcpkg_root/bootstrap-vcpkg.sh" -disableMetrics
    fi

    vcpkg_cmd=(
        "$vcpkg_root/vcpkg"
        install
        "--triplet=${vcpkg_target_triplet}"
        "--x-manifest-root=${source_dir}"
        "--x-install-root=${vcpkg_install_root}"
        "--feature-flags=manifests"
    )

    if [[ -d "$vcpkg_overlay_ports" ]]; then
        vcpkg_cmd+=("--overlay-ports=${vcpkg_overlay_ports}")
    fi
    if [[ -d "$vcpkg_overlay_triplets" ]]; then
        vcpkg_cmd+=("--overlay-triplets=${vcpkg_overlay_triplets}")
    fi

    if [[ "$vcpkg_allow_unsupported" == "1" ]]; then
        vcpkg_cmd+=("--allow-unsupported")
    fi

    echo "Installing dependencies with vcpkg"
    "${vcpkg_cmd[@]}"

    # Strip -pthread from installed .pc files for non-pthreads builds.
    # The Docker image's pkg-config wrapper prevents -pthread from cascading
    # between packages during the vcpkg build when requested by the wrapper.
    # This post-install strip ensures the wesnoth cmake step also gets clean flags.
    if [[ "$use_pthreads" != "1" ]]; then
        echo "Stripping -pthread from installed pkg-config files"
        find "$vcpkg_install_root" -name '*.pc' -exec \
            sed -i 's/-pthread//g; s/-sPTHREAD_POOL_SIZE=[0-9]*//g' {} +
    fi

    cmake_args+=(
        "-DCMAKE_TOOLCHAIN_FILE=${vcpkg_root}/scripts/buildsystems/vcpkg.cmake"
        "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=${emscripten_toolchain}"
        "-DVCPKG_TARGET_TRIPLET=${vcpkg_target_triplet}"
        "-DVCPKG_HOST_TRIPLET=${vcpkg_host_triplet}"
        "-DVCPKG_INSTALLED_DIR=${vcpkg_install_root}"
        "-DVCPKG_MANIFEST_MODE=OFF"
    )

    cmake -S "$source_dir" -B "$build_dir" -G Ninja "${cmake_args[@]}"
else
    emcmake cmake -S "$source_dir" -B "$build_dir" -G Ninja "${cmake_args[@]}"
fi

echo "Building wesnoth target"
cmake --build "$build_dir" -j "$jobs" --target wesnoth

echo "Collecting artifacts"
for ext in html js wasm data worker.js; do
    find "$build_dir" -maxdepth 1 -type f -name "wesnoth*.${ext}" -exec cp -f {} "$output_dir/" \;
done

# JSPI post-processing: Emscripten wraps all invoke_* imports with
# WebAssembly.Suspending, which causes V8 to throw "SuspendError: trying to
# suspend without WebAssembly.promising" during C++ global constructors
# (invoke_* is used for exception trampolines in try/catch blocks).
# Fix: remove invoke_* from the suspending import pattern so only
# __asyncjs__* and explicitly isAsync functions are wrapped.
if [[ " ${WESNOTH_EXTRA_EM_FLAGS:-} ${WESNOTH_EXTRA_EM_LINK_FLAGS:-} " == *"-sJSPI"* ]]; then
    echo "JSPI post-processing: removing invoke_* from Suspending import pattern"
    jsfile="${output_dir}/wesnoth.js"
    if [[ -f "$jsfile" ]]; then
        python3 -c "
import re, sys
with open(sys.argv[1], 'r') as f:
    content = f.read()
# Replace the import pattern that wraps invoke_* as Suspending.
# Before: /^(invoke_.*|__asyncjs__.*)$/
# After:  /^(__asyncjs__.*)$/
old = r'/^(invoke_.*|__asyncjs__.*)$/'
new = r'/^(__asyncjs__.*)$/'
if old in content:
    content = content.replace(old, new)
    with open(sys.argv[1], 'w') as f:
        f.write(content)
    print('  patched: removed invoke_* from Suspending imports')
else:
    print('  warning: invoke_* pattern not found in wesnoth.js', file=sys.stderr)
" "$jsfile"
    fi
fi

# EMULATE_FUNCTION_POINTER_CASTS + WASM_WORKERS post-processing:
# EMULATE_FUNCTION_POINTER_CASTS creates canonical WASM table entries with i64
# parameters.  Emscripten's AudioWorklet/WasmWorker dispatch uses
# getWasmTableEntry() to call callbacks directly from JS with spread args,
# which fails because JS passes regular Numbers but the canonical table entries
# expect BigInt (i64).
# Fix: inject a __callWasmTableEntry helper that wraps calls with a try-catch
# and converts args to BigInt on failure, then replace all spread-call patterns.
if [[ " ${WESNOTH_EXTRA_EM_FLAGS:-} ${WESNOTH_EXTRA_EM_LINK_FLAGS:-} " == *"-sWASM_WORKERS"* ]]; then
    echo "WASM_WORKERS post-processing: fixing getWasmTableEntry dispatch for EMULATE_FUNCTION_POINTER_CASTS"
    jsfile="${output_dir}/wesnoth.js"
    if [[ -f "$jsfile" ]]; then
        python3 -c "
import re, sys
with open(sys.argv[1], 'r') as f:
    content = f.read()
# Inject helper that handles BigInt conversion for EMULATE_FUNCTION_POINTER_CASTS
helper = 'var __callWasmTableEntry=function(idx,args){var fn=getWasmTableEntry(idx);try{return fn.apply(null,args)}catch(e){if(e instanceof TypeError&&e.message.includes(\"BigInt\")){return fn.apply(null,args.map(function(v){return BigInt(v)}))}throw e}};'
marker = 'getWasmTableEntry='
idx = content.find(marker)
if idx >= 0:
    end = content.find(';', idx) + 1
    content = content[:end] + helper + content[end:]
# Replace all getWasmTableEntry(X)(...Y) spread-call patterns
pattern = r'getWasmTableEntry\(([^)]+)\)\(\.\.\.([^)]+)\)'
content, count = re.subn(pattern, lambda m: '__callWasmTableEntry(%s,%s)' % (m.group(1), m.group(2)), content)
if count > 0:
    with open(sys.argv[1], 'w') as f:
        f.write(content)
    print('  patched: replaced %d getWasmTableEntry spread-call(s) with __callWasmTableEntry' % count)
else:
    print('  warning: no getWasmTableEntry spread-call patterns found', file=sys.stderr)
" "$jsfile"
    fi
fi

# JSPI + WASM_WORKERS: emscripten_sleep no-op on worker threads.
# The AudioWorklet thread can't suspend (no WebAssembly.promising wrapper on
# the entry point), so emscripten_sleep throws "SuspendError: trying to suspend
# without WebAssembly.promising".  Make it a no-op on worker/AudioWorklet threads.
if [[ " ${WESNOTH_EXTRA_EM_FLAGS:-} ${WESNOTH_EXTRA_EM_LINK_FLAGS:-} " == *"-sJSPI"* ]] && \
   [[ " ${WESNOTH_EXTRA_EM_FLAGS:-} ${WESNOTH_EXTRA_EM_LINK_FLAGS:-} " == *"-sWASM_WORKERS"* ]]; then
    echo "JSPI+WASM_WORKERS post-processing: making emscripten_sleep no-op on worker threads"
    jsfile="${output_dir}/wesnoth.js"
    if [[ -f "$jsfile" ]]; then
        python3 -c "
import sys
with open(sys.argv[1], 'r') as f:
    content = f.read()
old = 'var _emscripten_sleep=function(ms){let innerFunc=()=>new Promise(resolve=>setTimeout(resolve,ms));return Asyncify.handleAsync(innerFunc)}'
new = 'var _emscripten_sleep=function(ms){if(ENVIRONMENT_IS_AUDIO_WORKLET||ENVIRONMENT_IS_WASM_WORKER)return;let innerFunc=()=>new Promise(resolve=>setTimeout(resolve,ms));return Asyncify.handleAsync(innerFunc)}'
if old in content:
    content = content.replace(old, new)
    with open(sys.argv[1], 'w') as f:
        f.write(content)
    print('  patched: emscripten_sleep no-op on AudioWorklet/WasmWorker threads')
else:
    print('  warning: emscripten_sleep pattern not found', file=sys.stderr)
" "$jsfile"
    fi
fi

# JSPI + WASM_WORKERS: skip Suspending import wrapping on worker threads.
# Emscripten's instrumentWasmImports wraps async imports (those with .isAsync=true
# or matching __asyncjs__*) with WebAssembly.Suspending.  When the AudioWorklet
# thread enters WASM without WebAssembly.promising and calls any Suspending import,
# V8 throws SuspendError *before* the JS function body executes.
# Fix: skip the entire Suspending wrapping on WASM Worker / AudioWorklet threads,
# since they should never need JSPI suspension.
if [[ " ${WESNOTH_EXTRA_EM_FLAGS:-} ${WESNOTH_EXTRA_EM_LINK_FLAGS:-} " == *"-sJSPI"* ]] && \
   [[ " ${WESNOTH_EXTRA_EM_FLAGS:-} ${WESNOTH_EXTRA_EM_LINK_FLAGS:-} " == *"-sWASM_WORKERS"* ]]; then
    echo "JSPI+WASM_WORKERS post-processing: skipping Suspending imports on worker threads"
    jsfile="${output_dir}/wesnoth.js"
    if [[ -f "$jsfile" ]]; then
        python3 -c "
import sys
with open(sys.argv[1], 'r') as f:
    content = f.read()
old = 'instrumentWasmImports(imports){var importPattern='
new = 'instrumentWasmImports(imports){if(ENVIRONMENT_IS_WASM_WORKER)return;var importPattern='
if old in content:
    content = content.replace(old, new)
    with open(sys.argv[1], 'w') as f:
        f.write(content)
    print('  patched: instrumentWasmImports skips Suspending wrapping on WASM worker threads')
else:
    print('  warning: instrumentWasmImports pattern not found', file=sys.stderr)
" "$jsfile"
    fi
fi

# Copy shell output assets if they were generated under an emscripten-style output directory.
if [[ -d "${build_dir}/www" ]]; then
    rsync -a --delete "${build_dir}/www/" "${output_dir}/"
fi

if [[ "${package_data}" == "1" ]]; then
    if [[ "${data_lz4}" == "auto" ]]; then
        case " ${WESNOTH_EXTRA_EM_FLAGS:-} ${WESNOTH_EXTRA_EM_LINK_FLAGS:-} " in
            *" -sLZ4=1 "*) data_lz4="1" ;;
            *) data_lz4="0" ;;
        esac
    fi

    packager_args=(
        "${output_dir}/wesnoth.data"
        "--js-output=${output_dir}/wesnoth.data.js"
        "--use-preload-cache"
    )
    if [[ "${data_lz4}" == "1" ]]; then
        packager_args+=("--lz4")
    fi
    packager_args+=(
        "--preload" "${source_dir}/data@/data"
        "--preload" "${source_dir}/images@/images"
        "--preload" "${source_dir}/fonts@/fonts"
        "--preload" "${source_dir}/sounds@/sounds"
    )

    echo "Packaging Wesnoth data"
    python3 "${emscripten_root}/tools/file_packager.py" "${packager_args[@]}"
fi

web_asset_dir="${source_dir}/utils/dockerbuilds/emscripten"
cp -f "${web_asset_dir}/index.html" "${output_dir}/index.html"
cp -f "${web_asset_dir}/serve_coi.py" "${output_dir}/serve_coi.py"

echo "Done. Artifacts in: $output_dir"
