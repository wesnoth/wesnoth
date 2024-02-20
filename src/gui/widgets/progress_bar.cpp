/*
	Copyright (C) 2010 - 2024
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

#include "gui/widgets/progress_bar.hpp"

#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(progress_bar)

progress_bar::progress_bar(const implementation::builder_progress_bar& builder)
	: styled_widget(builder, type())
	, percentage_(static_cast<unsigned>(-1))
{
	// Force canvas update
	set_percentage(0);
}

void progress_bar::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool progress_bar::get_active() const
{
	return true;
}

unsigned progress_bar::get_state() const
{
	return ENABLED;
}

void progress_bar::set_percentage(unsigned percentage)
{
	percentage = std::min<unsigned>(percentage, 100);

	if(percentage_ != percentage) {
		percentage_ = percentage;

		for(auto & c : get_canvases())
		{
			c.set_variable("percentage", wfl::variant(percentage));
		}

		queue_redraw();
	}
}

bool progress_bar::disable_click_dismiss() const
{
	return false;
}

// }---------- DEFINITION ---------{

progress_bar_definition::progress_bar_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing progress bar " << id;

	load_resolutions<resolution>(cfg);
}

progress_bar_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in progress_bar.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for progress bar")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_progress_bar::builder_progress_bar(const config& cfg)
	: builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_progress_bar::build() const
{
	auto widget = std::make_unique<progress_bar>(*this);

	DBG_GUI_G << "Window builder: placed progress bar '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
