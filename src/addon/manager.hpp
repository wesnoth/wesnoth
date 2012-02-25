/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2012 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ADDON_MANAGER_HPP_INCLUDED
#define ADDON_MANAGER_HPP_INCLUDED

class config;

#include "addon/validation.hpp"

#include <string>
#include <vector>
#include <utility>

typedef std::pair< std::string, ADDON_TYPE > addon_list_item;
class config_changed_exception {};

bool remove_local_addon(const std::string& addon);

/**
 * Returns true if there's a local .pbl file stored for the specified add-on.
 */
bool have_addon_pbl_info(const std::string& addon_name);

/**
 * Returns true if the specified add-ons appear to be managed by a 'supported' VCS.
 *
 * Currently supported VCSes are: Subversion, Git, Mercurial.
 */
bool have_addon_in_vcs_tree(const std::string& addon_name);

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

/**
 * Replaces underscores to dress up file or dirnames as add-on titles.
 *
 * @todo In the future we should store more local information about add-ons and use
 *       this only as a fallback; it could be desirable to fetch translated names as well
 *       somehow.
 */
std::string make_addon_title(const std::string& id);

/** Returns a list of local add-ons that can be published. */
std::vector<std::string> available_addons();

/** Retrieves the names of all installed add-ons. */
std::vector<std::string> installed_addons();

/** Chekc whether the specified add-on is currently installed. */
bool is_addon_installed(const std::string& addon_name);

/** Archives an add-on into a config object for campaignd transactions. */
void archive_addon(const std::string& addon_name, class config& cfg);

/** Unarchives an add-on from campaignd's retrieved config object. */
void unarchive_addon(const class config& cfg);

/** Refreshes the per-session cache of add-on's version information structs. */
void refresh_addon_version_info_cache();

/** Returns a particular installed add-on's version information. */
const class version_info& get_addon_version_info(const std::string& addon);

#endif /* !ADDON_MANAGEMENT_HPP_INCLUDED */
