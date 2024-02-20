# vi: syntax=python:et:ts=4
from config_check_utils import *
from os import environ
from os.path import join
import SCons.Conftest

# Based on SCons.SConf.CheckLibWithHeader()
def CheckLibsWithSystemHeader(context, libs, header, language) -> bool:
    res = SCons.Conftest.CheckLib(context, libs,
            header = "#include <%s>" % header, language = language)
    context.did_show_result = 1
    return not res

def CheckLua(context, require_version):
    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])
    version = require_version.split(".")
    major_version = version[0]
    minor_version = version[1]

    include_subdirs_raw = [
            "",
            "lua" + major_version + minor_version,
            "lua" + major_version + "." + minor_version,
            "lua-" + major_version + "." + minor_version,
            ]
    include_subdirs = []
    for include_subdir_raw in include_subdirs_raw:
        include_subdirs[len(include_subdirs):] = [include_subdir_raw,
                join("include", include_subdir_raw)]

    # Add new names here when found in distributions.
    # Debian since lua5.2 5.2.3-2 uses luaX.Y-c++:
    #   https://salsa.debian.org/lua-team/lua5.2/-/commit/fa2dc77c
    # Arch since lua 5.4.4-4 uses lua++-X.Y:
    #   https://gitlab.archlinux.org/archlinux/packaging/packages/lua/-/commit/4e97e19d
    libs = [
            "lua" + major_version + minor_version + "-c++",
            "lua" + major_version + "." + minor_version + "-c++",
            "lua-" + major_version + "." + minor_version + "-c++",
            "lua." + major_version + "." + minor_version + "-c++",
            "lua++" + major_version + minor_version,
            "lua++" + major_version + "." + minor_version,
            "lua++-" + major_version + "." + minor_version,
            "lua++." + major_version + "." + minor_version,
            ]

    luadir = context.env.get("luadir", environ.get("LUA_DIR"))
    if luadir:
        for include_subdir in include_subdirs:
            includes = find_include([luadir], "lua.h",
                    include_subdir=include_subdir, default_prefixes=False)
            for prefix, include in includes:
                context.env.Append(CPPPATH = [join(prefix, include_subdir)])
                if CheckLibsWithSystemHeader(context, libs, "lua.h", "CXX"):
                    return True
                restore_env(context.env, backup)
        return False

    for include_subdir in include_subdirs:
        includes = find_include([context.env["prefix"]], "lua.h",
                include_subdir=include_subdir, default_prefixes=True)
        for prefix, include in includes:
            context.env.Append(CPPPATH = [join(prefix, include_subdir)])
            if CheckLibsWithSystemHeader(context, libs, "lua.h", "CXX"):
                return True
            restore_env(context.env, backup)
    return False

config_checks = { "CheckLua" : CheckLua }
