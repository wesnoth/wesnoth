#!/bin/sh

root_dir=$(cd "$(dirname "$0")/../../.." && pwd)
ios_tag=${IOS_TAG:-v0.0.1}
ios_triplet=${IOS_TRIPLET:-arm64-ios-simulator-wesnoth}
ios_deployment_target=${IOS_DEPLOYMENT_TARGET:-15.0}

manifest_root="$root_dir/projectfiles/Xcode/ios/vcpkg"
ios_deps_base="$root_dir/projectfiles/Xcode/temp/iOSCompileStuff-${ios_tag}"
vcpkg_install_root="$ios_deps_base/vcpkg_installed"
ios_prefix="$ios_deps_base/${ios_triplet}"
vcpkg_root=${VCPKG_ROOT:-$root_dir/projectfiles/Xcode/temp/vcpkg-ios}
overlay_ports_root="$ios_deps_base/overlay-ports"
gettext_overlay_port="$overlay_ports_root/gettext-libintl"
gettext_port_patch="$manifest_root/patches/gettext-libintl-ios.patch"

ios_activate_xcode_links=${IOS_ACTIVATE_XCODE_LINKS:-1}

need_cmd() {
	if ! command -v "$1" >/dev/null 2>&1; then
		echo "Error: missing required command: $1" >&2
		exit 1
	fi
}

create_dylib_shim() {
	dylib_name=$1
	archive_name=$2
	if [ ! -f "$ios_prefix/lib/$archive_name" ]; then
		echo "Warning: missing $archive_name for shim $dylib_name, using empty archive" >&2
		archive_name=$(basename "$empty_archive")
	fi
	ln -sfn "$archive_name" "$ios_prefix/lib/$dylib_name"
}

need_cmd git
need_cmd cmake
need_cmd rsync
need_cmd ar
need_cmd xcrun

mkdir -p "$ios_deps_base"

if [ ! -x "$vcpkg_root/vcpkg" ]; then
	echo "==> Bootstrapping vcpkg at $vcpkg_root"
	rm -rf "$vcpkg_root"
	git clone https://github.com/microsoft/vcpkg "$vcpkg_root"
	"$vcpkg_root/bootstrap-vcpkg.sh" -disableMetrics
fi

if [ ! -f "$gettext_port_patch" ]; then
	echo "Error: gettext overlay patch not found at: $gettext_port_patch" >&2
	exit 1
fi

echo "==> Preparing gettext-libintl overlay from the current vcpkg port"
rm -rf "$gettext_overlay_port"
mkdir -p "$overlay_ports_root"
rsync -a "$vcpkg_root/ports/gettext-libintl/" "$gettext_overlay_port/"
git -C "$gettext_overlay_port" apply "$gettext_port_patch"

echo "==> Installing iOS simulator dependencies via vcpkg"
"$vcpkg_root/vcpkg" install \
	--x-manifest-root="$manifest_root" \
	--triplet="$ios_triplet" \
	--x-install-root="$vcpkg_install_root" \
	--overlay-triplets="$manifest_root/triplets" \
	--overlay-ports="$overlay_ports_root"

mkdir -p "$ios_prefix/include" "$ios_prefix/lib"

# Sync vcpkg-managed headers/libs into the Xcode iOS dependency prefix.
rsync -a --delete "$vcpkg_install_root/$ios_triplet/include/" "$ios_prefix/include/"
rsync -a --delete "$vcpkg_install_root/$ios_triplet/lib/" "$ios_prefix/lib/"

sdk_path=$(xcrun --sdk iphonesimulator --show-sdk-path)

# Framework file refs in the legacy Xcode project still point at .dylib filenames.
# Generate compatibility symlinks to static archives for iOS simulator linking.
empty_archive="$ios_prefix/lib/libwesnoth-empty.a"
if [ ! -f "$empty_archive" ]; then
	empty_c="$ios_deps_base/wesnoth-empty.c"
	empty_o="$ios_deps_base/wesnoth-empty.o"
	echo "void wesnoth_empty_symbol(void) {}" >"$empty_c"
	"$(xcrun --sdk iphonesimulator -f clang)" \
		-arch arm64 \
		-isysroot "$sdk_path" \
		-mios-simulator-version-min="${ios_deployment_target}" \
		-c "$empty_c" \
		-o "$empty_o"
	ar -rcs "$empty_archive" "$empty_o"
	rm -f "$empty_c" "$empty_o"
fi

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
ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_triplet}/include" "$root_dir/projectfiles/Xcode/Headers.iOS"
ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_triplet}/lib" "$root_dir/projectfiles/Xcode/lib.iOS"

if [ "$ios_activate_xcode_links" = "1" ]; then
	echo "==> Activating iOS dependency symlinks for the Xcode project"
	ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_triplet}/include" "$root_dir/projectfiles/Xcode/Headers"
	ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_triplet}/lib" "$root_dir/projectfiles/Xcode/lib"
fi

echo "==> iOS simulator dependency prefix ready: $ios_prefix"
