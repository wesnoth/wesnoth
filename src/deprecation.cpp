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
#include "game_config.hpp"
#include "version.hpp"
#include "preferences/general.hpp"

// Set the default severity with the second parameter.
// -1 means the default is to never log on this domain.
// 0 would mean log errors only.
// 1 would mean log errors and warnings.
// and so on and so on.
static lg::log_domain log_deprecate("deprecation", -1);

std::string deprecated_message(const std::string& elem_name, DEP_LEVEL level, const version_info& version, const std::string& detail) {
	utils::string_map msg_params = {{"elem", elem_name}};
	lg::logger* log_ptr = nullptr;
	std::string message;
	switch(level) {
	case DEP_LEVEL::INDEFINITE:
		log_ptr = &lg::info();
		message = VGETTEXT("$elem has been deprecated indefinitely.", msg_params);
		break;
	case DEP_LEVEL::PREEMPTIVE:
		log_ptr = &lg::warn();
		if(game_config::wesnoth_version < version) {
			msg_params["version"] = version.str();
			message = VGETTEXT("$elem has been deprecated and may be removed in version $version.", msg_params);
		} else {
			message = VGETTEXT("$elem has been deprecated and may be removed at any time.", msg_params);
		}
		break;
	case DEP_LEVEL::FOR_REMOVAL:
		log_ptr = &lg::err();
		msg_params["version"] = version.str();
		message = VGETTEXT("$elem has been deprecated and will be removed in the next version ($version).", msg_params);
		break;
	case DEP_LEVEL::REMOVED:
		log_ptr = &lg::err();
		message = VGETTEXT("$elem has been deprecated and removed.", msg_params);
		break;
	default: // Not impossible, in case level was given an invalid value from a cast.
		utils::string_map err_params = {{"level", std::to_string(int(level))}};
		// Note: This message is duplicated in data/lua/core.lua
		// Any changes should be mirrorred there.
		std::string msg = VGETTEXT("Invalid deprecation level $level (should be 1-4)", err_params);
		LOG_STREAM(err, "general") << msg;
		return msg;
	}
	if(!detail.empty()) {
		message += "\n  ";
		message += detail;
	}
	if(log_ptr && !log_ptr->dont_log(log_deprecate)) {
		const lg::logger& out_log = *log_ptr;
		out_log(log_deprecate) << message << '\n';
		if(preferences::get("show_deprecation", false)) {
			lg::wml_error() << message << '\n';
		}
	}
	return message;
}