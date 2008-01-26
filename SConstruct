#
# SCons build description for the Wesnoth project
#
# This is a deliberately straight-line translation of the old configure.ac;
# it builds an autotools-like config.h for the C++ code.  The optipng
# and internal-data options are omitted.
#
version = "1.3.14+svn"
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
opts.Add(BoolOption('display_revision', 'Set to enable svn revision display', False))
opts.Add(BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False))
opts.Add(BoolOption('desktop_entry','Clear to disable desktop-entry', True))
opts.Add(PathOption('localedir', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept))
opts.Add(PathOption('icondir', 'sets the icons directory to a non-default location', "icons", PathOption.PathAccept))
opts.Add(PathOption('desktopdir', 'sets the desktop entry directory to a non-default location', "applications", PathOption.PathAccept))

#
# Setup
#

# FIXME: will need some elaboration under Windows
env = Environment(tools=['gcc'], options = opts)
env.Default("game")

#
# Program declarations (incomplete, no libraries yet)
#

if 0:
    env.Program("wesnothd", [
            "src/server/game.cpp",
            "src/server/input_stream.cpp",
            "src/server/metrics.cpp",
            "src/server/player.cpp",
            "src/server/proxy.cpp",
            "src/server/server.cpp",
            "src/server/monitor.cpp",
            "src/network.cpp",
            "src/network_worker.cpp",
            "src/loadscreen_empty.cp"
            ])

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

print "%s version %s, flags %s" % (env["CC"], cc_version, " ".join(env["CXXFLAGS"]))
if env["CC"] == "gcc":
    (major, minor, rev) = map(int, cc_version.split("."))
    if major*10+minor < 33:
        print "Your compiler version is too old"
        Exit(1)

targets = map(str, BUILD_TARGETS)

if 0:
    if ("game" in targets or "editor" in targets):
        if not conf.CheckLib('X11'):
            print "Needed X lib for game or editor and didn't find it; exiting!"
            Exit(1)
        if not conf.CheckLib('SDL'):
            print "Needed SDL lib for game or editor and didn't find it; exiting!"
            Exit(1)

if "game" not in map(str, BUILD_TARGETS):
    print "*** Game build disabled, suppressing Python support."
    env["python"] = False

env = conf.Finish()

test_build = "svn" in version

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
