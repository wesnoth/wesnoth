# vi: syntax=python:et:ts=4
from config_check_utils import *

def CheckBoostLib(context, boost_lib, require_version = None):
    env = context.env
    boostdir = env.get("boostdir", "/usr/include")
    boostlibdir = env.get("boostlibdir", "/usr/lib")
    backup = backup_env(env, ["CPPPATH", "LIBPATH", "LIBS"])

    boost_headers = { "regex" : "regex/config.hpp",
                      "iostreams" : "iostreams/constants.hpp",
                      "unit_test_framework" : "test/unit_test.hpp" }
    header_name = boost_headers.get(boost_lib, boost_lib + ".hpp")
    libname = "boost_" + boost_lib + env.get("boost_suffix", "")

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
        return True
    else:
        restore_env(env, backup)
        return False

def CheckBoost(context, boost_lib, require_version = None):
    if require_version:
        context.Message("Checking for Boost %s library version >= %s... " % (boost_lib, require_version))
    else:
        context.Message("Checking for Boost %s library... " % boost_lib)
    check_result = CheckBoostLib(context, boost_lib, require_version)
    if not check_result and not context.env.get("boost_suffix"):
        context.env["boost_suffix"] = "-mt"
        check_result = CheckBoostLib(context, boost_lib, require_version)
    if check_result:
        context.Result("yes")
    else:
        context.Result("no")
    return check_result

config_checks = { "CheckBoost" : CheckBoost }
