#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define VERSION "1.3.2+svn"
# define WESNOTH_DEFAULT_SERVER "server.wesnoth.org:14998"
# define PACKAGE "wesnoth"
# ifndef LOCALEDIR
#  define LOCALEDIR "translations"
# endif
#endif
/**
 * WML errors can be shown in 2 ways, as dialog or as chat message
 * if WML_ERROR_DIALOG == 1 it's shown as dialog else as chat message
 * for development versions the dialog should be used for the stable
 * releases the chat messages
 */
#define WML_ERROR_DIALOG 0

/**
 * Some older savegames of Wesnoth can't be loaded anymore this
 * variable defines the minimum required version
 */
#define MIN_SAVEGAME_VERSION "1.3.1"

#endif
