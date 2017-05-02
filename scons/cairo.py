from os import environ

def CheckCairo(context, min_version):
    context.Message("Checking for Cairo... ")
    env = context.env
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
