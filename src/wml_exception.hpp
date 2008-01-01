/* $Id$ */
/*
   Copyright (C) 2007 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file wml_exeption.hpp
//! Add a special kind of assert to validate whether the input from WML
//! doesn't contain any problems that might crash the game.

#ifndef WML_EXCEPTION_HPP_INCLUDED
#define WML_EXCEPTION_HPP_INCLUDED

#include "tstring.hpp"

#include <string>

class display;

//! The macro to use for the validation of WML
//!
//! @param cond         The condition to test, if false and exception is generated.
//! @param message      The translatable message to show at the user.
#ifdef _MSC_VER
 #if _MSC_VER < 1300
  #define __FUNCTION__ "(Unspecified)"
 #endif
#endif

// Sun Studio compilers call __func__ not __FUNCTION__
#ifdef __SUNPRO_CC
#define VALIDATE(cond, message) if(!(cond)) wml_exception(#cond, __FILE__, __LINE__, __func__, message)
#else
#define VALIDATE(cond, message) if(!(cond)) wml_exception(#cond, __FILE__, __LINE__, __FUNCTION__, message)
#endif

//! Helper function, don't call this directly.
void wml_exception(const char* cond, const char* file, 
	const int line, const char* function, const t_string& message);

//! Helper class, don't construct this directly.
struct twml_exception 
{
	twml_exception(const t_string& user_msg, const std::string& dev_msg);
		
	//! The message for the user explaining what went wrong. This message can
	//! be translated so the user gets a explanation in his/her native tongue.
	t_string user_message;

	//! The message for developers telling which problem was triggered, this
	//! shouldn't be translated. It's hard for a dev to parse errors in
	//! foreign tongues.
	std::string dev_message;

	//! Shows the error in a dialog.
	void show(display &disp);
};

//! Returns a standard message for a missing wml key.
t_string missing_mandatory_wml_key(const std::string& section, const std::string& key,
		const std::string& primary_key = "", const std::string& primary_value = "");

#endif

