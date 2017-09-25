#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

/**
 * @file wesconfig.h
 * Some defines: VERSION, PACKAGE, MIN_SAVEGAME_VERSION
 *
 * This file should only be modified by the packager of the tarball
 * before and after each release.
 */

#ifndef RC_INVOKED

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifndef LOCALEDIR
#  define LOCALEDIR "translations"
#endif

#ifndef PACKAGE
#define PACKAGE "wesnoth"
#endif

/**
 * Some older savegames of Wesnoth cannot be loaded anymore,
 * this variable defines the minimum required version.
 * It is only to be updated upon changes that break *all* saves/replays
 * (break as in crash Wesnoth, not compatibility issues like stat changes)
 * An example of such a change is changing the savegame format.
 */
#define MIN_SAVEGAME_VERSION "1.3.10"

#endif /* !RC_INVOKED */

//always use the version string in here, otherwise autotools can override in
//a bad way...
#ifdef VERSION
  #undef VERSION
#endif

#define VERSION "1.13.10"

// Used for the Windows executables' version info resource.
#define RC_VERSION_MAJOR        1
#define RC_VERSION_MINOR        13
#define RC_VERSION_REVISION     10

#endif
