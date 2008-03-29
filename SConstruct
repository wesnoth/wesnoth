#
# SCons build description for the Wesnoth project
#
# Prerequisites are:
# 1. Subversion command-line client programs svnstatus and svnversion.
# 2. Unix file(1), on installation only.
# 3. Unix convert(1), on installation only, if using tinygui.
#
# To do (list is now exhaustive):
#
# 1. Documentation formatting and installation
# 2. Manual page formatting and installation
# 3. Dummy locales
# 4. Translations handling other than installation (pot-update).
# 5. Making binary and data-only distribution tarballs

import os, sys, commands, shutil, sets
from SCons.Script import *

#
# Build-control options
#

opts = Options('.scons-option-cache')

# These are implemented
opts.Add(PathOption('datadir', 'read-only architecture-independent game data', "share/wesnoth", PathOption.PathAccept))
opts.Add(BoolOption('debug', 'Set to build for debugging', False))
opts.Add(PathOption('fifodir', 'directory for the wesnothd fifo socket file', "/var/run/wesnothd", PathOption.PathAccept))
opts.Add(BoolOption('fribidi','Clear to disable bidirectional-language support', True))
opts.Add(BoolOption('desktop_entry','Clear to disable desktop-entry', True))
opts.Add(PathOption('desktopdir', 'sets the desktop entry directory to a non-default location', "share/applications", PathOption.PathAccept))
opts.Add(PathOption('icondir', 'sets the icons directory to a non-default location', "share/icons", PathOption.PathAccept))
opts.Add(BoolOption('internal_data', 'Set to put data in Mac OS X application fork', False))
opts.Add(PathOption('localedir', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept))
opts.Add(BoolOption('lowmem', 'Set to reduce memory usage by removing extra functionality', False))
opts.Add(PathOption('prefix', 'autotools-style installation prefix', "/usr/local"))
opts.Add(PathOption('prefsdir', 'user preferences directory', ".wesnoth", PathOption.PathAccept))
opts.Add(BoolOption('prereqs','abort if prerequisites cannot be detected',True))
opts.Add(BoolOption('profile', 'Set to build for debugging', False))
opts.Add(BoolOption('python', 'Enable in-game python extensions.', True))
opts.Add(BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False))
opts.Add('server_gid', 'group id of the user who runs wesnothd', "")
opts.Add('server_uid', 'user id of the user who runs wesnothd', "")
opts.Add(BoolOption('smallgui', 'Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0), images normal size', False))
opts.Add(BoolOption('static', 'Set to enable static building of Wesnoth', False))
opts.Add(BoolOption('strict', 'Set to strict compilation', False))
opts.Add(BoolOption('tinygui', 'Set for GUI reductions for resolutions down to 320x240 (PDAs), resize images before installing', False))
opts.Add(BoolOption('verbose', 'Emit progress messages during data installation.', False))

# FIXME: These are not yet implemented
opts.Add(BoolOption('dummy_locales','Set to enable Wesnoth private locales', False))

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

Available build targets include the individual binaries:

    wesnoth wesnoth_editor wesnothd campaignd exploder cutter test

The following special build targets

    all = same as 'wesnoth wesnoth_editor exploder cutter' (*).
    TAGS = build tags for Emacs (*).
    install = install 'all' executables, also wmlscope/wmllint/wmlindent.
    install-wesnothd = install the Wesnoth multiplayer server.
    install-campaignd = install the Wesnoth campaign server.
    uninstall = uninstall all executables, tools, and servers.
    dist = make distribution tarball as wesnoth.tar.bz2 (*).
    data-dist = make data tarball as wesnoth-data.tar.bz2 (*).
    binary_dist = make data tarball as wesnoth-binaries.tar.bz2 (*).
    wesnoth-bundle = make Mac OS application bundle from game (*)
    wesnoth-editor-bundle = make Mac OS application bundle from editor (*).
    sanity-check = run a pre-release sanity check on the distribution.
    manual.en.html = HTML version of the English-language manual.

Files made by targets maked '(*)' are cleaned by cleaned by 'scons -c all'

Options are cached in a file named .scons-option-cache and persist to later
invocations.  The file is editable. Delete it to start fresh.  Current option
values can be listed with 'scons -h'.

If you set CXXFLAGS and/or LDFLAGS in the environmnt, the values will
be appended to the appropriate variables withn scons.  You can use this,
for example, to point scons at non-default library locations.
""" + opts.GenerateHelpText(env))

# Omits the 'test' target 
all = env.Alias("all", ["wesnoth", "wesnoth_editor", "wesnothd", "campaignd",
                        "cutter", "exploder"])
env.Default("all")

env.TargetSignatures('content')

#
# Most of our required runtime support is from Boost SDL.
#

def CheckPKGConfig(context, version):
     context.Message( 'Checking for pkg-config... ' )
     ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
     context.Result( ret )
     return ret

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
    boostdir = env.get("BOOSTDIR", "/usr/include")
    boostlibdir = env.get("BOOSTLIBS", "/usr/lib")
    backup = backup_env(env, ["CPPPATH", "LIBPATH", "LIBS"])

    boost_headers = { "regex" : "regex/config.hpp",
                      "iostreams" : "iostreams/stream.hpp" }
    header_name = boost_headers.get(boost_lib, boost_lib + ".hpp")
    libname = "boost_" + boost_lib + env.get("BOOST_SUFFIX", "")

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
    if not check_result and not context.env.get("BOOST_SUFFIX"):
        context.env["BOOST_SUFFIX"] = "-mt"
        check_result = CheckBoostLib(context, boost_lib, require_version)
    if check_result:
        context.Result("yes")
    else:
        context.Result("no")
    return check_result

def CheckPKG(context, name):
     context.Message( 'Checking for %s... ' % name )
     ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
     context.Result( ret )
     return ret

def CheckSDL(context, sdl_lib = "SDL", require_version = None):
    if require_version:
        version = require_version.split(".", 2)
        major_version = int(version[0])
        minor_version = int(version[1])
        try:
            patchlevel    = int(version[2])
        except (ValueError, IndexError):
            patch_level = 0

    sdldir = context.env.get("SDLDIR",
                             os.path.join(context.env["prefix"], "SDL"))
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

if env["prereqs"]:
    conf.CheckPKGConfig('0.15.0') and conf.CheckBoost("iostreams") or Die("Base prerequisites are not met.")

    have_client_prereqs = \
        conf.CheckSDL(require_version = '1.2.7') and \
        conf.CheckSDL("SDL_ttf", require_version = "2.0.8") and \
        conf.CheckSDL("SDL_mixer") and \
        conf.CheckSDL("SDL_image") and \
        conf.CheckOgg() and \
        conf.CheckPNG() or Warning("Client prerequisites are not met. wesnoth, wesnoth_editor, cutter and exploder cannot be built.")

    have_X = conf.CheckLib('X11') or Warning("wesnoth_editor cannot be built.")
    have_fribidi = False
    if env['fribidi']:
        have_fribidi = conf.CheckLib('fribidi') or Die("Can't find libfribidi, please install it or rebuild with fribidi=no.")

    have_server_prereqs = conf.CheckSDL('SDL_net') or Warning("Server prerequisites are not met. wesnothd and campaignd cannot be built.")

    if env["python"]:
        env["python"] = (float(sys.version[:3]) >= 2.4) and conf.CheckLib('python'+sys.version[:3]) or Warning("Python >= 2.4 not found. Python extensions will be disabled.")
else:
    have_client_prereqs = True
    have_python = True
    have_X = True
    have_fribidi = True
    have_server_prereqs = True

boost_test_dyn_link = boost_auto_test = False
if 'test' in COMMAND_LINE_TARGETS:
    boost_test_dyn_link = conf.CheckCXXHeader('boost/test/unit_test.hpp')
    boost_auto_test = conf.CheckCXXHeader('boost/test/unit_test.hpp')

env = conf.Finish()

#
# Implement configuration switches
#
extralibs=[]

# FIXME: Unix-specific.
# Link only on demand, so we don't need separate link lists for each binary
env["LINKFLAGS"].append("-Wl,--as-needed")

# Later in the recipe we will guarantee that src/revision.hpp exists 
env["CXXFLAGS"].append('-DHAVE_REVISION')

if env["debug"]:
    env["CXXFLAGS"] += Split("-O0 -DDEBUG -ggdb3 -W -Wall -ansi")
else:
    env["CXXFLAGS"] += Split("-O2 -ansi")

if env['static']:
    env["LINKFLAGS"].append("-all-static")

if env['profile']:
    env["CXXFLAGS"].append("-pg")
    env["LINKFLAGS"].append("-pg")

if env['strict']:
    env["CXXFLAGS"] += Split("-Werror -Wno-unused -Wno-sign-compare")

if env['tinygui']:
    env["CXXFLAGS"].append("-DUSE_TINY_GUI")

if env['smallgui']:
    env["CXXFLAGS"].append("-DUSE_SMALL_GUI")

if env['lowmem']:
    env["CXXFLAGS"].append("-DLOW_MEM")

if env['fribidi']:
        env["CXXFLAGS"].append("-DHAVE_FRIBIDI")
        extralibs.append("fribidi")

if env['raw_sockets']:
    env["CXXFLAGS"].append("-DNETWORK_USE_RAW_SOCKETS")

if env['internal_data']:
    env["CXXFLAGS"].append("-DUSE_INTERNAL_DATA")

if env['prefsdir']:
    env["CXXFLAGS"].append("-DPREFERENCES_DIR='\"%s\"'" % env['prefsdir'])

if env['fifodir']:
    env["CXXFLAGS"].append("-DFIFODIR='\"%s\"'" % env['fifodir'])

if env['python']:
    env["CXXFLAGS"].append("-DHAVE_PYTHON")

if env['localedir']:
    env["CXXFLAGS"].append("-DLOCALEDIR='\"%s\"'" % env['localedir'])
    if not os.path.isabs(env['localedir']):
        env["CXXFLAGS"].append("-DHAS_RELATIVE_LOCALEDIR")

# Simulate autools-like behavior of prefix on various paths
for d in ("datadir", "fifodir", "icondir", "desktopdir"):
     if not os.path.isabs(env[d]):
          env[d] = os.path.join(env["prefix"], env[d])

env["CXXFLAGS"].append("-DWESNOTH_PATH='\"%s\"'" % env['datadir'])

if 'CXXFLAGS' in os.environ:
    env.Append(CXXFLAGS = os.environ['CXXFLAGS'])

if 'LDFLAGS' in os.environ:
    env.Append(LINKFLAGS = os.environ['LDFLAGS'])

test_env = env.Clone()
if boost_test_dyn_link:
    env["CXXFLAGS"].append("-DBOOST_TEST_DYN_LINK")
    if boost_auto_test:
        test_env["CXXFLAGS"].append("-DWESNOTH_BOOST_AUTO_TEST_MAIN")
    else:
        test_env["CXXFLAGS"].append("-DWESNOTH_BOOST_TEST_MAIN")

cc_version = env["CCVERSION"]
if env["CC"] == "gcc":
    (major, minor, rev) = map(int, cc_version.split("."))
    if major*10+minor < 33:
        print "Your compiler version is too old"
        Exit(1)

#
# Libraries and source groups
#
# The png library specification is not needed everywhere.  Some versions of
# (probably) SDL_image must carry it internally.
boost_libs = Split("boost_iostreams boost_regex")
SDL_libs = Split("SDL_net SDL_ttf SDL_mixer SDL_image SDL")
commonlibs = SDL_libs + boost_libs + ["pthread", "png", "-lpython"+sys.version[:3]]
wesnothdlibs = ["SDL_net", "boost_iostreams-mt", "pthread"]
commonpath = ['src', '/usr/include/SDL', '/usr/include/python%s' % sys.version[:3]]

# Platform-specific support, straight from configure.ac
if sys.platform == 'win32':				# Microsoft Windows
    commonlibs.append("unicows")			# Windows Unicode lib
elif sys.platform == 'darwin':				# Mac OS X
    event["CXXFLAGS"].append("-framework Carbon")	# Carbon GUI

#color_range.cpp should be removed, but game_config depends on it.
#game_config has very few things that are needed elsewhere, it should be
#removed.  Requires moving path and version at least to other files.

libwesnoth_core_sources = [
    "src/color_range.cpp",
    "src/config.cpp",
    "src/filesystem.cpp",
    "src/game_config.cpp",
    "src/gettext.cpp",
    "src/log.cpp",
    "src/map.cpp",
    "src/network.cpp",
    "src/network_worker.cpp",
    "src/thread.cpp",
    "src/tstring.cpp",
    "src/util.cpp",
    "src/serialization/binary_or_text.cpp",
    "src/serialization/binary_wml.cpp",
    "src/serialization/parser.cpp",
    "src/serialization/preprocessor.cpp",
    "src/serialization/string_utils.cpp",
    "src/serialization/tokenizer.cpp",
    ]
env.Library("wesnoth_core", libwesnoth_core_sources, 
            CPPPATH = commonpath + ['src/serialization'])

libwesnoth_sources = [
    "src/astarnode.cpp",
    "src/astarsearch.cpp",
    "src/builder.cpp",
    "src/cavegen.cpp",
    "src/clipboard.cpp",
    "src/construct_dialog.cpp",
    "src/cursor.cpp",
    "src/display.cpp",
    "src/events.cpp",
    "src/filechooser.cpp",
    "src/font.cpp",
    "src/generic_event.cpp",
    "src/hotkeys.cpp",
    "src/image.cpp",
    "src/key.cpp",
    "src/language.cpp",
    "src/loadscreen.cpp",
    "src/map_create.cpp",
    "src/map_label.cpp",
    "src/mapgen.cpp",
    "src/mapgen_dialog.cpp",
    "src/marked-up_text.cpp",
    "src/minimap.cpp",
    "src/pathutils.cpp",
    "src/preferences.cpp",
    "src/preferences_display.cpp",
    "src/race.cpp",
    "src/random.cpp",
    "src/reports.cpp",
    "src/show_dialog.cpp",
    "src/sound.cpp",
    "src/soundsource.cpp",
    "src/terrain.cpp",
    "src/terrain_translation.cpp",
    "src/tooltips.cpp",
    "src/video.cpp",
    "src/theme.cpp",
    "src/widgets/button.cpp",
    "src/widgets/file_menu.cpp",
    "src/widgets/label.cpp",
    "src/widgets/menu.cpp",
    "src/widgets/menu_style.cpp",
    "src/widgets/progressbar.cpp",
    "src/widgets/scrollarea.cpp",
    "src/widgets/scrollbar.cpp",
    "src/widgets/slider.cpp",
    "src/widgets/textbox.cpp",
    "src/widgets/widget.cpp",
    "src/wml_exception.cpp",
    ]
env.Library("wesnoth", libwesnoth_sources, 
            CPPPATH = commonpath + ['src/serialization'])

libwesnothd_sources = [
    "src/loadscreen_empty.cpp",
    "src/tools/dummy_video.cpp",
    ]
env.Library("wesnothd", libwesnothd_sources, 
            CPPPATH = ['src', '/usr/include/SDL'])

libcampaignd_sources = [
    "src/publish_campaign.cpp",
    ]
env.Library("campaignd", libcampaignd_sources, 
            CPPPATH = commonpath)

libwesnoth_sdl_sources = [
    "src/sdl_utils.cpp",
    ]
env.Library("wesnoth_sdl", libwesnoth_sdl_sources, 
            CPPPATH = commonpath)

libcutter_sources = [
    "src/tools/exploder_utils.cpp",
    "src/tools/exploder_cutter.cpp",
    ]
env.Library("cutter", libcutter_sources, 
            CPPPATH = commonpath)

# Used by both 'wesnoth' and 'test' targets
wesnoth_sources = [
    "src/about.cpp",
    "src/actions.cpp",
    "src/ai.cpp",
    "src/ai_dfool.cpp",
    "src/ai_attack.cpp",
    "src/ai_move.cpp",
    "src/ai_python.cpp",
    "src/ai_village.cpp",
    "src/animated_game.cpp",
    "src/attack_prediction.cpp",
    "src/callable_objects.cpp",
    "src/config_adapter.cpp",
    "src/dialogs.cpp",
    "src/floating_textbox.cpp",
    "src/formula.cpp",
    "src/formula_ai.cpp",
    "src/formula_function.cpp",
    "src/formula_tokenizer.cpp",
    "src/game_display.cpp",
    "src/game_events.cpp",
    "src/game_preferences.cpp",
    "src/game_preferences_display.cpp",
    "src/gamestatus.cpp",
    "src/generate_report.cpp",
    "src/halo.cpp",
    "src/help.cpp",
    "src/intro.cpp",
    "src/leader_list.cpp",
    "src/menu_events.cpp",
    "src/mouse_events.cpp",
    "src/multiplayer.cpp",
    "src/multiplayer_ui.cpp",
    "src/multiplayer_wait.cpp",
    "src/multiplayer_connect.cpp",
    "src/multiplayer_create.cpp",
    "src/multiplayer_lobby.cpp",
    "src/pathfind.cpp",
    "src/playcampaign.cpp",
    "src/play_controller.cpp",
    "src/playmp_controller.cpp",
    "src/playsingle_controller.cpp",
    "src/playturn.cpp",
    "src/replay.cpp",
    "src/replay_controller.cpp",
    "src/sha1.cpp",
    "src/settings.cpp",
    "src/statistics.cpp",
    "src/team.cpp",
    "src/terrain_filter.cpp",
    "src/titlescreen.cpp",
    "src/unit.cpp",
    "src/unit_abilities.cpp",
    "src/unit_animation.cpp",
    "src/unit_display.cpp",
    "src/unit_frame.cpp",
    "src/unit_map.cpp",
    "src/unit_types.cpp",
    "src/upload_log.cpp",
    "src/variable.cpp",
    "src/variant.cpp",
    "src/widgets/combo.cpp",
    "src/widgets/scrollpane.cpp",
]

#
# Target declarations
#

if have_client_prereqs:
    wesnoth = env.Program("wesnoth", ["src/game.cpp"] + wesnoth_sources,
            CPPPATH = commonpath + ['src/server'],
            LIBS = ['wesnoth_core', 'wesnoth_sdl', 'wesnoth', 'campaignd'] + commonlibs + extralibs,
            LIBPATH = [".", "/lib", "/usr/lib"])
else:
    wesnoth = None

wesnoth_editor_sources = [
    "src/editor/editor.cpp",
    "src/editor/editor_layout.cpp",
    "src/editor/map_manip.cpp",
    "src/editor/editor_display.cpp",
    "src/editor/editor_palettes.cpp",
    "src/editor/editor_main.cpp",
    "src/editor/editor_dialogs.cpp",
    "src/editor/editor_undo.cpp",
    "src/animated_editor.cpp",
    "src/gamestatus_editor.cpp",
    ]
if have_client_prereqs and have_X:
    wesnoth_editor = env.Program("wesnoth_editor", wesnoth_editor_sources,
            CPPPATH = commonpath,
            LIBS = ['wesnoth_core', 'wesnoth_sdl', 'wesnoth'] + commonlibs + extralibs,
            LIBPATH = [".", "/lib", "/usr/lib"])
else:
    wesnoth_editor = None

campaignd_sources = [
    "src/campaign_server/campaign_server.cpp",
    ]
if have_server_prereqs:
    campaignd = env.Program("campaignd", campaignd_sources,
            CPPPATH = ['src', 'src/server', '/usr/include/SDL', '/usr/include/python%s' % sys.version[:3]],
            LIBS = ['wesnoth_core', 'wesnothd', 'campaignd', 'wesnoth'] + commonlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])
else:
    campaignd = None

wesnothd_sources = [
    "src/server/game.cpp",
    "src/server/input_stream.cpp",
    "src/server/metrics.cpp",
    "src/server/player.cpp",
    "src/server/proxy.cpp",
    "src/server/server.cpp",
    "src/server/simple_wml.cpp",
    ]
if have_server_prereqs:
    wesnothd = env.Program("wesnothd", wesnothd_sources,
            CPPPATH = ['src', 'src/server', '/usr/include/SDL'],
            LIBS =  ['wesnoth_core', 'wesnothd'] + wesnothdlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])
else:
    wesnothd = None

cutter_sources = [
    "src/tools/cutter.cpp",
    ]
if have_client_prereqs:
    cutter = env.Program("cutter", cutter_sources,
            CPPPATH = commonpath,
            LIBS =  ['cutter', 'wesnoth_core', 'wesnoth_sdl', 'wesnothd', 'wesnoth'] + commonlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])
else:
    cutter = None

exploder_sources = [
    "src/tools/exploder.cpp",
    "src/tools/exploder_composer.cpp",
    ]
if have_client_prereqs:
    exploder = env.Program("exploder", exploder_sources,
            CPPPATH = commonpath,
            LIBS =  ['cutter', 'wesnoth_core', 'wesnoth_sdl', 'wesnothd', 'wesnoth'] + commonlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])
else:
    exploder = None

# FIXME: test build presently fails at link time.
test_env = env.Clone()
test_sources = [
    "src/tests/main.cpp",
    "src/tests/test_util.cpp",
    ]
test_env.Program("test", test_sources,
            CPPPATH = commonpath + ['/usr/include'],
            LIBS =  ['wesnoth_core', 'wesnoth_sdl', 'wesnothd'] + commonlibs + ['boost_unit_test_framework'],
            LIBPATH = [".", "/lib", "/usr/lib"])

# FIXME: Currently this will only work under Linux
env["svnrev"] = commands.getoutput("svnversion -n . 2>/dev/null")
env.Depends('src/game_config.o', 'src/revision.hpp')
r = env.Command("src/revision.hpp", [],
                lambda target, source, env: open(str(target[0]), "w").write("#define REVISION \"%s\"\n" % env["svnrev"]))
env.AlwaysBuild(r)
env.TargetSignatures('content')
env.Clean(all, "src/revision.hpp")

#
# File inventory, for archive makes abd analysis tools
#
headers = [
    "src/tools/exploder_composer.hpp",
    "src/tools/exploder_utils.hpp",
    "src/tools/exploder_cutter.hpp",
    "src/serialization/tokenizer.hpp",
    "src/serialization/parser.hpp",
    "src/serialization/binary_or_text.hpp",
    "src/serialization/binary_wml.hpp",
    "src/serialization/preprocessor.hpp",
    "src/serialization/string_utils.hpp",
    "src/widgets/progressbar.hpp",
    "src/widgets/textbox.hpp",
    "src/widgets/combo.hpp",
    "src/widgets/file_menu.hpp",
    "src/widgets/scrollpane.hpp",
    "src/widgets/menu.hpp",
    "src/widgets/button.hpp",
    "src/widgets/label.hpp",
    "src/widgets/slider.hpp",
    "src/widgets/scrollbar.hpp",
    "src/widgets/widget.hpp",
    "src/widgets/scrollarea.hpp",
    "src/server/player.hpp",
    "src/server/game.hpp",
    "src/server/input_stream.hpp",
    "src/server/proxy.hpp",
    "src/server/metrics.hpp",
    "src/editor/editor_undo.hpp",
    "src/editor/map_manip.hpp",
    "src/editor/editor_layout.hpp",
    "src/editor/editor.hpp",
    "src/editor/editor_palettes.hpp",
    "src/editor/editor_dialogs.hpp",
    "src/about.hpp",
    "src/actions.hpp",
    "src/ai.hpp",
    "src/ai2.hpp",
    "src/ai_dfool.hpp",
    "src/ai_interface.hpp",
    "src/ai_python.hpp",
    "src/animated.hpp",
    "src/animated.i",
    "src/array.hpp",
    "src/astarnode.hpp",
    "src/attack_prediction.hpp",
    "src/builder.hpp",
    "src/cavegen.hpp",
    "src/clipboard.hpp",
    "src/color_range.hpp",
    "src/config.hpp",
    "src/config_adapter.hpp",
    "src/construct_dialog.hpp",
    "src/cursor.hpp",
    "src/dialogs.hpp",
    "src/display.hpp",
    "src/events.hpp",
    "src/file_chooser.hpp",
    "src/filesystem.hpp",
    "src/floating_textbox.hpp",
    "src/font.hpp",
    "src/game_config.hpp",
    "src/game_display.hpp",
    "src/game_errors.hpp",
    "src/game_events.hpp",
    "src/game_preferences.hpp",
    "src/gamestatus.hpp",
    "src/generic_event.hpp",
    "src/gettext.hpp",
    "src/global.hpp",
    "src/halo.hpp",
    "src/help.hpp",
    "src/hotkeys.hpp",
    "src/image.hpp",
    "src/intro.hpp",
    "src/key.hpp",
    "src/language.hpp",
    "src/leader_list.hpp",
    "src/loadscreen.hpp",
    "src/log.hpp",
    "src/map.hpp",
    "src/map_create.hpp",
    "src/map_label.hpp",
    "src/mapgen.hpp",
    "src/mapgen_dialog.hpp",
    "src/marked-up_text.hpp",
    "src/menu_events.hpp",
    "src/minimap.hpp",
    "src/mouse_events.hpp",
    "src/multiplayer.hpp",
    "src/multiplayer_connect.hpp",
    "src/multiplayer_create.hpp",
    "src/multiplayer_lobby.hpp",
    "src/multiplayer_ui.hpp",
    "src/multiplayer_wait.hpp",
    "src/network.hpp",
    "src/network_worker.hpp",
    "src/pathfind.hpp",
    "src/pathutils.hpp",
    "src/play_controller.hpp",
    "src/playcampaign.hpp",
    "src/playmp_controller.hpp",
    "src/playsingle_controller.hpp",
    "src/playturn.hpp",
    "src/preferences.hpp",
    "src/preferences_display.hpp",
    "src/publish_campaign.hpp",
    "src/race.hpp",
    "src/random.hpp",
    "src/replay.hpp",
    "src/replay_controller.hpp",
    "src/reports.hpp",
    "src/scoped_resource.hpp",
    "src/sha1.hpp",
    "src/settings.hpp",
    "src/sdl_utils.hpp",
    "src/show_dialog.hpp",
    "src/sound.hpp",
    "src/soundsource.hpp",
    "src/statistics.hpp",
    "src/team.hpp",
    "src/terrain.hpp",
    "src/terrain_filter.hpp",
    "src/terrain_translation.hpp",
    "src/theme.hpp",
    "src/thread.hpp",
    "src/titlescreen.hpp",
    "src/tooltips.hpp",
    "src/tstring.hpp",
    "src/unit.hpp",
    "src/unit_abilities.hpp",
    "src/unit_animation.hpp",
    "src/unit_display.hpp",
    "src/unit_frame.hpp",
    "src/unit_map.hpp",
    "src/unit_types.hpp",
    "src/upload_log.hpp",
    "src/util.hpp",
    "src/variable.hpp",
    "src/video.hpp",
    "src/wml_separators.hpp",
    "src/wesconfig.h",
    "src/wml_exception.hpp",
]

sources =   libwesnoth_sources + libwesnoth_core_sources + \
            libwesnothd_sources + libcampaignd_sources + \
            libwesnoth_sdl_sources + libcutter_sources + \
            wesnoth_editor_sources + campaignd_sources + wesnothd_sources + \
            cutter_sources + exploder_sources + test_sources

#
# Utility productions
#

env.Command("TAGS", sources, 'etags -l c++ $SOURCES')
env.Clean(all, 'TAGS')

#
# Installation productions
#

bindir = os.path.normpath(os.path.join(env['prefix'], "bin"))
pythonlib = os.path.join(env['prefix'] + "/lib/python/site-packages/wesnoth")
datadir = env['datadir']
fifodir = env['fifodir']
clientside = filter(lambda x : x, [wesnoth, wesnoth_editor, cutter, exploder])
daemons = filter(lambda x : x, [wesnothd, campaignd])
pythontools = Split("wmlscope wmllint wmlindent")
pythonmodules = Split("wmltools.py wmlparser.py wmldata.py wmliterator.py campaignserver_client.py libsvn.py __init__.py")

def CopyFilter(fn):
    "Filter out data-tree things that shouldn't be installed."
    return not ".svn" in str(fn) and not "Makefile" in str(fn)

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
        if env["tinygui"] and source.endswith("jpg") or source.endswith("png"):
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

clientside_env = env.Clone()
# TargetSignatures('content') causes a crash in the install
# production, at least in scons 0.97, right after the actions finish
# (thus, probably, at target-signature generation time).
clientside_env.TargetSignatures('build')
env.Alias('install', [
    clientside_env.Install(bindir, clientside),
    clientside_env.Install(bindir, map(lambda tool : 'data/tools/' + tool, pythontools)),
    clientside_env.Install(pythonlib, map(lambda module : 'data/tools/wesnoth/' + module, pythonmodules)),
    clientside_env.InstallFiltered(Dir(datadir), map(Dir, Split('data fonts icons images sounds translations')))
    ])
if have_client_prereqs and have_X and env["desktop_entry"]:
     if sys.platform == "darwin":
          env.Alias('install',
                    clientside_env.Install(env["icondir"],
                                           "icons/wesnoth-icon-Mac.png"))
     else:
          env.Alias('install',
                    clientside_env.Install(env["icondir"],
                                           "icons/wesnoth-icon.png"))
     if sys.platform == "darwin":
          env.Alias('install',
                    clientside_env.Install(env["icondir"],
                                           "icons/wesnoth_editor-icon-Mac.png"))
     else:
          env.Alias('install',
                    clientside_env.Install(env["icondir"],
                                           "icons/wesnoth_editor-icon.png"))
     env.Alias('install',
               clientside_env.Install(env["desktopdir"],
                                      ["icons/wesnoth.desktop",
                                       "icons/wesnoth_editor.desktop",
                                       ]))

# FIXME: Only works under Unixes
wesnothd_env = env.Clone()
wesnothd_env.TargetSignatures('build')
from os import access, F_OK
install_wesnothd = wesnothd_env.Install(bindir, wesnothd)
env.Alias("install-wesnothd", install_wesnothd)
if not access(fifodir, F_OK):
    wesnothd_env.AddPostAction(install_wesnothd, [
        Mkdir(fifodir),
        Chmod(fifodir, 0700),
        Action("chown %s:%s %s" %
               (env["server_uid"], env["server_gid"], fifodir)),
        ])

env.Alias("install-campaignd", env.Clone().Install(bindir, campaignd))

#
# Un-installation
#
deletions = map(lambda x: Delete(os.path.join(bindir, str(x[0]))), clientside + daemons) \
            + [Delete(datadir), Delete(pythonlib), Delete(fifodir)]
uninstall = env.Command('uninstall', '', deletions)
env.AlwaysBuild(uninstall)
env.Precious(uninstall)

#
# Making the manual
#
env.Command("manual.en.xml", "doc/manual/manual.txt",
	"asciidoc -b docbook -d book -n -a toc -o ${TARGET} ${SOURCE}")
env.Command("manual.en.html", "manual.en.xml",
	'xsltproc --nonet /etc/asciidoc/docbook-xsl/xhtml.xsl "${SOURCE}" >"${TARGET}"')

#
# Making a distribution tarball.
#
if 'dist' in COMMAND_LINE_TARGETS:	# Speedup, the manifest is expensive
    def dist_manifest():
        "Get an argument list suitable for passing to a distribution archiver."
        # Start by getting a list of all files under version control
        lst = commands.getoutput("svn -v status | awk '/^[^?]/ {print $4;}'")
        # Omit everything with a data/ prefix to cut the list length below the
        # shell argument limit, otherwise the archiver command will blow up.
        lst = filter(lambda f: os.path.isfile(f) and not f.startswith("data/"),
                     lst.split())
        # Add data/ back to the end of the list.  This is safe only because we
        # assume there will be no junk under data/.  But we'll filter out
        # filenames with tildes in them (Emacs backup files) just in case.
        lst.append("data/")
        return lst
    dist_env = env.Clone()
    dist_tarball = env.Tar('wesnoth.tar.bz2',
                           dist_manifest()+["src/revision.hpp"])
    dist_env.Append(TARFLAGS='-j --exclude=".svn" --exclude="~"',
               TARCOMSTR="Making distribution tarball...")
    env.Clean(all, 'wesnoth.tar.bz2')
    env.Alias('dist', dist_tarball)

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
    Action("cd utils; ./sanity-check"),
    Action("cd data/tools; make sanity-check"),
    ])
env.AlwaysBuild(sanity_check)
env.Precious(sanity_check)

# Local variables:
# mode: python
# end:
