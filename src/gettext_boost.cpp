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

#include "global.hpp"
#include "gettext.hpp"
#include "log.hpp"

#include <iostream>
#include <locale>
#include <boost/locale.hpp>
#include <set>


#define DBG_G LOG_STREAM(debug, lg::general)
#define LOG_G LOG_STREAM(info, lg::general)
#define WRN_G LOG_STREAM(warn, lg::general)
#define ERR_G LOG_STREAM(err, lg::general)

namespace 
{
	struct translation_manager
	{
		translation_manager()
			: loaded_paths_()
			, loaded_domains_()
			, current_language_()
			, generator_()
			, current_locale_()
		{
			generator_.use_ansi_encoding(false);
			update_locale();
		}
		void update_locale() { current_locale_ = generator_.generate(current_language_); }

		std::set<std::string> loaded_paths_;
		std::set<std::string> loaded_domains_;
		std::string current_language_;
		boost::locale::generator generator_;
		std::locale current_locale_;
	};

	translation_manager& get_manager()
	{
		static translation_manager* mng = new translation_manager();
		return *mng;
	}

}

namespace translation
{
	
std::string dgettext(const char* domain, const char* msgid)
{
	return boost::locale::dgettext(domain, msgid, get_manager().current_locale_);
}
std::string egettext(char const *msgid)
{
	return msgid[0] == '\0' ? msgid : boost::locale::gettext(msgid, get_manager().current_locale_);
}

std::string dsgettext (const char * domainname, const char *msgid)
{
	std::string msgval = dgettext (domainname, msgid);
	if (msgval == msgid) {
		const char* firsthat = std::strrchr (msgid, '^');
		if (firsthat == NULL)
			msgval = msgid;
		else
			msgval = firsthat + 1;
	}
	return msgval;
}

std::string dsngettext (const char * domainname, const char *singular, const char *plural, int n)
{
	std::string msgval = boost::locale::dngettext(domainname, singular, plural, n, get_manager().current_locale_);
	if (msgval == singular) {
		const char* firsthat = std::strrchr (singular, '^');
		if (firsthat == NULL)
			msgval = singular;
		else
			msgval = firsthat + 1;
	}
	return msgval;
}

void bind_textdomain(const char* domain, const char* direcory, const char* /*encoding*/)
{
	LOG_G << "adding textdomain '" << domain << "' in directory '" << direcory << "'\n";
	get_manager().generator_.add_messages_domain(domain);
	get_manager().generator_.add_messages_path(direcory);
	get_manager().update_locale();
}

void set_default_textdomain(const char* domain)
{
	get_manager().generator_.set_default_messages_domain(domain);
	get_manager().update_locale();
}


void set_language(const std::string& language, const std::vector<std::string>* /*alternates*/)
{
	// why shoudl we need alternates? which languages we support shoudl only be related 
	// to which languages we ship with and not which the os supports
	LOG_G << "setting language to  '" << language << "' \n";
	std::string::size_type at_pos = language.rfind('@');
	if(language.empty())
	{
		//Don't add "UTF-8" to default language.
		get_manager().current_language_ = "";
	}
	else if(at_pos  != std::string::npos)
	{
		get_manager().current_language_ = language.substr(0, at_pos) + ".UTF-8" + language.substr(at_pos);
	}
	else
	{
		get_manager().current_language_ = language + ".UTF-8";
	}
	get_manager().update_locale();
}

void init()
{

}
}
