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
opts.Add('python_install', 'Set to "yes" to enable installation of Python developer tools', "no")
opts.Add('lite', '"yes" to build lite version of wesnoth (without music or large images)', "no")
opts.Add('tinygui', '"yes" enables GUI reductions for resolutions down to 320x240, resize images before installing', "no")

#
# Check some preconditions
#
env = Environment(options = opts)
Help(opts.GenerateHelpText(env))
conf = Configure(env)

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

test_build = "svn" in version


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
