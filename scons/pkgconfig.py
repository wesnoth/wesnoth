# vi: syntax=python:et:ts=4

def CheckPKG(context, name):
    env = context.env
    context.Message( 'Checking for %s... ' % name )
    try:
        env.ParseConfig("pkg-config --libs --cflags $PKGCONFIG_FLAGS " + name)
        context.Result("yes")
        return True
    except OSError:
        context.Result("no")
        return False

config_checks = { "CheckPKG" : CheckPKG }
