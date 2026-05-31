# Emscripten (Web) Build Container

This directory provides a Dockerized build environment for creating a WebAssembly build of Wesnoth.

## Build the image

From `utils/dockerbuilds`:

```sh
./make_emscripten_image
```

By default this builds the image tag `wesnoth/wesnoth:emscripten`.

## Build Wesnoth for web

From `utils/dockerbuilds`:

```sh
./make_emscripten_build
```

Output artifacts are written to `utils/dockerbuilds/emscriptenbuild/`.
Dependency cache and built libraries are kept in `utils/dockerbuilds/emscripten-vcpkg-cache/`.

## Useful environment variables

- `DOCKERLOCALIMAGE`: override docker image tag.
- `OUTPUT_DIR`: override output directory (default `../emscriptenbuild`).
- `VCPKG_CACHE_DIR`: override persistent vcpkg cache directory (default `../emscripten-vcpkg-cache`).
- `WESNOTH_USE_PTHREADS`: `1` (default) builds a SharedArrayBuffer/pthreads variant, `0` disables pthreads.
- `WESNOTH_PROXY_TO_PTHREAD`: `1` (default) enables `-sPROXY_TO_PTHREAD=1`; set `0` to keep the main thread as the primary runtime thread.
- `WESNOTH_PTHREAD_POOL_SIZE`: size for `-sPTHREAD_POOL_SIZE` (default `4`); set `0` to omit the flag.
- `WESNOTH_USE_VCPKG`: `1` (default) installs dependencies through vcpkg for `wasm32-emscripten`.
- `WESNOTH_VCPKG_ROOT`: vcpkg checkout location inside the container (default `/vcpkg-cache/vcpkg`).
- `WESNOTH_VCPKG_INSTALL_ROOT`: vcpkg installed tree (default `/vcpkg-cache/installed`).
- `WESNOTH_VCPKG_TARGET_TRIPLET`: auto-defaults to `wasm32-emscripten-pthreads` when `WESNOTH_USE_PTHREADS=1`, otherwise `wasm32-emscripten`.
- `WESNOTH_VCPKG_HOST_TRIPLET`: optional host triplet override (auto-detected if unset).
- `WESNOTH_VCPKG_ALLOW_UNSUPPORTED`: set `1` to pass `--allow-unsupported` to vcpkg.
- `WESNOTH_VCPKG_OVERLAY_PORTS`: overlay port directory (default `/wesnoth/utils/dockerbuilds/emscripten/vcpkg-overlay-ports`).
- `WESNOTH_VCPKG_OVERLAY_TRIPLETS`: overlay triplet directory (default `/wesnoth/utils/dockerbuilds/emscripten/vcpkg-overlay-triplets`).
- `WESNOTH_VCPKG_BINARY_CACHE`: binary cache path inside the container (default `/vcpkg-cache/archives`).
- `WESNOTH_VCPKG_BINARY_SOURCES`: optional override for `VCPKG_BINARY_SOURCES`.
- `WESNOTH_PARALLEL_JOBS`: number of build jobs.
- `WESNOTH_CMAKE_BUILD_TYPE`: CMake build type (`RelWithDebInfo`, `Release`, etc.).
- `WESNOTH_EXTRA_EM_FLAGS`: additional Emscripten flags appended to compile/link flags.
- `WESNOTH_EXTRA_EM_LINK_FLAGS`: additional Emscripten flags appended only to linker flags (useful for `--preload-file`).
- `WESNOTH_EXTRA_CMAKE_ARGS`: extra CMake `-D...` args.
- `WESNOTH_BOOST_DIR`: optional `BoostConfig.cmake` directory for an Emscripten-compatible Boost build.
- `WESNOTH_CMAKE_PREFIX_PATH`: optional additional CMake prefix path (for cross-compiled dependencies).

Example (disable pthreads/SAB):

```sh
WESNOTH_USE_PTHREADS=0 ./make_emscripten_build
```

The first build can take a long time because it compiles the vcpkg dependency graph for wasm.

## Serving a pthreads/SAB build

If `WESNOTH_USE_PTHREADS=1`, browser runtime requires cross-origin isolation. Serve with at least:

- `Cross-Origin-Opener-Policy: same-origin`
- `Cross-Origin-Embedder-Policy: require-corp`

If assets are loaded from other origins, those resources must also be CORS/CORP-compatible.
