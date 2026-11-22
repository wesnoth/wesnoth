set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME iOS)
set(VCPKG_OSX_SYSROOT iphonesimulator)

# Force autotools ports to treat iOS simulator as cross-compilation.
# Exclude OpenSSL because it uses a custom Perl Configure script instead of autotools.
if(NOT PORT STREQUAL "openssl")
	set(VCPKG_MAKE_BUILD_TRIPLET "--build=aarch64-apple-darwin" "--host=aarch64-apple-iossimulator")
endif()
# Static libintl.a on Apple platforms uses CoreFoundation for locale detection,
# so link CoreFoundation globally to prevent undefined _CF* symbols in downstream builds.
set(VCPKG_LINKER_FLAGS "${VCPKG_LINKER_FLAGS} -framework CoreFoundation")
