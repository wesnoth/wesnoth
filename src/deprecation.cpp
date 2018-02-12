/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "deprecation.hpp"

#include "log.hpp"
#include "gettext.hpp"
#include "formula/string_utils.hpp"
#include "tstring.hpp"
#include "game_config.hpp"
#include "version.hpp"

static lg::log_domain log_wml("wml");

// Note that these strings are all duplicated in data/lua/core.lua
// Any changes here should also be reflected there.
void deprecated_message(const std::string& elem_name, int level, const version_info& version, const std::string& detail) {
	utils::string_map msg_params = {{"elem", elem_name}};
	lg::logger* log_ptr = nullptr;
	std::string message;
	if(level == 1) {
		log_ptr = &lg::info();
		message = VGETTEXT("$elem has been deprecated indefinitely.", msg_params);
	} else if(level == 2) {
		log_ptr = &lg::warn();
		if(game_config::wesnoth_version < version) {
			msg_params["version"] = version.str();
			message = VGETTEXT("$elem has been deprecated and may be removed in version $version.", msg_params);
		} else {
			message = VGETTEXT("$elem has been deprecated and may be removed at any time.", msg_params);
		}
	} else if(level == 3) {
		log_ptr = &lg::err();
		msg_params["version"] = version.str();
		message = VGETTEXT("$elem has been deprecated and will be removed in the next version ($version).", msg_params);
	} else if(level == 4) {
		log_ptr = &lg::err();
		message = VGETTEXT("$elem has been deprecated and removed.", msg_params);
	} else {
		utils::string_map err_params = {{"level", std::to_string(level)}};
		LOG_STREAM(err, "general") << VGETTEXT("Invalid deprecation level $level (should be 1-4)", err_params);
	}
	// TODO: Decide when to dump to wml_error()
	if(log_ptr && !log_ptr->dont_log(log_wml)) {
		const lg::logger& out_log = *log_ptr;
		out_log(log_wml) << message << '\n';
		//lg::wml_error() << message << '\n';
		if(!detail.empty()) {
			out_log(log_wml) << detail << '\n';
			//lg::wml_error() << detail << '\n';
		}
	}
}