/*
	Copyright (C) 2008 - 2023
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

#include "gui/widgets/spinner.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(spinner)

spinner::spinner(const implementation::builder_spinner& builder)
	: container_base(builder, type())
	, state_(ENABLED)
	, step_size_(1)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&spinner::signal_handler_left_button_down, this, std::placeholders::_2),
		event::dispatcher::back_pre_child);


//	grid* grid_ = find_widget<grid>(this, "_content_grid", false, true);
//	button* btn_prev = find_widget<button>(grid_, "_prev", false, true);
//	connect_signal_mouse_left_click(*btn_prev,
//		std::bind(&spinner::prev, this));
}

text_box* spinner::get_internal_text_box()
{
	return find_widget<text_box>(this, "_text", false, true);
}

void spinner::set_value(const int val)
{
	text_box* edit_area = get_internal_text_box();
	if (edit_area != nullptr) {
		edit_area->set_value(std::to_string(val));
	}
}

int spinner::get_value()
{
	/* Return 0 if invalid.
	 * TODO: give visual indication of wrong value
	 */
	int val;
	try {
		text_box* edit_area = get_internal_text_box();
		if (edit_area != nullptr) {
			val = stoi(edit_area->get_value());
		} else {
			val = 0;
		}
	} catch(std::invalid_argument const& ex) {
		val = 0;
	} catch(std::out_of_range const& ex) {
		val = 0;
	}

	return val;
}

void spinner::set_self_active(const bool active)
{
	state_ = active ? ENABLED : DISABLED;
}

bool spinner::get_active() const
{
	return state_ != DISABLED;
}

unsigned spinner::get_state() const
{
	return state_;
}

bool spinner::can_wrap() const
{
	return true;
}

void spinner::signal_handler_left_button_down(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	get_window()->keyboard_capture(this);
}

// }---------- DEFINITION ---------{

spinner_definition::spinner_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing spinner " << id;

	load_resolutions<resolution>(cfg);
}

spinner_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t is spinner.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for scroll label control")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", _("Missing required state for scroll label control")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", _("No grid defined for scroll label control"));
	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_spinner::builder_spinner(const config& cfg)
	: implementation::builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_spinner::build() const
{
	auto widget = std::make_unique<spinner>(*this);

	const auto conf = widget->cast_config_to<spinner_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);

	DBG_GUI_G << "Window builder: placed spinner '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
