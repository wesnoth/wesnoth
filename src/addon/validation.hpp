/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2014 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ADDON_VALIDATION_HPP_INCLUDED
#define ADDON_VALIDATION_HPP_INCLUDED

#include <vector>
#include <string>

class config;

/**
 * Default port number for the addon server.
 *
 * @note This might not be the best place to declare the variable, but it's
 * one of the few files shared by the server and the game.
 */
extern const unsigned short default_campaignd_port;

/**
 * Values used for add-on classification; UI-only
 * at the moment, in the future it could be used for
 * directory allocation too, removing the need for
 * the ADDON_GROUP constants (TODO).
 *
 * @note If you change the order or content of these, you'll also need
 * to update the @a addon_type_strings table found in validation.cpp.
 */
enum ADDON_TYPE {
	ADDON_UNKNOWN,		/**< a.k.a. anything. */
	ADDON_CORE,			/**< Total Conversion Core. */
	ADDON_SP_CAMPAIGN,	/**< Single-player campaign. */
	ADDON_SP_SCENARIO,	/**< Single-player scenario. */
	ADDON_SP_MP_CAMPAIGN, /**< Hybrid campaign. */
	ADDON_MP_CAMPAIGN,	/**< Multiplayer campaign. */
	ADDON_MP_SCENARIO,	/**< Multiplayer scenario. */
	ADDON_MP_MAPS,		/**< Multiplayer plain (no WML) map pack. */
	ADDON_MP_ERA,		/**< Multiplayer era. */
	ADDON_MP_FACTION,	/**< Multiplayer faction. */
	// NOTE: following two still require proper engine support
	ADDON_MP_MOD,		/**< Modification of the game for MP. */
	//ADDON_GUI,			// GUI add-ons/themes.
	ADDON_MEDIA,		/**< Miscellaneous content/media (unit packs, terrain packs, music packs, etc.). */
	ADDON_OTHER,		/**< an add-on that fits in no other category */
	ADDON_TYPES_COUNT
};

ADDON_TYPE get_addon_type(const std::string& str);
std::string get_addon_type_string(ADDON_TYPE type);

/** Checks whether an add-on id/name is legal or not. */
bool addon_name_legal(const std::string& name);
/** Checks whether an add-on file name is legal or not. */
bool addon_filename_legal(const std::string& name);
/** Probes an add-on archive for illegal names. */
bool check_names_legal(const config& dir);

std::string encode_binary(const std::string& str);
std::string unencode_binary(const std::string& str);
bool needs_escaping(char c);

#endif /* !ADDON_CHECKS_HPP_INCLUDED */
