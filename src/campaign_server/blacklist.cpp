/*
   Copyright (C) 2014 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "campaign_server/blacklist.hpp"

#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"

static lg::log_domain log_campaignd_bl("campaignd/blacklist");
#define LOG_BL LOG_STREAM(err, log_campaignd_bl)

namespace campaignd
{

blacklist::blacklist()
	: names_()
	, titles_()
	, descriptions_()
	, authors_()
	, ips_()
	, emails_()
{
}

blacklist::blacklist(const config& cfg)
	: names_()
	, titles_()
	, descriptions_()
	, authors_()
	, ips_()
	, emails_()
{
	this->read(cfg);
}

void blacklist::clear()
{
	names_.clear();
	titles_.clear();
	descriptions_.clear();

	authors_.clear();
	ips_.clear();
	emails_.clear();
}

void blacklist::read(const config& cfg)
{
	parse_str_to_globlist(cfg["name"], names_);
	parse_str_to_globlist(cfg["title"], titles_);
	parse_str_to_globlist(cfg["description"], descriptions_);

	parse_str_to_globlist(cfg["author"], authors_);
	parse_str_to_globlist(cfg["ip"], ips_);
	parse_str_to_globlist(cfg["email"], emails_);
}

void blacklist::parse_str_to_globlist(const std::string& str, blacklist::globlist& glist)
{
	glist = utils::split(str);
}

bool blacklist::is_blacklisted(const std::string& name,
							   const std::string& title,
							   const std::string& description,
							   const std::string& author,
							   const std::string& ip,
							   const std::string& email) const
{
	// Checks done in increasing order of performance impact and decreasing
	// order of relevance.
	return is_in_ip_masklist(ip, ips_) ||
		   is_in_globlist(email, emails_) ||
		   is_in_globlist(name, names_) ||
		   is_in_globlist(title, titles_) ||
		   is_in_globlist(author, authors_) ||
		   is_in_globlist(description, descriptions_);
}

bool blacklist::is_in_globlist(const std::string& str, const blacklist::globlist& glist) const
{
	if (!str.empty())
	{
		const std::string& lc_str = utf8::lowercase(str);
		for(const std::string& glob : glist)
		{
			const std::string& lc_glob = utf8::lowercase(glob);
			if (utils::wildcard_string_match(lc_str, lc_glob)) {
				LOG_BL << "Blacklisted field found: " << str << " (" << glob << ")\n";
				return true;
			}
		}
	}

	return false;
}

bool blacklist::is_in_ip_masklist(const std::string& ip, const blacklist::globlist& mlist) const
{
	if (!ip.empty())
	{
		for(const std::string& ip_mask : mlist)
		{
			if (ip_matches(ip, ip_mask)) {
				LOG_BL << "Blacklisted IP found: " << ip << " (" << ip_mask << ")\n";
				return true;
			}
		}
	}

	return false;
}

bool blacklist::ip_matches(const std::string& ip, const std::string& ip_mask) const
{
	// TODO: we want CIDR subnet mask matching here, not glob matching!
	return utils::wildcard_string_match(ip, ip_mask);
}

}
