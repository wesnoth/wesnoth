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

opts.Add(PathOption('bindir', 'Where to install binaries', "bin", PathOption.PathAccept))
opts.Add('cachedir', 'Directory that contains a cache of derived files.', '')
opts.Add(PathOption('datadir', 'read-only architecture-independent game data', "share/wesnoth", PathOption.PathAccept))
opts.Add(BoolOption('debug', 'Set to build for debugging', False))
opts.Add(BoolOption('dummy_locales','Set to enable Wesnoth private locales', False))
opts.Add(PathOption('fifodir', 'directory for the wesnothd fifo socket file', "/var/run/wesnothd", PathOption.PathAccept))
opts.Add(BoolOption('fribidi','Clear to disable bidirectional-language support', True))
opts.Add(BoolOption('desktop_entry','Clear to disable desktop-entry', True))
opts.Add(PathOption('desktopdir', 'sets the desktop entry directory to a non-default location', "share/applications", PathOption.PathAccept))
opts.Add(PathOption('icondir', 'sets the icons directory to a non-default location', "share/icons", PathOption.PathAccept))
opts.Add(BoolOption('internal_data', 'Set to put data in Mac OS X application fork', False))
opts.Add(PathOption('localedir', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept))
opts.Add(BoolOption('lowmem', 'Set to reduce memory usage by removing extra functionality', False))
opts.Add(BoolOption('nls','enable compile/install of gettext message catalogs',True))
opts.Add(PathOption('prefix', 'autotools-style installation prefix', "/usr/local"))
opts.Add(PathOption('prefsdir', 'user preferences directory', ".wesnoth", PathOption.PathAccept))
opts.Add(BoolOption('prereqs','abort if prerequisites cannot be detected',True))
opts.Add(BoolOption('profile', 'Set to build for debugging', False))
#opts.Add('program_suffix', 'suffix to append to names of installed programs',"")
opts.Add(BoolOption('python', 'Enable in-game python extensions.', True))
opts.Add(BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False))
opts.Add('server_gid', 'group id of the user who runs wesnothd', "")
opts.Add('server_uid', 'user id of the user who runs wesnothd', "")
opts.Add(BoolOption('smallgui', 'Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0), images normal size', False))
opts.Add(BoolOption('static', 'Set to enable static building of Wesnoth', False))
opts.Add(BoolOption('strict', 'Set to strict compilation', False))
opts.Add(BoolOption('tinygui', 'Set for GUI reductions for resolutions down to 320x240 (PDAs), resize images before installing', False))
opts.Add(BoolOption('verbose', 'Emit progress messages during data installation.', False))
opts.Add(PathOption('boostdir', 'Directory of boost installation.', '/usr/include'))
opts.Add(PathOption('boostlibdir', 'Directory where boost libraries are installed.', '/usr/lib'))
opts.Add('boost_suffix', 'Suffix of boost libraries.')

#
# Setup
#

env = Environment(options = opts)

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

# Omits the 'test' target 
all = env.Alias("all", ["wesnoth", "wesnoth_editor", "wesnothd", "campaignd",
                        "cutter", "exploder"])
env.Default(["wesnoth", "wesnothd"])

env.TargetSignatures('content')

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

    sdldir = context.env.get("SDLDIR", "/usr/")
    if sdl_lib == "SDL": 
        if require_version:
            context.Message("Checking for Simple DirectMedia Layer library version >= %d.%d.%d... " % (major_version, minor_version, patchlevel))
        else:
            context.Message("Checking for Simple DirectMedia Layer library... ")
        env = context.env
        env.AppendUnique(CPPPATH = [os.path.join(sdldir, "include/SDL")], LIBPATH = [os.path.join(sdldir, "lib")])
        if env["PLATFORM"] == "posix" or env["PLATFORM"] == "darwin":
            env.ParseConfig("sdl-config --cflags --libs")
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
        if (music == NULL)
            exit(1);
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

conf = Configure(env, custom_tests = { 'CheckPKGConfig' : CheckPKGConfig,
                                       'CheckPKG' : CheckPKG,
                                       'CheckSDL' : CheckSDL,
                                       'CheckOgg' : CheckOgg,
                                       'CheckPNG' : CheckPNG,
                                       'CheckBoost' : CheckBoost })

#if env["prereqs"]:
#    conf.CheckPKGConfig('0.15.0') or Die("Base prerequisites are not met.")

if env["prereqs"]:
    conf.CheckBoost("iostreams", require_version = "1.33.0") and \
    conf.CheckSDL(require_version = '1.2.7') or Die("Base prerequisites are not met.")

    have_client_prereqs = \
        conf.CheckBoost("regex") and \
        conf.CheckSDL("SDL_ttf", require_version = "2.0.8") and \
        conf.CheckSDL("SDL_mixer", require_version = '1.2.0') and \
        conf.CheckSDL("SDL_image", require_version = '1.2.0') and \
        conf.CheckOgg() and \
        conf.CheckPNG() or Warning("Client prerequisites are not met. wesnoth, wesnoth_editor, cutter and exploder cannot be built.")

    have_X = conf.CheckLib('X11') or Warning("wesnoth_editor cannot be built.")
    if env['fribidi']:
        env['fribidi'] = conf.CheckLibWithHeader('fribidi', 'fribidi/fribidi.h', 'C', 'fribidi_utf8_to_unicode(NULL,0,NULL);') or Warning("Can't find libfribidi, disabling freebidi support.")

    have_server_prereqs = conf.CheckSDL('SDL_net') or Warning("Server prerequisites are not met. wesnothd and campaignd cannot be built.")

    if env["python"]:
        env["python"] = (float(sys.version[:3]) >= 2.4) and conf.CheckLib('python'+sys.version[:3]) or Warning("Python >= 2.4 not found. Python extensions will be disabled.")
        env.Append(CPPPATH = ["/usr/include/python%s" % sys.version[:3]])
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

# Later in the recipe we will guarantee that src/revision.hpp exists 
env.Append(CPPDEFINES = 'HAVE_REVISION')

if env["debug"]:
    env.AppendUnique(CXXFLAGS = Split("-O0 -DDEBUG -ggdb3 -W -Wall -ansi"))
else:
    env.AppendUnique(CXXFLAGS = Split("-O2 -ansi"))

if env['static']:
    env.AppendUnique(LINKFLAGS = "-all-static")

if env['profile']:
    env.AppendUnique(CXXFLAGS = "-pg")
    env.AppendUnique(LINKFLAGS = "-pg")

if env['strict']:
    env.AppendUnique(CXXFLAGS = Split("-Werror -Wno-unused -Wno-sign-compare"))

if env['tinygui']:
    env.Append(CPPDEFINES = "USE_TINY_GUI")

if env['smallgui']:
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
for d in ("bindir", "datadir", "fifodir", "icondir", "desktopdir"):
     if not os.path.isabs(env[d]):
          env[d] = os.path.join(env["prefix"], env[d])

env.Append(CPPDEFINES = "WESNOTH_PATH='\"%s\"'" % env['datadir'])

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

cc_version = env["CCVERSION"]
if env["CC"] == "gcc":
    (major, minor, rev) = map(int, cc_version.split("."))
    if major*10+minor < 33:
        print "Your compiler version is too old"
        Exit(1)

# Platform-specific support, straight from configure.ac
if env["PLATFORM"] == 'win32':				# Microsoft Windows
    env.Append(LIBS = "unicows")			# Windows Unicode lib
elif env["PLATFORM"] == 'darwin':			# Mac OS X
    env.Append(FRAMEWORKS = "Carbon")			# Carbon GUI

Export(Split("env have_client_prereqs have_X have_server_prereqs"))
SConscript(dirs = Split("src doc po"))
Import(Split("sources wesnoth wesnoth_editor wesnothd cutter exploder campaignd"))

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
docdir = os.path.join(env['prefix'], "share/doc/wesnoth")
installable_subs = Split('data fonts icons images sounds')
if env['nls']:
    installable_subs.append("translations")
if env['dummy_locales']:
    installable_subs.append("locales")
fifodir = env['fifodir']
mandir = os.path.join(env["prefix"], "share/man")
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
        if env["tinygui"] and (source.endswith("jpg") or source.endswith("png")):
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

def InstallLocalizedManPage(alias, page, env):
    actions = []
    for (sourcedir, targetdir) in localized_man_dirs.items():
        sourcefile = os.path.join(sourcedir, page)
        if os.path.isfile(sourcefile):
            env.Alias(alias, env.Install(targetdir, sourcefile))

# TargetSignatures('content') causes a crash in the install
# production, at least in scons 0.97, right after the actions finish
# (thus, probably, at target-signature generation time).  So
# disable it for installation productions.
install_env = env.Clone()
install_env.TargetSignatures('build')

# Now the actual installation productions

install_data = install_env.InstallFiltered(Dir(datadir),
                                       map(Dir, installable_subs))

install_manual = install_env.InstallFiltered(Dir(docdir),
                                       Dir("doc/manual"))

# The game and associated resources
install_env.Alias("install-wesnoth", [
    install_env.Install(bindir, wesnoth),
    install_env.Install(os.path.join(mandir, "man6"), "doc/man/wesnoth.6"),
    install_data, install_manual])
if have_client_prereqs and have_X and env["desktop_entry"]:
     if sys.platform == "darwin":
         install_env.Alias("install-wesnoth",
                               install_env.Install(env["icondir"],
                                              "icons/wesnoth-icon-Mac.png"))
     else:
         install_env.Alias("install-wesnoth",
                              install_env.Install(env["icondir"],
                                             "icons/wesnoth-icon.png"))
     install_env.Alias("install-wesnoth",
               install_env.Install(env["desktopdir"],
                                      "icons/wesnoth.desktop"))
InstallLocalizedManPage("install-wesnoth", "wesnoth.6", env)

# The editor and associated resources
install_env.Alias("install-wesnoth_editor", [
    install_env.Install(bindir, wesnoth_editor),
    install_env.Install(os.path.join(mandir, "man6"),
                                "doc/man/wesnoth_editor.6"),
    install_data, install_manual])
if have_client_prereqs and have_X and env["desktop_entry"]:
     if sys.platform == "darwin":
          install_env.Alias("install-wesnoth_editor",
                               install_env.Install(env["icondir"],
                                              "icons/wesnoth_editor-icon-Mac.png"))
     else:
          install_env.Alias("install-wesnoth_editor",
                               install_env.Install(env["icondir"],
                                              "icons/wesnoth_editor-icon.png"))
     install_env.Alias("install-wesnoth_editor",
                          install_env.Install(env["desktopdir"],
                                         "icons/wesnoth_editor.desktop"))
InstallLocalizedManPage("install-wesnoth_editor", "wesnoth_editor.6", env)

# Python tools
install_env.Alias("install-pytools", [
    install_env.Install(bindir,
                      map(lambda tool: 'data/tools/' + tool, pythontools)),
    install_env.Install(pythonlib,
                      map(lambda module: 'data/tools/wesnoth/' + module, pythonmodules)),
    ])

# Wesnoth MP server install
install_wesnothd = install_env.Install(bindir, wesnothd)
install_env.Alias("install-wesnothd", [
                                install_wesnothd,
                                install_env.Install(os.path.join(mandir, "man6"), "doc/man/wesnothd.6")
                                ])
for lang in filter(CopyFilter, os.listdir("doc/man")):
     sourcefile = os.path.join("doc/man", lang, "wesnothd.6")
     if os.path.isfile(sourcefile):
          targetdir = os.path.join(mandir, lang, "man6")
          install_env.Alias('install-wesnothd',
               install_env.Install(targetdir, sourcefile))
if not access(fifodir, F_OK):
    install_env.AddPostAction(install_wesnothd, [
        Mkdir(fifodir),
        Chmod(fifodir, 0700),
        Action("chown %s:%s %s" %
               (env["server_uid"], env["server_gid"], fifodir)),
        ])

# Wesnoth campaign server
install_env.Alias("install-campaignd", Install(bindir, campaignd))

# And the artists' tools
install_env.Alias("install-cutter", Install(bindir, cutter))
install_env.Alias("install-exploder", Install(bindir, exploder))

# Compute things for default install based on which targets have been created.
install_env.Alias('install', [])
for installable in ('wesnoth', 'wesnoth_editor',
                    'wesnothd', 'campaignd',
                    'exploder', 'cutter'):
    if os.path.exists(installable) or installable in COMMAND_LINE_TARGETS or "all" in COMMAND_LINE_TARGETS:
        install_env.Alias('install', install_env.Alias('install-'+installable))

#
# Un-installation
#
deletions = map(lambda x: Delete(os.path.join(bindir, str(x[0]))), clientside + daemons) \
            + [Delete(datadir), Delete(pythonlib), Delete(fifodir), Delete(docdir)] \
            + map(lambda x: Delete(os.path.join(mandir, "man6", x)), [ "wesnoth.6", "wesnoth_editor.6", "wesnothd.6" ]) \
            + Flatten(map(lambda mandir : map(lambda x: Delete(os.path.join(mandir, x)), [ "wesnoth.6", "wesnoth_editor.6", "wesnothd.6" ]), localized_man_dirs.values()))
uninstall = env.Command('uninstall', '', deletions)
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
    dist_env.Append(TARFLAGS='-j -T dist.manifest',
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
