#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
PROJECT_PATH="$ROOT_DIR/projectfiles/Xcode/The Battle for Wesnoth.xcodeproj"
SCHEME_NAME="The Battle for Wesnoth"
LOG_FILE="${1:-}"

need_cmd() {
	if ! command -v "$1" >/dev/null 2>&1; then
		echo "Error: missing required command: $1" >&2
		exit 1
	fi
}

need_cmd xcodebuild
need_cmd xcrun
need_cmd rg

if [[ -z "$LOG_FILE" ]]; then
	LOG_FILE="$(mktemp -t wesnoth-ios-link-audit.XXXXXX.log)"
	echo "Using temporary log: $LOG_FILE"
else
	echo "Using log path: $LOG_FILE"
fi

if [[ ! -d "$PROJECT_PATH" ]]; then
	echo "Error: Xcode project not found at: $PROJECT_PATH" >&2
	exit 1
fi

echo "==> Running iOS simulator build probe"
set +e
xcodebuild \
	-project "$PROJECT_PATH" \
	-scheme "$SCHEME_NAME" \
	-configuration Debug \
	-sdk iphonesimulator \
	-destination 'generic/platform=iOS Simulator' \
	build >"$LOG_FILE" 2>&1
BUILD_STATUS=$?
set -e

if [[ "$BUILD_STATUS" -eq 0 ]]; then
	echo "Build succeeded."
else
	echo "Build failed (expected for current dependency layout)."
fi

echo
echo "==> First linker errors"
rg -n "ld: |clang\\+\\+: error: linker command failed" "$LOG_FILE" | head -n 12 || echo "(no linker error lines found)"

echo
echo "==> Link command markers"
if rg -q " -framework Cocoa " "$LOG_FILE"; then
	echo "found: -framework Cocoa"
else
	echo "missing: -framework Cocoa"
fi
if rg -q " -lcurl " "$LOG_FILE"; then
	echo "found: -lcurl"
else
	echo "missing: -lcurl"
fi

echo
echo "==> iOS SDK presence check"
IOS_SDK="$(xcrun --sdk iphonesimulator --show-sdk-path)"
for rel in "usr/lib/libcurl.tbd" "System/Library/Frameworks/Cocoa.framework"; do
	if [[ -e "$IOS_SDK/$rel" ]]; then
		echo "present: $rel"
	else
		echo "missing: $rel"
	fi
done

echo
echo "==> Bundled Xcode dylib platform summary"
DYNAMIC_LIB_DIR="$ROOT_DIR/projectfiles/Xcode/lib"
if [[ ! -d "$DYNAMIC_LIB_DIR" ]]; then
	echo "missing directory: $DYNAMIC_LIB_DIR"
	exit 1
fi

TMP_PLATFORM_LIST="$(mktemp -t wesnoth-ios-platforms.XXXXXX.txt)"
trap 'rm -f "$TMP_PLATFORM_LIST"' EXIT

find -L "$DYNAMIC_LIB_DIR" -maxdepth 1 -type f -name '*.dylib' | sort | while read -r dylib; do
	platform="$(xcrun vtool -show-build "$dylib" 2>/dev/null | awk '/platform / {print $2; exit}')"
	platform="${platform:-UNKNOWN}"
	echo "$platform $(basename "$dylib")"
done | tee "$TMP_PLATFORM_LIST" >/dev/null

awk '{count[$1] += 1} END {for (p in count) printf "%s %d\n", p, count[p]}' "$TMP_PLATFORM_LIST" | sort

echo
echo "Audit complete. Full build log: $LOG_FILE"
