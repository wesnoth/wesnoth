/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
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
/* FIXME: workaround, dsgettext2 is dsgettext but not processed by xgettext */
/* better is the approach in trunk of refactoring code to not use */
/* multiple text domains in one file */
#define dsgettext2 dsgettext

#ifdef GETTEXT_DOMAIN
# define _(String) dgettext(GETTEXT_DOMAIN,String)
# ifdef gettext
#  undef gettext
# endif
# define gettext(String) dgettext(GETTEXT_DOMAIN,String)
#else
# define _(String) gettext(String)
#endif

#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#endif
