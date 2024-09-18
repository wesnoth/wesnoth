/*
	Copyright (C) 2007 - 2024
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
 *  @file
 *  Implementation for wml_exception.hpp.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "wml_exception.hpp"

#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "formula/string_utils.hpp"
#include "log.hpp"

static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

void throw_wml_exception(
		  const char* cond
		, const char* file
		, const int line
		, const char *function
		, const std::string& message
		, const std::string& dev_message)
{
	std::ostringstream sstr;
	if(cond) {
		sstr << "Condition '" << cond << "' failed at ";
	} else {
		sstr << "Unconditional failure at ";
	}

	sstr << file << ":" << line << " in function '" << function << "'.";

	if(!dev_message.empty()) {
		sstr << " Extra development information: " << dev_message;
	}

	throw wml_exception(message, sstr.str());
}

void wml_exception::show() const
{
	std::ostringstream sstr;

	// The extra spaces between the \n are needed, otherwise the dialog doesn't show
	// an empty line.
	sstr << _("An error due to possibly invalid WML occurred\nThe error message is :")
		<< "\n" << user_message << "\n \n"
		<< _("When reporting the bug please include the following error message :")
		<< "\n" << dev_message;

	gui2::show_error_message(sstr.str());
}

std::string missing_mandatory_wml_key(
		  const std::string &section
		, const std::string &key
		, const std::string& primary_key
		, const std::string& primary_value)
{
	utils::string_map symbols;
	symbols["section"] = section;
	symbols["key"] = key;
	if(!primary_key.empty()) {
		assert(!primary_value.empty());

		symbols["primary_key"] = primary_key;
		symbols["primary_value"] = primary_value;

		return VGETTEXT("In section ‘[$section|]’ where ‘$primary_key|’ = "
			"‘$primary_value’ the mandatory key ‘$key|’ isn’t set.", symbols);
	} else {
		return VGETTEXT("In section ‘[$section|]’ the "
			"mandatory key ‘$key|’ isn’t set.", symbols);
	}
}

std::string missing_mandatory_wml_tag(
		  const std::string &section
		, const std::string &tag)
{
	utils::string_map symbols;
	symbols["section"] = section;
	symbols["tag"] = tag;
	return VGETTEXT("In section ‘[$section|]’ the "
				"mandatory subtag ‘[$tag|]’ is missing.", symbols);
}
