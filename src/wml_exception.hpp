/*
   Copyright (C) 2007 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Add a special kind of assert to validate whether the input from WML
 * doesn't contain any problems that might crash the game.
 */

#pragma once

#include "config.hpp"
#include "lua_jailbreak_exception.hpp"

#include <string>

/**
 * The macro to use for the validation of WML
 *
 *  @param cond         The condition to test, if false and exception is generated.
 *  @param message      The translatable message to show at the user.
 */
#ifndef __func__
 #ifdef __FUNCTION__
  #define __func__ __FUNCTION__
 #endif
#endif

#define VALIDATE(cond, message)                                           \
	do {                                                                  \
		if(!(cond)) {                                                     \
			throw_wml_exception(#cond, __FILE__, __LINE__, __func__, message);  \
		}                                                                 \
	} while(0)

#define VALIDATE_WITH_DEV_MESSAGE(cond, message, dev_message)             \
	do {                                                                  \
		if(!(cond)) {                                                     \
			throw_wml_exception(#cond                                           \
					, __FILE__                                            \
					, __LINE__                                            \
					, __func__                                            \
					, message                                             \
					, dev_message);                                       \
		}                                                                 \
	} while(0)

#define FAIL(message)                                                     \
	do {                                                                  \
		throw_wml_exception(nullptr, __FILE__, __LINE__, __func__, message);       \
	} while(0)

#define FAIL_WITH_DEV_MESSAGE(message, dev_message)                       \
	do {                                                                  \
		throw_wml_exception(nullptr                                                \
				, __FILE__                                                \
				, __LINE__                                                \
				, __func__                                                \
				, message                                                 \
				, dev_message);                                           \
	} while(0)

/**
 *  Helper function, don't call this directly.
 *
 *  @param cond         The textual presentation of the test that failed.
 *  @param file         The file in which the test failed.
 *  @param line         The line at which the test failed.
 *  @param function     The function in which the test failed.
 *  @param message      The translated message to show the user.
 */
NORETURN void throw_wml_exception(
		  const char* cond
		, const char* file
		, int line
		, const char *function
		, const std::string& message
		, const std::string& dev_message = "");

/** Helper class, don't construct this directly. */
struct wml_exception
	: public lua_jailbreak_exception
{
	wml_exception(const std::string& user_msg, const std::string& dev_msg)
		: user_message(user_msg)
		, dev_message(dev_msg)
	{
	}

	~wml_exception() NOEXCEPT {}

	/**
	 *  The message for the user explaining what went wrong. This message can
	 *  be translated so the user gets a explanation in his/her native tongue.
	 */
	std::string user_message;

	/**
	 *  The message for developers telling which problem was triggered, this
	 *  shouldn't be translated. It's hard for a dev to parse errors in
	 *  foreign tongues.
	 */
	std::string dev_message;

	/**
	 * Shows the error in a dialog.
	 */
	void show();
private:
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(wml_exception)
};

/**
 * Returns a standard message for a missing wml key.
 *
 * @param section                 The section is which the key should appear
 *                                (this should include the section brackets).
 *                                It may contain parent sections to make it
 *                                easier to find the wanted sections. They are
 *                                listed like [parent][child][section].
 * @param key                     The omitted key.
 * @param primary_key             The primary key of the section.
 * @param primary_value           The value of the primary key (mandatory if
 *                                primary key isn't empty).
 *
 * @returns                       The error message.
 */
std::string missing_mandatory_wml_key(
		  const std::string& section
		, const std::string& key
		, const std::string& primary_key = ""
		, const std::string& primary_value = "");
/**
 * Returns a standard warning message for using a deprecated wml key.
 *
 * @param key                     The deprecated key.
 * @param removal_version         The version in which the key will be
 *                                removed key.
 *
 * @returns                       The warning message.
 */
std::string deprecate_wml_key_warning(
		  const std::string& key
		, const std::string& removal_version);

/**
 * Returns a standard warning message for using a deprecated renamed wml key.
 *
 * @param deprecated_key          The deprecated key.
 * @param key                     The new key to be used.
 * @param removal_version         The version in which the key will be
 *                                removed key.
 *
 * @returns                       The warning message.
 */
std::string deprecated_renamed_wml_key_warning(
		  const std::string& deprecated_key
		, const std::string& key
		, const std::string& removal_version);

/**
 * Returns a config attribute, using either the old name or the new one.
 *
 * The function first tries the find the attribute using @p key and if that
 * doesn't find the attribute it tries @p deprecated_key. If that test finds
 * an attribute it will issue a warning and return the result. Else returns
 * an empty attribute.
 *
 * @note This function is not a member of @ref config, since that would add
 * additional dependencies to the core library.
 *
 * @param cfg                     The config to get the attribute from.
 * @param deprecated_key          The deprecated key.
 * @param key                     The new key to be used.
 * @param removal_version         The version in which the key will be
 *                                removed key.
 *
 * @returns                       The attribute found as described above.
 */
const config::attribute_value& get_renamed_config_attribute(
		  const config& cfg
		, const std::string& deprecated_key
		, const std::string& key
		, const std::string& removal_version);
