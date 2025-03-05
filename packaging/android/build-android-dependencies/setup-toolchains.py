#!/usr/bin/env python3
import json
from subprocess import run
from pathlib import Path
from os import environ

api = environ.get("API", 28)
sdk = Path(environ.get("ANDROID_HOME", environ.get("ANDROID_SDK", "/opt/android-sdk-update-manager")))
ndk = Path(environ.get("ANDROID_NDK_ROOT", environ.get("ANDROID_NDK_HOME", sdk / "ndk/23.1.7779620")))

abis = json.load(open(ndk / "meta/abis.json"))
for abi, abi_data in abis.items():
    triple = abi_data["llvm_triple"]
    arch = abi_data["arch"]
    prefix = Path(environ.get("PREFIXDIR", "/tmp/android-prefix")) / abi
    prefix.mkdir(parents = True, exist_ok = True)
    with open(prefix / "android.env", "w") as envfile:
        envfile.write(f"""
export NDK={ndk}
export ANDROID_NDK_ROOT={ndk}
export ANDROID_ARCH={arch}
export BITNESS={abi_data["bitness"]}
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export PATH=$TOOLCHAIN/bin:$PATH

export TARGET={triple}
export HOST={abi_data["triple"]}

export API={api}

export AR=$TOOLCHAIN/bin/llvm-ar
export CC="$TOOLCHAIN/bin/clang -target $TARGET$API"
export AS=$CC
export CXX="$TOOLCHAIN/bin/clang++ -target $TARGET$API"
export LD=$TOOLCHAIN/bin/ld
export LDFLAGS=-Wl,--undefined-version
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip
export PKG_CONFIG=/usr/bin/pkg-config
export PKG_CONFIG_LIBDIR={prefix}/lib/pkgconfig
export PKG_CONFIG_PATH={prefix}/lib/pkgconfig
""")

    with open(prefix / "android.ini", "w") as crossfile:
        crossfile.write(f"""
[constants]
ndk_home = '{ndk}'
toolchain = ndk_home / 'toolchains/llvm/prebuilt/linux-x86_64'
target = '{triple}'
api = '{api}'
common_flags = ['-target', target + api]
prefix = '{prefix}'

[host_machine]
system = 'android'
cpu = '{arch}'
cpu_family = '{arch}'
endian = 'little'

[properties]
sys_root = '{prefix}'
pkg_config_libdir = '{prefix}' / 'lib/pkgconfig'

[built-in options]
libdir = prefix / 'lib'
includedir = prefix / 'include'
c_args = common_flags + ['-I' + includedir]
cpp_args = common_flags + ['-I' + includedir]
c_link_args = common_flags + ['-L' + libdir]
cpp_link_args = common_flags + ['-L' + libdir]

[binaries]
ar = toolchain / 'bin/llvm-ar'
c = toolchain / 'bin' / 'clang'
cxx = toolchain / 'bin' / 'clang++'
ld = toolchain / 'bin/ld'
ranlib = toolchain / 'bin/llvm-ranlib'
strip = toolchain / 'bin/llvm-strip'
""")
    with open(prefix / "android.jam", "w") as jamfile:
        jamfile.write(f"""
using clang : android
: {ndk}/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++
: <compileflags>"--target={triple}{api}" <linkflags>"--target={triple}{api}"
<compileflags>-I{prefix}/include <linkflags>-L{prefix}/lib
<compileflags>-DBOOST_ALL_NO_EMBEDDED_GDB_SCRIPTS
;
""")
