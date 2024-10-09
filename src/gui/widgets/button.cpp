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

#include "gui/widgets/button.hpp"

#include "gui/core/log.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder/helper.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include "sound.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(button)

button::button(const implementation::builder_button& builder)
	: styled_widget(builder, type())
	, clickable_item()
	, state_(ENABLED)
	, retval_(retval::NONE)
	, success_(false)
{
	connect_signal<event::MOUSE_ENTER>(
			std::bind(&button::signal_handler_mouse_enter, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::MOUSE_LEAVE>(
			std::bind(&button::signal_handler_mouse_leave, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&button::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_UP>(
			std::bind(&button::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&button::signal_handler_left_button_click, this, std::placeholders::_2, std::placeholders::_3));
}

void button::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool button::get_active() const
{
	return state_ != DISABLED;
}

unsigned button::get_state() const
{
	return state_;
}

void button::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		queue_redraw();
	}
}

void button::set_success(bool success) {
	success_ = success;
	if (success) {
		set_state(SUCCESS);
	}
}

void button::signal_handler_mouse_enter(const event::ui_event event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void button::signal_handler_mouse_leave(const event::ui_event event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if (success_) {
		set_state(SUCCESS);
	} else {
		set_state(ENABLED);
	}
	handled = true;
}

void button::signal_handler_left_button_down(const event::ui_event event,
											  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	window* window = get_window();
	if(window) {
		window->mouse_capture();
	}

	set_state(PRESSED);
	handled = true;
}

void button::signal_handler_left_button_up(const event::ui_event event,
											bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void button::signal_handler_left_button_click(const event::ui_event event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	if(retval_ != retval::NONE) {
		window* window = get_window();
		if(window) {
			window->set_retval(retval_);
			return;
		}
	}

	handled = true;
}

// }---------- DEFINITION ---------{

button_definition::button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing button " << id;

	load_resolutions<resolution>(cfg);
}

button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in button.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("button_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("button_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_pressed", missing_mandatory_wml_tag("button_definition][resolution", "state_pressed")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("button_definition][resolution", "state_focused")));
	// state_success is optional, so error message not needed.
	if (cfg.optional_child("state_success")) {
		state.emplace_back(cfg.mandatory_child("state_success"));
	}

}

// }---------- BUILDER -----------{

namespace implementation
{

builder_button::builder_button(const config& cfg)
	: builder_styled_widget(cfg)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"].to_int())
{
}

std::unique_ptr<widget> builder_button::build() const
{
	auto widget = std::make_unique<button>(*this);

	widget->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed button '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
