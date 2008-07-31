/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ADDON_MANAGEMENT_HPP_INCLUDED
#define ADDON_MANAGEMENT_HPP_INCLUDED

class config;

#include "addon_checks.hpp"

#include <string>
#include <vector>
#include <utility>

typedef std::pair< std::string, ADDON_TYPE > addon_list_item;
class config_changed_exception {};

void remove_local_addon(const std::string& addon);

/**
 * Gets the publish information for an add-on.
 *
 * @param addon_name              The add-on's main directory/file name.
 * @param cfg                     A config object to store the add-on's
 *                                properties.
 */
void get_addon_info(const std::string& addon_name, class config& cfg);

/**
 * Sets the publish information for an add-on
 *
 * @param addon_name              The add-on's main directory/file name.
 * @param cfg                     A config object from which the add-on's
 *                                properties are copied.
 */
void set_addon_info(const std::string& addon_name, const class config& cfg);

/** Returns a list of local add-ons that can be published. */
std::vector<std::string> available_addons();

/** Retrieves the names of all installed add-ons. */
std::vector<std::string> installed_addons();

/** Archives an add-on into a config object for campaignd transactions. */
void archive_addon(const std::string& addon_name, class config& cfg);

/** Unarchives an add-on from campaignd's retrieved config object. */
void unarchive_addon(const class config& cfg);

/** Shows the game add-ons manager dialog.
 *  @param disp game_display instance to draw on.
 */
void manage_addons(class game_display& disp);

class addon_version_info_not_sane_exception {};

//! Struct representing an add-on's version information.
struct addon_version_info
{
	addon_version_info(); //!< Default constructor.
	//! String conversion constructor.
	//! @param src_str A version string to initialize numbers from. It must
	//!                be in the format "X.Y.Z", where X is the major version
	//!                number, Y the minor and Z the revision level. If the
	//!                string does not match this format, the object's sanity
	//!                flag will be set to false.
	addon_version_info(const std::string& src_str);
	addon_version_info(const addon_version_info&); //!< Copy constructor.
	//! List constructor.
	addon_version_info(unsigned major, unsigned minor, unsigned rev, bool sane_flag);

	//! Assignment operator.
	addon_version_info& operator=(const addon_version_info&);
	
	//! Returns a string of the form "major.minor.revision".
	//! Throws addon_version_info_not_sane_exception if the information given
	//! when constructing the object was not correctly formatted.
	std::string str(void);
	//! Shortcut to str().
	operator std::string() { return this->str(); }
	//! Returns the sanity state of the information given when
	//! constructing the object.
	operator bool() { return this->sane; }
	
	unsigned vmajor;		//!< Major (leading) version number.
	unsigned vminor;		//!< Minor (middle) version number.
	unsigned revision;		//!< Revision (trailing) version number.
	bool sane;				//!< Sanity flag.
};

//! Equality operator for addon_version_info.
inline bool operator==(const addon_version_info& l, const addon_version_info& r) {
	return(l.sane && r.sane && l.vmajor == r.vmajor && l.vminor == r.vminor &&
	       l.revision == r.revision);
}
//! Inequality operator for addon_version_info.
inline bool operator!=(const addon_version_info& l, const addon_version_info& r) {
	return !operator==(l,r);
}
//! Greater-than operator for addon_version_info.
bool operator>(const addon_version_info&, const addon_version_info&);
//! Less-than operator for addon_version_info.
bool operator<(const addon_version_info&, const addon_version_info&);
//! Greater-than-or-equal operator for addon_version_info.
bool operator>=(const addon_version_info&, const addon_version_info&);
//! Less-than-or-equal operator for addon_version_info.
bool operator<=(const addon_version_info&, const addon_version_info&);

//! Refreshes the per-session cache of add-on's version
//! information structs.
void refresh_addon_version_info_cache();
//! Returns a particular installed add-on's version information.
const addon_version_info& get_addon_version_info(const std::string& addon);

#endif /* !ADDON_MANAGEMENT_HPP_INCLUDED */
