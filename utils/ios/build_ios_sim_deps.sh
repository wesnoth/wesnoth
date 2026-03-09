#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
IOS_TAG="${IOS_TAG:-v0.0.1}"
IOS_TRIPLET="${IOS_TRIPLET:-arm64-ios-simulator-wesnoth}"
IOS_DEPLOYMENT_TARGET="${IOS_DEPLOYMENT_TARGET:-15.0}"

MANIFEST_ROOT="$ROOT_DIR/utils/ios/vcpkg"
IOS_DEPS_BASE="$ROOT_DIR/projectfiles/Xcode/temp/iOSCompileStuff-${IOS_TAG}"
VCPKG_INSTALL_ROOT="$IOS_DEPS_BASE/vcpkg_installed"
IOS_PREFIX="$IOS_DEPS_BASE/${IOS_TRIPLET}"
VCPKG_ROOT="${VCPKG_ROOT:-$ROOT_DIR/projectfiles/Xcode/temp/vcpkg-codex-ios}"

OPENSSL_VERSION="${OPENSSL_VERSION:-3.6.1}"
OPENSSL_TARBALL="$IOS_DEPS_BASE/downloads/openssl-${OPENSSL_VERSION}.tar.gz"
OPENSSL_SRC_ROOT="$IOS_DEPS_BASE/build"
IOS_ACTIVATE_XCODE_LINKS="${IOS_ACTIVATE_XCODE_LINKS:-1}"
IOS_REBUILD_OPENSSL="${IOS_REBUILD_OPENSSL:-0}"

need_cmd() {
	if ! command -v "$1" >/dev/null 2>&1; then
		echo "Error: missing required command: $1" >&2
		exit 1
	fi
}

need_cmd git
need_cmd cmake
need_cmd curl
need_cmd perl
need_cmd make
need_cmd xcrun

mkdir -p "$IOS_DEPS_BASE" "$IOS_DEPS_BASE/downloads" "$IOS_DEPS_BASE/build"

if [[ ! -x "$VCPKG_ROOT/vcpkg" ]]; then
	echo "==> Bootstrapping vcpkg at $VCPKG_ROOT"
	rm -rf "$VCPKG_ROOT"
	git clone --depth 1 https://github.com/microsoft/vcpkg "$VCPKG_ROOT"
	"$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
fi

# Some manifests reference historical trees outside a depth-1 clone.
if [[ "$(git -C "$VCPKG_ROOT" rev-parse --is-shallow-repository 2>/dev/null || echo false)" == "true" ]]; then
	echo "==> Expanding shallow vcpkg clone for versioned port checkout"
	git -C "$VCPKG_ROOT" fetch --unshallow
fi

echo "==> Installing iOS simulator dependencies via vcpkg"
"$VCPKG_ROOT/vcpkg" install \
	--x-manifest-root="$MANIFEST_ROOT" \
	--triplet="$IOS_TRIPLET" \
	--x-install-root="$VCPKG_INSTALL_ROOT" \
	--overlay-triplets="$MANIFEST_ROOT/triplets" \
	--overlay-ports="$MANIFEST_ROOT/overlay-ports"

mkdir -p "$IOS_PREFIX/include" "$IOS_PREFIX/lib"

# Sync vcpkg-managed headers/libs into the Xcode iOS dependency prefix.
rsync -a "$VCPKG_INSTALL_ROOT/$IOS_TRIPLET/include/" "$IOS_PREFIX/include/"
rsync -a "$VCPKG_INSTALL_ROOT/$IOS_TRIPLET/lib/" "$IOS_PREFIX/lib/"

SDK_PATH="$(xcrun --sdk iphonesimulator --show-sdk-path)"
CPU_COUNT="$(sysctl -n hw.ncpu 2>/dev/null || echo 8)"

if [[ "$IOS_REBUILD_OPENSSL" == "1" || ! -f "$IOS_PREFIX/lib/libssl.a" || ! -f "$IOS_PREFIX/lib/libcrypto.a" ]]; then
	if [[ ! -f "$OPENSSL_TARBALL" ]]; then
		echo "==> Downloading OpenSSL ${OPENSSL_VERSION}"
		curl -L "https://github.com/openssl/openssl/archive/refs/tags/openssl-${OPENSSL_VERSION}.tar.gz" -o "$OPENSSL_TARBALL"
	fi

	OPENSSL_TOPDIR="$(tar -tzf "$OPENSSL_TARBALL" | head -n 1 | cut -d/ -f1)"
	OPENSSL_SRC_DIR="$OPENSSL_SRC_ROOT/$OPENSSL_TOPDIR"

	rm -rf "$OPENSSL_SRC_DIR"
	tar -xzf "$OPENSSL_TARBALL" -C "$OPENSSL_SRC_ROOT"

	echo "==> Building OpenSSL ${OPENSSL_VERSION} for iOS simulator"
	(
		cd "$OPENSSL_SRC_DIR"
		export CC="$(xcrun --sdk iphonesimulator -f clang)"
		export CFLAGS="-arch arm64 -isysroot $SDK_PATH -mios-simulator-version-min=${IOS_DEPLOYMENT_TARGET}"
		export CXXFLAGS="$CFLAGS"
		export LDFLAGS="$CFLAGS"
		perl ./Configure \
			ios64-xcrun \
			no-ui \
			no-asm \
			no-shared \
			no-module \
			no-apps \
			no-tests \
			no-docs \
			--openssldir=/etc/ssl \
			--libdir=lib \
			--prefix="$IOS_PREFIX"
		make -j"$CPU_COUNT" build_sw
		make install_sw
	)
else
	echo "==> Reusing existing OpenSSL from $IOS_PREFIX/lib"
fi

# Framework file refs in the legacy Xcode project still point at .dylib filenames.
# Generate compatibility symlinks to static archives for iOS simulator linking.
EMPTY_ARCHIVE="$IOS_PREFIX/lib/libwesnoth-empty.a"
if [[ ! -f "$EMPTY_ARCHIVE" ]]; then
	EMPTY_C="$IOS_DEPS_BASE/wesnoth-empty.c"
	EMPTY_O="$IOS_DEPS_BASE/wesnoth-empty.o"
	echo "void wesnoth_empty_symbol(void) {}" >"$EMPTY_C"
	"$(xcrun --sdk iphonesimulator -f clang)" \
		-arch arm64 \
		-isysroot "$SDK_PATH" \
		-mios-simulator-version-min="${IOS_DEPLOYMENT_TARGET}" \
		-c "$EMPTY_C" \
		-o "$EMPTY_O"
	ar -rcs "$EMPTY_ARCHIVE" "$EMPTY_O"
	rm -f "$EMPTY_C" "$EMPTY_O"
fi

create_dylib_shim() {
	local dylib_name="$1"
	local archive_name="$2"
	if [[ ! -f "$IOS_PREFIX/lib/$archive_name" ]]; then
		echo "Warning: missing $archive_name for shim $dylib_name, using empty archive" >&2
		archive_name="$(basename "$EMPTY_ARCHIVE")"
	fi
	ln -sfn "$archive_name" "$IOS_PREFIX/lib/$dylib_name"
}

create_dylib_shim "libboost_atomic-mt.dylib" "libboost_atomic.a"
create_dylib_shim "libboost_charconv-mt.dylib" "libboost_charconv.a"
create_dylib_shim "libboost_chrono-mt.dylib" "libboost_chrono.a"
create_dylib_shim "libboost_context-mt.dylib" "libboost_context.a"
create_dylib_shim "libboost_coroutine-mt.dylib" "libboost_coroutine.a"
create_dylib_shim "libboost_filesystem-mt.dylib" "libboost_filesystem.a"
create_dylib_shim "libboost_iostreams-mt.dylib" "libboost_iostreams.a"
create_dylib_shim "libboost_locale-mt.dylib" "libboost_locale.a"
create_dylib_shim "libboost_prg_exec_monitor-mt.dylib" "libboost_prg_exec_monitor.a"
create_dylib_shim "libboost_program_options-mt.dylib" "libboost_program_options.a"
create_dylib_shim "libboost_random-mt.dylib" "libboost_random.a"
create_dylib_shim "libboost_regex-mt.dylib" "libboost_regex.a"
create_dylib_shim "libboost_system-mt.dylib" "libboost_system.a"
create_dylib_shim "libboost_thread-mt.dylib" "libboost_thread.a"
create_dylib_shim "libboost_timer-mt.dylib" "libboost_timer.a"
create_dylib_shim "libboost_unit_test_framework-mt.dylib" "libboost_unit_test_framework.a"
create_dylib_shim "libbz2.1.0.dylib" "libbz2.a"
create_dylib_shim "libcairo.2.dylib" "libcairo.a"
create_dylib_shim "libcrypto.1.1.dylib" "libcrypto.a"
create_dylib_shim "libexpat.1.dylib" "libexpat.a"
create_dylib_shim "libffi.8.dylib" "libffi.a"
create_dylib_shim "libfontconfig.1.dylib" "libfontconfig.a"
create_dylib_shim "libfreetype.6.dylib" "libfreetype.a"
create_dylib_shim "libfribidi.0.dylib" "libfribidi.a"
create_dylib_shim "libgio-2.0.0.dylib" "libgio-2.0.a"
create_dylib_shim "libglib-2.0.0.dylib" "libglib-2.0.a"
create_dylib_shim "libgmodule-2.0.0.dylib" "libgmodule-2.0.a"
create_dylib_shim "libgobject-2.0.0.dylib" "libgobject-2.0.a"
create_dylib_shim "libgraphite2.3.dylib" "libgraphite2.a"
create_dylib_shim "libgthread-2.0.0.dylib" "libgthread-2.0.a"
create_dylib_shim "libharfbuzz.0.dylib" "libharfbuzz.a"
create_dylib_shim "libiconv.2.dylib" "libiconv.a"
create_dylib_shim "libintl.8.dylib" "libintl.a"
create_dylib_shim "libogg.0.dylib" "libogg.a"
create_dylib_shim "libpango-1.0.0.dylib" "libpango-1.0.a"
create_dylib_shim "libpangocairo-1.0.0.dylib" "libpangocairo-1.0.a"
create_dylib_shim "libpangoft2-1.0.0.dylib" "libpangoft2-1.0.a"
create_dylib_shim "libpcre.1.dylib" "libpcre.a"
create_dylib_shim "libpcre2-8.0.dylib" "libpcre2-8.a"
create_dylib_shim "libpixman-1.0.dylib" "libpixman-1.a"
create_dylib_shim "libpng16.16.dylib" "libpng16.a"
create_dylib_shim "libreadline.8.2.dylib" "libreadline.a"
create_dylib_shim "libSDL2-2.0.0.dylib" "libSDL2.a"
create_dylib_shim "libSDL2_image-2.0.0.dylib" "libSDL2_image.a"
create_dylib_shim "libSDL2_mixer-2.0.0.dylib" "libSDL2_mixer.a"
create_dylib_shim "libsharpyuv.0.dylib" "libsharpyuv.a"
create_dylib_shim "libssl.1.1.dylib" "libssl.a"
create_dylib_shim "libvorbis.0.dylib" "libvorbis.a"
create_dylib_shim "libvorbisfile.dylib" "libvorbisfile.a"
create_dylib_shim "libwebp.7.dylib" "libwebp.a"
create_dylib_shim "libwebpdemux.2.dylib" "libwebpdemux.a"
create_dylib_shim "libz.1.dylib" "libz.a"

echo "==> Linking convenience iOS include/lib symlinks"
ln -sfn "temp/iOSCompileStuff-${IOS_TAG}/${IOS_TRIPLET}/include" "$ROOT_DIR/projectfiles/Xcode/Headers.iOS"
ln -sfn "temp/iOSCompileStuff-${IOS_TAG}/${IOS_TRIPLET}/lib" "$ROOT_DIR/projectfiles/Xcode/lib.iOS"

if [[ "$IOS_ACTIVATE_XCODE_LINKS" == "1" ]]; then
	echo "==> Activating iOS dependency symlinks for the Xcode project"
	ln -sfn "temp/iOSCompileStuff-${IOS_TAG}/${IOS_TRIPLET}/include" "$ROOT_DIR/projectfiles/Xcode/Headers"
	ln -sfn "temp/iOSCompileStuff-${IOS_TAG}/${IOS_TRIPLET}/lib" "$ROOT_DIR/projectfiles/Xcode/lib"
fi

echo "==> iOS simulator dependency prefix ready: $IOS_PREFIX"
