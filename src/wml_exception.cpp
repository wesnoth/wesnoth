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
 *  @file
 *  Implementation for wml_exception.hpp.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"
#include "wml_exception.hpp"

#include "display.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "formula_string_utils.hpp"


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

void twml_exception::show(display &disp)
{
	std::stringstream sstr;

	// The extra spaces between the \n are needed, otherwise the dialog doesn't show
	// an empty line.
	sstr << _("An error due to possibly invalid WML occurred\nThe error message is :")
		<< "\n" << user_message << "\n \n"
		<< _("When reporting the bug please include the following error message :")
		<< "\n" << dev_message;

	gui2::show_error_message(disp.video(), sstr.str());
}

t_string missing_mandatory_wml_key(const std::string& section, const std::string& key,
		const std::string& primary_key, const std::string& primary_value)
{
	utils::string_map symbols;
	symbols["section"] = section;
	symbols["key"] = key;
	if(!primary_key.empty()) {
		assert(!primary_value.empty());

		symbols["primary_key"] = primary_key;
		symbols["primary_value"] = primary_value;

		return t_string(vgettext("In section '[$section|]' where '$primary_key| = "
			"$primary_value' the mandatory key '$key|' isn't set.", symbols));
	} else {
		return t_string(vgettext("In section '[$section|]' the "
			"mandatory key '$key|' isn't set.", symbols));
	}
}
