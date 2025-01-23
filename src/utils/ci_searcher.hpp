
/*
	Copyright (C) 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gettext.hpp"
#include "serialization/string_utils.hpp"

namespace translation
{
/** Returns a function which performs locale-aware case-insensitive search. */
inline auto make_ci_matcher(std::string_view filter_text)
{
	return [words = utils::split(filter_text, ' ')](auto&&... to_match) {
		for(const auto& word : words) {
			if(!(translation::ci_search(to_match, word) || ...)) {
				return false;
			}
		}

		return true;
	};
}

} // namespace translation
