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
#include "interface.hpp"

class CVideo;
class vconfig;

namespace storyscreen {

enum STORY_RESULT {
	NEXT,
	BACK,
	QUIT
};

class part;
class part_ui;
class floating_image;

class controller
{
public:
	typedef std::shared_ptr< part    > part_pointer_type;
	typedef std::shared_ptr< part_ui > render_pointer_type;

	controller(CVideo& video, const vconfig& data, const std::string& scenario_name,
		   int segment_index);

	/**
	 * Display all story screen parts in a first..last sequence.
	 */
	STORY_RESULT show(START_POSITION startpos=START_BEGINNING);

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

	CVideo& video_;
	const events::event_context evt_context_;

	std::string scenario_name_;
	int segment_index_;

	// The part cache.
	std::vector< part_pointer_type > parts_;
};

} // end namespace storyscreen

#endif /* ! STORYSCREEN_CONTROLLER_HPP_INCLUDED */
