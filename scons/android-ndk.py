# vi: syntax=python:et:ts=4
import json

from SCons.Builder import Builder

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
    env["ANDROID_ABI"] = abi
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

    assert env["android_home"]
    env["ENV"]["ANDROID_HOME"] = env["android_home"]
    env.AppendENVPath("PATH", env.subst("$android_home/build-tools/34.0.0/"))
    print(env["ENV"]["PATH"])
    env["DEX"] = "d8"
    env["AAPT"] = "aapt2"
    env["ANDROID_JAR"] = "$android_home/platforms/android-$android_api/android.jar"
    env.Append(JAVACLASSPATH = ["$ANDROID_JAR"])
    #env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1
    #env.Append(CCFLAGS = ["-fPIC", "-shared"])

    dex = Builder(
        action = "$DEX --lib $ANDROID_JAR --output $TARGET.dir --classpath $DEXCLASSPATH $SOURCES",
        suffix = ".dex"
        )
    env["BUILDERS"]["Dex"] = dex

    aapt_compile = Builder(
        action = "$AAPT compile $SOURCE -o $TARGET.dir",
        suffix = ".flat",
        single_source = True
        )
    env["BUILDERS"]["AaptCompile"] = aapt_compile

    aapt_link = Builder(
        action = "$AAPT link --proto-format -o $TARGET --manifest $SOURCES -I $JAVACLASSPATH --java $AAPTJAVADIR",
        suffix = ".apk"
    )
    env["BUILDERS"]["AaptLink"] = aapt_link
