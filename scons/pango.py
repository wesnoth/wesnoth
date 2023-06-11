# vi: syntax=python:et:ts=4
import os, sys
from os.path import join
from os import environ
from SCons.Util import AppendPath

def CheckPango(context, backend, require_version = None):
    context.Message("Checking for Pango with " + backend + " backend... ")
    env = context.env

    try:
        version_arg = ""
        if require_version:
            version_arg = env["ESCAPE"](" >= ") + require_version
        env.ParseConfig("pkg-config --libs --cflags $PKG_CONFIG_FLAGS pango" + backend + version_arg)
        context.Result("yes")
        return True
    except OSError:
        context.Result("no")
        return False

config_checks = { "CheckPango" : CheckPango }
