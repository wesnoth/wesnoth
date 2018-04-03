/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "exceptions.hpp"
#include "formatter.hpp"

#include <memory>
#include <string>
#include <vector>

class terrain_type_data;
class unit_type;

namespace help
{
/** Thrown when the help manager fails to parse something. */
struct parse_error : public game::error
{
	parse_error(const std::string& msg) : game::error(msg) {}
};

/** Thrown when trying to create a sub-section deeper than the max allowed depth. */
struct max_recursion_reached : public game::error
{
	max_recursion_reached(const std::string& msg) : game::error(msg) {}
};

struct string_less
{
	bool operator()(const std::string& s1, const std::string& s2) const;
};

enum UNIT_DESCRIPTION_TYPE {FULL_DESCRIPTION, NO_DESCRIPTION, NON_REVEALING_DESCRIPTION};

UNIT_DESCRIPTION_TYPE description_type(const unit_type& type);

std::string hidden_symbol(bool hidden = true);

/** An ID beginning with '.' denotes a hidden section or topic. */
bool is_visible_id(const std::string& id);

/// Return true if the id is valid for user defined topics and
/// sections. Some IDs are special, such as toplevel and may not be
/// be defined in the config.
bool is_valid_id(const std::string& id);

/// Prepend all chars with meaning inside attributes with a backslash.
std::string escape(const std::string &s);

inline std::string make_link(const std::string& text, const std::string& dst)
{
    // some sorting done on list of links may rely on the fact that text is first
    return formatter() << "<ref>text='" << help::escape(text) << "' dst='" << help::escape(dst) << "'</ref>";
}

inline std::string jump_to(const unsigned pos)
{
    return formatter() << "<jump>to=" << pos << "</jump>";
}

inline std::string jump(const unsigned amount)
{
    return formatter() << "<jump>amount=" << amount << "</jump>";
}

using ter_data_cache = std::shared_ptr<terrain_type_data>;

/// Load the appropriate terrain types data to use
ter_data_cache load_terrain_types_data();

} // namespace help
