/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_WIDGETS_MINIMAP_HPP_INCLUDED
#define GUI_WIDGETS_MINIMAP_HPP_INCLUDED

#include "gui/widgets/control.hpp"

class config;

namespace gui2 {

/**
 * The basic minimap class.
 *
 * This minimap can only show a minimap, but it can't be interacted with. For
 * that the tminimap_interactive class will be created.
 */
class tminimap : public tcontrol
{
public:
	tminimap() : 
		tcontrol(1),
		map_data_(),
		terrain_(NULL),
		left_border_(0),
		right_border_(0),
		top_border_(0),
		bottom_border_(0)
	{
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from tcontrol. */
	void set_active(const bool /*active*/) {}

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return 0; }

	/** Inherited from tcontrol. */
	bool does_block_easy_close() const { return false; }
#ifndef NEW_DRAW
	/** Inherited from tcontrol. */
	void draw(surface& surface, const bool force = false, 
		const bool invalidate_background = false);
#else
	/** Inherited from tcontrol. */
	void draw_background(surface& frame_buffer);
#endif
	/** 
	 * Inherited from tcontrol.
	 *
	 * Since the old minimap might be smaller we always need to do a full
	 * redraw. Also the new map might be no map in which case the old one would
	 * be shown.
	 */
	bool needs_full_redraw() const { return true; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_map_data(const std::string& map_data) 
		{ if(map_data != map_data_) { map_data_ = map_data; set_dirty(); } }

	std::string get_map_data() const { return map_data_; }

	const std::string& map_data() const { return map_data_; }

	void set_config(const ::config* terrain) { terrain_ = terrain; }

	/** Sets all border variables, no function to set one at the time. */
	void set_borders(const unsigned left, 
		const unsigned right, const unsigned top, const unsigned bottom);

private:

	/** The map data to be used to generate the map. */
	std::string map_data_;

	/** 
	 * The config object with the terrain data. 
	 *
	 * This config must be set before the object can be drawn.
	 */
	const ::config* terrain_;

	/** 
	 * The minimap widget might have some drawing on the borders, these
	 * variables hold the border size. The minimap itself is not drawn on that
	 * area.
	 */
	unsigned left_border_;
	unsigned right_border_;
	unsigned top_border_;
	unsigned bottom_border_;

	/** Draws the minimap itself. */
	virtual void draw_map(surface& surface);

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const 
		{ static const std::string type = "minimap"; return type; }
};

} // namespace gui2

#endif

