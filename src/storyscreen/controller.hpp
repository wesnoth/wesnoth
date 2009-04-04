/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Copyright (C) 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/controller.hpp
 * This code is work in progress, and the interfaces may change.
 * It is supposed to completely replace the old story screens code
 * at intro.cpp, introducing new WML conventions while at it.
 */

#ifndef STORYSCREEN_CONTROLLER_HPP_INCLUDED
#define STORYSCREEN_CONTROLLER_HPP_INCLUDED

#include "events.hpp"
#include "variable.hpp"
#include "video.hpp"

class display;
class game_state;
// class vconfig;

namespace storyscreen {

class page;
class floating_image;

class controller
{
public:
	controller(display& disp, const vconfig& data, const std::string& scenario_name);
	~controller();

	/**
	* Display story screen pages.
	*/
	void show_all_pages() const;


private:
	// Executes WML flow instructions and inserts pages.
	void resolve_wml(const vconfig& cfg);
	// Used by ctor.
	void build_pages() {
		resolve_wml(data_);
	}
	// Used by dtor.
	void clear_pages();

	display& disp_;
	const resize_lock disp_resize_lock_;
	const events::event_context evt_context_;

	vconfig data_;
	std::string scenario_name_;

	// The page cache.
	std::vector<page*> pages_;

	// The state of the world.
	game_state* gamestate_;

public:
	struct no_pages {};

};

} // end namespace storyscreen

#endif /* ! STORYSCREEN_CONTROLLER_HPP_INCLUDED */
