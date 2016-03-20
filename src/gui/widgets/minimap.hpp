/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_MINIMAP_HPP_INCLUDED
#define GUI_WIDGETS_MINIMAP_HPP_INCLUDED

#include "gui/widgets/control.hpp"

#include "gui/auxiliary/widget_definition.hpp"
#include "gui/auxiliary/window_builder.hpp"

class config;

namespace gui2
{

// ------------ WIDGET -----------{

/**
 * The basic minimap class.
 *
 * This minimap can only show a minimap, but it can't be interacted with. For
 * that the tminimap_interactive class will be created.
 */
class tminimap : public tcontrol
{
public:
	tminimap() : tcontrol(1), map_data_(), terrain_(NULL)
	{
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) OVERRIDE;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/** See @ref twidget::disable_click_dismiss. */
	bool disable_click_dismiss() const OVERRIDE;

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_map_data(const std::string& map_data)
	{
		if(map_data != map_data_) {
			map_data_ = map_data;
			set_is_dirty(true);
		}
	}

	std::string get_map_data() const
	{
		return map_data_;
	}

	const std::string& map_data() const
	{
		return map_data_;
	}

	void set_config(const ::config* terrain)
	{
		terrain_ = terrain;
	}

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

	/** See @ref twidget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer,
									  int x_offset,
									  int y_offset) OVERRIDE;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;
};

// }---------- DEFINITION ---------{

struct tminimap_definition : public tcontrol_definition
{
	explicit tminimap_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_minimap : public tbuilder_control
{
	explicit tbuilder_minimap(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
