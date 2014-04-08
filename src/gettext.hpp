/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GETTEXT_HPP_INCLUDED
#define GETTEXT_HPP_INCLUDED

/**
 * How to use gettext for wesnoth source files:
 * -# include this header file in the .cpp file
 * -# make sure, that the source file is listed in the respective POTFILES.in
 *    for the textdomain, in the case of wesnoth-lib it is this file:
 *    po/wesnoth-lib/POTFILES.in
 * -# add the following include to set the correct textdomain, in this example
 *    wesnoth-lib (not required for the domain 'wesnoth', required for all
 *    other textdomains).
 *    @code
 *    #define GETTEXT_DOMAIN "wesnoth-lib"
 *    @endcode
 *
 * This should be all that is required to have your strings that are marked
 * translatable in the po files and translated ingame. So you at least have
 * to mark the strings translatable, too. ;)
 */

// gettext-related declarations

#include <libintl.h>

#ifdef setlocale
// Someone in libintl world decided it was a good idea to define a "setlocale" macro.
#undef setlocale
#endif

const char* egettext(const char*);
const char* sgettext(const char*);
const char* dsgettext(const char * domainname, const char *msgid);
const char* sngettext(const char *singular, const char *plural, int n);
const char* dsngettext(const char * domainname, const char *singular, const char *plural, int n);

//A Hack to make the eclipse-cdt parser happy.
#ifdef __CDT_PARSER__
#define GETTEXT_DOMAIN ""
#endif

#ifdef GETTEXT_DOMAIN
# define _(String) dsgettext(GETTEXT_DOMAIN,String)
# define _n(String1,String2,Int) dsngettext(String1,String2,Int)
# ifdef gettext
#  undef gettext
# endif
# define gettext(String) dgettext(GETTEXT_DOMAIN,String)
# define sgettext(String) dsgettext(GETTEXT_DOMAIN,String)
# define sngettext(String1,String2,Int) dsngettext(GETTEXT_DOMAIN,String1,String2,Int)
#else
# define _(String) sgettext(String)
# define _n(String1,String2,Int) sngettext(String1,String2,Int)
#endif

#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#endif
