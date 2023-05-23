# vi: syntax=python:et:ts=4
import json

#Intended usage:
#- env["ndkdir"] set to ndk home
#- env["android_api"] set to target api
#- host needs to be set to android-$ABI where $ABI is one of keys in 
#    ndkdir/meta/abis.json
#- prefix needs to be set to sysroot containing cross-compiled 
#    dependencies for matching android abi

def exists(env):
    return True

def generate(env):
    api = env["android_api"]
    ndk = env["ndkdir"]

    assert ndk

    android, abi = env["host"].split("-", 1)
    assert android == "android"

    abi_spec = json.load(open(ndk + "/meta/abis.json"))[abi]
    print(abi_spec)
    env["ANDROID_TOOLCHAIN"] = f"{ndk}/toolchains/llvm/prebuilt/linux-x86_64"
    env["AR"] = "$ANDROID_TOOLCHAIN/bin/llvm-ar"
    env["CC"] = "$ANDROID_TOOLCHAIN/bin/clang"
    env["CXX"] = "$ANDROID_TOOLCHAIN/bin/clang++"
    #env["LINK"] = "$ANDROID_TOOLCHAIN/bin/ld"
    env["RANLIB"] = "$ANDROID_TOOLCHAIN/bin/llvm-ranlib"
    env["ANDROID_LLVM_TRIPLE"] = abi_spec["llvm_triple"]
    env.Append(CCFLAGS = "-target $ANDROID_LLVM_TRIPLE$android_api")
    env.Append(LINKFLAGS = "-target $ANDROID_LLVM_TRIPLE$android_api")
    env.Append(LIBS = ["android", "log", "GLESv1_CM", "GLESv2"])
    env.Append(CPPDEFINES = ["SDL_MAIN_HANDLED"])
