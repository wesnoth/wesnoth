/*
	Copyright (C) 2007 - 2025
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
	} while(false)

#define VALIDATE_WML_CHILD(cfg, key, message)                                             \
    ([](auto c, auto k) {                                                             \
        if(auto child = c.optional_child(k)) { return *child; }                       \
        throw_wml_exception( "Missing [" key "]", __FILE__, __LINE__, __func__, message); \
    })(cfg, key)                                                                          \

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
	} while(false)

#define FAIL(message)                                                     \
	do {                                                                  \
		throw_wml_exception(nullptr, __FILE__, __LINE__, __func__, message);       \
	} while(false)

#define FAIL_WITH_DEV_MESSAGE(message, dev_message)                       \
	do {                                                                  \
		throw_wml_exception(nullptr                                                \
				, __FILE__                                                \
				, __LINE__                                                \
				, __func__                                                \
				, message                                                 \
				, dev_message);                                           \
	} while(false)

/**
 *  Helper function, don't call this directly.
 *
 *  @param cond         The textual presentation of the test that failed.
 *  @param file         The file in which the test failed.
 *  @param line         The line at which the test failed.
 *  @param function     The function in which the test failed.
 *  @param message      The translated message to show the user.
 *  @param dev_message  Any additional information that might be useful to a developer.
 */
[[noreturn]] void throw_wml_exception(
		  const char* cond
		, const char* file
		, int line
		, const char *function
		, const std::string& message
		, const std::string& dev_message = "");

/** Helper class, don't construct this directly. */
struct wml_exception final
	: public lua_jailbreak_exception
{
	wml_exception(const std::string& user_msg, const std::string& dev_msg)
		: user_message(user_msg)
		, dev_message(dev_msg)
	{
		this->store();
	}

	~wml_exception() noexcept {}

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
	void show() const;
private:
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(wml_exception)
};

/**
 * Returns a standard message for a missing wml key (attribute).
 *
 * @param section                 The section in which the key should appear.
 *                                Shouldn't include leading or trailing brackets,
 *                                as they're already in the translatable string;
 *                                but if it has to include brackets in the middle,
 *                                for example "parent][child][section", then it
 *                                seems reasonable include the outer ones too.
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
 * Returns a standard message for a missing wml child (tag).
 *
 * @param section                 The section in which the child should appear.
 *                                Same meaning as for missing_mandatory_wml_key().
 * @param tag                     The omitted tag.
 *
 * @returns                       The error message.
 */
std::string missing_mandatory_wml_tag(
		  const std::string& section
		, const std::string& tag);
