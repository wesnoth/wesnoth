/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Base class for the AI and AI-ai_manager contract.
 * @file
 */

#include "interface.hpp"
#include "log.hpp"

namespace ai {

static lg::log_domain log_ai("ai/general");

// =======================================================================
//
// =======================================================================
std::string interface::describe_self() const
{
	return "? [ai]";
}

// This is defined in the source file so that it can easily access the logger
bool ai_factory::is_duplicate(const std::string& name)
{
	if (get_list().find(name) != get_list().end()) {
		LOG_STREAM(err, log_ai) << "Error: Attempt to double-register AI " << name << std::endl;
		return true;
	}
	return false;
}

} //end of namespace ai
