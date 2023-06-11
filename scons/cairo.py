import sys
from os import environ
from os.path import join
from SCons.Util import AppendPath

def CheckCairo(context, min_version):
    context.Message("Checking for Cairo... ")
    env = context.env

    try:
        version_arg = env["ESCAPE"](" >= ") + min_version
        env.ParseConfig("pkg-config --libs --cflags $PKG_CONFIG_FLAGS cairo" + version_arg)
        context.Result("yes")
        return True
    except OSError:
        context.Result("no")
        return False

config_checks = { "CheckCairo" : CheckCairo }
