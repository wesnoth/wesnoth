/* $Id$ */
/*
   Copyright (C) 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/tips.hpp"

#include "config.hpp"
#include "foreach.hpp"

namespace gui2 {

ttip::ttip(const t_string& text, const t_string& source)
	: text_(text)
	, source_(source)
{
}

namespace tips {


/** @todo Implement the filtering of the tips. */
std::vector<ttip> load(const config& cfg)
{
	std::vector<ttip> result;

	foreach(const config &tip, cfg.child_range("tip")) {
		result.push_back(ttip(tip["text"], tip["source"]));
	}

	return result;
}

/** @todo Implement the filtering of the tips. */
std::vector<ttip> shuffle(const std::vector<ttip>& tips)
{
	std::vector<ttip> result = tips;
	std::random_shuffle(result.begin(), result.end());
	return result;
}

} // namespace tips

} // namespace gui2
