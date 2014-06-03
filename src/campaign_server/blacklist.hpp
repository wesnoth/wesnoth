/*
   Copyright (C) 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CAMPAIGN_SERVER_BLACKLIST_HPP_INCLUDED
#define CAMPAIGN_SERVER_BLACKLIST_HPP_INCLUDED

#include "config.hpp"

#include <boost/noncopyable.hpp>

namespace campaignd
{

class blacklist : private boost::noncopyable
{
public:
	typedef std::string       glob;
	typedef std::vector<glob> globlist;

	blacklist();
	explicit blacklist(const config& cfg);

	void clear();

	/**
	 * Initializes the blacklist from WML.
	 *
	 * @param cfg WML node object with the contents of the [blacklist] tag.
	 */
	void read(const config& cfg);

	/**
	 * Writes the blacklist to a WML node.
	 *
	 * @param cfg WML node object to write to. Any existing contents are
	 *            erased by this method.
	 */
	void write(config& cfg) const;

	/**
	 * Whether an add-on described by these fields is blacklisted.
	 *
	 * Empty parameters are ignored.
	 */
	bool is_blacklisted(const std::string& name,
						const std::string& title,
						const std::string& description,
						const std::string& author,
						const std::string& ip,
						const std::string& email) const;

private:
	globlist names_;
	globlist titles_;
	globlist descriptions_;

	globlist authors_;
	globlist ips_;
	globlist emails_;

	void parse_str_to_globlist(const std::string& str, globlist& glist);

	bool is_in_globlist(const std::string& str, const globlist& glist) const;

	bool is_in_ip_masklist(const std::string& ip, const globlist& mlist) const;
	bool ip_matches(const std::string& ip, const glob& ip_mask) const;
};

}

#endif
