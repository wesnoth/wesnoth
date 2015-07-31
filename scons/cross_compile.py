# vi: syntax=python:et:ts=4
import os

def setup_cross_compile(env):
    if "mingw" in env["host"]:
        env["PLATFORM"] = "win32"
        env["PROGSUFFIX"] = ".exe"
        env.Tool("mingw")
        env.Append(LINKFLAGS = ["-static-libgcc", "-static-libstdc++"])
    else:
        env.Tool("default")

    if env["host"]:
        tools = [
            "CXX",
            "CC",
            "AR",
            "RANLIB",
            "RC"
            ]
        for tool in tools:
            if env.has_key(tool):
                env[tool] = env["host"] + "-" + env[tool]

        env.PrependUnique(CPPPATH="$prefix/include", LIBPATH="$prefix/lib")
        if not env["sdldir"] and env["PLATFORM"] == "win32":
            env["sdldir"] = "$prefix"

        os.environ["PKG_CONFIG_LIBDIR"] = ''
        env["ENV"]["PKG_CONFIG_LIBDIR"] = ''
