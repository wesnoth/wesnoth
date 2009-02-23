
#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

/**
 * @file wesconfig.h
 * Some defines: VERSION, PACKAGE, MIN_SAVEGAME_VERSION
 *
 * This file should only be modified by the packager of the tarball
 * before and after each release.
 */

// without this ifdef DUMMYLOCALES break, so leave it in even though is seems
// to not have any real purpose...
#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# ifndef LOCALEDIR
#  define LOCALEDIR "translations"
# endif
#endif

//always use the version string in here, otherwise autotools can override in
//a bad way...
#ifdef VERSION
  #undef VERSION
#endif
#define VERSION "1.5.10+svn"

#ifndef PACKAGE
#define PACKAGE "wesnoth"
#endif

/**
 * Some older savegames of Wesnoth cannot be loaded anymore,
 * this variable defines the minimum required version.
 * It is only to be updated upon changes that break *all* saves/replays
 * (break as in crash Wesnoth, not compatibility issues like stat changes)
 */
#define MIN_SAVEGAME_VERSION "1.3.10"

#endif
