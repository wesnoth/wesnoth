#
# SCons build description for the Wesnoth project
#
# This is a deliberately straight-line translation of the old configure.ac.
# The optipng and internal-data options are omitted.  The following constanta
# should be set before release.
#
import os, sys, commands

#
# Build-control options
#

opts = Options()

# These are implemented in the build section
opts.Add(BoolOption('debug', 'Set to build for debugging', False))
opts.Add(BoolOption('profile', 'Set to build for debugging', False))
opts.Add(BoolOption('strict', 'Set to strict compilation', False))
opts.Add(BoolOption('static', 'Set to enable static building of Wesnoth', False))
opts.Add(BoolOption('smallgui', 'Set for GUI reductions for resolutions down to 800x480 (eeePC, Nokia 8x0), resize images before installing', False))
opts.Add(BoolOption('tinygui', 'Set for GUI reductions for resolutions down to 320x240 (PDAs), resize images before installing', False))
opts.Add(BoolOption('lowmem', 'Set to reduce memory usage by removing extra functionality', False))
opts.Add(BoolOption('fribidi','Clear to disable bidirectional-language support', True))
opts.Add(BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False))
opts.Add(PathOption('prefsdir', 'user preferences directory', ".wesnoth", PathOption.PathAccept))
opts.Add(BoolOption('python', 'Enable in-game python extensions.', True))

# These are implemented in the installation productions
opts.Add(PathOption('prefix', 'autotools-style installation prefix', "/usr/local"))
opts.Add(PathOption('datadir', 'read-only architecture-independent game data', "share/wesnoth", PathOption.PathAccept))

# FIXME: These are not yet implemented
opts.Add(BoolOption('lite', 'Set to build lite version of wesnoth (no music or large images)', False))
opts.Add(BoolOption('dummy_locales','Set to enable Wesnoth private locales', False))
opts.Add(PathOption('fifodir', 'directory for the wesnothd fifo socket file', "/var/run/wesnothd", PathOption.PathAccept))
opts.Add('server_uid', 'user id of the user who runs wesnothd', "")
opts.Add('server_gid', 'group id of the user who runs wesnothd', "")
#opts.Add(BoolOption('internal_data', 'Set to put data in Mac OS X application fork', False))
opts.Add(BoolOption('desktop_entry','Clear to disable desktop-entry', True))
opts.Add(PathOption('localedir', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept))
opts.Add(PathOption('icondir', 'sets the icons directory to a non-default location', "icons", PathOption.PathAccept))
opts.Add(PathOption('desktopdir', 'sets the desktop entry directory to a non-default location', "applications", PathOption.PathAccept))

#
# Setup
#

# FIXME: Currently this will only work under Linux
svnrev = commands.getoutput("svnversion -n 2>/dev/null")

env = Environment(options = opts)

env.TargetSignatures('content')

env["CXXFLAGS"].append('-DSVNREV=\'"%s"\'' % svnrev)

# Omits the 'test' target 
all = env.Alias("all", ["wesnoth", "wesnoth_editor", "wesnothd", "campaignd",
                        "cutter", "exploder"])
env.Default("all")

#
# Configuration
#

#The 'install' target installs whatever you currently have built.  If
#you have built wmllint/wmlscope/wmlindent the Python helper modules
#will also be installed.

Help("""\
Available build targets include:

    wesnoth wesnoth_editor wesnothd campaignd exploder cutter
    all = all installables
    test = unit test binary (not an installable)
    TAGS = build tags for Emacs (cleaned by 'scons -c all').
    install = install all executables and tools
    uninstall = uninstall all executables and tools

""" + opts.GenerateHelpText(env))
conf = Configure(env)

#
# Check some preconditions
#

if float(sys.version[:3]) < 2.4:
    print "Python version is too old, 2.4 or greater is required,"
    Exit(1)

targets = map(str, BUILD_TARGETS)

if ("wesnoth" in targets or "wesnoth_editor" in targets):
    if not conf.CheckLib('X11'):
        print "Needed X lib for game or editor and didn't find it; exiting!"
        Exit(1)
    if not conf.CheckLib('SDL'):
        print "Needed SDL lib for game or editor and didn't find it; exiting!"
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
    if env['fribidi'] and conf.CheckLib('fribidi'):
        print "Can't find libfribidi, please install it or rebuild with fribidi=no."
        Exit(1)

if ("wesnoth" in targets or "wesnothd" in targets or "campaignd" in targets):
    if not conf.CheckLib('SDL_net'):
        print "Needed SDL network lib and didn't find it; exiting!"
        Exit(1)

if "all" in targets or "wesnoth" in targets:
    if not conf.CheckLib('python'+sys.version[:3]):
        print "Needed Python lib for game and didn't find it; exiting!"
        Exit(1)

env = conf.Finish()

#
# Implement configuration switches
#
extralibs=[]

if env["debug"]:
    env["CXXFLAGS"] += Split("-O0 -DDEBUG -ggdb3 -W -Wall -ansi")
else:
    env["CXXFLAGS"] += Split("-O2 -ansi")

if env['static']:
    env["LDFLAGS"].append("-all-static")

if env['profile']:
    env["CXXFLAGS"].append("-pg")

if env['strict']:
    env["CXXFLAGS"].append("-Werror -Wno-unused -Wno-sign-compare")

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

if env['prefsdir']:
    env["CXXFLAGS"].append("-DPREFERENCES_DIR='\"%s\"'" % env['prefsdir'])

if env['python']:
    env["CXXFLAGS"].append("-DHAVE_PYTHON")

# Simulate autools-like behavior of prefix and datadir
if not env["datadir"].startswith("/"):
    env["datadir"] = os.path.join(env["prefix"], env["datadir"])

env["CXXFLAGS"].append("-DWESNOTH_PATH='\"%s\"'" % env['datadir'])

cc_version = env["CCVERSION"]
if env["CC"] == "gcc":
    (major, minor, rev) = map(int, cc_version.split("."))
    if major*10+minor < 33:
        print "Your compiler version is too old"
        Exit(1)

#
# Libraries and source groups
#
boost_libs = Split("boost_iostreams-mt boost_regex")
SDL_libs = Split("SDL_net SDL_ttf SDL_mixer SDL_image SDL")
commonlibs = SDL_libs + boost_libs + ["pthread", "-lpython"+sys.version[:3]]
commonpath = ['src', '/usr/include/SDL', '/usr/include/python%s' % sys.version[:3]]

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
            CPPPATH = commonpath)

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

wesnoth = env.Program("wesnoth", ["src/game.cpp"] + wesnoth_sources,
            CPPPATH = commonpath + ['src/server'],
            LIBS = ['wesnoth_core', 'wesnoth_sdl', 'wesnoth', 'campaignd'] + commonlibs + extralibs,
            LIBPATH = [".", "/lib", "/usr/lib"])

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
wesnoth_editor = env.Program("wesnoth_editor", wesnoth_editor_sources,
            CPPPATH = commonpath,
            LIBS = ['wesnoth_core', 'wesnoth_sdl', 'wesnoth'] + commonlibs + extralibs,
            LIBPATH = [".", "/lib", "/usr/lib"])

campaignd_sources = [
    "src/campaign_server/campaign_server.cpp",
    ]
campaignd = env.Program("campaignd", campaignd_sources,
            CPPPATH = ['src', 'src/server', '/usr/include/SDL', '/usr/include/python%s' % sys.version[:3]],
            LIBS = ['wesnoth_core', 'wesnothd', 'campaignd', 'wesnoth'] + commonlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])

wesnothd_sources = [
    "src/server/game.cpp",
    "src/server/input_stream.cpp",
    "src/server/metrics.cpp",
    "src/server/player.cpp",
    "src/server/proxy.cpp",
    "src/server/server.cpp",
    "src/server/simple_wml.cpp",
    ]
wesnothd = env.Program("wesnothd", wesnothd_sources,
            CPPPATH = ['src', 'src/server', '/usr/include/SDL', '/usr/include/python%s' % sys.version[:3]],
            LIBS =  ['wesnoth_core', 'wesnothd'] + commonlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])

cutter_sources = [
    "src/tools/cutter.cpp",
    ]
cutter = env.Program("cutter", cutter_sources,
            CPPPATH = commonpath,
            LIBS =  ['cutter', 'wesnoth_core', 'wesnoth_sdl', 'wesnothd', 'wesnoth'] + commonlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])

exploder_sources = [
    "src/tools/exploder.cpp",
    "src/tools/exploder_composer.cpp",
    ]
exploder = env.Program("exploder", exploder_sources,
            CPPPATH = commonpath,
            LIBS =  ['cutter', 'wesnoth_core', 'wesnoth_sdl', 'wesnothd', 'wesnoth'] + commonlibs,
            LIBPATH = [".", "/lib", "/usr/lib"])

# FIXME: test build presently fails at link time.
test_sources = [
    "src/tests/main.cpp",
    "src/tests/test_util.cpp",
    ]
env.Program("test", test_sources,
            CPPPATH = commonpath + ['/usr/include'],
            LIBS =  ['wesnoth_core', 'wesnoth_sdl', 'wesnothd'] + commonlibs + ['boost_unit_test_framework'],
            LIBPATH = [".", "/lib", "/usr/lib"])

# FIXME: Include this in gameconfig.cpp when we switch over to scons.
# Because of the content check, scons will do the right thing.
# At that point the following line and -DSVNREV can be removed from CXXFLAGS.
env.Depends('src/game_config.o', 'revision_stamp.h')
r = env.Command("revision_stamp.h", [],
            'echo "#define REVISION \"%s\"" >revision_stamp.h' % svnrev)
env.AlwaysBuild(r)

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
pythonbins = [wesnoth, wesnoth_editor, cutter, exploder]
pythontools = Split("wmlscope wmllint wmlindent")
pythonmodules = Split("wmltools.py wmlparser.py wmldata.py wmliterator.py campaignserver_client.py libsvn.py __init__.py")

for binary in pythonbins:
    env.Install(bindir, binary)
for tool in pythontools:
    env.Install(bindir, 'data/tools/' + tool)
for module in pythonmodules:
    env.Install(pythonlib, 'data/tools/wesnoth/' + module)
for subdir in Split('data fonts icons images sounds'):
    env.Install(datadir, subdir)
env.Alias('install', [bindir, datadir, pythonlib])

#
# Un-installation
#
deletions = map(lambda x: Delete(os.path.join(bindir, str(x[0]))), pythonbins) \
            + [Delete(datadir), Delete(pythonlib)]
uninstall = env.Command('uninstall', '', deletions)
env.AlwaysBuild(uninstall)
env.Precious(uninstall)


#
# Known problems:
#
# 1. We don't yet check for SDL version too old
# 2. We don't check for Ogg Vorbis support in SDL_mixer
# 3. Translations are not yet installed.
# 4. Installation craps out with a mysterious "Is a directory" error.
# FIXME tags other problems

# Local variables:
# mode: python
# end:
