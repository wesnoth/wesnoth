# vi: syntax=python:et:ts=4
from pkgconfig import run_pkg_config

def CheckLua(context, require_version):
    env = context.env
    backup = env.Clone().Dictionary()

    context.Message("Checking for Lua development files version " + require_version + "... ")

    version = ".".join(require_version.split(".")[0:2])

    if env.get("luadir"):
        env.Append(LIBPATH = ["$luadir"], CPPPATH = ["$luadir/include"], LIBS = "lua" + version)
        found = True
    else:
        found = run_pkg_config(env, "lua >= " + require_version) or run_pkg_config(env, "lua" + version + " >= " + require_version)

    result = found and context.TryLink("""
    #include <lualib.h>
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
