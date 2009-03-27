# vi: syntax=python:et:ts=4

import os

def run_pkg_config(context, name):
    env = context.env
    try:
        env["ENV"]["PKG_CONFIG_PATH"] = os.environ.get("PKG_CONFIG_PATH")
        env.ParseConfig("pkg-config --libs --cflags --silence-errors $PKGCONFIG_FLAGS \"" + name + "\"")
        context.Log("Found '" + name + "' with pkg-config.\n")
        return True
    except OSError:
        context.Log("Failed to find '" + name + "' with pkg-config.\n")
        return False

def CheckPKG(context, name):
    env = context.env
    context.Message( 'Checking for %s... ' % name )
    if run_pkg_config(context, name):
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

config_checks = { "CheckPKG" : CheckPKG }
