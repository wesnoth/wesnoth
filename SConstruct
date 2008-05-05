# vi: syntax=python:et:ts=4
#
# SCons build description for the Wesnoth project
#
# Prerequisites are:
# 1. Subversion command-line client programs svnstatus and svnversion.
# 2. Unix file(1), on installation only.
# 3. Unix convert(1), on installation only, if using tinygui.
# 4. msgfmt(1) for making builds with i18n support.
# 5. graph-includes for making the project dependency graph.

import os, sys, shutil, sets, re, commands
from glob import glob
from subprocess import Popen, PIPE
from os import access, F_OK
from SCons.Script import *

# Warn user of current set of build options.
if os.path.exists('.scons-option-cache'):
    optfile = file('.scons-option-cache')
    print "Saved options:", optfile.read().replace("\n", ", ")[:-2]
    optfile.close()

#
# Build-control options
#

opts = Options('.scons-option-cache')

opts.AddOptions(
    ListOption('default_targets', 'Targets that will be built if no target is specified in command line.',
        "wesnoth,wesnothd", Split("wesnoth wesnothd wesnoth_editor campaignd cutter exploder")),
    PathOption('bindir', 'Where to install binaries', "bin", PathOption.PathAccept),
    ('cachedir', 'Directory that contains a cache of derived files.', ''),
    PathOption('datadir', 'read-only architecture-independent game data', "share/$datadirname", PathOption.PathAccept),
    BoolOption('debug', 'Set to build for debugging', False),
    BoolOption('dummy_locales','Set to enable Wesnoth private locales', False),
    PathOption('fifodir', 'directory for the wesnothd fifo socket file', "/var/run/wesnothd", PathOption.PathAccept),
    BoolOption('fribidi','Clear to disable bidirectional-language support', True),
    BoolOption('desktop_entry','Clear to disable desktop-entry', True),
    PathOption('datarootdir', 'sets the root of data directories to a non-default location', "share", PathOption.PathAccept),
    PathOption('datadirname', 'sets the name of data directory', "wesnoth", PathOption.PathAccept),
    PathOption('desktopdir', 'sets the desktop entry directory to a non-default location', "$datarootdir/applications", PathOption.PathAccept),
    PathOption('icondir', 'sets the icons directory to a non-default location', "$datarootdir/icons", PathOption.PathAccept),
    BoolOption('internal_data', 'Set to put data in Mac OS X application fork', False),
    PathOption('localedir', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept),
    PathOption('mandir', 'sets the man pages directory to a non-default location', "$datarootdir/man", PathOption.PathAccept),
    PathOption('docdir', 'sets the doc directory to a non-default location', "$datarootdir/doc/wesnoth", PathOption.PathAccept),
    BoolOption('lowmem', 'Set to reduce memory usage by removing extra functionality', False),
    BoolOption('nls','enable compile/install of gettext message catalogs',True),
    PathOption('prefix', 'autotools-style installation prefix', "/usr/local", PathOption.PathAccept),
    PathOption('prefsdir', 'user preferences directory', ".wesnoth", PathOption.PathAccept),
    PathOption('destdir', 'prefix to add to all installation paths.', "", PathOption.PathAccept),
    BoolOption('prereqs','abort if prerequisites cannot be detected',True),
    BoolOption('profile', 'Set to build for profiling', False),
    ('program_suffix', 'suffix to append to names of installed programs',""),
    BoolOption('python', 'Enable in-game python extensions.', True),
    BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False),
    ('server_gid', 'group id of the user who runs wesnothd', ""),
    ('server_uid', 'user id of the user who runs wesnothd', ""),
    EnumOption('gui', 'Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0) or 320x240 (PDAs)', "normal", ["normal", "small", "tiny"]),
    BoolOption('static', 'Set to enable static building of Wesnoth', False),
    BoolOption('strict', 'Set to strict compilation', False),
    BoolOption('verbose', 'Emit progress messages during data installation.', False),
    PathOption('sdldir', 'Directory of SDL installation.', "", PathOption.PathAccept),
    PathOption('boostdir', 'Directory of boost installation.', '/usr/include'),
    PathOption('boostlibdir', 'Directory where boost libraries are installed.', '/usr/lib'),
    ('boost_suffix', 'Suffix of boost libraries.'),
    PathOption('gettextdir', 'Root directory of Gettext\'s installation.', "", PathOption.PathAccept), 
    )

#
# Setup
#

env = Environment(tools=["tar"], options = opts)
if env["PLATFORM"] == "win32":
    env.Tool("mingw")
else:
    env.Tool("default")

opts.Save('.scons-option-cache', env)

Help("""Arguments may be a mixture of switches and targets an any order.
Switches apply to the entire build regrdless of where they are in the order.
Important switches include:

    prefix=/usr     probably what you want for production tools
    debug=yes       enable compiler and linker debugging switches

With no arguments, the recipe builds wesnoth and wesnothd.  Available
build targets include the individual binaries:

    wesnoth wesnoth_editor wesnothd campaignd exploder cutter test

You can make the following special build targets:

    all = wesnoth wesnoth_editor exploder cutter wesnothd campaignd (*).
    TAGS = build tags for Emacs (*).
    wesnoth-deps.png = project dependency graph
    install = install all executables that currently exist, and any data needed
    install-wesnothd = install the Wesnoth multiplayer server.
    install-campaignd = install the Wesnoth campaign server.
    install-cutter = install the castle cutter
    install-exploder = install the castle exploder
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
    wesnoth-editor-bundle = make Mac OS application bundle from editor (*).
    sanity-check = run a pre-release sanity check on the distribution.
    manual = regenerate English-language manual and, possibly, localized manuals if appropriate xmls exist.

Files made by targets marked '(*)' are cleaned by cleaned by 'scons -c all'

Options are cached in a file named .scons-option-cache and persist to later
invocations.  The file is editable. Delete it to start fresh.  Current option
values can be listed with 'scons -h'.

If you set CXXFLAGS and/or LDFLAGS in the environment, the values will
be appended to the appropriate variables within scons.  You can use this,
for example, to point scons at non-default library locations.
""" + opts.GenerateHelpText(env))

if env["cachedir"]:
    CacheDir(env["cachedir"])

#
# Generic pkg-config support is not presentluy used, but might be in the future
#

def CheckPKGConfig(context, version):
     context.Message( 'Checking for pkg-config... ' )
     ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
     context.Result( ret )
     return ret

def CheckPKG(context, name):
     context.Message( 'Checking for %s... ' % name )
     ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
     context.Result( ret )
     return ret

#
# Most of our required runtime support is from Boost SDL.
#

def backup_env(env, vars):
    backup = dict()
    for var in vars:
        backup[var] = env.get(var, [])
    return backup

def restore_env(env, backup):
    for var in backup.keys():
        env[var] = backup[var]

def CheckCPlusPlus(context, gcc_version = None):
    message = "Checking whether C++ compiler works "
    test_program = """
    #include <iostream>
    int main()
    {
    std::cout << "Hello, world\\n";
    }
    """
    if gcc_version and "gcc" in context.env["TOOLS"]:
        message += "(g++ version >= %s required)" % gcc_version
        import operator
        version = gcc_version.split(".", 3)
        version = map(int, version)
        version = map(lambda x,y: x or y, version, (0,0,0))
        multipliers = (10000, 100, 1)
        version_num = sum(map(operator.mul, version, multipliers))
        test_program += """
        #define GCC_VERSION (__GNUC__ * 10000 \\
                           + __GNUC_MINOR__ * 100 \\
                           + __GNUC_PATCHLEVEL__)
        #if GCC_VERSION < %d
        #error Compiler version is too old!
        #endif
        \n""" % version_num
    message += "... "
    context.Message(message)
    if context.TryBuild(context.env.Program, test_program, ".cpp") == 1 and context.lastTarget.get_contents() != "":
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

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

def CheckPython(context):
    env = context.env
    backup = backup_env(env, ["CPPPATH", "LIBPATH", "LIBS"])
    context.Message("Checking for Python... ")
    import distutils.sysconfig
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

def CheckSDL(context, sdl_lib = "SDL", require_version = None):
    if require_version:
        version = require_version.split(".", 2)
        major_version = int(version[0])
        minor_version = int(version[1])
        try:
            patchlevel    = int(version[2])
        except (ValueError, IndexError):
            patch_level = 0

    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])

    sdldir = context.env.get("sdldir", "")
    if sdl_lib == "SDL": 
        if require_version:
            context.Message("Checking for Simple DirectMedia Layer library version >= %d.%d.%d... " % (major_version, minor_version, patchlevel))
        else:
            context.Message("Checking for Simple DirectMedia Layer library... ")
        env = context.env
        if sdldir:
            env.AppendUnique(CPPPATH = [os.path.join(sdldir, "include/SDL")], LIBPATH = [os.path.join(sdldir, "lib")])
        else:
            for foo_config in [
                "pkg-config --cflags --libs sdl > $TARGET",
                "sdl-config --cflags --libs > $TARGET"
                ]:
                result, output = context.TryAction(foo_config)
                if result == 1:
                    env.MergeFlags(output)
                    break
        if env["PLATFORM"] == "win32":
            env.AppendUnique(CCFLAGS = ["-D_GNU_SOURCE"])
            env.AppendUnique(LIBS = Split("mingw32 SDLmain SDL"))
            env.AppendUnique(LINKFLAGS = ["-mwindows"])
    else:
        if require_version:
            context.Message("Checking for %s library version >= %d.%d.%d... " % (sdl_lib, major_version, minor_version, patchlevel))
        else:
            context.Message("Checking for %s library... " % sdl_lib)
        context.env.AppendUnique(LIBS = [sdl_lib])
    test_program = """
        #include <%s.h> 
        \n""" % sdl_lib
    if require_version:
        test_program += "#if SDL_VERSIONNUM(%s, %s, %s) < SDL_VERSIONNUM(%d, %d, %d)\n#error Library is too old!\n#endif\n" % \
            (sdl_lib.upper() + "_MAJOR_VERSION", \
             sdl_lib.upper() + "_MINOR_VERSION", \
             sdl_lib.upper() + "_PATCHLEVEL", \
             major_version, minor_version, patchlevel)
    test_program += """
        int main(int argc, char** argv)
        {
        }
        \n"""
    if context.TryLink(test_program, ".c"):
        context.Result("yes")
        return True
    else:
        context.Result("no")
        restore_env(context.env, backup)
        return False

def CheckOgg(context):
    test_program = '''
    #include <SDL_mixer.h>
    #include <stdlib.h>

    int main(int argc, char **argv)
    {
        Mix_Music* music = Mix_LoadMUS("data/core/music/main_menu.ogg");
        if (music == NULL) {
            exit(1);
        }
        exit(0);
    }
\n
'''
    #context.env.AppendUnique(LIBS = "SDL_mixer")
    context.Message("Checking for Ogg Vorbis support in SDL... ")
    (result, output) = context.TryRun(test_program, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

def CheckPNG(context):
    test_program = '''
    #include <SDL_image.h>
    #include <stdlib.h>

    int main(int argc, char **argv)
    {
            SDL_RWops *src;
            char *testimage = "images/buttons/button-pressed.png";

            src = SDL_RWFromFile(testimage, "rb");
            if (src == NULL) {
                    exit(2);
            }
            exit(!IMG_isPNG(src));
    }
\n
'''
    context.Message("Checking for PNG support in SDL... ")
    (result, output) = context.TryRun(test_program, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

#
# Check some preconditions
#

def Die(message):
    print message
    Exit(1)

def Warning(message):
    print message
    return False

conf = Configure(env, custom_tests = { 'CheckCPlusPlus' : CheckCPlusPlus,
                                       'CheckPython' : CheckPython,
                                       'CheckPKGConfig' : CheckPKGConfig,
                                       'CheckPKG' : CheckPKG,
                                       'CheckSDL' : CheckSDL,
                                       'CheckOgg' : CheckOgg,
                                       'CheckPNG' : CheckPNG,
                                       'CheckBoost' : CheckBoost })

if env["prereqs"]:
    if env["gettextdir"]:
        env.AppendUnique(CPPPATH = os.path.join(env["gettextdir"], "include"),
                         LIBPATH = os.path.join(env["gettextdir"], "lib"),
                         LIBS = ["intl"])
    conf.CheckCPlusPlus(gcc_version = "3.3") and \
    conf.CheckBoost("iostreams", require_version = "1.33.0") and \
    conf.CheckCHeader("libintl.h", "<>") and \
    conf.CheckSDL(require_version = '1.2.7') or Die("Base prerequisites are not met.")

    have_client_prereqs = \
        conf.CheckBoost("regex") and \
        conf.CheckSDL("SDL_ttf", require_version = "2.0.8") and \
        conf.CheckSDL("SDL_mixer", require_version = '1.2.0') and \
        conf.CheckSDL("SDL_image", require_version = '1.2.0') and \
        conf.CheckOgg() or Warning("Client prerequisites are not met. wesnoth, wesnoth_editor, cutter and exploder cannot be built.")

    if env["PLATFORM"] == "win32":
        have_X = True
    else:
        have_X = conf.CheckLib('X11') or Warning("wesnoth_editor cannot be built.")
    if env['fribidi']:
        env['fribidi'] = conf.CheckLibWithHeader('fribidi', 'fribidi/fribidi.h', 'C', 'fribidi_utf8_to_unicode(NULL,0,NULL);') or Warning("Can't find libfribidi, disabling freebidi support.")

    have_server_prereqs = conf.CheckSDL('SDL_net') or Warning("Server prerequisites are not met. wesnothd and campaignd cannot be built.")

    if env["python"]:
        env["python"] = (float(sys.version[:3]) >= 2.4) and conf.CheckPython() or Warning("Python >= 2.4 not found. Python extensions will be disabled.")
else:
    have_client_prereqs = True
    have_X = True
    have_server_prereqs = True

env.Append(CPPPATH = ["#/src"])

boost_test_dyn_link = boost_auto_test = False
if 'test' in COMMAND_LINE_TARGETS:
    boost_test_dyn_link = boost_auto_test = conf.CheckBoost('unit_test_framework')

have_msgfmt = env.WhereIs("msgfmt")
if not have_msgfmt:
     env["nls"] = False
if not have_msgfmt:
     print "NLS tools are not present..."
if not env['nls']:
     print "NLS catalogue installation is disabled."

env = conf.Finish()

#
# Implement configuration switches
#

# FIXME: Unix-specific.
# Link only on demand, so we don't need separate link lists for each binary
env.Append(LINKFLAGS = "-Wl,--as-needed")

env.Replace(CPPDEFINES = [])

if env["debug"]:
    env.AppendUnique(CXXFLAGS = Split("-O0 -DDEBUG -ggdb3 -W -Wall -ansi"))
else:
    if env["PLATFORM"] != "win32":
        env.AppendUnique(CXXFLAGS = Split("-O2 -ansi"))

if env['static']:
    env.AppendUnique(LINKFLAGS = "-all-static")

if env['profile']:
    env.AppendUnique(CXXFLAGS = "-pg")
    env.AppendUnique(LINKFLAGS = "-pg")

if env['strict']:
    env.AppendUnique(CXXFLAGS = Split("-Werror -Wno-unused -Wno-sign-compare"))

if env['gui'] == 'tiny':
    env.Append(CPPDEFINES = "USE_TINY_GUI")

if env['gui'] == 'small':
    env.Append(CPPDEFINES = "USE_SMALL_GUI")

if env['lowmem']:
    env.Append(CPPDEFINES = "LOW_MEM")

if env['fribidi']:
    env.Append(CPPDEFINES = "HAVE_FRIBIDI")

if env['raw_sockets']:
    env.Append(CPPDEFINES = "NETWORK_USE_RAW_SOCKETS")

if env['internal_data']:
    env.Append(CPPDEFINES = "USE_INTERNAL_DATA")

if env['prefsdir']:
    env.Append(CPPDEFINES = "PREFERENCES_DIR='\"%s\"'" % env['prefsdir'] )

if env['fifodir']:
    env.Append(CPPDEFINES = "FIFODIR='\"%s\"'" % env['fifodir'] )

if env['python']:
    env.Append(CPPDEFINES = "HAVE_PYTHON")

if env['localedir']:
    env.Append(CPPDEFINES = "LOCALEDIR='\"%s\"'" % env['localedir'] )
    if not os.path.isabs(env['localedir']):
        env.Append(CPPDEFINES = "HAS_RELATIVE_LOCALEDIR")

if env['dummy_locales']:
    env.Append(CPPDEFINES = "USE_DUMMYLOCALES")

# Simulate autools-like behavior of prefix on various paths
for d in ("bindir", "datadir", "fifodir", "icondir", "desktopdir", "mandir", "docdir"):
     if not os.path.isabs(env[d]):
          env[d] = os.path.join(env["prefix"], env[d])

if env["PLATFORM"] != "win32":
    env.Append(CPPDEFINES = "WESNOTH_PATH='\"%s\"'" % env['datadir'])

for d in ("bindir", "datadir", "fifodir", "icondir", "desktopdir", "mandir", "docdir"):
    env[d] = os.path.join("/", env["destdir"], env[d].lstrip("/"))
env["prefix"] = os.path.join("/", env["destdir"], env["prefix"].lstrip("/"))

if 'CXXFLAGS' in os.environ:
    env.Append(CXXFLAGS = os.environ['CXXFLAGS'])

if 'LDFLAGS' in os.environ:
    env.Append(LINKFLAGS = os.environ['LDFLAGS'])

test_env = env.Clone()
if boost_test_dyn_link:
    test_env.Append(CPPDEFINES = "BOOST_TEST_DYN_LINK")
    if boost_auto_test:
        test_env.Append(CPPDEFINES = "WESNOTH_BOOST_AUTO_TEST_MAIN")
    else:
        test_env.Append(CPPDEFINES = "WESNOTH_BOOST_TEST_MAIN")
Export("test_env")

if env["PLATFORM"] == 'win32':
    env.Append(LIBS = ["wsock32", "intl"])
if env["PLATFORM"] == 'darwin':			# Mac OS X
    env.Append(FRAMEWORKS = "Carbon")			# Carbon GUI

Export(Split("env have_client_prereqs have_X have_server_prereqs"))
SConscript(dirs = Split("src doc po"))
Import(Split("sources wesnoth wesnoth_editor wesnothd cutter exploder campaignd"))

# Omits the 'test' target 
all = env.Alias("all", [wesnoth, wesnoth_editor, wesnothd, campaignd,
                        cutter, exploder])
env.Default(map(eval, env["default_targets"]))

#
# Utility productions (Unix-like systems only)
#

# Make a tags file for Emacs
env.Command("TAGS", sources, 'etags -l c++ $SOURCES')
env.Clean(all, 'TAGS')

#
# Dummy locales
#

if env["dummy_locales"]:
    env.Command(Dir("locales/C"), [], "-mkdir -p locales;echo | localedef --force \"$TARGET\" 2> /dev/null")
    language_cfg_re = re.compile(r"data/languages/(.*)\.cfg")
    language_cfgs = glob("data/languages/*.cfg")
    languages = Flatten(map(language_cfg_re.findall, language_cfgs))
    languages = map(lambda x: x + "@wesnoth", languages)
    for language in languages:
        env.Command(
            os.path.join("locales", language),
            "locales/C",
            "ln -sf $SOURCE.filebase $TARGET"
            )

#
# Unix installation productions
#
# These will not be portable to Windows or Mac. They assume a Unix-like
# directory structure and FreeDesktop standard locations foicon, app,
# and doc files.
#

bindir = env['bindir']
pythonlib = os.path.join(env['prefix'] + "/lib/python/site-packages/wesnoth")
datadir = env['datadir']
docdir = env['docdir']
installable_subs = Split('data fonts icons images sounds')
if env['nls']:
    installable_subs.append("translations")
if env['dummy_locales']:
    installable_subs.append("locales")
fifodir = env['fifodir']
mandir = env["mandir"]
clientside = filter(lambda x : x, [wesnoth, wesnoth_editor, cutter, exploder])
daemons = filter(lambda x : x, [wesnothd, campaignd])
pythontools = Split("wmlscope wmllint wmlindent wesnoth_addon_manager")
pythonmodules = Split("wmltools.py wmlparser.py wmldata.py wmliterator.py campaignserver_client.py libsvn.py __init__.py")
localized_man_dirs = {}

def CopyFilter(fn):
    "Filter out data-tree things that shouldn't be installed."
    return not ".svn" in str(fn) and not "Makefile" in str(fn)

for lang in filter(CopyFilter, os.listdir("doc/man")):
     sourcedir = os.path.join("doc/man", lang)
     if os.path.isdir(sourcedir):
          targetdir = os.path.join(mandir, lang, "man6")
          localized_man_dirs[sourcedir] = targetdir

def InstallFilteredHook(target, source, env):
    if type(target) == type([]):
        target = target[0]
    target = str(target)
    if type(source) == type([]):
        map(lambda f: InstallFilteredHook(target, f, env), source)
    elif os.path.isdir(str(source)):
        if CopyFilter(source):
            target = os.path.join(target, os.path.basename(str(source))) 
            if not os.path.exists(target):
                 if env["verbose"]:
                      print "Make directory", target
                 os.makedirs(target)
            map(lambda f: InstallFilteredHook(target, os.path.join(str(source), f), env), os.listdir(str(source)))
    elif CopyFilter(source):
        if (env["gui"] == "tiny") and (source.endswith("jpg") or source.endswith("png")):
             (status, output) = commands.getstatusoutput("file "+ source)
             output = output.replace(" x ", "x")
             target = os.path.join(target, os.path.basename(source))
             if "RGBA" in output or "alpha" in output:
                 command = "convert -filter point -resize %s %s %s"
             else:
                 command = "convert -filter point -resize %s %s %s"
             if status == 0:
                 for (large, small) in (("1024x768","320x240"),
                                        ("640x480","240x180"),
                                        ("205x205","240x180")):
                      if large in output:
                           command = command % (small, source, target)
                           break
                 else:
                      command = command % ("50%", source, target)
                 if env["verbose"]:
                      print command
                 os.system(command)
                 return None
        # Just copy non-images, and images if tinygui is off
        if env["verbose"]:
             print "cp %s %s" % (str(source), target)
        shutil.copy2(str(source), target)
    return None
env.Append(BUILDERS={'InstallFiltered':Builder(action=InstallFilteredHook)})

def InstallWithSuffix(env, target, source):
    if not source:
        return source
    return env.InstallAs(os.path.join(target, source[0].name + env["program_suffix"]), source)

#env.AddMethod(InstallWithSuffix)
from SCons.Script.SConscript import SConsEnvironment
SConsEnvironment.InstallWithSuffix = InstallWithSuffix

def InstallLocalizedManPage(alias, page, env):
    actions = []
    for (sourcedir, targetdir) in localized_man_dirs.items():
        sourcefile = os.path.join(sourcedir, page)
        if os.path.isfile(sourcefile):
            env.Alias(alias, env.Install(targetdir, sourcefile))

# Now the actual installation productions

install_data = env.InstallFiltered(Dir(env.subst(datadir)),
                               map(Dir, installable_subs))

install_manual = env.InstallFiltered(Dir(env.subst(docdir)),
                                     Dir("doc/manual"))

# The game and associated resources
env.Alias("install-wesnoth", [
    env.InstallWithSuffix(bindir, wesnoth),
    env.Install(os.path.join(mandir, "man6"), "doc/man/wesnoth.6"),
                install_data, install_manual])
if have_client_prereqs and have_X and env["desktop_entry"]:
     if sys.platform == "darwin":
         env.Alias("install-wesnoth",
            env.Install(env["icondir"],
                            "icons/wesnoth-icon-Mac.png"))
     else:
         env.Alias("install-wesnoth",
            env.Install(env["icondir"],
                            "icons/wesnoth-icon.png"))
     env.Alias("install-wesnoth",
         env.Install(env["desktopdir"],
                         "icons/wesnoth.desktop"))
InstallLocalizedManPage("install-wesnoth", "wesnoth.6", env)

# The editor and associated resources
env.Alias("install-wesnoth_editor", [
    env.InstallWithSuffix(bindir, wesnoth_editor),
    env.Install(os.path.join(mandir, "man6"),
                                     "doc/man/wesnoth_editor.6"),
                                     install_data, install_manual])
if have_client_prereqs and have_X and env["desktop_entry"]:
    if sys.platform == "darwin":
        env.Alias("install-wesnoth_editor",
            env.Install(env["icondir"],
                            "icons/wesnoth_editor-icon-Mac.png"))
    else:
        env.Alias("install-wesnoth_editor",
            env.Install(env["icondir"],
                            "icons/wesnoth_editor-icon.png"))
    env.Alias("install-wesnoth_editor",
        env.Install(env["desktopdir"],
                        "icons/wesnoth_editor.desktop"))
InstallLocalizedManPage("install-wesnoth_editor", "wesnoth_editor.6", env)

# Python tools
env.Alias("install-pytools", [
    env.Install(bindir,
                map(lambda tool: 'data/tools/' + tool, pythontools)),
    env.Install(pythonlib,
                map(lambda module: 'data/tools/wesnoth/' + module, pythonmodules)),
    ])

# Wesnoth MP server install
install_wesnothd = env.InstallWithSuffix(bindir, wesnothd)
env.Alias("install-wesnothd", [
                           install_wesnothd,
                           env.Install(os.path.join(mandir, "man6"), "doc/man/wesnothd.6")
                           ])
for lang in filter(CopyFilter, os.listdir("doc/man")):
    sourcefile = os.path.join("doc/man", lang, "wesnothd.6")
    if os.path.isfile(sourcefile):
        targetdir = os.path.join(mandir, lang, "man6")
        env.Alias('install-wesnothd',
            env.Install(targetdir, sourcefile))
if not access(fifodir, F_OK):
    env.AddPostAction(install_wesnothd, [
        Mkdir(fifodir),
        Chmod(fifodir, 0700),
        Action("chown %s:%s %s" %
               (env["server_uid"], env["server_gid"], fifodir)),
        ])

# Wesnoth campaign server
env.Alias("install-campaignd", env.InstallWithSuffix(bindir, campaignd))

# And the artists' tools
env.Alias("install-cutter", env.InstallWithSuffix(bindir, cutter))
env.Alias("install-exploder", env.InstallWithSuffix(bindir, exploder))

# Compute things for default install based on which targets have been created.
install = env.Alias('install', [])
for installable in ('wesnoth', 'wesnoth_editor',
                    'wesnothd', 'campaignd',
                    'exploder', 'cutter'):
    if os.path.exists(installable) or installable in COMMAND_LINE_TARGETS or "all" in COMMAND_LINE_TARGETS:
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
uninstall = env.Command('uninstall', '', Flatten(Uninstall(Alias("install"))))
env.AlwaysBuild(uninstall)
env.Precious(uninstall)

#
# Making a distribution tarball.
#
if 'dist' in COMMAND_LINE_TARGETS:	# Speedup, the manifest is expensive
    def dist_manifest():
        "Get an argument list suitable for passing to a distribution archiver."
        # Start by getting a list of all files under version control
        lst = commands.getoutput("svn -v status | awk '/^[^?]/ {print $4;}'").split()
        lst = filter(os.path.isfile, lst)
        return lst
    dist_env = env.Clone()
    dist_tarball = dist_env.Tar('wesnoth.tar.bz2', [])
    open("dist.manifest", "w").write("\n".join(dist_manifest() + ["src/revision.hpp"]))
    dist_env.Append(TARFLAGS='-j -T dist.manifest --transform "s,^,wesnoth/,"',
               TARCOMSTR="Making distribution tarball...")
    dist_env.AlwaysBuild(dist_tarball)
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
# Making Mac OS X application bundles
#
env.Alias("wesnoth-bundle",
          env.Command("Battle For Wesnoth.app", "wesnoth", [
              Mkdir("${TARGET}/Contents"),
              Mkdir("${TARGET}/Contents/MacOS"),
              Mkdir("${TARGET}/Contents/Resources"),
              Action('echo "APPL????" > "${TARGET}/Contents/PkgInfo"'),
              Copy("${TARGET}/Contents/MacOS/wesnoth", "wesnoth"),
              ]))
env.Clean(all, "Battle For Wesnoth.app")    
env.Alias("wesnoth-editor-bundle",
          env.Command("Battle For Wesnoth Editor.app", "wesnoth_editor", [
              Mkdir("${TARGET}/Contents"),
              Mkdir("${TARGET}/Contents/MacOS"),
              Mkdir("${TARGET}/Contents/Resources"),
              Action('echo "APPL????" > "${TARGET}/Contents/PkgInfo"'),
              Copy("${TARGET}/Contents/MacOS/wesnoth_editor", "wesnoth_editor"),
              ]))
env.Clean(all, "Battle For Wesnoth Editor.app")

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
