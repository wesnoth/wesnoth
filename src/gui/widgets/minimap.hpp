/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
class gamemap;
class texture;

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

	void set_map_data(const std::string& map_data);

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

	/** Game map generated from the provided data. */
	std::unique_ptr<gamemap> map_;

	/** Drawing function passed to the background canvas. */
	void canvas_draw_background(texture& tex);

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
