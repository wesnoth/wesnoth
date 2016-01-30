/*
   Copyright (C) 2009 - 2016 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
#include "video.hpp" //for resize_lock

#include <boost/shared_ptr.hpp>

class display;
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
	controller(display& disp, const vconfig& data, const std::string& scenario_name,
		   int segment_index);

	/**
	 * Display all story screen parts in a first..last sequence.
	 */
	STORY_RESULT show(START_POSITION startpos=START_BEGINNING);

private:
	typedef boost::shared_ptr< part    > part_pointer_type;
	typedef boost::shared_ptr< part_ui > render_pointer_type;

	// Executes WML flow instructions and inserts parts.
	void resolve_wml(const vconfig& cfg);

	display& disp_;
	const resize_lock disp_resize_lock_;
	const events::event_context evt_context_;

	std::string scenario_name_;
	int segment_index_;

	// The part cache.
	std::vector< part_pointer_type > parts_;
};

} // end namespace storyscreen

#endif /* ! STORYSCREEN_CONTROLLER_HPP_INCLUDED */
