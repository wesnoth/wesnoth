#!/usr/bin/env bash
# Docker-based test: verify Wesnoth web multiplayer connects to wesnothd.
# Runs serve_coi.py, ws_proxy_node.js, and Playwright all inside a single Docker container.
#
# Usage: ./test_multiplayer_docker.sh [screenshot_dir]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${REPO_ROOT}/utils/dockerbuilds/emscriptenbuild"
SCREENSHOT_DIR="${1:-/tmp/wesnoth-multiplayer-docker-test}"

mkdir -p "${SCREENSHOT_DIR}"

PW_VERSION="1.57.0"
IMAGE="mcr.microsoft.com/playwright:v${PW_VERSION}-jammy"

echo "=== Wesnoth Web Multiplayer Docker Test ==="
echo "  Build dir: ${BUILD_DIR}"
echo "  Screenshot dir: ${SCREENSHOT_DIR}"
echo "  Docker image: ${IMAGE}"

docker run --rm \
  -v "${REPO_ROOT}:/workspace" \
  -v "${SCREENSHOT_DIR}:/screenshots" \
  -v "wesnoth-playwright-node:/opt/wesnoth-playwright-node" \
  -w /workspace \
  "${IMAGE}" \
  bash -lc '
    set -euo pipefail
    PW_VERSION="'"${PW_VERSION}"'"

    # Setup playwright node modules
    mkdir -p /opt/wesnoth-playwright-node
    cd /opt/wesnoth-playwright-node
    if [[ ! -f node_modules/.pw_version ]] || [[ "$(cat node_modules/.pw_version)" != "${PW_VERSION}" ]]; then
      rm -rf node_modules package.json package-lock.json
      npm init -y >/dev/null 2>&1 || true
      npm install --no-audit --no-fund --silent playwright@${PW_VERSION} ws >/dev/null
      echo "${PW_VERSION}" > node_modules/.pw_version
    fi
    export NODE_PATH=/opt/wesnoth-playwright-node/node_modules

    echo "=== Starting game server on :8040 ==="
    cd /workspace/utils/dockerbuilds/emscripten
    python3 ./serve_coi.py --host 127.0.0.1 --port 8040 --dir ../emscriptenbuild >/tmp/serve.log 2>&1 &
    SRV_PID=$!

    echo "=== Starting WebSocket proxy on :8041 ==="
    node ./ws_proxy_node.js --port 8041 -v >/tmp/ws_proxy.log 2>&1 &
    WS_PID=$!

    cleanup() {
      echo ""
      echo "=== ws_proxy log ==="
      cat /tmp/ws_proxy.log 2>/dev/null || true
      echo ""
      kill ${SRV_PID} ${WS_PID} 2>/dev/null || true
      wait ${SRV_PID} ${WS_PID} 2>/dev/null || true
    }
    trap cleanup EXIT

    sleep 2

    echo "=== Running Playwright multiplayer test ==="
    node ./test_multiplayer_playwright.js http://127.0.0.1:8040/ /screenshots
  '

echo ""
echo "=== Screenshots ==="
ls -la "${SCREENSHOT_DIR}"/*.png 2>/dev/null || echo "  (none found)"
echo ""
echo "Done."
