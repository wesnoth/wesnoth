#include "gettext.hpp"

#include <cstring>

const char* sgettext (const char *msgid)
{
	const char *msgval = gettext (msgid);
	if (msgval == msgid) {
		msgval = strrchr (msgid, '|');
		if (msgval == NULL)
			msgval = msgid;
		else
			msgval++;
	}
	return msgval;
}
