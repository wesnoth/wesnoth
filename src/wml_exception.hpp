/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Add a special kind of assert to validate whether the input from WML
 * doesn't contain any problems that might crash the game.
 */

#ifndef WML_EXCEPTION_HPP_INCLUDED
#define WML_EXCEPTION_HPP_INCLUDED

#include "exceptions.hpp"

class display;

/**
 * The macro to use for the validation of WML
 *
 *  @param cond         The condition to test, if false and exception is generated.
 *  @param message      The translatable message to show at the user.
 */
#ifdef _MSC_VER
 #if _MSC_VER < 1300
  #define __FUNCTION__ "(Unspecified)"
 #endif
#endif

#ifndef __func__
 #ifdef __FUNCTION__
  #define __func__ __FUNCTION__
 #endif
#endif

#define VALIDATE(cond, message) if(!(cond)) wml_exception(#cond, __FILE__, __LINE__, __func__, message)

/**
 *  Helper function, don't call this directly.
 *
 *  @param cond         The textual presentation of the test that failed.
 *  @param file         The file in which the test failed.
 *  @param line         The line at which the test failed.
 *  @param function     The funtion in which the test failed.
 *  @param message      The translated message to show the user.
 */
void wml_exception(const char* cond, const char* file,
	int line, const char *function, const std::string &message);

/** Helper class, don't construct this directly. */
struct twml_exception: game::error
{
	twml_exception(const std::string &user_msg, const std::string &dev_msg)
		: game::error(user_msg), dev_message(dev_msg) {}

	~twml_exception() throw() {}

	/**
	 *  The message for developers telling which problem was triggered, this
	 *  shouldn't be translated. It's hard for a dev to parse errors in
	 *  foreign tongues.
	 */
	std::string dev_message;

	/**
	 * Shows the error in a dialog.
	 *  @param disp         The display object to show the message on.
	 */
	void show(display &disp);
};

/**
 *  Returns a standard message for a missing wml key.
 *
 *  @param section      The section is which the key should appear.
 *  @param key          The ommitted key.
 *  @param primary_key  The primary key of the section.
 *  @param primary_value
 *                      The value of the primary key (mandatory if primary key
 *                      isn't empty).
 *
 *  @return             The error message.
 */
std::string missing_mandatory_wml_key(const std::string& section, const std::string& key,
		const std::string& primary_key = "", const std::string& primary_value = "");

#endif

