
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
# define VERSION "1.5.0-svn"
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
#define MIN_SAVEGAME_VERSION "1.3.10"

#endif
