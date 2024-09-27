/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/
#include "gettext.hpp"
#include "log.hpp"
#include "filesystem.hpp"

#include <algorithm>
#include <iterator>
#include <locale>
#include <map>
#include <boost/locale.hpp>
#include <set>
#include <type_traits>

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
			LOG_G << "Generating default locale";
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
				ERR_G << "Failed to generate default locale string. message:" << e.what();
			}
			LOG_G << "Finished generating default locale, default is now '" << name_ << "'";
		}

		std::string name_;
	};
	class wesnoth_message_format : public bl::message_format<char>
	{
	public:
		wesnoth_message_format(const std::locale& base, const std::set<std::string>& domains, const std::set<std::string>& paths)
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
			DBG_G << "Loading po files for language " << lang_name_long;
			for(auto& domain : domains) {
				DBG_G << "Searching for po files for domain " << domain;
				std::string path;
				for(auto base_path : paths) {
					DBG_G << "Searching in dir " << base_path;
					if(base_path[base_path.length()-1] != '/') {
						base_path += '/';
					}
					base_path += domain;
					base_path += '/';
					path = base_path + lang_name_long + ".po";
					DBG_G << "  Trying path " << path;
					if(filesystem::file_exists(path)) {
						break;
					}
					path = base_path + lang_name_short + ".po";
					DBG_G << "  Trying path " << path;
					if(filesystem::file_exists(path)) {
						break;
					}
				}
				if(!filesystem::file_exists(path)) {
					continue;
				}
				LOG_G << "Loading language file from " << path;
				try {
					filesystem::scoped_istream po_file = filesystem::istream_file(path);
					po_file->exceptions(std::ios::badbit);
					const auto& cat = spirit_po::default_catalog::from_istream(*po_file);
					extra_messages_.emplace(get_base().domain(domain), cat);
				} catch(const spirit_po::catalog_exception& e) {
					// Treat any parsing error in the same way as the file not existing - just leave
					// this domain untranslated but continue to load other domains.
					log_po_error(lang_name_long, domain, e.what());
				} catch(const std::ios::failure&) {
					log_po_error(lang_name_long, domain, strerror(errno));
				}
			}
		}

		static void log_po_error(const std::string& lang, const std::string& dom, const std::string& detail) {
			ERR_G << "Error opening language file for " << lang << ", textdomain " << dom
				<< ":\n  " << detail;
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

#if BOOST_VERSION < 108300
		const char* get(int domain_id, const char* ctx, const char* sid, int n) const override
#else
		const char* get(int domain_id, const char* ctx, const char* sid, bl::count_type n) const override
#endif
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
		std::map<int, spirit_po::default_catalog> extra_messages_;
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
				LOG_G << "Found boost locale backend: '" << name << "'";
			}

			generator_.use_ansi_encoding(false);
#if BOOST_VERSION < 108100
			generator_.categories(bl::message_facet | bl::information_facet | bl::collation_facet | bl::formatting_facet | bl::convert_facet);
			generator_.characters(bl::char_facet);
#else
			generator_.categories(bl::category_t::message | bl::category_t::information | bl::category_t::collation | bl::category_t::formatting | bl::category_t::convert);
			generator_.characters(bl::char_facet_t::char_f);
#endif
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
					  << "', skipping textdomain";
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

		/* This is called three times: once during the constructor, before any .mo files' paths have
		 * been added to the generator, once after adding the mainline .mo files, and once more
		 * after adding all add-ons. Corrupt .mo files might make the called functions throw, and so
		 * this might fail as soon as we've added message paths.
		 *
		 * Throwing exceptions from here is (in 1.15.18) going to end up in wesnoth.cpp's "Caught
		 * general ...  exception" handler, so the effect of letting an exception escape this
		 * function is an immediate exit. Given that, it doesn't seem useful to change the assert
		 * to a throw, at least not within the 1.16 branch.
		 *
		 * Postcondition: current_locale_ is a valid boost-generated locale, supplying the bl::info
		 * facet. If there are corrupt .mo files, the locale might have no translations loaded.
		 */
		void update_locale_internal()
		{
			try
			{
				LOG_G << "attempting to generate locale by name '" << current_language_ << "'";
				current_locale_ = generator_.generate(current_language_);
				current_locale_ = std::locale(current_locale_, new wesnoth_message_format(current_locale_, loaded_domains_, loaded_paths_));
				const bl::info& info = std::use_facet<bl::info>(current_locale_);
				LOG_G << "updated locale to '" << current_language_ << "' locale is now '" << current_locale_.name() << "' ( "
				      << "name='" << info.name()
				      << "' country='"  << info.country()
				      << "' language='"  << info.language()
				      << "' encoding='"  << info.encoding()
				      << "' variant='"  << info.variant() << "')";
			}
			catch(const bl::conv::conversion_error& e)
			{
				assert(std::has_facet<bl::info>(current_locale_));
				const bl::info& info = std::use_facet<bl::info>(current_locale_);
				ERR_G << "Failed to update locale due to conversion error (" << e.what() << ") locale is now: "
				      << "name='" << info.name()
				      << "' country='" << info.country()
				      << "' language='" << info.language()
				      << "' encoding='" << info.encoding()
				      << "' variant='" << info.variant()
				      << "'";
			}
			catch(const std::runtime_error& e)
			{
				assert(std::has_facet<bl::info>(current_locale_));
				const bl::info& info = std::use_facet<bl::info>(current_locale_);
				ERR_G << "Failed to update locale due to runtime error (" << e.what() << ") locale is now: "
				      << "name='" << info.name()
				      << "' country='" << info.country()
				      << "' language='" << info.language()
				      << "' encoding='" << info.encoding()
				      << "' variant='" << info.variant()
				      << "'";
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
#if BOOST_VERSION < 108100
			res << "generator categories='" << generator_.categories() << "'";
#else
			res << "generator categories='" <<
				static_cast<std::underlying_type<bl::category_t>::type>(generator_.categories()) << "'";
#endif
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
	return bl::dgettext(domain, msgid, get_manager().get_locale());
}
std::string egettext(char const *msgid)
{
	return msgid[0] == '\0' ? msgid : bl::gettext(msgid, get_manager().get_locale());
}

std::string dsgettext (const char * domainname, const char *msgid)
{
	std::string msgval = dgettext (domainname, msgid);
	if (msgval == msgid) {
		const char* firsthat = std::strchr (msgid, '^');
		if (firsthat == nullptr)
			msgval = msgid;
		else
			msgval = firsthat + 1;
	}
	return msgval;
}

namespace {

inline const char* is_unlocalized_string2(const std::string& str, const char* singular, const char* plural)
{
	if (str == singular) {
		return singular;
	}

	if (str == plural) {
		return plural;
	}

	return nullptr;
}

}

std::string dsngettext (const char * domainname, const char *singular, const char *plural, int n)
{
	std::string msgval = bl::dngettext(domainname, singular, plural, n, get_manager().get_locale());

	auto original = is_unlocalized_string2(msgval, singular, plural);
	if (original) {
		const char* firsthat = std::strchr (original, '^');
		if (firsthat == nullptr)
			msgval = original;
		else
			msgval = firsthat + 1;
	}
	return msgval;
}

void bind_textdomain(const char* domain, const char* directory, const char* /*encoding*/)
{
	LOG_G << "adding textdomain '" << domain << "' in directory '" << directory << "'";
	get_manager().add_messages_domain(domain);
	get_manager().add_messages_path(directory);
	get_manager().update_locale();
}

void set_default_textdomain(const char* domain)
{
	LOG_G << "set_default_textdomain: '" << domain << "'";
	get_manager().set_default_messages_domain(domain);
}


void set_language(const std::string& language, const std::vector<std::string>* /*alternates*/)
{
	// why should we need alternates? which languages we support should only be related
	// to which languages we ship with and not which the os supports
	LOG_G << "setting language to  '" << language << "'";
	get_manager().set_language(language);
}

int compare(const std::string& s1, const std::string& s2)
{

	try {
		return std::use_facet<std::collate<char>>(get_manager().get_locale()).compare(s1.c_str(), s1.c_str() + s1.size(), s2.c_str(), s2.c_str() + s2.size());
	} catch(const std::bad_cast&) {
		static bool bad_cast_once = false;

		if(!bad_cast_once) {
			ERR_G << "locale set-up for compare() is broken, falling back to std::string::compare()";
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

	try {
#if BOOST_VERSION < 108100
		return std::use_facet<bl::collator<char>>(get_manager().get_locale()).compare(
			bl::collator_base::secondary, s1, s2);
#else
		return std::use_facet<bl::collator<char>>(get_manager().get_locale()).compare(
			bl::collate_level::secondary, s1, s2);
#endif
	} catch(const std::bad_cast&) {
		static bool bad_cast_once = false;

		if(!bad_cast_once) {
			ERR_G << "locale set-up for icompare() is broken, falling back to std::string::compare()";

			try { //just to be safe.
				ERR_G << get_manager().debug_description();
			} catch (const std::exception& e) {
				ERR_G << e.what();
			}
			bad_cast_once = true;
		}

		// Let's convert at least ASCII letters to lowercase to get a somewhat case-insensitive comparison.
		return ascii_to_lowercase(s1).compare(ascii_to_lowercase(s2));
	}
#endif
}

std::string strftime(const std::string& format, const std::tm* time)
{
	std::basic_ostringstream<char> dummy;
	dummy.imbue(get_manager().get_locale());	// TODO: Calling imbue() with hard-coded locale appears to work with put_time in glibc, but not with get_locale()...
	// Revert to use of boost (from 1.14) instead of std::put_time() because the latter does not appear to handle locale properly in Linux
	dummy << bl::as::ftime(format) << mktime(const_cast<std::tm*>(time));

	return dummy.str();
}

bool ci_search(const std::string& s1, const std::string& s2)
{
	const std::locale& locale = get_manager().get_locale();

	std::string ls1 = bl::to_lower(s1, locale);
	std::string ls2 = bl::to_lower(s2, locale);

	return std::search(ls1.begin(), ls1.end(),
	                   ls2.begin(), ls2.end()) != ls1.end();
}

const boost::locale::info& get_effective_locale_info()
{
	return std::use_facet<boost::locale::info>(get_manager().get_locale());
}
}
