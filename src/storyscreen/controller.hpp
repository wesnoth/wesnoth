/*
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * @file
 * Storyscreen controller (interface).
 */

#pragma once

#include "events.hpp"
#include "storyscreen/parser.hpp"

#include <memory>
#include <string>

namespace storyscreen
{
class part;
class floating_image;

class controller : private story_parser
{
public:
	typedef std::shared_ptr<part> part_pointer_type;

	controller(const vconfig& data, const std::string& scenario_name);

	part_pointer_type get_part(int index) const
	{
		return parts_.at(index);
	}

	int max_parts() const
	{
		return parts_.size();
	}

private:
	/** Inherited from story_parser. */
	virtual bool resolve_wml_helper(const std::string& key, const vconfig& node) override;

	std::string scenario_name_;

	// The part cache.
	std::vector<part_pointer_type> parts_;
};

} // end namespace storyscreen
