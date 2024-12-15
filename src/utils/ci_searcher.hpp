
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
/** Helper class to perform locale-aware case-insensitive search. */
class ci_searcher
{
public:
	ci_searcher(std::string_view filter_text)
		: words_(utils::split(filter_text, ' '))
	{
	}

	/** Returns true if any filter words match any of the provided strings. */
	template<typename... Criteria>
	bool operator()(Criteria&&... criteria) const
	{
		if(empty()) {
			return true;
		}

		for(const auto& word : words_) {
			if(!(translation::ci_search(criteria, word) || ...)) {
				return false;
			}
		}

		return true;
	}

	/** Returns true if there are no filter terms. */
	bool empty() const
	{
		return words_.empty();
	}

private:
	std::vector<std::string> words_;
};

} // namespace translation
