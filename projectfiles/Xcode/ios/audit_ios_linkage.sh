#!/bin/sh

root_dir=$(cd "$(dirname "$0")/../../.." && pwd)
project_path="$root_dir/projectfiles/Xcode/The Battle for Wesnoth.xcodeproj"
scheme_name="The Battle for Wesnoth"
log_file=${1:-}

need_cmd() {
	if ! command -v "$1" >/dev/null 2>&1; then
		echo "Error: missing required command: $1" >&2
		exit 1
	fi
}

need_cmd xcodebuild
need_cmd xcrun
need_cmd rg

if [ -z "$log_file" ]; then
	log_file=$(mktemp -t wesnoth-ios-link-audit.XXXXXX.log)
	echo "Using temporary log: $log_file"
else
	echo "Using log path: $log_file"
fi

if [ ! -d "$project_path" ]; then
	echo "Error: Xcode project not found at: $project_path" >&2
	exit 1
fi

echo "==> Running iOS simulator build probe"
xcodebuild \
	-project "$project_path" \
	-scheme "$scheme_name" \
	-configuration Debug \
	-sdk iphonesimulator \
	-destination 'generic/platform=iOS Simulator' \
	build >"$log_file" 2>&1
build_status=$?

if [ "$build_status" -eq 0 ]; then
	echo "Build succeeded."
else
	echo "Build failed (expected for current dependency layout)."
fi

echo
echo "==> First linker errors"
rg -n "ld: |clang\\+\\+: error: linker command failed" "$log_file" | head -n 12 || echo "(no linker error lines found)"

echo
echo "==> Link command markers"
if rg -q " -framework Cocoa " "$log_file"; then
	echo "found: -framework Cocoa"
else
	echo "missing: -framework Cocoa"
fi
if rg -q " -lcurl " "$log_file"; then
	echo "found: -lcurl"
else
	echo "missing: -lcurl"
fi

echo
echo "==> iOS SDK presence check"
ios_sdk=$(xcrun --sdk iphonesimulator --show-sdk-path)
for rel in "usr/lib/libcurl.tbd" "System/Library/Frameworks/Cocoa.framework"; do
	if [ -e "$ios_sdk/$rel" ]; then
		echo "present: $rel"
	else
		echo "missing: $rel"
	fi
done

echo
echo "==> Bundled Xcode dylib platform summary"
dynamic_lib_dir="$root_dir/projectfiles/Xcode/lib"
if [ ! -d "$dynamic_lib_dir" ]; then
	echo "missing directory: $dynamic_lib_dir"
	exit 1
fi

tmp_platform_list=$(mktemp -t wesnoth-ios-platforms.XXXXXX.txt)
trap 'rm -f "$tmp_platform_list"' EXIT

find -L "$dynamic_lib_dir" -maxdepth 1 -type f -name '*.dylib' | sort | while read -r dylib; do
	platform=$(xcrun vtool -show-build "$dylib" 2>/dev/null | awk '/platform / {print $2; exit}')
	platform="${platform:-UNKNOWN}"
	echo "$platform $(basename "$dylib")"
done | tee "$tmp_platform_list" >/dev/null

awk '{count[$1] += 1} END {for (p in count) printf "%s %d\n", p, count[p]}' "$tmp_platform_list" | sort

echo
echo "Audit complete. Full build log: $log_file"
