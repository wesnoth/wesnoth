# vi: syntax=python:et:ts=4
#
# SCons build description for the Wesnoth project
#
# Prerequisites are:
# 1. autorevision for getting the repository revision level.
# 2. msgfmt(1) for making builds with i18n support.
# 3. graph-includes for making the project dependency graph.

EnsureSConsVersion(0,98,3)

import os, sys, shutil, re, subprocess
from glob import glob
from subprocess import Popen, PIPE, call
from os import access, F_OK

# Warn user of current set of build options.
AddOption('--option-cache', dest='option_cache', nargs=1, type = 'string', action = 'store', metavar = 'FILE', help='file with cached construction variables', default = '.scons-option-cache')
if GetOption("option_cache") != "" and os.path.exists(GetOption("option_cache")):
    optfile = open(GetOption("option_cache"))
    print("Saved options: {}".format(optfile.read().replace("\n", ", ")[:-2]))
    optfile.close()

#
# Get the Wesnoth version number
#

config_h_re = re.compile(r"^.*#define\s*(\S*)\s*\"(\S*)\".*$", re.MULTILINE)
build_config = dict( config_h_re.findall(File("src/wesconfig.h").get_contents().decode("utf-8")) )
try:
    version = build_config["VERSION"]
    print("Building Wesnoth version %s" % version)
except KeyError:
    print("Couldn't determine the Wesnoth version number, bailing out!")
    sys.exit(1)

#
# Build-control options
#

opts = Variables(GetOption("option_cache"))

def OptionalPath(key, val, env):
    if val:
        PathVariable.PathIsDir(key, val, env)

opts.AddVariables(
    ListVariable('default_targets', 'Targets that will be built if no target is specified in command line.',
        "wesnoth,wesnothd", Split("wesnoth wesnothd campaignd boost_unit_tests")),
    EnumVariable('build', 'Build variant: release, debug, or profile', "release", ["release", "debug", "profile"]),
    PathVariable('build_dir', 'Build all intermediate files(objects, test programs, etc) under this dir', "build", PathVariable.PathAccept),
    ('extra_flags_config', "Extra compiler and linker flags to use for configuration and all builds. Whether they're compiler or linker is determined by env.ParseFlags. Unknown flags are compile flags by default. This applies to all extra_flags_* variables", ""),
    ('extra_flags_release', 'Extra compiler and linker flags to use for release builds', ""),
    ('extra_flags_debug', 'Extra compiler and linker flags to use for debug builds', ""),
    ('extra_flags_profile', 'Extra compiler and linker flags to use for profile builds', ""),
    BoolVariable('enable_lto', 'Whether to enable Link Time Optimization for build=release', False),
    ('arch', 'What -march option to use for build=release, will default to pentiumpro on Windows', ""),
    BoolVariable('glibcxx_debug', 'Whether to define _GLIBCXX_DEBUG and _GLIBCXX_DEBUG_PEDANTIC for build=debug', False),
    EnumVariable('profiler', 'profiler to be used for build=profile', "gprof", ["gprof", "gcov", "gperftools", "perf"]),
    EnumVariable('pgo_data', 'whether to generate profiling data for PGO, or use existing profiling data', "", ["", "generate", "use"]),
    BoolVariable('use_srcdir', 'Whether to place object files in src/ or not', False),
    PathVariable('bindir', 'Where to install binaries', "bin", PathVariable.PathAccept),
    ('cachedir', 'Directory that contains a cache of derived files.', ''),
    PathVariable('datadir', 'read-only architecture-independent game data', "$datarootdir/$datadirname", PathVariable.PathAccept),
    PathVariable('fifodir', 'directory for the wesnothd fifo socket file', "/var/run/wesnothd", PathVariable.PathAccept),
    BoolVariable('fribidi','Clear to disable bidirectional-language support', True),
    BoolVariable('desktop_entry','Clear to disable desktop-entry', True),
    BoolVariable('systemd','Install systemd unit file for wesnothd', bool(WhereIs("systemd"))),
    PathVariable('datarootdir', 'sets the root of data directories to a non-default location', "share", PathVariable.PathAccept),
    PathVariable('datadirname', 'sets the name of data directory', "wesnoth$version_suffix", PathVariable.PathAccept),
    PathVariable('desktopdir', 'sets the desktop entry directory to a non-default location', "$datarootdir/applications", PathVariable.PathAccept),
    PathVariable('icondir', 'sets the icons directory to a non-default location', "$datarootdir/icons", PathVariable.PathAccept),
    BoolVariable('internal_data', 'Set to put data in Mac OS X application fork', False),
    PathVariable('localedirname', 'sets the locale data directory to a non-default location', "translations", PathVariable.PathAccept),
    PathVariable('mandir', 'sets the man pages directory to a non-default location', "$datarootdir/man", PathVariable.PathAccept),
    PathVariable('docdir', 'sets the doc directory to a non-default location', "$datarootdir/doc/wesnoth", PathVariable.PathAccept),
    PathVariable('python_site_packages_dir', 'sets the directory where python modules are installed', "lib/python/site-packages/wesnoth", PathVariable.PathAccept),
    BoolVariable('notifications', 'Enable support for desktop notifications', True),
    BoolVariable('nls','enable compile/install of gettext message catalogs',True),
    BoolVariable('png', 'Clear to disable writing png files for screenshots, images', True),
    PathVariable('prefix', 'autotools-style installation prefix', "/usr/local", PathVariable.PathAccept),
    PathVariable('prefsdir', 'user preferences directory', "", PathVariable.PathAccept),
    PathVariable('default_prefs_file', 'default preferences file name', "", PathVariable.PathAccept),
    PathVariable('destdir', 'prefix to add to all installation paths.', "/", PathVariable.PathAccept),
    BoolVariable('prereqs','abort if prerequisites cannot be detected',True),
    ('program_suffix', 'suffix to append to names of installed programs',"$version_suffix"),
    ('version_suffix', 'suffix that will be added to default values of prefsdir, program_suffix and datadirname', ""),
    BoolVariable('forum_user_handler', 'Enable forum user handler in wesnothd', False),
    ('server_gid', 'group id of the user who runs wesnothd', ""),
    ('server_uid', 'user id of the user who runs wesnothd', ""),
    BoolVariable('strict', 'Set to strict compilation', False),
    BoolVariable('static_test', 'Staticaly build against boost test (Not supported yet)', False),
    BoolVariable('verbose', 'Emit progress messages during data installation.', False),
    PathVariable('sdldir', 'Directory of SDL installation.', "", OptionalPath),
    PathVariable('boostdir', 'Directory of boost installation.', "", OptionalPath),
    PathVariable('boostlibdir', 'Directory where boost libraries are installed.', "", OptionalPath),
    ('boost_suffix', 'Suffix of boost libraries.'),
    PathVariable('gettextdir', 'Root directory of Gettext\'s installation.', "", OptionalPath), 
    PathVariable('gtkdir', 'Directory where GTK SDK is installed.', "", OptionalPath),
    PathVariable('luadir', 'Directory where Lua binary package is unpacked.', "", OptionalPath),
    ('host', 'Cross-compile host.', ''),
    EnumVariable('multilib_arch', 'Address model for multilib compiler: 32-bit or 64-bit', "", ["", "32", "64"]),
    ('jobs', 'Set the number of parallel compilations', "1", lambda key, value, env: int(value), int),
    BoolVariable('distcc', 'Use distcc', False),
    BoolVariable('ccache', "Use ccache", False),
    ('ctool', 'Set c compiler command if not using standard compiler.'),
    ('cxxtool', 'Set c++ compiler command if not using standard compiler.'),
    EnumVariable('cxx_std', 'Target c++ std version', '11', ['11', '14', '1z']),
    BoolVariable('openmp', 'Enable openmp use.', False),
    ('sanitize', 'Enable clang and GCC sanitizer functionality. A comma separated list of sanitize suboptions must be passed as value.', ''),
    BoolVariable("fast", "Make scons faster at cost of less precise dependency tracking.", False),
    BoolVariable("lockfile", "Create a lockfile to prevent multiple instances of scons from being run at the same time on this working copy.", False),
    BoolVariable("OS_ENV", "Forward the entire OS environment to scons", False),
    BoolVariable("history", "Clear to disable GNU history support in lua console", True)
    )

#
# Setup
#

toolpath = ["scons"]
for repo in Dir(".").repositories:
  # SCons repositories are additional dirs to look for source and lib files.
  # It is possible to make out of tree builds by running SCons outside of this
  # source code root and supplying this path with -Y option.
  toolpath.append(repo.abspath + "/scons")
sys.path = toolpath + sys.path
env = Environment(tools=["tar", "gettext_tool", "install", "python_devel", "scanreplace"], options = opts, toolpath = toolpath)

if env["lockfile"]:
    print("Creating lockfile")
    lockfile = os.path.abspath("scons.lock")
    open(lockfile, "wx").write(str(os.getpid()))
    import atexit
    atexit.register(os.remove, lockfile)

if GetOption("option_cache") != "":
    opts.Save(GetOption("option_cache"), env)
env.SConsignFile("$build_dir/sconsign.dblite")

# If OS_ENV was enabled, copy the entire OS environment.
if env['OS_ENV']:
    env['ENV'] = os.environ

# Make sure the user's environment is always available
env['ENV']['PATH'] = os.environ.get("PATH")
term = os.environ.get('TERM')
if term is not None:
    env['ENV']['TERM'] = term

if env["PLATFORM"] == "win32":
    env.Tool("mingw")
else:
    from cross_compile import *
    setup_cross_compile(env)

env.Tool("system_include")

if 'HOME' in os.environ:
    env['ENV']['HOME'] = os.environ['HOME']

if env.get('ctool',""):
    env['CC'] = env['ctool']
    env['CXX'] = env['ctool'].rstrip("cc") + "++"

if env.get('cxxtool',""):
    env['CXX'] = env['cxxtool']

if env['jobs'] > 1:
    SetOption("num_jobs", env['jobs'])
else:
    env['jobs'] = 1

if env['distcc']: 
    env.Tool('distcc')

if env['ccache']: env.Tool('ccache')

if 'TRAVIS' in os.environ:
    SDL2_version = '2.0.2'
else:
    SDL2_version = '2.0.4'

boost_version = '1.50.0'


Help("""Arguments may be a mixture of switches and targets in any order.
Switches apply to the entire build regardless of where they are in the order.
Important switches include:

    prefix=/usr     probably what you want for production tools
    use_srcdir=yes  build directly in the distribution root;
                    you'll need this if you're trying to use Emacs compile mode.
    build=release   build the release build variant with appropriate flags
                        in build/release and copy resulting binaries
                        into distribution/working copy root.
    build=debug     same for debug build variant
                    binaries will be copied with -debug suffix
    build=profile   build with instrumentation for a supported profiler
                    binaries will be copied with -profile suffix

With no arguments, the recipe builds wesnoth and wesnothd.  Available
build targets include the individual binaries:

    wesnoth wesnothd campaignd boost_unit_tests

You can make the following special build targets:

    all = wesnoth wesnothd campaignd boost_unit_tests (*).
    TAGS = build tags for Emacs (*).
    wesnoth-deps.png = project dependency graph
    install = install all executables that currently exist, and any data needed
    install-wesnothd = install the Wesnoth multiplayer server.
    install-campaignd = install the Wesnoth campaign server.
    install-pytools = install all Python tools and modules
    uninstall = uninstall all executables, tools, modules, and servers.
    pot-update = generate gettext message catalog templates and merge them with localized message catalogs
    update-po = merge message catalog templates with localized message catalogs for particular lingua
    update-po4a = update translations of manual and manpages
    af bg ca ... = linguas for update-po
    dist = make distribution tarball as wesnoth.tar.bz2 (*).
    data-dist = make data tarball as wesnoth-data.tar.bz2 (*).
    binary-dist = make data tarball as wesnoth-binaries.tar.bz2 (*).
    wesnoth-bundle = make Mac OS application bundle from game (*)
    windows-installer = create Windows distribution with NSIS (*)
    sanity-check = run a pre-release sanity check on the distribution.
    manual = regenerate English-language manual and, possibly, localized manuals if appropriate xmls exist.

Files made by targets marked with '(*)' are cleaned by 'scons -c all'

Options are cached in a file named .scons-option-cache and persist to later
invocations.  The file is editable.  Delete it to start fresh. You can also use a different file by
specifying --option-cache=FILE command line argument. Current option values can be listed with 'scons -h'.

If you set CXXFLAGS and/or LDFLAGS in the environment, the values will
be appended to the appropriate variables within scons.
""" + opts.GenerateHelpText(env))

if GetOption("help"):
    Return()

if env["cachedir"] and not env['ccache']:
    CacheDir(env["cachedir"])

if env["fast"]:
    env.Decider('MD5-timestamp')
    SetOption('max_drift', 1)
    SetOption('implicit_cache', 1)

if not os.path.isabs(env["prefix"]):
    print("Warning: prefix is set to relative dir. destdir setting will be ignored.")
    env["destdir"] = ""

#
# work around long command line problem on windows
# see http://www.scons.org/wiki/LongCmdLinesOnWin32
#
if sys.platform == 'win32':
    try:
        import win32file
        import win32event
        import win32process
        import win32security
        import string

        def my_spawn(sh, escape, cmd, args, spawnenv):
            for var in spawnenv:
                spawnenv[var] = spawnenv[var].encode('ascii', 'replace')

            sAttrs = win32security.SECURITY_ATTRIBUTES()
            StartupInfo = win32process.STARTUPINFO()
            newargs = ' '.join(map(escape, args[1:]))
            cmdline = cmd + " " + newargs

            # check for any special operating system commands
            if cmd == 'del':
                for arg in args[1:]:
                    win32file.DeleteFile(arg)
                exit_code = 0
            else:
                # otherwise execute the command.
                hProcess, hThread, dwPid, dwTid = win32process.CreateProcess(None, cmdline, None, None, 1, 0, spawnenv, None, StartupInfo)
                win32event.WaitForSingleObject(hProcess, win32event.INFINITE)
                exit_code = win32process.GetExitCodeProcess(hProcess)
                win32file.CloseHandle(hProcess)
                win32file.CloseHandle(hThread)
            return exit_code

        env['SPAWN'] = my_spawn
    except ImportError:
        def subprocess_spawn(sh, escape, cmd, args, env):
            return call(' '.join(args))
        env['SPAWN'] = subprocess_spawn

#
# Check some preconditions
#
print("---[checking prerequisites]---")

def Info(message):
    print("INFO: " + message)
    return True

def Warning(message):
    print("WARNING: " + message)
    return False

from metasconf import init_metasconf
configure_args = dict(
    custom_tests = init_metasconf(env, ["ieee_754", "cplusplus", "python_devel", "sdl", "boost", "cairo", "pango", "pkgconfig", "gettext_tool", "lua"]),
    config_h = "$build_dir/config.h",
    log_file="$build_dir/config.log", conf_dir="$build_dir/sconf_temp")

env.MergeFlags(env["extra_flags_config"])

if env["multilib_arch"]:
    multilib_flag = "-m" + env["multilib_arch"]
    env.AppendUnique(CCFLAGS = [multilib_flag], LINKFLAGS = [multilib_flag])

# Some tests need to load parts of boost
env.PrependENVPath('LD_LIBRARY_PATH', env["boostlibdir"])

# Some tests require at least C++11
if "gcc" in env["TOOLS"]:
    env.AppendUnique(CCFLAGS = Split("-W -Wall"), CFLAGS = ["-std=c99"])
    env.AppendUnique(CXXFLAGS = "-std=c++" + env["cxx_std"])

if env["prereqs"]:
    conf = env.Configure(**configure_args)

    if env["PLATFORM"] == "posix":
        conf.CheckCHeader("poll.h", "<>")
        conf.CheckCHeader("sys/poll.h", "<>")
        conf.CheckCHeader("sys/select.h", "<>")
        if conf.CheckCHeader("sys/sendfile.h", "<>"):
            conf.CheckFunc("sendfile")
    conf.CheckLib("m")
    conf.CheckFunc("round")

    def CheckIEEE754(conf):
        if not env["host"]:
            return conf.CheckIEEE754()
        else:
            Warning("You are cross-compiling. Skipping IEEE 754 test.")
            return True

    def CheckAsio(conf):
        if env["PLATFORM"] == 'win32':
            conf.env.Append(LIBS = ["libws2_32"])
            have_libpthread = True
        else:
            have_libpthread = conf.CheckLib("pthread")
        return have_libpthread & \
            conf.CheckBoost("system") & \
            conf.CheckBoost("asio", header_only = True)

    def have_sdl_other():
        return \
            conf.CheckSDL(require_version = SDL2_version) & \
            conf.CheckSDL("SDL2_ttf", header_file = "SDL_ttf") & \
            conf.CheckSDL("SDL2_mixer", header_file = "SDL_mixer") & \
            conf.CheckSDL("SDL2_image", header_file = "SDL_image")

    have_server_prereqs = (\
        CheckIEEE754(conf) & \
        conf.CheckCPlusPlus(gcc_version = "4.8") & \
        conf.CheckLib("libcrypto") & \
        conf.CheckBoost("iostreams", require_version = boost_version) & \
        conf.CheckBoostIostreamsGZip() & \
        conf.CheckBoostIostreamsBZip2() & \
        CheckAsio(conf) & \
        conf.CheckBoost("random", require_version = boost_version) & \
        conf.CheckBoost("smart_ptr", header_only = True) & \
        conf.CheckBoost("system") & \
        conf.CheckBoost("filesystem", require_version = boost_version) & \
        conf.CheckBoost("locale") \
            and Info("Base prerequisites are met")) \
            or Warning("Base prerequisites are not met")

    env = conf.Finish()

    client_env = env.Clone()
    conf = client_env.Configure(**configure_args)
    have_client_prereqs = have_server_prereqs & have_sdl_other() & \
        (('TRAVIS' in os.environ and os.environ["TRAVIS_OS_NAME"] == "osx") or (conf.CheckLib("vorbisfile") & \
        conf.CheckOgg())) & \
        conf.CheckPNG() & \
        conf.CheckJPG() & \
        conf.CheckCairo(min_version = "1.10") & \
        conf.CheckPango("cairo", require_version = "1.21.3") & \
        conf.CheckPKG("fontconfig") & \
        conf.CheckBoost("program_options", require_version = boost_version) & \
        conf.CheckBoost("thread") & \
        conf.CheckBoost("regex") \
            or Warning("Client prerequisites are not met. wesnoth cannot be built")

    have_X = False
    if have_client_prereqs:
        if env["PLATFORM"] != "win32":
            have_X = conf.CheckLib('X11')

        env["notifications"] = env["notifications"] and conf.CheckPKG("dbus-1")
        if env["notifications"]:
            client_env.Append(CPPDEFINES = ["HAVE_LIBDBUS"])

        client_env['fribidi'] = client_env['fribidi'] and (conf.CheckPKG('fribidi >= 0.10.9') or Warning("Can't find FriBiDi, disabling FriBiDi support."))
        if client_env['fribidi']:
            client_env.Append(CPPDEFINES = ["HAVE_FRIBIDI"])

        env["png"] = env["png"] and conf.CheckLib("png")
        if env["png"]:
            client_env.Append(CPPDEFINES = ["HAVE_LIBPNG"])

        env["history"] = env["history"] and (conf.CheckLib("history") or Warning("Can't find GNU history, disabling history support."))
        if env["history"]:
            client_env.Append(CPPDEFINES = ["HAVE_HISTORY"])

    if env["forum_user_handler"]:
        flags = env.ParseFlags("!mysql_config --libs --cflags")
        try: # Some versions of mysql_config add -DNDEBUG but we don't want it
            flags["CPPDEFINES"].remove("NDEBUG")
        except ValueError:
            pass
        env.Append(CPPDEFINES = ["HAVE_MYSQLPP"])
        env.MergeFlags(flags)

    client_env = conf.Finish()

    test_env = client_env.Clone()
    conf = test_env.Configure(**configure_args)

    have_test_prereqs = have_client_prereqs and conf.CheckBoost('unit_test_framework') \
                            or Warning("Unit tests are disabled because their prerequisites are not met")
    test_env = conf.Finish()
    if not have_test_prereqs and "boost_unit_tests" in env["default_targets"]:
        env["default_targets"].remove("boost_unit_tests")

    print("  " + env.subst("If any config checks fail, look in $build_dir/config.log for details"))
    print("  If a check fails spuriously due to caching, use --config=force to force its rerun")

else:
    have_client_prereqs = True
    have_X = False
    if env["PLATFORM"] != "win32":
        have_X = True
    have_server_prereqs = True
    have_test_prereqs = True
    test_env = env.Clone()
    client_env = env.Clone()


have_msgfmt = env["MSGFMT"]
if not have_msgfmt:
     env["nls"] = False
if not have_msgfmt:
     print("NLS tools are not present...")
if not env['nls']:
     print("NLS catalogue installation is disabled.")

#
# Implement configuration switches
#
print("---[applying configuration]---")

for env in [test_env, client_env, env]:
    build_root="#/"
    if os.path.isabs(env["build_dir"]):
        build_root = ""
    env.Prepend(CPPPATH = [build_root + "$build_dir", "#/src"])

    env.Append(CPPDEFINES = ["HAVE_CONFIG_H"])

    if "gcc" in env["TOOLS"]:
        if env['openmp']:
            env.AppendUnique(CXXFLAGS = ["-fopenmp"], LIBS = ["gomp"])

        if env['strict']:
            env.AppendUnique(CCFLAGS = Split("-Werror $(-Wno-unused-local-typedefs$) $(-Wno-maybe-uninitialized$)"))
            env.AppendUnique(CXXFLAGS = Split("-Wold-style-cast"))
        if env['sanitize']:
            env.AppendUnique(CCFLAGS = ["-fsanitize=" + env["sanitize"]], LINKFLAGS = ["-fsanitize=" + env["sanitize"]])
        
# #
# Start determining options for debug build
# #
        
        debug_flags = "-O0 -DDEBUG -ggdb3"
        
        if env["glibcxx_debug"] == True:
            glibcxx_debug_flags = "_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC"
        else:
            glibcxx_debug_flags = ""
        
# #
# End determining options for debug build
# Start setting options for release build
# #
        
# default compiler flags
        rel_comp_flags = "-O3"
        rel_link_flags = ""
        
# use the arch if provided, or if on Windows and no arch was passed in then default to pentiumpro
# without setting to pentiumpro, compiling on Windows with 64-bit tdm-gcc and -O3 currently fails
        if env["arch"]:
            env["arch"] = " -march=" + env["arch"]
        
        if env["PLATFORM"] == "win32" and not env["arch"]:
            env["arch"] = " -march=pentiumpro"
        
        rel_comp_flags = rel_comp_flags + env["arch"]
        
# PGO and LTO setup
        if env["CC"] == "gcc":
            if env["pgo_data"] == "generate":
                rel_comp_flags = rel_comp_flags + " -fprofile-generate=pgo_data/"
                rel_link_flags = "-fprofile-generate=pgo_data/"
            
            if env["pgo_data"] == "use":
                rel_comp_flags = rel_comp_flags + " -fprofile-correction -fprofile-use=pgo_data/"
                rel_link_flags = "-fprofile-correction -fprofile-use=pgo_data/"

            if env["enable_lto"] == True:
                rel_comp_flags = rel_comp_flags + " -flto=" + str(env["jobs"])
                rel_link_flags = rel_comp_flags + " -fuse-ld=gold"
        elif "clang" in env["CXX"]:
            if env["pgo_data"] == "generate":
                rel_comp_flags = rel_comp_flags + " -fprofile-instr-generate=pgo_data/wesnoth-%p.profraw"
                rel_link_flags = "-fprofile-instr-generate=pgo_data/wesnoth-%p.profraw"
            
            if env["pgo_data"] == "use":
                rel_comp_flags = rel_comp_flags + " -fprofile-instr-use=pgo_data/wesnoth.profdata"
                rel_link_flags = "-fprofile-instr-use=pgo_data/wesnoth.profdata"

            if env["enable_lto"] == True:
                rel_comp_flags = rel_comp_flags + " -flto=thin"
                rel_link_flags = rel_comp_flags + " -fuse-ld=lld"

# #
# End setting options for release build
# Start setting options for profile build
# #
        
        if env["profiler"] == "gprof":
            prof_comp_flags = "-pg"
            prof_link_flags = "-pg"
        
        if env["profiler"] == "gcov":
            prof_comp_flags = "-fprofile-arcs -ftest-coverage"
            prof_link_flags = "-fprofile-arcs"
        
        if env["profiler"] == "gperftools":
            prof_comp_flags = ""
            prof_link_flags = "-Wl,--no-as-needed,-lprofiler"
        
        if env["profiler"] == "perf":
            prof_comp_flags = "-ggdb -Og"
            prof_link_flags = ""
        
# #
# End setting options for profile build
# #

    if "clang" in env["CXX"]:
        # Silence warnings about unused -I options and unknown warning switches.
        env.AppendUnique(CCFLAGS = Split("-Qunused-arguments -Wno-unknown-warning-option -Werror=non-virtual-dtor"))

    if env['internal_data']:
        env.Append(CPPDEFINES = "USE_INTERNAL_DATA")

    if have_X:
        env.Append(CPPDEFINES = "_X11")

# Simulate autools-like behavior of prefix on various paths
    installdirs = Split("bindir datadir fifodir icondir desktopdir mandir docdir python_site_packages_dir")
    for d in installdirs:
        env[d] = os.path.join(env["prefix"], env[d])

    if env["PLATFORM"] == 'win32':
        env.Append(LIBS = ["wsock32", "iconv", "z", "shlwapi", "winmm"], CCFLAGS = ["-mthreads"], LINKFLAGS = ["-mthreads"], CPPDEFINES = ["_WIN32_WINNT=0x0501"])

    if env["PLATFORM"] == 'darwin':            # Mac OS X
        env.Append(FRAMEWORKS = "Cocoa")            # Cocoa GUI

if not env['static_test']:
    test_env.Append(CPPDEFINES = "BOOST_TEST_DYN_LINK")

try:
    if call(env.subst("utils/autorevision.sh -t h > $build_dir/revision.h"), shell=True) == 0:
        env["have_autorevision"] = True
        if not call(env.subst("cmp -s $build_dir/revision.h src/revision.h"), shell=True) == 0:
            call(env.subst("cp $build_dir/revision.h src/revision.h"), shell=True)
except:
    pass

Export(Split("env client_env test_env have_client_prereqs have_server_prereqs have_test_prereqs"))
SConscript(dirs = Split("po doc packaging/windows packaging/systemd"))

binaries = Split("wesnoth wesnothd campaignd boost_unit_tests")
builds = {
    "release" : dict(CCFLAGS = Split(rel_comp_flags) , LINKFLAGS  = Split(rel_link_flags)),
    "debug"   : dict(CCFLAGS = Split(debug_flags)    , CPPDEFINES = Split(glibcxx_debug_flags)),
    "profile" : dict(CCFLAGS = Split(prof_comp_flags), LINKFLAGS  = Split(prof_link_flags))
    }
build = env["build"]

for env in [test_env, client_env, env]:
    env.AppendUnique(**builds[build])
    env.Append(CXXFLAGS = Split(os.environ.get('CXXFLAGS', [])), LINKFLAGS = Split(os.environ.get('LDFLAGS', [])))
    env.MergeFlags(env["extra_flags_" + build])

if env["use_srcdir"] == True:
    build_dir = ""
else:
    build_dir = os.path.join("$build_dir", build)

if build == "release" : build_suffix = ""
else                  : build_suffix = "-" + build
Export("build_suffix")
env.SConscript("src/SConscript", variant_dir = build_dir, duplicate = False)
Import(binaries + ["sources"])
binary_nodes = [eval(binary) for binary in binaries]
all = env.Alias("all", [Alias(binary) for binary in binaries])
env.Default([Alias(target) for target in env["default_targets"]])

if have_client_prereqs and env["nls"]:
    env.Requires("wesnoth", Dir("translations"))

#
# clean out any PGO-related files
#

env.Clean(all, "pgo_data/")

#
# Utility productions (Unix-like systems only)
#

# Make a tags file for Emacs
# Exuberant Ctags doesn't understand the -l c++ flag so if the etags fails try the ctags version
env.Command("TAGS", sources, 'etags -l c++ $SOURCES.srcpath || (ctags --tag-relative=yes -f src/tags $SOURCES.srcpath)')
env.Clean(all, 'TAGS')

#
# Unix installation productions
#
# These will not be portable to Windows or Mac. They assume a Unix-like
# directory structure and FreeDesktop standard locations foicon, app,
# and doc files.
#

for d in installdirs:
    locals()[d] = os.path.join(env["destdir"], env[d].lstrip("/"))
installable_subs = Split('data fonts images sounds')
if os.path.isabs(env["localedirname"]):
    env["localedir"] = env["localedirname"]
else:
    env["localedir"] = "$datadir/$localedirname"
        
pythontools = Split("wmlscope wmllint wmlindent wesnoth_addon_manager")
pythonmodules = Split("wmltools.py wmlparser.py wmldata.py wmliterator.py campaignserver_client.py __init__.py")

def CopyFilter(fn):
    "Filter out data-tree things that shouldn't be installed."
    return not ".git" in str(fn) and not "Makefile" in str(fn)

env["copy_filter"] = CopyFilter

linguas = Split(File("po/LINGUAS").get_contents().decode("utf-8"))

def InstallManpages(env, component):
    env.InstallData("mandir", component, os.path.join("doc", "man", component + ".6"), "man6")
    for lingua in linguas:
        manpage = os.path.join("doc", "man", lingua, component + ".6")
        env.InstallData("mandir", component, manpage, os.path.join(lingua, "man6"))

# Now the actual installation productions

# The game and associated resources
env.InstallBinary(wesnoth)
env.InstallData("datadir", "wesnoth", [Dir(sub) for sub in installable_subs])
env.InstallData("docdir",  "wesnoth", [Glob("doc/manual/*.html"), Dir("doc/manual/styles"), Dir("doc/manual/images")])
if env["nls"]:
    env.InstallData("localedir", "wesnoth", Dir("translations"))
    env.InstallData("datadir", "wesnoth", "l10n-track")
InstallManpages(env, "wesnoth")
if have_client_prereqs and have_X and env["desktop_entry"]:
     if sys.platform == "darwin":
         env.InstallData("icondir", "wesnoth", "icons/wesnoth-icon-Mac.png")
     else:
         env.InstallData("icondir", "wesnoth", "icons/wesnoth-icon.png")
     env.InstallData("desktopdir", "wesnoth", "icons/wesnoth.desktop")

# Python tools
env.InstallData("bindir", "pytools", [os.path.join("data", "tools", tool) for tool in pythontools])
env.InstallData("python_site_packages_dir", "pytools", [os.path.join("data", "tools", "wesnoth", module) for module in pythonmodules])

# Wesnoth MP server install
env.InstallBinary(wesnothd)
InstallManpages(env, "wesnothd")
if not access(fifodir, F_OK):
    fifodir = env.Command(fifodir, [], [
        Mkdir(fifodir),
        Chmod(fifodir, 0o700),
        Action("chown %s:%s %s" %
               (env["server_uid"], env["server_gid"], fifodir)),
        ])
    AlwaysBuild(fifodir)
    env.Alias("install-wesnothd", fifodir)
if env["systemd"]:
    env.InstallData("prefix", "wesnothd", "#packaging/systemd/wesnothd.service", "lib/systemd/system")
    env.InstallData("prefix", "wesnothd", "#packaging/systemd/wesnothd.conf", "lib/tmpfiles.d")

# Wesnoth campaign server
env.InstallBinary(campaignd)

# Compute things for default install based on which targets have been created.
install = env.Alias('install', [])
for installable in ('wesnoth',
                    'wesnothd', 'campaignd'):
    if os.path.exists(installable + build_suffix) or installable in COMMAND_LINE_TARGETS or "all" in COMMAND_LINE_TARGETS:
        env.Alias('install', env.Alias('install-'+installable))

#
# Un-installation
#
def Uninstall(nodes):
    deletes = []
    for node in nodes:
        if node.__class__ == install[0].__class__:
            deletes.append(Uninstall(node.sources))
        else:
            deletes.append(Delete(str(node)))
    return deletes
uninstall = env.Command('uninstall', '', Flatten(Uninstall(Alias("install"))) or "")
env.AlwaysBuild(uninstall)
env.Precious(uninstall)

#
# Making a distribution tarball.
#
env["version"] = build_config.get("VERSION")
if 'dist' in COMMAND_LINE_TARGETS:    # Speedup, the manifest is expensive
    def dist_manifest():
        "Get an argument list suitable for passing to a distribution archiver."
        # Start by getting a list of all files under version control
        lst = subprocess.check_output("git ls-files | grep -v 'data\/test\/.*' | awk '/^[^?]/ {print $4;}'", shell=True).split()
        lst = filter(os.path.isfile, lst)
        return lst
    dist_tarball = env.Tar('wesnoth-${version}.tar.bz2', [])
    open("dist.manifest", "w").write("\n".join(dist_manifest() + ["src/revision.hpp"]))
    env.Append(TARFLAGS='-j -T dist.manifest --transform "s,^,wesnoth-$version/,"',
               TARCOMSTR="Making distribution tarball...")
    env.AlwaysBuild(dist_tarball)
    env.Clean(all, 'wesnoth.tar.bz2')
    env.Alias('dist', dist_tarball)

#
# Make binary distribution (from installed client side)
#
bin_tar_env = env.Clone()
bin_tarball = bin_tar_env.Tar('wesnoth-binary.tar.bz2',
                              os.path.join(bindir,"wesnoth"))
bin_tar_env.Append(TARFLAGS='-j', TARCOMSTR="Making binary tarball...")
env.Clean(all, 'wesnoth-binary.tar.bz2')
env.Alias('binary-dist', bin_tarball)

#
# Make data distribution (from installed client side)
#
data_tar_env = env.Clone()
data_tarball = data_tar_env.Tar('wesnoth-data.tar.bz2', datadir)
data_tar_env.Append(TARFLAGS='-j', TARCOMSTR="Making data tarball...")
env.Clean(all, 'wesnoth-data.tar.bz2')
env.Alias('data-dist', data_tarball)

#
# Windows installer
#

env.WindowsInstaller([
    wesnoth, wesnothd,
    Dir(installable_subs), env["nls"] and Dir("translations") or [],
    glob("*.dll")
    ])

#
# Making Mac OS X application bundles
#
env.Alias("wesnoth-bundle",
          env.Command("Wesnoth.app", "wesnoth", [
              Mkdir("${TARGET}/Contents"),
              Mkdir("${TARGET}/Contents/MacOS"),
              Mkdir("${TARGET}/Contents/Resources"),
              Action('echo "APPL????" > "${TARGET}/Contents/PkgInfo"'),
              Copy("${TARGET}/Contents/MacOS/Wesnoth", "wesnoth"),
              Copy("${TARGET}/Contents/MacOS/wesnothd", "wesnothd"),
              Copy("${TARGET}/Contents/Info.plist", "projectfiles/Xcode/Info.plist"),
              Action(r"""sed -i '' 's/\$[{].*[}]/Wesnoth/' "${TARGET}/Contents/Info.plist" """),
              Copy("${TARGET}/Contents/Resources/data", "data"),
              Copy("${TARGET}/Contents/Resources/English.lproj", "projectfiles/Xcode/English.lproj"),
              Copy("${TARGET}/Contents/Resources/fonts", "fonts"),
              Copy("${TARGET}/Contents/Resources/fonts.conf", "projectfiles/Xcode/Resources/fonts.conf"),
              Copy("${TARGET}/Contents/Resources/Growl Registration Ticket.growlRegDict", "projectfiles/Xcode/Resources/Growl Registration Ticket.growlRegDict"),
              Copy("${TARGET}/Contents/Resources/icon.icns", "projectfiles/Xcode/Resources/icon.icns"),
              Copy("${TARGET}/Contents/Resources/images", "images"),
              Copy("${TARGET}/Contents/Resources/SDLMain.nib", "projectfiles/Xcode/Resources/SDLMain.nib"),
              Copy("${TARGET}/Contents/Resources/sounds", "sounds"),
              Copy("${TARGET}/Contents/Resources/translations", "translations"),
              ]))
env.Clean(all, "Wesnoth.app")

#
# Sanity checking
#
sanity_check = env.Command('sanity-check', '', [
    Action("cd utils; ./sanity_check"),
    Action("cd data/tools; make sanity-check"),
    ])
env.AlwaysBuild(sanity_check)
env.Precious(sanity_check)

#
# Make the project dependency graph (requires graph-includes).
#
env.Command("wesnoth-deps.dot", [],
            "graph-includes -verbose --class wesnoth \
            -sysI /usr/include/c++/4.0 -sysI /usr/include -sysI /usr/include/SDL \
            --prefixstrip src/ -I src src > ${TARGET}")
env.Command("wesnoth-deps.png", "wesnoth-deps.dot",
            "dot -Tpng -o ${TARGET} ${SOURCE}")
env.Clean(all, ["wesnoth-deps.dot", "wesnoth-deps.png"])

# Local variables:
# mode: python
# end:
