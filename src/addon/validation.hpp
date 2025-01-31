/*
	Copyright (C) 2008 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

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

enum class ADDON_CHECK_STATUS : unsigned int
{
	//
	// General errors
	//
	SUCCESS						= 0x0,			/**< No error */
	UNAUTHORIZED				= 0x1,			/**< Authentication failed */
	DENIED						= 0x2,			/**< Upload denied */
	USER_DOES_NOT_EXIST 		= 0x3, 			/**< Requested forum authentication for a user that doesn't exist on the forums */
	UNEXPECTED_DELTA			= 0xD,			/**< Delta for a non-existent add-on */
	//
	// Structure errors
	//
	EMPTY_PACK					= 0x100,		/**< Empty pack */
	BAD_DELTA					= 0x101,		/**< Bad delta pack */
	BAD_NAME					= 0x102,		/**< Bad add-on name */
	NAME_HAS_MARKUP				= 0x104,		/**< Markup in add-on name */
	ILLEGAL_FILENAME			= 0x10A,		/**< Bad filename */
	FILENAME_CASE_CONFLICT		= 0x10B,		/**< Filename case conflict */
	INVALID_UTF8_NAME			= 0x1FF,		/**< Invalid UTF-8 sequence in add-on name */
	//
	// .pbl errors
	//
	NO_TITLE					= 0x200,		/**< No title specified */
	NO_AUTHOR					= 0x201,		/**< No author specified */
	NO_VERSION					= 0x202,		/**< No version specified */
	NO_DESCRIPTION				= 0x203,		/**< No description specified */
	NO_EMAIL					= 0x204,		/**< No email specified */
	NO_PASSPHRASE				= 0x205,		/**< No passphrase specified */
	TITLE_HAS_MARKUP			= 0x206,		/**< Markup in add-on title */
	BAD_TYPE					= 0x207,		/**< Bad add-on type */
	VERSION_NOT_INCREMENTED		= 0x208,		/**< Version number is not an increment */
	INVALID_UTF8_ATTRIBUTE		= 0x2FF,		/**< Invalid UTF-8 sequence in add-on metadata */
	BAD_FEEDBACK_TOPIC_ID       = 0x209,        /**< The provided topic ID for the addon's feedback forum thread is invalid */
	FEEDBACK_TOPIC_ID_NOT_FOUND = 0x2A0,        /**< The provided topic ID for the addon's feedback forum thread wasn't found in the forum database */
	AUTH_TYPE_MISMATCH 			= 0x2B0, 		/**< The addon's forum_auth value does not match its previously set value */
	ICON_TOO_LARGE	 			= 0x2C0, 		/**< The add-on's icon is too large (presumably a DataURI) */
	//
	// Server errors
	//
	SERVER_UNSPECIFIED			= 0xF000,		/**< Unspecified server error */
	SERVER_READ_ONLY			= 0xF001,		/**< Server read-only mode on */
	SERVER_ADDONS_LIST			= 0xF002,		/**< Corrupted server add-ons list */
	SERVER_DELTA_NO_VERSIONS	= 0xF003,		/**< No versions to deltify against */
	SERVER_FORUM_AUTH_DISABLED 	= 0xF004, 		/**< The remote add-ons server does not support forum authorization */
};

std::string addon_check_status_desc(unsigned int code);

inline std::string addon_check_status_desc(ADDON_CHECK_STATUS code)
{
	return addon_check_status_desc(static_cast<unsigned int>(code));
}

std::string translated_addon_check_status(unsigned int code);

inline std::string translated_addon_check_status(ADDON_CHECK_STATUS code)
{
	return translated_addon_check_status(static_cast<unsigned int>(code));
}

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
	ADDON_MOD,		/**< Modification of the game. */
	ADDON_MEDIA,		/**< Miscellaneous content/media (unit packs, terrain packs, music packs, etc.). */
	ADDON_THEME,		/**< GUI2 Themes */
	ADDON_OTHER,		/**< an add-on that fits in no other category */
	ADDON_TYPES_COUNT
};

ADDON_TYPE get_addon_type(const std::string& str);
std::string get_addon_type_string(ADDON_TYPE type);

/** Checks whether an add-on id/name is legal or not. */
bool addon_name_legal(const std::string& name);
/** Checks whether an add-on file name is legal or not. */
bool addon_filename_legal(const std::string& name);
/** Checks whether an add-on icon is too large. */
bool addon_icon_too_large(const std::string& icon);
constexpr std::size_t max_icon_size = 500'000;

/**
 * Scans an add-on archive for illegal names.
 *
 * @param dir     The WML container for the root [dir] node where the search
 *                should begin.
 * @param badlist If provided and not null, any illegal names encountered will
 *                be added to this list. This also makes the archive scan more
 *                thorough instead of returning on the first illegal name
 *                encountered.
 *
 * @returns True if no illegal names were found.
 */
bool check_names_legal(const config& dir, std::vector<std::string>* badlist = nullptr);
/**
 * Scans an add-on archive for case-conflicts.
 *
 * Case conflicts may cause trouble on case-insensitive filesystems.
 *
 * @param dir     The WML container for the root [dir] node where the search
 *                should begin.
 * @param badlist If provided and not null, any case conflicts encountered will
 *                be added to this list. This also makes the archive scan more
 *                thorough instead of returning on the first conflict
 *                encountered.
 *
 * @returns True if no conflicts were found.
 */
bool check_case_insensitive_duplicates(const config& dir, std::vector<std::string>* badlist = nullptr);

std::string encode_binary(const std::string& str);
std::string unencode_binary(const std::string& str);
bool needs_escaping(char c);

std::string file_hash(const config& file);
bool comp_file_hash(const config& file_a, const config& file_b);
void write_hashlist(config& hashlist, const config& data);
bool contains_hashlist(const config& from, const config& to);
void make_updatepack(config& pack, const config& from, const config& to);
