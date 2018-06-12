/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
#include "filesystem.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <fstream>
#include <locale>
#include <mutex>
#include <boost/locale.hpp>
#include <set>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#include "spirit_po/spirit_po.hpp"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#define DBG_G LOG_STREAM(debug, lg::general())
#define LOG_G LOG_STREAM(info, lg::general())
#define WRN_G LOG_STREAM(warn, lg::general())
#define ERR_G LOG_STREAM(err, lg::general())

namespace bl = boost::locale;
namespace
{
	std::mutex& get_mutex() { static std::mutex* m = new std::mutex(); return *m; }

	class default_utf8_locale_name
	{
	public:
		static const std::string& name()
		{
			//Use pointers because we don't want it to be destructed at program end.
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
				//NOTE: the default_locale objects needs to live as least as long as the locale_info object. Otherwise the program will segfault.
				std::locale default_locale = bl::generator().generate("");
				const bl::info& locale_info = std::use_facet<bl::info>(default_locale);
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
	class wesnoth_message_format : public bl::message_format<char>
	{
		using po_catalog = spirit_po::catalog<>;
	public:
		wesnoth_message_format(std::locale base, const std::set<std::string>& domains, const std::set<std::string>& paths)
			: base_loc_(base)
		{
			const bl::info& inf = std::use_facet<bl::info>(base);
			if(inf.language() == "c") {
				return;
			}
			std::string lang_name_short = inf.language();
			std::string lang_name_long = lang_name_short;
			if(!inf.country().empty()) {
				lang_name_long += '_';
				lang_name_long += inf.country();
			}
			if(!inf.variant().empty()) {
				lang_name_long += '@';
				lang_name_long += inf.variant();
				lang_name_short += '@';
				lang_name_short += inf.variant();
			}
			DBG_G << "Loading po files for language " << lang_name_long << '\n';
			for(auto& domain : domains) {
				DBG_G << "Searching for po files for domain " << domain << '\n';
				std::string path;
				for(auto base_path : paths) {
					DBG_G << "Searching in dir " << base_path << '\n';
					if(base_path[base_path.length()-1] != '/') {
						base_path += '/';
					}
					base_path += domain;
					base_path += '/';
					path = base_path + lang_name_long + ".po";
					DBG_G << "  Trying path " << path << '\n';
					if(filesystem::file_exists(path)) {
						break;
					}
					path = base_path + lang_name_short + ".po";
					DBG_G << "  Trying path " << path << '\n';
					if(filesystem::file_exists(path)) {
						break;
					}
				}
				if(!filesystem::file_exists(path)) {
					continue;
				}
				std::ifstream po_file;
				po_file.exceptions(std::ios::badbit);
				LOG_G << "Loading language file from " << path << '\n';
				try {
					po_file.open(path);
					const po_catalog& cat = po_catalog::from_istream(po_file);
					extra_messages_.emplace(get_base().domain(domain), cat);
				} catch(const spirit_po::catalog_exception& e) {
					throw_po_error(lang_name_long, domain, e.what());
				} catch(const std::ios::failure&) {
					throw_po_error(lang_name_long, domain, strerror(errno));
				}
			}
		}

		[[noreturn]] static void throw_po_error(const std::string& lang, const std::string& dom, const std::string& detail) {
			std::ostringstream err;
			err << "Error opening language file for " << lang << ", textdomain " << dom
				<< ":\n  " << detail << '\n';
			ERR_G << err.rdbuf() << std::flush;
			throw game::error(err.str());
		}

		const char* get(int domain_id, const char* ctx, const char* msg_id) const override
		{
			auto& base = get_base();
			const char* msg = base.get(domain_id, ctx, msg_id);
			if(msg == nullptr) {
				auto iter = extra_messages_.find(domain_id);
				if(iter == extra_messages_.end()) {
					return nullptr;
				}
				auto& catalog = iter->second;
				const char* lookup = ctx ? catalog.pgettext(ctx, msg_id) : catalog.gettext(msg_id);
				if(lookup != msg_id) {
					// (p)gettext returns the input pointer if the string was not found
					msg = lookup;
				}
			}
			return msg;
		}

		const char* get(int domain_id, const char* ctx, const char* sid, int n) const override
		{
			auto& base = get_base();
			const char* msg = base.get(domain_id, ctx, sid, n);
			if(msg == nullptr) {
				auto iter = extra_messages_.find(domain_id);
				if(iter == extra_messages_.end()) {
					return nullptr;
				}
				auto& catalog = iter->second;
				const char* lookup = ctx ? catalog.npgettext(ctx, sid, sid, n) : catalog.ngettext(sid, sid, n);
				if(lookup != sid) {
					// n(p)gettext returns one of the input pointers if the string was not found
					msg = lookup;
				}
			}
			return msg;
		}

		int domain(const std::string& domain) const override
		{
			auto& base = get_base();
			return base.domain(domain);
		}

		const char* convert(const char* msg, std::string& buffer) const override
		{
			auto& base = get_base();
			return base.convert(msg, buffer);
		}
	private:
		const bl::message_format<char>& get_base() const
		{
			return std::use_facet<bl::message_format<char>>(base_loc_);
		}

		std::locale base_loc_;
		std::map<int, po_catalog> extra_messages_;
	};
	struct translation_manager
	{
		translation_manager()
			: loaded_paths_()
			, loaded_domains_()
			, current_language_(default_utf8_locale_name::name())
			, generator_()
			, current_locale_()
			, is_dirty_(true)
		{
			const bl::localization_backend_manager& g_mgr = bl::localization_backend_manager::global();
			for(const std::string& name : g_mgr.get_all_backends())
			{
				LOG_G << "Found boost locale backend: '" << name << "'\n";
			}

			generator_.use_ansi_encoding(false);
			generator_.categories(bl::message_facet | bl::information_facet | bl::collation_facet | bl::formatting_facet);
			generator_.characters(bl::char_facet);
			// We cannot have current_locale_ be a non boost-generated locale since it might not supply
			// the bl::info facet. As soon as we add message paths, update_locale_internal might fail,
			// for example because of invalid .mo files. So make sure we call it at least once before adding paths/domains
			update_locale_internal();
		}

		void add_messages_domain(const std::string& domain)
		{
			if(loaded_domains_.find(domain) != loaded_domains_.end())
			{
				return;
			}

			if(domain.find('/') != std::string::npos)
			{
				// Forward slash has a specific meaning in Boost.Locale domain
				// names, specifying the encoding. We use UTF-8 for everything
				// so we can't possibly support that, and odds are it's a user
				// mistake (as in bug #23839).
				ERR_G << "illegal textdomain name '" << domain
					  << "', skipping textdomain\n";
				return;
			}

			generator_.add_messages_domain(domain);
			loaded_domains_.insert(domain);
		}

		void add_messages_path(const std::string& path)
		{
			if(loaded_paths_.find(path) != loaded_paths_.end())
			{
				return;
			}
			generator_.add_messages_path(path);
			loaded_paths_.insert(path);
		}

		void set_default_messages_domain(const std::string& domain)
		{
			generator_.set_default_messages_domain(domain);
			update_locale();
		}

		void set_language(const std::string& language)
		{
			std::string::size_type at_pos = language.rfind('@');
			if(language.empty())
			{
				current_language_ = default_utf8_locale_name::name();
			}
			else if(at_pos  != std::string::npos)
			{
				current_language_ = language.substr(0, at_pos) + ".UTF-8" + language.substr(at_pos);
			}
			else
			{
				current_language_ = language + ".UTF-8";
			}
			update_locale();
		}

		void update_locale()
		{
			is_dirty_ = true;
		}

		void update_locale_internal()
		{
			try
			{
				LOG_G << "attempting to generate locale by name '" << current_language_ << "'\n";
				current_locale_ = generator_.generate(current_language_);
				current_locale_ = std::locale(current_locale_, new wesnoth_message_format(current_locale_, loaded_domains_, loaded_paths_));
				const bl::info& info = std::use_facet<bl::info>(current_locale_);
				LOG_G << "updated locale to '" << current_language_ << "' locale is now '" << current_locale_.name() << "' ( "
				      << "name='" << info.name()
				      << "' country='"  << info.country()
				      << "' language='"  << info.language()
				      << "' encoding='"  << info.encoding()
				      << "' variant='"  << info.variant() << "')\n";
			}
			catch(const bl::conv::conversion_error&)
			{
				assert(std::has_facet<bl::info>(current_locale_));
				const bl::info& info = std::use_facet<bl::info>(current_locale_);
				ERR_G << "Failed to update locale due to conversion error, locale is now: "
				      << "name='" << info.name()
				      << "' country='" << info.country()
				      << "' language='" << info.language()
				      << "' encoding='" << info.encoding()
				      << "' variant='" << info.variant()
				      << "'" << std::endl;
			}
			is_dirty_ = false;
		}

		std::string debug_description()
		{
			std::stringstream res;
			const bl::localization_backend_manager& g_mgr = bl::localization_backend_manager::global();
			for(const std::string& name : g_mgr.get_all_backends())
			{
				res << "has backend: '" << name << "',";
			}
			if(std::has_facet<bl::info>(current_locale_)) {
				const bl::info& info = std::use_facet<bl::info>(current_locale_);
				res << " locale: (name='" << info.name()
			 	     << "' country='" << info.country()
			 	     << "' language='" << info.language()
			 	     << "' encoding='" << info.encoding()
			 	     << "' variant='" << info.variant()
			 	     << "'),";
			}
			if(std::has_facet<bl::collator<char>>(current_locale_)) {
				res << "has bl::collator<char> facet, ";
			}
			res << "generator categories='" << generator_.categories() << "'";
			return res.str();
		}

		const std::locale& get_locale()
		{
			if(is_dirty_)
			{
				update_locale_internal();
			}
			return current_locale_;
		}

	private:
		std::set<std::string> loaded_paths_;
		std::set<std::string> loaded_domains_;
		std::string current_language_;
		bl::generator generator_;
		std::locale current_locale_;
		bool is_dirty_;
	};

	translation_manager& get_manager()
	{
		static translation_manager* mng = new translation_manager();
		return *mng;
	}

	// Converts ASCII letters to lowercase. Ignores Unicode letters.
	std::string ascii_to_lowercase(const std::string& str)
	{
		std::string result;
		result.reserve(str.length());
		std::transform(str.begin(), str.end(), std::back_inserter(result), [](char c)
		{
			return c >= 'A' && c <= 'Z' ? c | 0x20 : c;
		});
		return result;
	}
}

namespace translation
{

std::string dgettext(const char* domain, const char* msgid)
{
	std::lock_guard<std::mutex> lock(get_mutex());
	return bl::dgettext(domain, msgid, get_manager().get_locale());
}
std::string egettext(char const *msgid)
{
	std::lock_guard<std::mutex> lock(get_mutex());
	return msgid[0] == '\0' ? msgid : bl::gettext(msgid, get_manager().get_locale());
}

std::string dsgettext (const char * domainname, const char *msgid)
{
	std::string msgval = dgettext (domainname, msgid);
	if (msgval == msgid) {
		const char* firsthat = std::strrchr (msgid, '^');
		if (firsthat == nullptr)
			msgval = msgid;
		else
			msgval = firsthat + 1;
	}
	return msgval;
}

std::string dsngettext (const char * domainname, const char *singular, const char *plural, int n)
{
	//TODO: only the next line needs to be in the lock.
	std::lock_guard<std::mutex> lock(get_mutex());
	std::string msgval = bl::dngettext(domainname, singular, plural, n, get_manager().get_locale());
	if (msgval == singular) {
		const char* firsthat = std::strrchr (singular, '^');
		if (firsthat == nullptr)
			msgval = singular;
		else
			msgval = firsthat + 1;
	}
	return msgval;
}

void bind_textdomain(const char* domain, const char* directory, const char* /*encoding*/)
{
	LOG_G << "adding textdomain '" << domain << "' in directory '" << directory << "'\n";
	std::lock_guard<std::mutex> lock(get_mutex());
	get_manager().add_messages_domain(domain);
	get_manager().add_messages_path(directory);
	get_manager().update_locale();
}

void set_default_textdomain(const char* domain)
{
	LOG_G << "set_default_textdomain: '" << domain << "'\n";
	std::lock_guard<std::mutex> lock(get_mutex());
	get_manager().set_default_messages_domain(domain);
}


void set_language(const std::string& language, const std::vector<std::string>* /*alternates*/)
{
	// why should we need alternates? which languages we support should only be related
	// to which languages we ship with and not which the os supports
	LOG_G << "setting language to  '" << language << "' \n";
	std::lock_guard<std::mutex> lock(get_mutex());
	get_manager().set_language(language);
}

int compare(const std::string& s1, const std::string& s2)
{
	std::lock_guard<std::mutex> lock(get_mutex());

	try {
		return std::use_facet<std::collate<char>>(get_manager().get_locale()).compare(s1.c_str(), s1.c_str() + s1.size(), s2.c_str(), s2.c_str() + s2.size());
	} catch(const std::bad_cast&) {
		static bool bad_cast_once = false;

		if(!bad_cast_once) {
			ERR_G << "locale set-up for compare() is broken, falling back to std::string::compare()\n";
			bad_cast_once = true;
		}

		return s1.compare(s2);
	}
}

int icompare(const std::string& s1, const std::string& s2)
{
	// todo: maybe we should replace this preprocessor check with a std::has_facet<bl::collator<char>> check?
#ifdef __APPLE__
	// https://github.com/wesnoth/wesnoth/issues/2094
	return compare(ascii_to_lowercase(s1), ascii_to_lowercase(s2));
#else
	std::lock_guard<std::mutex> lock(get_mutex());

	try {
		return std::use_facet<bl::collator<char>>(get_manager().get_locale()).compare(
			bl::collator_base::secondary, s1, s2);
	} catch(const std::bad_cast&) {
		static bool bad_cast_once = false;

		if(!bad_cast_once) {
			ERR_G << "locale set-up for icompare() is broken, falling back to std::string::compare()\n";
			
			try { //just to be safe.
				ERR_G << get_manager().debug_description() << "\n";
			} catch (const std::exception& e) {
				ERR_G << e.what() << "\n";
			}
			bad_cast_once = true;
		}
<<<<<<< HEAD:src/gettext_boost.cpp

		// Let's convert at least ASCII letters to lowercase to get a somewhat case-insensitive comparison.
		return ascii_to_lowercase(s1).compare(ascii_to_lowercase(s2));
	}
#endif
}
=======
>>>>>>> c9bb31d1cc9182a4450f29ae529416d2192ab4f9:src/gettext.cpp

		// Let's convert at least ASCII letters to lowercase to get a somewhat case-insensitive comparison.
		return ascii_to_lowercase(s1).compare(ascii_to_lowercase(s2));
	}
#endif
}

std::string strftime(const std::string& format, const std::tm* time)
{
	std::basic_ostringstream<char> dummy;
	std::lock_guard<std::mutex> lock(get_mutex());
	dummy.imbue(get_manager().get_locale());
	dummy << std::put_time(time, format.c_str());

	return dummy.str();
}

}
