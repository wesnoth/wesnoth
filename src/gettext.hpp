#ifndef GETTEXT_HPP_INCLUDED
#define GETTEXT_HPP_INCLUDED

#include "config.hpp"

// gettext-related declarations
#include <libintl.h>
const char* sgettext (const char*);
const char* vgettext (const char*,const string_map&);
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#endif
