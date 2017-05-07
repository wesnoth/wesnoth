import sys
from os import environ
from os.path import join
from SCons.Util import AppendPath

def CheckCairo(context, min_version):
    context.Message("Checking for Cairo... ")
    env = context.env

    gtkdir = env.get("gtkdir", environ.get("GTK_BASEPATH"))
    if gtkdir:
        environ["PATH"] = AppendPath(environ["PATH"], join(gtkdir, "bin"))
        environ["PKG_CONFIG_PATH"] = AppendPath(environ.get("PKG_CONFIG_PATH", ""), join(gtkdir, "lib/pkgconfig"))
        if sys.platform != "win32":
            env["PKGCONFIG_FLAGS"] = "--define-variable=prefix=" + gtkdir

    try:
        env["ENV"]["PKG_CONFIG_PATH"] = environ.get("PKG_CONFIG_PATH", "")
        version_arg = env["ESCAPE"](" >= ") + min_version
        env.ParseConfig("pkg-config --libs --cflags $PKGCONFIG_FLAGS cairo" + version_arg)
        context.Result("yes")
        return True
    except OSError:
        context.Result("no")
        return False

config_checks = { "CheckCairo" : CheckCairo }
