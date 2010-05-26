/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
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
		terrain_(NULL)
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
	bool disable_click_dismiss() const { return false; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_map_data(const std::string& map_data)
		{ if(map_data != map_data_) { map_data_ = map_data; set_dirty(); } }

	std::string get_map_data() const { return map_data_; }

	const std::string& map_data() const { return map_data_; }

	void set_config(const ::config* terrain) { terrain_ = terrain; }

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
	 * Gets the image for the minimap.
	 *
	 * @param w                   The wanted width of the image.
	 * @param h                   The wanted height of the image.
	 *
	 * @returns                   The image, NULL upon error.
	 */
	const surface get_image(const int w, const int h) const;

	/** Inherited from tcontrol. */
	void impl_draw_background(surface& frame_buffer);

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;
};

} // namespace gui2

#endif

