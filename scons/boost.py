# vi: syntax=python:et:ts=4
from config_check_utils import *
from os.path import join
from glob import glob

def find_boost(env):
    include = find_include([env["prefix"]], "boost/config.hpp", "")
    if include:
        prefix, includefile = include[0]
        env["boostdir"] = join(prefix, "include")
        env["boostlibdir"] = join(prefix, "lib")
        if not env.get("boost_suffix"):
            if glob(join(prefix, "lib", "libboost_*-mt.*")):
                env["boost_suffix"] = "-mt"
            else:
                env["boost_suffix"] = ""
        return

def CheckBoost(context, boost_lib, require_version = None):
    env = context.env
    if require_version:
        context.Message("Checking for Boost %s library version >= %s... " % (boost_lib, require_version))
    else:
        context.Message("Checking for Boost %s library... " % boost_lib)

    if not env.get("boostdir", "") and not env.get("boostlibdir", ""):
        find_boost(env)
    boostdir = env.get("boostdir", "")
    boostlibdir = env.get("boostlibdir", "")
    backup = env.Clone().Dictionary()

    boost_headers = { "regex" : "regex/config.hpp",
                      "iostreams" : "iostreams/constants.hpp",
                      "unit_test_framework" : "test/unit_test.hpp" }
    header_name = boost_headers.get(boost_lib, boost_lib + ".hpp")
    libname = "boost_" + boost_lib + env.get("boost_suffix", "")

    if env["fast"]:
        env.AppendUnique(CXXFLAGS = "-I" + boostdir, LIBPATH = [boostlibdir])
    else:
        env.AppendUnique(CPPPATH = [boostdir], LIBPATH = [boostlibdir])
    env.AppendUnique(LIBS = [libname])

    test_program = """
        #include <boost/%s>
        \n""" % header_name
    if require_version:
        version = require_version.split(".", 2)
        major = int(version[0])
        minor = int(version[1])
        try:
            sub_minor = int(version[2])
        except (ValueError, IndexError):
            sub_minor = 0
        test_program += "#include <boost/version.hpp>\n"
        test_program += \
            "#if BOOST_VERSION < %d\n#error Boost version is too old!\n#endif\n" \
            % (major * 100000 + minor * 100 + sub_minor)
    test_program += """
        int main()
        {
        }
        \n"""
    if context.TryLink(test_program, ".cpp"):
        context.Result("yes")
        return True
    else:
        context.Result("no")
        env.Replace(**backup)
        return False

config_checks = { "CheckBoost" : CheckBoost }
