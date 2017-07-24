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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/minimap.hpp"

#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "sdl/rect.hpp"
#include "terrain/type_data.hpp"
#include "../../minimap.hpp" // We want the file in src/
#include "video.hpp"

#include "utils/functional.hpp"

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
	: styled_widget(builder, get_control_type())
	, map_data_()
	, terrain_(nullptr)
	, map_(nullptr)
{
	get_canvas(0).set_draw_function(std::bind(&minimap::canvas_draw_background, this, _1));
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

	try {
		map_.reset(new gamemap(std::make_shared<terrain_type_data>(*terrain_), map_data_));
	} catch(incorrect_map_format_error& e) {
		map_.reset(nullptr);
		ERR_CF << "Error while loading the map: " << e.message << '\n';
	}

	// Flag the background canvas as dirty so the minimap is redrawn.
	get_canvas(0).set_is_dirty(true);
}

void minimap::canvas_draw_background(texture& tex)
{
	if(map_) {
		image::render_minimap(tex, *map_, nullptr, nullptr, true);
	}
}

// }---------- DEFINITION ---------{

minimap_definition::minimap_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing minimap " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_minimap
 *
 * == Minimap ==
 *
 * @macro = minimap_description
 *
 * The following states exist:
 * * state_enabled, the minimap is enabled.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="minimap_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="minimap_definition"}
 * @end{parent}{name="gui/"}
 */
minimap_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in minimap.hpp.
	state.emplace_back(cfg.child("state_enabled"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{minimap_description}
 *
 *        A minimap to show the gamemap, this only shows the map and has no
 *        interaction options. This version is used for map previews, there
 *        will be a another version which allows interaction.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_minimap
 *
 * == Minimap ==
 *
 * @macro = minimap_description
 *
 * A minimap has no extra fields.
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="minimap"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @end{tag}{name="minimap"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_minimap::builder_minimap(const config& cfg) : builder_styled_widget(cfg)
{
}

widget* builder_minimap::build() const
{
	minimap* widget = new minimap(*this);

	DBG_GUI_G << "Window builder: placed minimap '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
