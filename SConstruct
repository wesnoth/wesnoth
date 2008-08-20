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
from subprocess import Popen, PIPE, call
from os import access, F_OK

# Warn user of current set of build options.
if os.path.exists('.scons-option-cache'):
    optfile = file('.scons-option-cache')
    print "Saved options:", optfile.read().replace("\n", ", ")[:-2]
    optfile.close()

#
# Build-control options
#

opts = Options('.scons-option-cache')

def OptionalPath(key, val, env):
    if val:
        PathOption.PathIsDir(key, val, env)

opts.AddOptions(
    ListOption('default_targets', 'Targets that will be built if no target is specified in command line.',
        "wesnoth,wesnothd,test", Split("wesnoth wesnothd wesnoth_editor campaignd cutter exploder test")),
    EnumOption('build', 'Build variant: debug, release or profile', "release", ["release", "debug", "profile"]),
    ('extra_flags_release', 'Extra compiler and linker flags to use for release builds', ""),
    ('extra_flags_debug', 'Extra compiler and linker flags to use for debug builds', ""),
    ('extra_flags_profile', 'Extra compiler and linker flags to use for profile builds', ""),
    PathOption('bindir', 'Where to install binaries', "bin", PathOption.PathAccept),
    ('cachedir', 'Directory that contains a cache of derived files.', ''),
    PathOption('datadir', 'read-only architecture-independent game data', "$datarootdir/$datadirname", PathOption.PathAccept),
    BoolOption('dummy_locales','Set to enable Wesnoth private locales', False),
    PathOption('fifodir', 'directory for the wesnothd fifo socket file', "/var/run/wesnothd", PathOption.PathAccept),
    BoolOption('fribidi','Clear to disable bidirectional-language support', True),
    BoolOption('desktop_entry','Clear to disable desktop-entry', True),
    PathOption('datarootdir', 'sets the root of data directories to a non-default location', "share", PathOption.PathAccept),
    PathOption('datadirname', 'sets the name of data directory', "wesnoth$version_suffix", PathOption.PathAccept),
    PathOption('desktopdir', 'sets the desktop entry directory to a non-default location', "$datarootdir/applications", PathOption.PathAccept),
    BoolOption('editor2', 'set to build the new map editor in the "wesnoth" target (inside the game)', True),
    PathOption('icondir', 'sets the icons directory to a non-default location', "$datarootdir/icons", PathOption.PathAccept),
    BoolOption('internal_data', 'Set to put data in Mac OS X application fork', False),
    PathOption('localedirname', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept),
    PathOption('mandir', 'sets the man pages directory to a non-default location', "$datarootdir/man", PathOption.PathAccept),
    PathOption('docdir', 'sets the doc directory to a non-default location', "$datarootdir/doc/wesnoth", PathOption.PathAccept),
    PathOption('python_site_packages_dir', 'sets the directory where python modules are installed', "lib/python/site-packages/wesnoth", PathOption.PathAccept),
    BoolOption('lowmem', 'Set to reduce memory usage by removing extra functionality', False),
    BoolOption('nls','enable compile/install of gettext message catalogs',True),
    PathOption('prefix', 'autotools-style installation prefix', "/usr/local", PathOption.PathAccept),
    PathOption('prefsdir', 'user preferences directory', ".wesnoth$version_suffix", PathOption.PathAccept),
    PathOption('destdir', 'prefix to add to all installation paths.', "", PathOption.PathAccept),
    PathOption('windows_release_dir', 'Directory where windows release will be prepared.', "", PathOption.PathAccept),
    BoolOption('prereqs','abort if prerequisites cannot be detected',True),
    ('program_suffix', 'suffix to append to names of installed programs',"$version_suffix"),
    ('version_suffix', 'suffix that will be added to default values of prefsdir, program_suffix and datadirname', ""),
    BoolOption('python', 'Enable in-game python extensions.', True),
    BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False),
    ('server_gid', 'group id of the user who runs wesnothd', ""),
    ('server_uid', 'user id of the user who runs wesnothd', ""),
    EnumOption('gui', 'Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0) or 320x240 (PDAs)', "normal", ["normal", "small", "tiny"]),
    BoolOption('static', 'Set to enable static building of Wesnoth', False),
    BoolOption('strict', 'Set to strict compilation', False),
    BoolOption('static_test', 'Staticaly build against boost test (Not supported yet)', False),
    BoolOption('verbose', 'Emit progress messages during data installation.', False),
    PathOption('sdldir', 'Directory of SDL installation.', "", OptionalPath),
    PathOption('boostdir', 'Directory of boost installation.', "", OptionalPath),
    PathOption('boostlibdir', 'Directory where boost libraries are installed.', "", OptionalPath),
    ('boost_suffix', 'Suffix of boost libraries.'),
    PathOption('gettextdir', 'Root directory of Gettext\'s installation.', "", OptionalPath), 
    PathOption('gtkdir', 'Directory where GTK SDK is installed.', "", OptionalPath),
    ('host', 'Cross-compile host.', ''),
    BoolOption('distcc', "Use distcc", False),
    BoolOption('ccache', "Use ccache", False),
    ('cxxtool', 'Set c++ compiler command if not using standard compiler.'),
    BoolOption("fast", "Make scons faster at cost of less precise dependency tracking.", False)
    )

#
# Setup
#

sys.path.insert(0, "./scons")
env = Environment(tools=["tar", "gettext", "install", "python_devel"], options = opts, toolpath = ["scons"])

opts.Save('.scons-option-cache', env)

# Make sure the user's environment is always available
env['ENV']['PATH'] = os.environ["PATH"]
if env["PLATFORM"] == "win32":
    env.Tool("mingw")
else:
    from cross_compile import *
    setup_cross_compile(env)

if env.get('cxxtool',""):
    env['CXX'] = env['cxxtool']
    env['ENV']['HOME'] = os.environ['HOME']

if env['distcc']: env.Tool('distcc')
if env['ccache']: env.Tool('ccache')


Help("""Arguments may be a mixture of switches and targets in any order.
Switches apply to the entire build regardless of where they are in the order.
Important switches include:

    prefix=/usr     probably what you want for production tools
    build=release   build the release build variant with appropriante flags
                        in build/release and copy resulting binaries
                        into distribution/working copy root.
    build=debug     same for debug build variant, binaries will be copied with -debug suffix

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

Files made by targets marked with '(*)' are cleaned by 'scons -c all'

Options are cached in a file named .scons-option-cache and persist to later
invocations.  The file is editable.  Delete it to start fresh.  Current option
values can be listed with 'scons -h'.

If you set CXXFLAGS and/or LDFLAGS in the environment, the values will
be appended to the appropriate variables within scons.
""" + opts.GenerateHelpText(env, sort=cmp))

if env["cachedir"] and not env['ccache']:
    CacheDir(env["cachedir"])

if env["fast"]:
    EnsureSConsVersion(0,98)
    env.Decider('MD5-timestamp')
    SetOption('max_drift', 1)
    SetOption('implicit_cache', 1)

#
# Check some preconditions
#

def Die(message):
    print message
    Exit(1)

def Warning(message):
    print message
    return False

from metasconf import init_metasconf
conf = Configure(env, custom_tests = init_metasconf(env, ["cplusplus", "python_devel", "sdl", "boost", "pango"]), config_h = "config.h")

if env["prereqs"]:
    if env["gettextdir"]:
        env.AppendUnique(CPPPATH = os.path.join(env["gettextdir"], "include"),
                         LIBPATH = os.path.join(env["gettextdir"], "lib"),
                         LIBS = ["intl"])
    conf.CheckCPlusPlus(gcc_version = "3.3") and \
    conf.CheckBoost("iostreams", require_version = "1.33.0") and \
    conf.CheckBoost("smart_ptr", header_only = True) and \
    conf.CheckCHeader("libintl.h", "<>") and \
    conf.CheckSDL(require_version = '1.2.7') or Die("Base prerequisites are not met.")

    have_client_prereqs = \
        conf.CheckPango("cairo") and \
        conf.CheckBoost("regex") and \
        conf.CheckSDL("SDL_ttf", require_version = "2.0.8") and \
        conf.CheckSDL("SDL_mixer", require_version = '1.2.0') and \
        conf.CheckSDL("SDL_image", require_version = '1.2.0') and \
        conf.CheckOgg() or Warning("Client prerequisites are not met. wesnoth, wesnoth_editor, cutter and exploder cannot be built.")

    have_X = False
    if env["PLATFORM"] != "win32":
        have_X = conf.CheckLib('X11')

    if env['fribidi']:
        env['fribidi'] = conf.CheckLibWithHeader('fribidi', 'fribidi/fribidi.h', 'C', 'fribidi_utf8_to_unicode(NULL,0,NULL);') or Warning("Can't find libfribidi, disabling freebidi support.")

    if env["PLATFORM"] == "posix":
        conf.CheckCHeader("poll.h", "<>")
        conf.CheckCHeader("sys/poll.h", "<>")
        conf.CheckCHeader("sys/select.h", "<>")

    have_server_prereqs = conf.CheckSDL('SDL_net') or Warning("Server prerequisites are not met. wesnothd and campaignd cannot be built.")

    have_test_prereqs =  have_client_prereqs and have_server_prereqs and conf.CheckBoost('unit_test_framework', require_version = "1.34.0") or Warning("Unit tests are disabled because their prerequisites are not met.")

#    have_boost_asio = \
#        conf.CheckBoost("system", require_version = "1.35.0") and \
#        conf.CheckBoost("asio", require_version = "1.35.0", header_only = True) or \
#        Warning("Boost 1.35 not found using old networking code.i")
#    
#    env["have_boost_asio"] = have_boost_asio;

    if env["python"]:
        env["python"] = (float(sys.version[:3]) >= 2.4) and conf.CheckPython() or Warning("Python >= 2.4 not found. Python extensions will be disabled.")
else:
    have_client_prereqs = True
    have_X = False
    if env["PLATFORM"] != "win32":
        have_X = True
    have_server_prereqs = True
    have_test_prereqs = True

env.Append(CPPPATH = ["#/src", "#/"])


have_msgfmt = env["MSGFMT"]
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

# FIXME: gcc-specific.
# Link only on demand, so we don't need separate link lists for each binary
env.Append(LINKFLAGS = "-Wl,--as-needed")

env.Replace(CPPDEFINES = ["HAVE_CONFIG_H"])

if env['static']:
    env.AppendUnique(LINKFLAGS = "-all-static")

env.AppendUnique(CXXFLAGS = Split("-W -Wall -Wno-unused -Wno-sign-compare -ansi"))
if env['strict']:
    env.AppendUnique(CXXFLAGS = "-Werror")

if env['gui'] == 'tiny':
    env.Append(CPPDEFINES = "USE_TINY_GUI")

if env['gui'] == 'small':
    env.Append(CPPDEFINES = "USE_SMALL_GUI")

if env['lowmem']:
    env.Append(CPPDEFINES = "LOW_MEM")

if env['internal_data']:
    env.Append(CPPDEFINES = "USE_INTERNAL_DATA")

if env['editor2']:
    env.Append(CPPDEFINES = "USE_EDITOR2")

if have_X:
    env.Append(CPPDEFINES = "_X11")

# Simulate autools-like behavior of prefix on various paths
installdirs = Split("bindir datadir fifodir icondir desktopdir mandir docdir python_site_packages_dir")
for d in installdirs:
    env[d] = os.path.join(env["prefix"], env[d])

if env["PLATFORM"] == 'win32':
    env.Append(LIBS = ["wsock32", "intl", "z"], CXXFLAGS = ["-mthreads"], LINKFLAGS = ["-mthreads"])
if env["PLATFORM"] == 'darwin':            # Mac OS X
    env.Append(FRAMEWORKS = "Carbon")            # Carbon GUI

if os.path.exists('.git'):
    try:
        env["svnrev"] = Popen(Split("git-svn find-rev HEAD"), stdout=PIPE).communicate()[0].rstrip("\n")
    except:
        env["svnrev"] = ""
else:
    try:
        env["svnrev"] = Popen(Split("svnversion -n ."), stdout=PIPE).communicate()[0]
    except:
        env["svnrev"] = ""

Export(Split("env have_client_prereqs have_server_prereqs have_test_prereqs"))
SConscript(dirs = Split("po doc packaging/windows"))

binaries = Split("wesnoth wesnoth_editor wesnothd cutter exploder campaignd test")
builds = {
    "debug"   : dict(CXXFLAGS = Split("-O0 -DDEBUG -ggdb3")),
    "release" : dict(CXXFLAGS = "-O2"),
    "profile" : dict(CXXFLAGS = "-pg", LINKFLAGS = "-pg")
    }
build = env["build"]

env.AppendUnique(**builds[build])
env.Append(CXXFLAGS = os.environ.get('CXXFLAGS', []), LINKFLAGS = os.environ.get('LDFLAGS', []))
env.MergeFlags(env["extra_flags_" + build])

test_env = env.Clone()
if not env['static_test']:
    test_env.Append(CPPDEFINES = "BOOST_TEST_DYN_LINK")
Export("test_env")

SConscript("src/SConscript", build_dir = os.path.join("build", build), exports = "env")
Import(binaries + ["sources"])
binary_nodes = map(eval, binaries)
if build == "release" : build_suffix = "" + env["PROGSUFFIX"]
else                  : build_suffix = "-" + build + env["PROGSUFFIX"]
from install import HardLink
wc_binaries = [ bin and env.Command(bin[0].name.split(".")[0] + build_suffix, bin, HardLink("$TARGET", "$SOURCE")) or None for bin in binary_nodes ]
map(lambda bin, node, wc_bin: Alias(bin, [node, wc_bin]), binaries, binary_nodes, wc_binaries)
all = env.Alias("all", map(Alias, binaries))
env.Default(map(Alias, env["default_targets"]))

#
# Utility productions (Unix-like systems only)
#

# Make a tags file for Emacs
# Exuberant Ctags doesn't understand the -l c++ flag so if the etags fails try the ctags version
env.Command("TAGS", sources, 'etags -l c++ $SOURCES.srcpath || (ctags --tag-relative=yes -f src/tags $SOURCES.srcpath)')
env.Clean(all, 'TAGS')

#
# Dummy locales
#

if env["nls"] and env['PLATFORM'] != 'win32':
    env.Command(Dir("locales/C"), [], "-mkdir -p locales;echo | localedef --force \"$TARGET\" 2> /dev/null")
    language_cfg_re = re.compile(r"data/languages/(.*)\.cfg")
    language_cfgs = glob("data/languages/*.cfg")
    languages = Flatten(map(language_cfg_re.findall, language_cfgs))
    languages = map(lambda x: x + "@wesnoth", languages)
    for language in languages:
        env.Command(
            Dir(os.path.join("locales", language)),
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

env = env.Clone()
for d in installdirs:
    env[d] = os.path.join("/", env["destdir"], env[d].lstrip("/"))
bindir = env['bindir']
pythonlib = env['python_site_packages_dir']
datadir = env['datadir']
docdir = env['docdir']
installable_subs = Split('data fonts icons images sounds')
if env['nls']:
    installable_subs.append("translations")
if env['nls'] and env['PLATFORM'] != 'win32':
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

env["copy_filter"] = CopyFilter

for lang in filter(CopyFilter, os.listdir("doc/man")):
     sourcedir = os.path.join("doc/man", lang)
     if os.path.isdir(sourcedir):
          targetdir = os.path.join(mandir, lang, "man6")
          localized_man_dirs[sourcedir] = targetdir

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
env.AlwaysBuild(install_data, install_manual)

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
uninstall = env.Command('uninstall', '', Flatten(Uninstall(Alias("install"))))
env.AlwaysBuild(uninstall)
env.Precious(uninstall)

#
# Making a distribution tarball.
#
config_h_re = re.compile(r"^.*#define\s*(\S*)\s*\"(\S*)\".*$", re.MULTILINE)
build_config = dict( config_h_re.findall(File("config.h.dummy").get_contents()) )
env["version"] = build_config.get("PACKAGE_VERSION")
if 'dist' in COMMAND_LINE_TARGETS:    # Speedup, the manifest is expensive
    def dist_manifest():
        "Get an argument list suitable for passing to a distribution archiver."
        # Start by getting a list of all files under version control
        lst = commands.getoutput("svn -v status | awk '/^[^?]/ {print $4;}'").split()
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
