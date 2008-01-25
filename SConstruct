#
# SCons build description for the Wesnoth project
#
# This is a deliberately straight-line translation of the old configure.ac;
# it builds an autotools-like config.h for the C++ code.  The optipng
# option is omitted.
#
version = "1.3.14+svn"
min_savegame_version = "1.3.10"

#
# Build-control options
#

opts = Options()
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
opts.Add(BoolOption('internal_data', 'Set to put data in Mac OS X application fork', False))
opts.Add(BoolOption('tinygui', 'Set for GUI reductions for resolutions down to 320x240, resize images before installing', False))
opts.Add(BoolOption('display_revision', 'Set to enable svn revision display', False))
opts.Add(BoolOption('raw_sockets', 'Set to use raw receiving sockets in the multiplayer network layer rather than the SDL_net facilities', False))
# These will get generated into config.h
opts.Add(PathOption('datadir', 'read-only architecture-independent data', "/usr/share/"))
opts.Add(PathOption('DATADIR', 'sets the Wesnoth data directory to a non-default location', "wesnoth", PathOption.PathAccept))
opts.Add(PathOption('LOCALEDIR', 'sets the locale data directory to a non-default location', "translations", PathOption.PathAccept))

import os, sys, commands

# Check some preconditions
#
env = Environment(options = opts)
Help("""\
Available build targets include: game editor server campaign-server tools.
The 'install' target installs whatever you currently have built.
If you have built tools and Python is available the Python helper modules
will also be installed.

""" + opts.GenerateHelpText(env))
conf = Configure(env)

# Every environment symbol that is all caps (in particular those set by options)
# gets copied into configsyms so we can generate an autoconf-style config.h
# file from it.
configsyms = {}
for key in env.Dictionary().keys():
    if key.isupper() or key in ["datadir"]:
        configsyms[key] = env.Dictionary()[key]

# Check the C++ compiler and version
(status, gcc_version) = commands.getstatusoutput("g++ --version")
if status:
    print "GCC is not installed", status
    sys.exit(1)
else:
    gcc_version = gcc_version.split()[2]

    debug = ARGUMENTS.get('debug', 'no')
    if debug == "yes":
        env["CPPFLAGS"] = Split("-O0 -DDEBUG -ggdb3 -W -Wall -ansi")
    else:
        env["CPPFLAGS"] = Split("-O2 -W -Wall -ansi")

    if env['tinygui']:
        env["CPPFLAGS"].append(" -DUSE_TINY_GUI")

    if env['lowmem']:
        env["CPPFLAGS"].append("-DLOW_MEM")

    if env['raw_sockets']:
        env["CPPFLAGS"].append("-DNETWORK_USE_RAW_SOCKETS")

    print "GCC version %s, flags %s" % (gcc_version, " ".join(env["CPPFLAGS"]))
    (major, minor, rev) = map(int, gcc_version.split("."))
    if major*10+minor < 33:
        print "Your GCC version is too old"
        sys.exit(1)


test_build = "svn" in version

if "/" in configsyms["LOCALEDIR"]:	# FIXME: Will this break on Windows?
    configsyms["FULLLOCALEDIR"] = configsyms["LOCALEDIR"]
    configsyms["HAS_RELATIVE_LOCALEDIR"] = 0
else:
    configsyms["FULLLOCALEDIR"] = os.path.join(configsyms["datadir"], configsyms["DATADIR"], configsyms["LOCALEDIR"])
    configsyms["HAS_RELATIVE_LOCALEDIR"] = 1

env = conf.Finish()

#
# Declare a default target and deduce from it
#

env.Default("game")

if "game" not in map(str, BUILD_TARGETS):
    print "*** Game build disabled, suppressing Python support."
    env["python"] = False

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

# More build tests to crib from:
# http://silvertree.googlecode.com/svn/trunk/scons/

# Local variables:
# mode: python
# end:
