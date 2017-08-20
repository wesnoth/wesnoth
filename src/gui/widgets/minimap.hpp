/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/styled_widget.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

class config;

namespace gui2
{
namespace implementation
{
struct builder_minimap;
}

// ------------ WIDGET -----------{

/**
 * The basic minimap class.
 *
 * This minimap can only show a minimap, but it can't be interacted with. For
 * that the tminimap_interactive class will be created.
 */
class minimap : public styled_widget
{
public:
	explicit minimap(const implementation::builder_minimap& builder);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** See @ref styled_widget::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref styled_widget::get_active. */
	virtual bool get_active() const override;

	/** See @ref styled_widget::get_state. */
	virtual unsigned get_state() const override;

	/** See @ref widget::disable_click_dismiss. */
	bool disable_click_dismiss() const override;

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
	 * @returns                   The image, nullptr upon error.
	 */
	const surface get_image(const int w, const int h) const;

	/** See @ref widget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer,
									  int x_offset,
									  int y_offset) override;

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;
};

// }---------- DEFINITION ---------{

struct minimap_definition : public styled_widget_definition
{
	explicit minimap_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_minimap : public builder_styled_widget
{
	explicit builder_minimap(const config& cfg);

	using builder_styled_widget::build;

	widget* build() const;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
