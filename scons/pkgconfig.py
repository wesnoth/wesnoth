# vi: syntax=python:et:ts=4

import os

def CheckPKG(context, name):
    env = context.env
    context.Message( 'Checking for %s... ' % name )
    try:
        env["ENV"]["PKG_CONFIG_PATH"] = os.environ.get("PKG_CONFIG_PATH")
        env.ParseConfig("pkg-config --libs --cflags $PKGCONFIG_FLAGS '" + name + "'")
        context.Result("yes")
        return True
    except OSError:
        context.Result("no")
        return False

config_checks = { "CheckPKG" : CheckPKG }
