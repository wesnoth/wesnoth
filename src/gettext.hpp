#ifndef GETTEXT_HPP_INCLUDED
#define GETTEXT_HPP_INCLUDED

// gettext-related declarations
#include <libintl.h>
const char* sgettext (const char*);
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#endif
