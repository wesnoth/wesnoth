/*
	Copyright (C) 2008 - 2021
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

#include "gui/widgets/panel.hpp"

#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gettext.hpp"
#include "sdl/rect.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(panel)

panel::panel(const implementation::builder_styled_widget& builder, const std::string& control_type)
	: container_base(builder, control_type.empty() ? type() : control_type)
{
}

SDL_Rect panel::get_client_rect() const
{
	const auto conf = cast_config_to<panel_definition>();
	assert(conf);

	SDL_Rect result = get_rectangle();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

bool panel::get_active() const
{
	return true;
}

unsigned panel::get_state() const
{
	return 0;
}

void panel::impl_draw_background(surface& frame_buffer, int x_offset, int y_offset)
{
	DBG_GUI_D << LOG_HEADER << " size " << get_rectangle() << ".\n";

	get_canvas(0).blit(frame_buffer,
				   calculate_blitting_rectangle(x_offset, y_offset));
}

void panel::impl_draw_foreground(surface& frame_buffer, int x_offset, int y_offset)
{
	DBG_GUI_D << LOG_HEADER << " size " << get_rectangle() << ".\n";

	get_canvas(1).blit(frame_buffer,
				   calculate_blitting_rectangle(x_offset, y_offset));
}

point panel::border_space() const
{
	const auto conf = cast_config_to<panel_definition>();
	assert(conf);

	return point(conf->left_border + conf->right_border, conf->top_border + conf->bottom_border);
}

void panel::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

panel_definition::panel_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing panel " << id << '\n';

	load_resolutions<resolution>(cfg);
}

panel_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, top_border(cfg["top_border"])
	, bottom_border(cfg["bottom_border"])
	, left_border(cfg["left_border"])
	, right_border(cfg["right_border"])
{
	// The panel needs to know the order.
	state.emplace_back(cfg.child("background"));
	state.emplace_back(cfg.child("foreground"));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_panel::builder_panel(const config& cfg)
	: builder_styled_widget(cfg), grid(nullptr)
{
	const config& c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = std::make_shared<builder_grid>(c);
}

widget* builder_panel::build() const
{
	panel* widget = new panel(*this);

	DBG_GUI_G << "Window builder: placed panel '" << id << "' with definition '"
			  << definition << "'.\n";

	widget->init_grid(*grid);
	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
