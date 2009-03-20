# vi: syntax=python:et:ts=4
import sys, os
from config_check_utils import *
import distutils.sysconfig

def exists():
    return True

def PythonExtension(env, target, source, **kv):
    return env.SharedLibrary(target, source, SHLIBPREFIX='', SHLIBSUFFIX=distutils.sysconfig.get_config_var("SO"), **kv)

def generate(env):
    env.AddMethod(PythonExtension)

def CheckPython(context):
    env = context.env
    backup = backup_env(env, ["CPPPATH", "LIBPATH", "LIBS"])
    context.Message("Checking for Python... ")
    env.AppendUnique(CPPPATH = distutils.sysconfig.get_python_inc())
    version = distutils.sysconfig.get_config_var("VERSION")
    if not version:
        version = sys.version[:3]
    if env["PLATFORM"] == "win32":
        version = version.replace('.', '')
    env.AppendUnique(LIBPATH = distutils.sysconfig.get_config_var("LIBDIR") or \
                               os.path.join(distutils.sysconfig.get_config_var("prefix"), "libs") )
    env.AppendUnique(LIBS = "python" + version)
    test_program = """
    #include <Python.h>
    int main()
    {
        Py_Initialize();
    }
    \n"""
    if context.TryLink(test_program, ".c"):
        context.Result("yes")
        return True
    else:
        context.Result("no")
        restore_env(context.env, backup)
        return False

config_checks = { "CheckPython" : CheckPython }
