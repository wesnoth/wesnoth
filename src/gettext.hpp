/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GETTEXT_HPP_INCLUDED
#define GETTEXT_HPP_INCLUDED

// gettext-related declarations

#include <libintl.h>

const char* egettext(const char*);
const char* sgettext(const char*);
const char* dsgettext(const char * domainname, const char *msgid);
const char* sngettext(const char *singular, const char *plural, int n);
const char* dsngettext(const char * domainname, const char *singular, const char *plural, int n);

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
