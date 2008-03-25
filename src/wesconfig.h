
#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

//! @file wesconfig.h
//! Some defines: VERSION, PACKAGE, MIN_SAVEGAME_VERSION
//!
//! This file should only be modified by the packager of the tarball
//! before and after each release.

// We are building with scons, so Python cannot be absent. 
// this definition has to be done somewhere else or the normal builds
// via autotools are broken, which is ATM not acceptable
//#define HAVE_PYTHON

# define VERSION "1.5.0-svn"
# define PACKAGE "wesnoth"
# ifndef LOCALEDIR
#  define LOCALEDIR "translations"
# endif

/**
 * Some older savegames of Wesnoth cannot be loaded anymore,
 * this variable defines the minimum required version.
 * It is only to be updated upon changes that break *all* saves/replays
 * (break as in crash Wesnoth, not compatibility issues like stat changes)
 */
#define MIN_SAVEGAME_VERSION "1.3.10"

#endif
