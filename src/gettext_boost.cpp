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
#include <boost/foreach.hpp>
#include <set>


#define DBG_G LOG_STREAM(debug, lg::general)
#define LOG_G LOG_STREAM(info, lg::general)
#define WRN_G LOG_STREAM(warn, lg::general)
#define ERR_G LOG_STREAM(err, lg::general)

namespace bl = boost::locale;
namespace
{
	class default_utf8_locale_name
	{
	public:
		static const std::string& name()
		{
			//Use pointers becasue we don't want it to be destructed at programm end.
			static default_utf8_locale_name* lname = new default_utf8_locale_name();
			return lname->name_;
		}
	private:
		default_utf8_locale_name() 
			: name_()
		{
			LOG_G << "Generating default locale\n";
			try
			{
				//NOTE: the default_locale objects needs to live as least as long as the locale_info object. Otherwise the programm will segfault.
				std::locale default_locale = bl::generator().generate("");
				const boost::locale::info& locale_info = std::use_facet< boost::locale::info >(default_locale);
				name_ += locale_info.language();
				if(!locale_info.country().empty())
					name_ += "_" + locale_info.country();
				name_ += ".UTF-8";
				if(!locale_info.variant().empty())
					name_ += "@" + locale_info.variant();
			}
			catch(const std::exception& e)
			{
				ERR_G << "Failed to generate default locale string. message:" << e.what() << std::endl;
			}
			LOG_G << "Finished generating default locale, default is now '" << name_ << "'\n";
		}

		std::string name_;
	};

	struct translation_manager
	{
		translation_manager()
			: loaded_paths_()
			, loaded_domains_()
			, current_language_()
			, generator_()
			, current_locale_()
		{
			current_language_ = default_utf8_locale_name::name();
			const bl::localization_backend_manager& g_mgr = bl::localization_backend_manager::global();
			BOOST_FOREACH(const std::string& name, g_mgr.get_all_backends())
			{
				LOG_G << "Found boost locale backend: '" << name << "'\n";
			}
			generator_.use_ansi_encoding(false);
			update_locale();
		}
		void update_locale()
		{
			try
			{
				LOG_G << "attemptng to generate locale by name '" << current_language_ << "'\n";
				current_locale_ = generator_.generate(current_language_);
				const boost::locale::info& info = std::use_facet< boost::locale::info >(current_locale_);
				LOG_G << "updated locale to '" << current_language_ << "' locale is now '" << current_locale_.name() << "' ( "
				      << "name='" << info.name()
				      << "' country='"  << info.country()
				      << "' language='"  << info.language()
				      << "' encoding='"  << info.encoding()
				      << "' variant='"  << info.variant() << "')\n";
			}
			catch(const boost::locale::conv::conversion_error&)
			{
				const boost::locale::info& info = std::use_facet< boost::locale::info >(current_locale_);
				ERR_G << "Failed to update locale due to conversion error, locale is now: " 
				      << "name='" << info.name()
				      << "' country='" << info.country()
				      << "' language='" << info.language()
				      << "' encoding='" << info.encoding()
				      << "' variant='" << info.variant() 
				      << "'" << std::endl;
			}
		}

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
	LOG_G << "set_default_textdomain: '" << domain << "'\n";
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
		get_manager().current_language_ = default_utf8_locale_name::name();	
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
