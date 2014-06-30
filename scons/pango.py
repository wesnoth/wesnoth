# vi: syntax=python:et:ts=4
import os, sys
from os.path import join
from os import environ
from SCons.Util import AppendPath

def CheckPango(context, backend, require_version = None):
    context.Message("Checking for Pango with " + backend + " backend... ")
    env = context.env
    gtkdir = env.get("gtkdir", os.environ.get("GTK_BASEPATH"))
    if gtkdir:
        environ["PATH"] = AppendPath(environ["PATH"], join(gtkdir, "bin"))
        environ["PKG_CONFIG_PATH"] = AppendPath(environ.get("PKG_CONFIG_PATH", ""), join(gtkdir, "lib/pkgconfig"))
        if sys.platform != "win32":
            env["PKGCONFIG_FLAGS"] = "--define-variable=prefix=" + gtkdir

    try:
        env["ENV"]["PKG_CONFIG_PATH"] = environ.get("PKG_CONFIG_PATH")
        version_arg = ""
        if require_version:
            version_arg = env["ESCAPE"](" >= ") + require_version
        env.ParseConfig("pkg-config --libs --cflags $PKGCONFIG_FLAGS pango" + backend + version_arg)
        context.Result("yes")
        return True
    except OSError:
        context.Result("no")
        return False

config_checks = { "CheckPango" : CheckPango }
