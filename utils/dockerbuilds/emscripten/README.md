# Emscripten (Web) Build Container

This directory provides a Dockerized build environment for creating a WebAssembly build of Wesnoth.

## Build the image

From `utils/dockerbuilds`:

```sh
./make_emscripten_image
```

By default this builds the image tag `wesnoth/wesnoth:emscripten`.
The image currently pins the Emscripten SDK base image to `5.0.7`.

## Build Wesnoth for web

From `utils/dockerbuilds`:

```sh
./make_emscripten_build
```

Output artifacts are written to `utils/dockerbuilds/emscriptenbuild/`.
Dependency cache and built libraries are kept in `utils/dockerbuilds/emscripten-vcpkg-cache/`.
The wrapper reads `emscripten/current_experiment.env` first if that file exists,
then lets environment variables override those defaults. Docker must be able to
bind-mount the checkout, output directory, and cache directory. If you run
Docker from inside a devcontainer that talks to a host Docker daemon, run these
commands from a host-visible checkout/path.

To keep large generated files outside the source tree, override the output and
cache paths:

```sh
OUTPUT_DIR=/tmp/wesnoth-emscriptenbuild \
VCPKG_CACHE_DIR=/tmp/wesnoth-emscripten-vcpkg-cache \
WESNOTH_WEB_HISTORY_ROOT=/tmp/wesnoth-emscripten-history \
./make_emscripten_build
```

The single-threaded JSPI variant used for browser smoke testing can be built
with:

```sh
OUTPUT_DIR=/tmp/wesnoth-emscriptenbuild \
VCPKG_CACHE_DIR=/tmp/wesnoth-emscripten-vcpkg-cache \
WESNOTH_WEB_HISTORY_ROOT=/tmp/wesnoth-emscripten-history \
WESNOTH_USE_PTHREADS=0 \
WESNOTH_PROXY_TO_PTHREAD=0 \
WESNOTH_EXTRA_CMAKE_ARGS='-DCMAKE_MODULE_PATH=/wesnoth/utils/dockerbuilds/emscripten/cmake-modules' \
WESNOTH_EXTRA_EM_FLAGS='-sLZ4=1 -sJSPI' \
WESNOTH_EXTRA_EM_LINK_FLAGS='-lidbfs.js' \
./make_emscripten_build
```

Expected build outputs include:

- `index.html`
- `wesnoth.js`
- `wesnoth.wasm`
- `wesnoth.data`
- `wesnoth.data.js`
- `serve_coi.py`

## Serve and smoke-test the build

From the output directory:

```sh
python3 ./serve_coi.py --host 127.0.0.1 --port 8040 --dir .
```

Then open `http://127.0.0.1:8040/index.html` in Chromium or Chrome.
The JSPI build requires a browser with WebAssembly JS Promise Integration
support; older Chromium builds may need the
`--enable-features=WebAssemblyJSPromiseIntegration` flag. Firefox and Safari
are not expected to run this JSPI build.

For an automated visible-pixels check from `utils/dockerbuilds/emscripten`,
install Playwright outside the repository and point `NODE_PATH` at it:

```sh
mkdir -p /tmp/wesnoth-playwright-smoke
(
  cd /tmp/wesnoth-playwright-smoke
  npm init -y
  npm install playwright@1.57.0
  PLAYWRIGHT_BROWSERS_PATH=/tmp/wesnoth-playwright-browsers \
    npx playwright install chromium
)
PLAYWRIGHT_BROWSERS_PATH=/tmp/wesnoth-playwright-browsers \
NODE_PATH=/tmp/wesnoth-playwright-smoke/node_modules \
node ./run_playwright_check.js \
  http://127.0.0.1:8040/index.html \
  /tmp/wesnoth-web-console.json \
  /tmp/wesnoth-web-result.json \
  /tmp/wesnoth-web.png \
  15000
```

## Useful environment variables

- `DOCKERLOCALIMAGE`: override docker image tag.
- `OUTPUT_DIR`: override output directory (default `../emscriptenbuild`).
- `VCPKG_CACHE_DIR`: override persistent vcpkg cache directory (default `../emscripten-vcpkg-cache`).
- `WESNOTH_WEB_HISTORY_ROOT`: override the directory used for build-history text files and immutable build bundles (default `../../output` from `utils/dockerbuilds`).
- `WESNOTH_USE_PTHREADS`: `1` (default) builds a SharedArrayBuffer/pthreads variant, `0` disables pthreads.
- `WESNOTH_PROXY_TO_PTHREAD`: `1` (default) enables `-sPROXY_TO_PTHREAD=1`; set `0` to keep the main thread as the primary runtime thread.
- `WESNOTH_PTHREAD_POOL_SIZE`: size for `-sPTHREAD_POOL_SIZE` (default `4`); set `0` to omit the flag.
- `WESNOTH_PACKAGE_DATA`: set `0` to skip `wesnoth.data` and `wesnoth.data.js` generation; default `1`.
- `WESNOTH_DATA_LZ4`: `auto` follows `-sLZ4=1`; set `1` or `0` to force file-packager compression behavior.
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

The first build can take a long time because it compiles the vcpkg dependency graph for wasm.

## Serving a pthreads/SAB build

If `WESNOTH_USE_PTHREADS=1`, browser runtime requires cross-origin isolation. Serve with at least:

- `Cross-Origin-Opener-Policy: same-origin`
- `Cross-Origin-Embedder-Policy: require-corp`

If assets are loaded from other origins, those resources must also be CORS/CORP-compatible.
The provided `serve_coi.py` sends these headers and is safe to use for both
pthreads and non-pthreads builds.

## Directory structure

```
emscripten/
|-- Dockerfile                  # Build container definition
|-- build-web.sh                # Main build script
|-- index.html                  # Deployed HTML loader
|-- serve_coi.py                # Local COI dev server
|-- cmake-modules/              # Emscripten CMake Find module overrides
|-- vcpkg-overlay-ports/        # Emscripten vcpkg port overlays
|-- vcpkg-overlay-triplets/     # Wasm32 triplet definitions
|-- runtime-seed/               # Default runtime preference placeholders
`-- run_playwright_check.js     # Visible-pixels smoke test
```
