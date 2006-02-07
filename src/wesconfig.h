#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define VERSION "1.1.1"
# define WESNOTH_DEFAULT_SERVER "devsrv.wesnoth.org:14999"
# define PACKAGE "wesnoth"
# ifndef LOCALEDIR
#  define LOCALEDIR "translations"
# endif
#endif

#endif
