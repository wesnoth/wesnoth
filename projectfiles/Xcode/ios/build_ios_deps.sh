#!/bin/sh

root_dir=$(cd "$(dirname "$0")/../../.." && pwd)
ios_tag=${IOS_TAG:-v0.0.1}
ios_deployment_target=${IOS_DEPLOYMENT_TARGET:-15.0}

ios_sim_triplet=${IOS_SIM_TRIPLET:-arm64-ios-simulator-wesnoth}
ios_device_triplet=${IOS_DEVICE_TRIPLET:-arm64-ios-wesnoth}
ios_build_simulator_deps=${IOS_BUILD_SIMULATOR_DEPS:-1}
ios_build_device_deps=${IOS_BUILD_DEVICE_DEPS:-1}

manifest_root="$root_dir/projectfiles/Xcode/ios/vcpkg"
ios_deps_base="$root_dir/projectfiles/Xcode/temp/iOSCompileStuff-${ios_tag}"
vcpkg_install_base="$ios_deps_base/vcpkg_installed"
vcpkg_root=${VCPKG_ROOT:-$root_dir/projectfiles/Xcode/temp/vcpkg-ios}
overlay_ports_root="$ios_deps_base/overlay-ports"
gettext_overlay_port="$overlay_ports_root/gettext-libintl"
gettext_port_patch="$manifest_root/patches/gettext-libintl-ios.patch"

sim_prefix="$ios_deps_base/${ios_sim_triplet}"
device_prefix="$ios_deps_base/${ios_device_triplet}"
ios_activate_xcode_links=${IOS_ACTIVATE_XCODE_LINKS:-1}

need_cmd() {
	if ! command -v "$1" >/dev/null 2>&1; then
		echo "Error: missing required command: $1" >&2
		exit 1
	fi
}

prepare_gettext_overlay() {
	echo "==> Preparing gettext-libintl overlay from the current vcpkg port"
	rm -rf "$gettext_overlay_port"
	mkdir -p "$overlay_ports_root"
	rsync -a "$vcpkg_root/ports/gettext-libintl/" "$gettext_overlay_port/"
	git -C "$gettext_overlay_port" apply "$gettext_port_patch"
}

install_triplet() {
	triplet_name=$1
	install_root="$vcpkg_install_base/$triplet_name"
	echo "==> Installing iOS dependencies via vcpkg for $triplet_name"
	"$vcpkg_root/vcpkg" install \
		--x-manifest-root="$manifest_root" \
		--triplet="$triplet_name" \
		--x-install-root="$install_root" \
		--overlay-triplets="$manifest_root/triplets" \
		--overlay-ports="$overlay_ports_root"
}

sync_prefix_from_vcpkg() {
	triplet_name=$1
	prefix_dir=$2
	install_root="$vcpkg_install_base/$triplet_name"

	mkdir -p "$prefix_dir/include" "$prefix_dir/lib"

	# Sync vcpkg-managed headers/libs into the Xcode iOS dependency prefix.
	rsync -a --delete "$install_root/$triplet_name/include/" "$prefix_dir/include/"
	rsync -a --delete "$install_root/$triplet_name/lib/" "$prefix_dir/lib/"
}

create_empty_archive_for_prefix() {
	prefix_dir=$1
	sdk_name=$2
	min_flag=$3

	empty_archive="$prefix_dir/lib/libwesnoth-empty.a"
	if [ ! -f "$empty_archive" ]; then
		sdk_path=$(xcrun --sdk "$sdk_name" --show-sdk-path)
		safe_triplet=$(basename "$prefix_dir")
		empty_c="$ios_deps_base/wesnoth-empty-${safe_triplet}.c"
		empty_o="$ios_deps_base/wesnoth-empty-${safe_triplet}.o"
		echo "void wesnoth_empty_symbol(void) {}" >"$empty_c"
		"$(xcrun --sdk "$sdk_name" -f clang)" \
			-arch arm64 \
			-isysroot "$sdk_path" \
			"$min_flag" \
			-c "$empty_c" \
			-o "$empty_o"
		ar -rcs "$empty_archive" "$empty_o"
		rm -f "$empty_c" "$empty_o"
	fi
	echo "$empty_archive"
}

create_dylib_shim() {
	prefix_dir=$1
	sdk_name=$2
	min_flag=$3
	empty_archive_name=$4
	dylib_name=$5
	archive_name=$6

	archive_path="$prefix_dir/lib/$archive_name"
	if [ ! -f "$archive_path" ]; then
		echo "Warning: missing $archive_name for shim $dylib_name, using empty archive" >&2
		archive_path="$prefix_dir/lib/$empty_archive_name"
	fi

	sdk_path=$(xcrun --sdk "$sdk_name" --show-sdk-path)
	clang_bin=$(xcrun --sdk "$sdk_name" -f clang)

	# Generate a real dylib shim so Xcode copy-framework steps can bitcode-strip it.
	"$clang_bin" \
		-dynamiclib \
		-arch arm64 \
		-isysroot "$sdk_path" \
		"$min_flag" \
		-Wl,-all_load \
		"$archive_path" \
		-Wl,-undefined,dynamic_lookup \
		-install_name "@rpath/$dylib_name" \
		-o "$prefix_dir/lib/$dylib_name"
}

create_all_shims_for_prefix() {
	prefix_dir=$1
	sdk_name=$2
	min_flag=$3
	empty_archive_name=$(basename "$4")

	# The existing Xcode project file still has Frameworks / Copy Frameworks refs
	# that point at explicit .dylib filenames. Generate compatibility dylibs from
	# the static archives for those refs.
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_atomic-mt.dylib" "libboost_atomic.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_charconv-mt.dylib" "libboost_charconv.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_chrono-mt.dylib" "libboost_chrono.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_context-mt.dylib" "libboost_context.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_coroutine-mt.dylib" "libboost_coroutine.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_filesystem-mt.dylib" "libboost_filesystem.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_iostreams-mt.dylib" "libboost_iostreams.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_locale-mt.dylib" "libboost_locale.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_prg_exec_monitor-mt.dylib" "libboost_prg_exec_monitor.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_program_options-mt.dylib" "libboost_program_options.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_random-mt.dylib" "libboost_random.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_regex-mt.dylib" "libboost_regex.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_system-mt.dylib" "libboost_system.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_thread-mt.dylib" "libboost_thread.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_timer-mt.dylib" "libboost_timer.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libboost_unit_test_framework-mt.dylib" "libboost_unit_test_framework.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libbz2.1.0.dylib" "libbz2.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libcairo.2.dylib" "libcairo.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libcrypto.1.1.dylib" "libcrypto.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libexpat.1.dylib" "libexpat.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libffi.8.dylib" "libffi.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libfontconfig.1.dylib" "libfontconfig.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libfreetype.6.dylib" "libfreetype.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libfribidi.0.dylib" "libfribidi.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libgio-2.0.0.dylib" "libgio-2.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libglib-2.0.0.dylib" "libglib-2.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libgmodule-2.0.0.dylib" "libgmodule-2.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libgobject-2.0.0.dylib" "libgobject-2.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libgraphite2.3.dylib" "libgraphite2.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libgthread-2.0.0.dylib" "libgthread-2.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libharfbuzz.0.dylib" "libharfbuzz.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libiconv.2.dylib" "libiconv.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libintl.8.dylib" "libintl.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libogg.0.dylib" "libogg.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libpango-1.0.0.dylib" "libpango-1.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libpangocairo-1.0.0.dylib" "libpangocairo-1.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libpangoft2-1.0.0.dylib" "libpangoft2-1.0.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libpcre.1.dylib" "libpcre.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libpcre2-8.0.dylib" "libpcre2-8.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libpixman-1.0.dylib" "libpixman-1.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libpng16.16.dylib" "libpng16.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libreadline.8.2.dylib" "libreadline.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libSDL2-2.0.0.dylib" "libSDL2.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libSDL2_image-2.0.0.dylib" "libSDL2_image.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libSDL2_mixer-2.0.0.dylib" "libSDL2_mixer.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libsharpyuv.0.dylib" "libsharpyuv.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libssl.1.1.dylib" "libssl.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libvorbis.0.dylib" "libvorbis.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libvorbisfile.dylib" "libvorbisfile.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libwebp.7.dylib" "libwebp.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libwebpdemux.2.dylib" "libwebpdemux.a"
	create_dylib_shim "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive_name" "libz.1.dylib" "libz.a"
}

build_prefix() {
	triplet_name=$1
	prefix_dir=$2
	sdk_name=$3
	min_flag=$4

	install_triplet "$triplet_name"
	sync_prefix_from_vcpkg "$triplet_name" "$prefix_dir"

	empty_archive=$(create_empty_archive_for_prefix "$prefix_dir" "$sdk_name" "$min_flag")
	create_all_shims_for_prefix "$prefix_dir" "$sdk_name" "$min_flag" "$empty_archive"

	echo "==> iOS dependency prefix ready: $prefix_dir"
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

prepare_gettext_overlay

if [ "$ios_build_simulator_deps" = "1" ]; then
	build_prefix "$ios_sim_triplet" "$sim_prefix" "iphonesimulator" "-mios-simulator-version-min=${ios_deployment_target}"
fi

if [ "$ios_build_device_deps" = "1" ]; then
	build_prefix "$ios_device_triplet" "$device_prefix" "iphoneos" "-miphoneos-version-min=${ios_deployment_target}"
fi

if [ "$ios_build_simulator_deps" = "1" ]; then
	echo "==> Linking convenience simulator include/lib symlinks"
	ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_sim_triplet}/include" "$root_dir/projectfiles/Xcode/Headers.iOS"
	ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_sim_triplet}/lib" "$root_dir/projectfiles/Xcode/lib.iOS"
fi

if [ "$ios_build_device_deps" = "1" ]; then
	echo "==> Linking convenience device include/lib symlinks"
	ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_device_triplet}/include" "$root_dir/projectfiles/Xcode/Headers.device"
	ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_device_triplet}/lib" "$root_dir/projectfiles/Xcode/lib.device"
fi

if [ "$ios_activate_xcode_links" = "1" ]; then
	echo "==> Activating iOS dependency symlinks for the Xcode project"
	if [ "$ios_build_simulator_deps" = "1" ]; then
		ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_sim_triplet}/include" "$root_dir/projectfiles/Xcode/Headers"
		ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_sim_triplet}/lib" "$root_dir/projectfiles/Xcode/lib"
	elif [ "$ios_build_device_deps" = "1" ]; then
		ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_device_triplet}/include" "$root_dir/projectfiles/Xcode/Headers"
		ln -sfn "temp/iOSCompileStuff-${ios_tag}/${ios_device_triplet}/lib" "$root_dir/projectfiles/Xcode/lib"
	fi
fi

echo "==> Done. Built triplets:"
[ "$ios_build_simulator_deps" = "1" ] && echo "    - $ios_sim_triplet"
[ "$ios_build_device_deps" = "1" ] && echo "    - $ios_device_triplet"
