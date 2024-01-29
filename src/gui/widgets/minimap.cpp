/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/minimap.hpp"

#include "draw.hpp"
#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "sdl/rect.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"
#include "../../minimap.hpp" // We want the file in src/

#include <functional>

#include <algorithm>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM_INDENT(err, log_config)

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

// Define this to enable debug output for the minimap cache.
//#define DEBUG_MINIMAP_CACHE

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(minimap)

minimap::minimap(const implementation::builder_minimap& builder)
	: styled_widget(builder, type())
	, map_data_()
{
}

void minimap::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool minimap::get_active() const
{
	return true;
}

unsigned minimap::get_state() const
{
	return 0;
}

bool minimap::disable_click_dismiss() const
{
	return false;
}

void minimap::set_map_data(const std::string& map_data)
{
	if(map_data == map_data_) {
		return;
	}

	map_data_ = map_data;
	queue_redraw();

	try {
		map_ = std::make_unique<gamemap>(map_data_);
	} catch(const incorrect_map_format_error& e) {
		map_.reset(nullptr);
		ERR_CF << "Error while loading the map: " << e.message;
	}
}

bool minimap::impl_draw_background()
{
	if(map_) {
		image::render_minimap(get_width(), get_height(), *map_, nullptr, nullptr, nullptr, true);
	}
	return true;
}

// }---------- DEFINITION ---------{

minimap_definition::minimap_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing minimap " << id;

	load_resolutions<resolution>(cfg);
}

minimap_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in minimap.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for minimap control")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_minimap::builder_minimap(const config& cfg) : builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_minimap::build() const
{
	auto widget = std::make_unique<minimap>(*this);

	DBG_GUI_G << "Window builder: placed minimap '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
