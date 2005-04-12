#include "global.hpp"

#include "gettext.hpp"

#include <cstring>

char const *egettext(char const *msgid)
{
	return msgid[0] == '\0' ? msgid : gettext(msgid);
}

const char* sgettext (const char *msgid)
{
	const char *msgval = gettext (msgid);
	if (msgval == msgid) {
		msgval = strrchr (msgid, '^');
		if (msgval == NULL)
			msgval = msgid;
		else
			msgval++;
	}
	return msgval;
}

const char* dsgettext (const char * domainname, const char *msgid)
{
	bind_textdomain_codeset(domainname, "UTF-8");
	const char *msgval = dgettext (domainname, msgid);
	if (msgval == msgid) {
		msgval = strrchr (msgid, '^');
		if (msgval == NULL)
			msgval = msgid;
		else
			msgval++;
	}
	return msgval;
}
