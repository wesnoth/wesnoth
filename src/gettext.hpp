#ifndef GETTEXT_HPP_INCLUDED
#define GETTEXT_HPP_INCLUDED

// gettext-related declarations

#include "config.hpp"

#include <libintl.h>
#include <string>

const char* egettext (const char*);
const char* sgettext (const char*);
const char* dsgettext (const char * domainname, const char *msgid);
std::string vgettext (const char*,const string_map&);

#ifdef GETTEXT_DOMAIN
# define _(String) dgettext(GETTEXT_DOMAIN,String)
#else
# define _(String) gettext(String)
#endif

#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#endif
