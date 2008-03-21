#
# SCons build description for the Wesnoth project
#
# This is a deliberately straight-line translation of the old configure.ac;
# it builds an autotools-like config.h for the C++ code.  The optipng
# and internal-data options are omitted.
#
version = "1.5.0-svn"
min_savegame_version = "1.3.10"

#
# Local scons modules
#
import os, sys, commands

#
# Build-control options
#

opts = Options()
opts.Add(PathOption('prefix', 'autotools-style installation prefix', "/usr/local"))
opts.Add(PathOption('datadir', 'read-only architecture-independent game data', "wesnoth", PathOption.PathAccept))
opts.Add(BoolOption('debug', 'Set to build for debugging', False))
opts.Add(BoolOption('tests', 'Set to enable static building of Wesnoth', False))
opts.Add(BoolOption('python','Clear to disable Python support', True))
opts.Add(BoolOption('lite', 'Set to build lite version of wesnoth (no music or large images)', False))
opts.Add(BoolOption('tinygui', 'Set for GUI reductions for resolutions down to 320x240, resize images before installing', False))
opts.Add(BoolOption('lowmem', 'Set to reduce memory usage by removing extra functionality', False))
opts.Add(BoolOption('fribidi','Clear to disable bidirectional-language support', True))
opts.Add(BoolOption('dummy_locales','Set to enable Wesnoth private locales', False))
opts.Add(PathOption('fifodir', 'directory for the wesnothd fifo socket file', "/var/run/wesnothd", PathOption.PathAccept))
opts.Add('server_uid', 'user id of the user who runs wesnothd', "")
opts.Add('server_gid', 'group id of the user who runs wesnothd', "")
opts.Add(BoolOption('server_monitor', 'Set to enable enable server monitor thread; libgtop2 is required', False))
#opts.Add(BoolOption('internal_data', 'Set to put data in Mac OS X application fork', False))
opts.Add(BoolOption('tinygui', 'Set for GUI reductions for resolutions down to 320x240, resize images before installing', False))
opts.Add(BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False))
opts.Add(BoolOption('desktop_entry','Clear to disable desktop-entry', True))
opts.Add(PathOption('localedir', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept))
opts.Add(PathOption('icondir', 'sets the icons directory to a non-default location', "icons", PathOption.PathAccept))
opts.Add(PathOption('desktopdir', 'sets the desktop entry directory to a non-default location', "applications", PathOption.PathAccept))

#
# Setup
#

env = Environment(options = opts)
all = env.Alias("all", ["wesnoth", "wesnoth_editor", "wesnothd", "campaignd"])
env.Default("all")

#
# Program declarations
#
boost_libs = Split("boost_iostreams-mt boost_regex")
SDL_libs = Split("SDL_net SDL_ttf SDL_mixer SDL pthread SDL_image")
commonlibs = SDL_libs + boost_libs

libwesnoth_core_sources = [
    "src/color_range.cpp",
    "src/config.cpp",
    "src/filesystem.cpp",
    "src/game_config.cpp",
    "src/gettext.cpp",
    "src/log.cpp",
    "src/map.cpp",
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
            CPPPATH = ['src', 'src/serialization', "/usr/include/SDL"])

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
	"src/sdl_utils.cpp",
	"src/show_dialog.cpp",
	"src/sound.cpp",
	"src/soundsource.cpp",
	"src/terrain.cpp",
	"src/terrain_translation.cpp",
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
            CPPPATH = ['src', 'src/serialization', "/usr/include/SDL"])

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
    "src/game.cpp",
    "src/generate_report.cpp",
    "src/generic_event.cpp",
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
    "src/network.cpp",
    "src/network_worker.cpp",
    "src/pathfind.cpp",
    "src/playcampaign.cpp",
    "src/play_controller.cpp",
    "src/playmp_controller.cpp",
    "src/playsingle_controller.cpp",
    "src/playturn.cpp",
    "src/publish_campaign.cpp",
    "src/replay.cpp",
    "src/replay_controller.cpp",
    "src/sha1.cpp",
    "src/settings.cpp",
    "src/statistics.cpp",
    "src/team.cpp",
    "src/terrain_filter.cpp",
    "src/titlescreen.cpp",
    "src/tooltips.cpp",
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
env.Program("wesnoth", wesnoth_sources,
            CPPPATH = ['src', 'src/server', "/usr/include/SDL"],
            LIBS = commonlibs + ['wesnoth_core', 'wesnoth'],
            LIBPATH = [".", "src", "/lib", "/usr/lib"])

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
	"src/generic_event.cpp",
	"src/tooltips.cpp",
        ]
env.Program("wesnoth_editor", wesnoth_editor_sources,
            CPPPATH = ['src', 'src/server', "/usr/include/SDL"],
            LIBS = commonlibs + ['wesnoth_core', 'wesnoth'],
            LIBPATH = [".", "src", "/lib", "/usr/lib"])

campaignd_sources = [
	"src/campaign_server/campaign_server.cpp",
	"src/network.cpp",
	"src/network_worker.cpp",
	"src/publish_campaign.cpp",
	"src/loadscreen_empty.cpp",
        ]
env.Program("campaignd", campaignd_sources,
            CPPPATH = ['src', 'src/server', "/usr/include/SDL"],
            LIBS = commonlibs + ['wesnoth_core', 'wesnoth'],
            LIBPATH = [".", "src", "/lib", "/usr/lib"])

wesnothd_sources = [
    "src/server/game.cpp",
    "src/server/input_stream.cpp",
    "src/server/metrics.cpp",
    "src/server/player.cpp",
    "src/server/proxy.cpp",
    "src/server/server.cpp",
    "src/server/simple_wml.cpp",
    "src/network.cpp",
    "src/network_worker.cpp",
    "src/loadscreen_empty.cpp"
    ]
env.Program("wesnothd", wesnothd_sources,
            CPPPATH = ['src', 'src/server', "/usr/include/SDL"],
            LIBS = commonlibs + ['wesnoth_core'],
            LIBPATH = [".", "src", "/lib", "/usr/lib"])

#
# Utility productions
#

tags = env.Command("TAGS",
            libwesnoth_sources + libwesnoth_core_sources + \
            wesnoth_editor_sources + campaignd_sources + wesnothd_sources,
            'etags -l c++ $SOURCES')
env.Clean(all, 'TAGS')

#
# Configuration
#

Help("""\
Available build targets include: game editor server campaign-server tools.
The 'install' target installs whatever you currently have built.
If you have built tools and Python is available the Python helper modules
will also be installed.

""" + opts.GenerateHelpText(env))
conf = Configure(env)

envdict = env.Dictionary()

# Simulate autools-like behavior of prefix and datadir
if not "/" in envdict["datadir"]:
    env["datadir"] = os.path.join(envdict["prefix"], envdict["datadir"])

#
# Check some preconditions
#

cc_version = env["CCVERSION"]

debug = ARGUMENTS.get('debug', 'no')
if debug == "yes":
    env["CXXFLAGS"] = Split("-O0 -DDEBUG -ggdb3 -W -Wall -ansi")
else:
    env["CXXFLAGS"] = Split("-O2 -W -Wall -ansi")

if env['tinygui']:
    env["CXXFLAGS"].append(" -DUSE_TINY_GUI")

if env['lowmem']:
    env["CXXFLAGS"].append("-DLOW_MEM")

if env['raw_sockets']:
    env["CXXFLAGS"].append("-DNETWORK_USE_RAW_SOCKETS")

#print "%s version %s, flags %s" % (env["CC"], cc_version, " ".join(env["CXXFLAGS"]))
if env["CC"] == "gcc":
    (major, minor, rev) = map(int, cc_version.split("."))
    if major*10+minor < 33:
        print "Your compiler version is too old"
        Exit(1)

targets = map(str, BUILD_TARGETS)

if ("wesnoth" in targets or "wesnoth_editor" in targets):
    if not conf.CheckLib('X11'):
        print "Needed X lib for game or editor and didn't find it; exiting!"
        Exit(1)
    if not conf.CheckLib('SDL'):
        print "Needed SDL lib for game or editor and didn't find it; exiting!"
        Exit(1)
    if not conf.CheckLib('SDL_net'):
        print "Needed SDL network lib for game or editor and didn't find it; exiting!"
        Exit(1)
    if not conf.CheckLib('SDL_ttf'):
        print "Needed SDL ttf font lib for game or editor and didn't find it; exiting!"
        Exit(1)
    if not conf.CheckLib('SDL_mixer'):
        print "Needed SDL sound mixer lib for game or editor and didn't find it; exiting!"
        Exit(1)
    if not conf.CheckLib('SDL_image'):
        print "Needed SDL image lib for game or editor and didn't find it; exiting!"
        Exit(1)

if "game" not in map(str, BUILD_TARGETS):
    print "*** Game build disabled, suppressing Python support."
    env["python"] = False

env = conf.Finish()

#
# Generate the config file
#

configsyms = {}

configsyms["DATADIR"] = envdict["datadir"]
configsyms["LOCALEDIR"] = envdict["localedir"]
configsyms["USE_DUMMYLOCALES"] = envdict["dummy_locales"]
#configsyms["USE_INTERNAL_DATA"] = envdict["internal_data"]

if "/" in configsyms["LOCALEDIR"]:	# FIXME: Will this break on Windows?
    configsyms["FULLLOCALEDIR"] = configsyms["LOCALEDIR"]
    configsyms["HAS_RELATIVE_LOCALEDIR"] = 0
else:
    configsyms["FULLLOCALEDIR"] = os.path.join(configsyms["DATADIR"], configsyms["LOCALEDIR"])
    configsyms["HAS_RELATIVE_LOCALEDIR"] = 1

if not envdict["icondir"]:
    envdict["icondir"] = os.path.join(envdict["datadir"], "icons")
configsyms["APP_ICON"] = envdict["icondir"]

if not envdict["desktopdir"]:
    envdict["desktopdir"] = os.path.join(envdict["datadir"], "applicationa")
configsyms["APP_ENTRY"] = envdict["desktopdir"]

def config_build(target, source, env):
    # Build a config.h file from configsyms
    assert(str(source[0]) == "SConstruct")
    wfp = open(str(target[0]), "w")
    wfp.write("// This file is geneated by an scons recipe.  Do not hand-hack!\n")
    for (sym, val) in configsyms.items():
        if type(val) == type(""):
            wfp.write('#define %s	"%s"\n' % (sym, val))
        elif val == True:
            wfp.write("#define %s	1\n" % sym)
        elif val == False:
            wfp.write("#undef %s\n" % sym)
        else:
            wfp.write("#define %s	%s\n" % (sym, val))
    wfp.close()
    return None
config_builder = Builder(action = config_build)
env.Append(BUILDERS = {'Config' : config_builder})
env.Config("src/config.h", "SConstruct")

#
# How to build the Wesnoth configuration file
#

wesconfig_h = '''
#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

//! @file wesconfig.h
//! Some defines: VERSION, PACKAGE, MIN_SAVEGAME_VERSION
//!
//! DO NOT MODIFY THIS FILE !!!
//! modify SConstruct otherwise the settings will be overwritten.



#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define VERSION "%(version)s"
# define PACKAGE "wesnoth"
# ifndef LOCALEDIR
#  define LOCALEDIR "translations"
# endif
#endif

/**
 * Some older savegames of Wesnoth cannot be loaded anymore,
 * this variable defines the minimum required version.
 * It is only to be updated upon changes that break *all* saves/replays
 * (break as in crash wesnoth, not compatibility issues like stat changes)
 */
#define MIN_SAVEGAME_VERSION "%(min_savegame_version)s"

#endif
'''

def wesconfig_build(target, source, env):
    # Build a file from the wesconfig_h template
    assert(str(source[0]) == "SConstruct")
    wfp = open(str(target[0]), "w")
    wfp.write(wesconfig_h % globals())
    wfp.close()
    return None
wesconfig_builder = Builder(action = wesconfig_build)
env.Append(BUILDERS = {'Wesconfig' : wesconfig_builder})
env.Wesconfig("src/wesconfig.h", "SConstruct")

# Build tests to crib from:
# http://silvertree.googlecode.com/svn/trunk/{SConstruct,scons/}
#
# Tips on MacOS scons usage
# http://www.scons.org/wiki/MacOSX
#
# Scons missing features:
# 1. [] overloading should be used more -- in particular, environment and
#    options dictionaries should be directly accessible through it.
# 2. Where's the command-existence test?
# 3. New builder: Make target from string in SConstruct itself.

# Local variables:
# mode: python
# end:
