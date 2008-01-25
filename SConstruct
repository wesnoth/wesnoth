#
# SCons build description for the Wesnoth project
#
version = "1.3.14+svn"
min_savegame_version = "1.3.10"

#
# Build-control options
#

opts = Options()
opts.Add('debug', '"yes" to build for debugging', "no")
opts.Add('tests', '"yes" to enable static building of Wesnoth', "no")
opts.Add('python','"no" to disable Python support', 'yes')
opts.Add('lite', '"yes" to build lite version of wesnoth (without music or large images)', "no")
opts.Add('tinygui', '"yes" enables GUI reductions for resolutions down to 320x240, resize images before installing', "no")
opts.Add('optipng', '"yes" to run optipng PNG compression before installing graphics', "no")
opts.Add('lowmem', '"yes" to reduce memory usage by removing extra functionality', "no")
opts.Add('fribidi','"no" to disable bidirectional-language support', 'yes')
opts.Add('dummy-locales','"yes" to disable Wesnoth private locales', 'no')
#opts.Add('game', '"no" to disable compilation of game', 'yes')
#opts.Add('server', '"yes" to enable compilation of server', 'no')
#opts.Add('editor', '"yes" to enable compilation of editor', 'no')
#opts.Add('campaign-server', '"yes" to enable compilation of campaign server', 'no')
#opts.Add('python_install', 'Set to "yes" to enable installation of Python developer tools', "no")
#opts.Add('tool_install', 'Set to "yes" to build tools for artists and WML maintainers', "no")
opts.Add('datadir', 'read-only architecture-independent data', "/usr/share/")
opts.add('fifodir', 'directory for the wesnothd fifo socket file', "/var/run.wesnothd")
opts.add('server-uid', 'user id of the user who runs wesnothd', "")
opts.add('server-gid', 'group id of the user who runs wesnothd', "")
opts.Add('server-monitor', '"yes" to enable enable server monitor thread; libgtop2 is required', 'no')
# These will get generated into config.h
opts.Add('DATADIR', 'sets the Wesnoth data directory to a non-default location', "wesnoth")
opts.Add('LOCALEDIR', 'sets the locale data directory to a non-default location', "translations")

#
# Check some preconditions
#
env = Environment(options = opts)
Help(opts.GenerateHelpText(env))
conf = Configure(env)

configsyms = {}
for (key, value) in ARGUMENTS.items():
    if key.isupper():
        configsyms[key] = value

# Check the C++ compiler and version
import sys, commands
(status, gcc_version) = commands.getstatusoutput("g++ --version")
if status:
    print "GCC is not installed", status
    sys.exit(1)
else:
    debug = ARGUMENTS.get('debug', 'no')
    if debug == "yes":
        cxxflags = "-O0 -DDEBUG -ggdb3 -W -Wall -ansi"
    else:
        cxxflags = "-O2 -W -Wall -ansi"
    gcc_version = gcc_version.split()[2]
    print "GCC version %s, flags %s" % (gcc_version, cxxflags)
    (major, minor, rev) = map(int, gcc_version.split("."))
    if major*10+minor < 33:
        print "Your GCC version is too old"
        sys.exit(1)

if ARGUMENTS.get('tinygui', 'no') == 'yes':
    cxxflags += " -DUSE_TINY_GUI"

if ARGUMENTS.get('lowmem', 'no') == 'yes':
    cxxflags += " -DLOW_MEM"

test_build = "svn" in version

if "/" in configsyms["LOCALEDIR"]:	# FIXME: Will this break on Windows?
    configsyms["FULLLOCALEDIR"] = configsyms["LOCALEDIR"]
    configsyms["HAS_RELATIVE_LOCALEDIR"] = 0
else:
    configsyms["FULLLOCALEDIR"] = os.path.join(ARGUMENTS["datadir"], ARGUMENTS["DATADIR"], ARGUMENTS["LOCALEDIR"])
    configsyms["HAS_RELATIVE_LOCALEDIR"] = 1

# FIXME: Check for availability of optipng if optpng is yes

env = conf.Finish()

#
# Declare a default target
#

env.Default("wesnoth")

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

# Local variables:
# mode: python
# end:
