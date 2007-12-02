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

//! @file wml_exeption.cpp
//! Implementation for wml_exeption.hpp.

#include "wml_exception.hpp"

#include "display.hpp"
#include "gettext.hpp"
#include "show_dialog.hpp"

#include <sstream>

//! Helper function, don't call this directly.
//!
//! @param cond         The textual presentation if the test that failed.
//! @param file         The file in which the test failed.
//! @param line         The line at which the test failed.
//! @param function     The funtion in which the test failed.
//! @param message      The translatable message to show the user.
void wml_exception(const char* cond, const char* file, 
	const int line, const char* function, const t_string& message)
{
	std::stringstream sstr;
	sstr << "Condition '" << cond << "' failed at " 
		<< file << ":" << line << " in function '" << function << "'.";
	
	throw twml_exception(message, sstr.str());
}

twml_exception::twml_exception(const t_string& user_msg, const std::string& dev_msg) :
	user_message(user_msg),
	dev_message(dev_msg)
{
}

//! Shows the error in a dialog.
//!
//! @param disp         The display object to show the message on.
void twml_exception::show(display &disp)
{
	std::stringstream sstr;

	// The extra spaces between the \n are needed, otherwise the dialog doesn't show
	// an empty line.
	sstr << _("An error due to possible invalid WML occured\nThe error message is :")
		<< "\n" << user_message << "\n \n" 
		<< _("When reporting the bug please include the following error message :")
		<< "\n" << dev_message;

	gui::show_error_message(disp, sstr.str());
}
