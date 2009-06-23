# vi: syntax=python:et:ts=4
from pkgconfig import run_pkg_config
from config_check_utils import find_include

from os.path import join

def CheckLua(context, require_version):
    env = context.env
    backup = env.Clone().Dictionary()

    context.Message("Checking for Lua development files version " + require_version + "... ")

    version = ".".join(require_version.split(".")[0:2])

    if env.get("luadir"):
        env.Append(LIBPATH = ["$luadir"], CPPPATH = ["$luadir/include"], LIBS = "lua" + version)
        found = True
    else:
        found = run_pkg_config(context, "lua >= " + require_version) or run_pkg_config(context, "lua" + version + " >= " + require_version) or run_pkg_config(context, "lua-" + version + " >= " + require_version)
        if not found:
            try:
                prefix, include = find_include([env["prefix"]], "lualib.h", "", not env["host"])[0]
                found = True
                context.Log("Found Lua header " + include + ".\n")
                env.Append(LIBPATH = [join(prefix, "lib")], CPPPATH = [join(prefix, "include")], LIBS = ["lua"])
            except IndexError:
                pass

    result = found and context.TryLink("""
    #include <lualib.h>
    #include <lauxlib.h>
    int main() { luaL_newstate(); }
    """, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        env.Replace(**backup)
        return False

config_checks = { "CheckLua" : CheckLua }
