# vi: syntax=python:et:ts=4
from config_check_utils import find_include
from os.path import join, dirname, basename, normpath
import sys
from glob import glob
import re

def find_boost(env):
    prefixes = [env["prefix"], "C:\\Boost"]
    crosscompile = env["host"]
    include = find_include(prefixes, "boost/config.hpp", default_prefixes=not crosscompile)
    if include:
        prefix, includefile = include[0]
        env["boostdir"] = join(prefix, "include")
        env["boostlibdir"] = join(prefix, "lib")
        if not env.get("boost_suffix"):
            for libdir in ["lib", "lib64"]:
                if glob(join(prefix, libdir, "libboost_*-mt.*")):
                    env["boost_suffix"] = "-mt"
                    env["boostlibdir"] = join(prefix, libdir)
                    break
            else:
                env["boost_suffix"] = ""
        return

    includes = find_include(prefixes, "boost/config.hpp", include_subdir="include/boost-*")
    if includes:
        versions = []
        for prefix, includefile in includes:
            try:
                testname = basename(dirname(dirname(includefile)))
                major, minor = re.findall(r"^boost-(\d*)_(\d*)$", testname)[0]
                versions.append((int(major), int(minor)))
            except IndexError:
                versions.append((0,0))
        version_nums = [100000 * major_minor[0] + 100 * major_minor[1] for major_minor in versions]
        include_index = version_nums.index(max(version_nums))
        prefix, includefile = includes[include_index]
        version = versions[include_index]
        env["boostdir"] = join(prefix, "include", "boost-" + str(version[0]) + "_" + str(version[1]))
        env["boostlibdir"] = join(prefix, "lib")
        if not env.get("boost_suffix"):
            libs = glob(join(prefix, "lib", "libboost_*"))
            for lib in libs:
                try:
                    env["boost_suffix"] = re.findall(r"libboost_\w*(-.*%d_%d)" % tuple(version), lib)[0]
                    break
                except:
                    pass

def CheckBoost(context, boost_lib, require_version = None, header_only = False):
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
                      "locale" : "locale/info.hpp",
                      "unit_test_framework" : "test/unit_test.hpp",
                      "filesystem" : "filesystem/operations.hpp",
                      "random" : "random/random_number_generator.hpp",
                      "system" : "system/error_code.hpp"}

    header_name = boost_headers.get(boost_lib, boost_lib + ".hpp")
    libname = "boost_" + boost_lib + env.get("boost_suffix", "")

    if sys.platform == "win32" or normpath(boostdir) != "/usr/include":
        if env["fast"]:
            env.AppendUnique(CXXFLAGS = ["-isystem", boostdir], LIBPATH = [boostlibdir])
        else:
            env.AppendUnique(CPPPATH = [boostdir], LIBPATH = [boostlibdir])
    if not header_only:
        env.PrependUnique(LIBS = [libname])
    if boost_lib == "thread" and env["PLATFORM"] == "posix":
        env.AppendUnique(CCFLAGS = ["-pthread"], LINKFLAGS = ["-pthread"])

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

    if boost_lib == "unit_test_framework":
        test_program += """
        boost::unit_test::test_suite* init_unit_test_suite ( int, char**)
        {
        }
        \n"""

    test_program += """
        // Workaround for sdl #defining main breaking non sdl programs
        #ifdef main
        #undef main
        #endif
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

def CheckBoostIostreamsGZip(context):
    env = context.env
    backup = env.Clone().Dictionary()

    context.Message("Checking for gzip support in Boost Iostreams... ")
    test_program = """
        #include <boost/iostreams/filtering_stream.hpp>
        #include <boost/iostreams/filter/gzip.hpp>

        int main()
        {
            boost::iostreams::filtering_stream<boost::iostreams::output> filter;
            filter.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params()));
        }
        \n"""

    for zlib in ["", "z"]:
        env.Append(LIBS = [zlib])
        comment = ""
        if zlib:
                comment = "        //Trying to link against '%s'.\n" % zlib
        if context.TryLink(comment + test_program, ".cpp"):
            context.Result("yes")
            return True
        else:
            env.Replace(**backup)

    context.Result("no")
    return False

def CheckBoostIostreamsBZip2(context):
    env = context.env
    backup = env.Clone().Dictionary()

    context.Message("Checking for bzip2 support in Boost Iostreams... ")
    test_program = """
        #include <boost/iostreams/filtering_stream.hpp>
        #include <boost/iostreams/filter/bzip2.hpp>

        int main()
        {
            boost::iostreams::filtering_stream<boost::iostreams::output> filter;
            filter.push(boost::iostreams::bzip2_compressor(boost::iostreams::bzip2_params()));
        }
        \n"""

    # bzip2 library name when it's statically compiled into Boost
    boostname = "boost_bzip2" + env.get("boost_suffix", "")

    for libbz2 in ["", "bz2", boostname]:
        env.Append(LIBS = [libbz2])
        comment = ""
        if libbz2:
                comment = "        //Trying to link against '%s'.\n" % libbz2
        if context.TryLink(comment + test_program, ".cpp"):
            context.Result("yes")
            return True
        else:
            env.Replace(**backup)

    context.Result("no")
    return False

def CheckBoostLocaleBackends(context, backends):
    env = context.env
    backup = env.Clone().Dictionary()

    context.Message("Checking for available Boost Locale backends... ")
    test_program = """
        #include <boost/locale/localization_backend.hpp>
        #include <iostream>
        #include <assert.h>

        int main()
        {
            auto manager { boost::locale::localization_backend_manager::global() };
            auto backends { manager.get_all_backends() };
            assert(backends.size() >= 1);
            for(auto backend : backends) {
                std::cout << backend << std::endl;
            }
        }
        \n"""

    if(env["PLATFORM"] != "win32"):
        env.Append(LIBS = ["icudata", "icui18n", "icuuc"])

    res, output = context.TryRun(test_program, ".cpp")
    result = False
    found_backends = "no"
    if res:
        supported_backends = output.splitlines()
        result = bool(set(backends).intersection(supported_backends))
        found_backends = ",".join(supported_backends)

    context.Result(found_backends)
    if result:
        return True
    else:
        env.Replace(**backup)

    return False

config_checks = { "CheckBoost" : CheckBoost, "CheckBoostIostreamsGZip" : CheckBoostIostreamsGZip, "CheckBoostIostreamsBZip2" : CheckBoostIostreamsBZip2, "CheckBoostLocaleBackends" : CheckBoostLocaleBackends }
