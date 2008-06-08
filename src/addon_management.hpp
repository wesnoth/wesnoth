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

typedef std::pair< std::string, ADDON_GROUP > addon_list_item;

void remove_local_addon(const std::string& addon, ADDON_GROUP addon_type);

//! Gets the publish information for an add-on
//! @param addon_name The add-on's main directory/file name.
//! @param addon_type The type of add-on for locating it in the directory tree.
//! @param cfg A config object to store the add-on's properties.
void get_addon_info(const std::string& addon_name, ADDON_GROUP addon_type, class config& cfg);

//! Sets the publish information for an add-on
//! @param addon_name The add-on's main directory/file name.
//! @param addon_type The type of add-on for locating it in the directory tree.
//! @param cfg A config object from which the add-on's properties are copied.
void set_addon_info(const std::string& addon_name, ADDON_GROUP addon_type, const class config& cfg);

//! Returns a list of local add-ons that can be published.
//! @param addon_type The type of add-on for locating it in the directory tree.
std::vector<std::string> available_addons(ADDON_GROUP addons_type);

//! Returns a list of all kinds of local add-ons that can be published.
std::vector< addon_list_item > enumerate_all_available_addons();

//! Retrieves the names of all installed add-ons of a kind.
//! @param addon_type The type of add-on for locating it in the directory tree.
std::vector<std::string> installed_addons(ADDON_GROUP addons_type);

//! Archives an add-on into a config object for campaignd transactions.
void archive_addon(const std::string& addon_name, ADDON_GROUP addon_type, class config& cfg);
//! Unarchives an add-on from campaignd's retrieved config object.
void unarchive_addon(const class config& cfg);

#endif /* !ADDON_MANAGEMENT_HPP_INCLUDED */
