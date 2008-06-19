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

#endif /* !ADDON_MANAGEMENT_HPP_INCLUDED */
