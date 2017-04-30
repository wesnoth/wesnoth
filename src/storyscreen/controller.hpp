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

#ifndef STORYSCREEN_CONTROLLER_HPP_INCLUDED
#define STORYSCREEN_CONTROLLER_HPP_INCLUDED

#include "events.hpp"

#include <memory>
#include <string>

class CVideo;
class vconfig;

namespace storyscreen {

class part;
class floating_image;

class controller
{
public:
	typedef std::shared_ptr< part    > part_pointer_type;

	controller(const vconfig& data, const std::string& scenario_name);

	part_pointer_type get_part(int index) const
	{
		return parts_[index];
	}

	int max_parts() const
	{
		return parts_.size();
	}

private:
	// Executes WML flow instructions and inserts parts.
	void resolve_wml(const vconfig& cfg);

	std::string scenario_name_;

	// The part cache.
	std::vector< part_pointer_type > parts_;
};

} // end namespace storyscreen

#endif /* ! STORYSCREEN_CONTROLLER_HPP_INCLUDED */
