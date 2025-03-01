/*
	Copyright (C) 2009 - 2025
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

#include "gui/widgets/repeating_button.hpp"

#include "gui/core/log.hpp"
#include "gui/core/timer.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(repeating_button)

repeating_button::repeating_button(const implementation::builder_repeating_button& builder)
	: styled_widget(builder, type())
	, clickable_item()
	, state_(ENABLED)
	, repeat_timer_(0)
{
	connect_signal<event::MOUSE_ENTER>(std::bind(
			&repeating_button::signal_handler_mouse_enter, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::MOUSE_LEAVE>(std::bind(
			&repeating_button::signal_handler_mouse_leave, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&repeating_button::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_UP>(std::bind(
			&repeating_button::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));
}

repeating_button::~repeating_button()
{
	if(repeat_timer_) {
		remove_timer(repeat_timer_);
	}
}

void repeating_button::connect_signal_mouse_left_down(
		const event::signal& signal)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(signal);
}

void repeating_button::disconnect_signal_mouse_left_down(
		const event::signal& signal)
{
	disconnect_signal<event::LEFT_BUTTON_DOWN>(signal);
}

void repeating_button::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool repeating_button::get_active() const
{
	return state_ != DISABLED;
}

unsigned repeating_button::get_state() const
{
	return state_;
}

void repeating_button::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		queue_redraw();

		if(state_ == DISABLED && repeat_timer_) {
			remove_timer(repeat_timer_);
			repeat_timer_ = 0;
		}
	}
}

void repeating_button::signal_handler_mouse_enter(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void repeating_button::signal_handler_mouse_leave(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(ENABLED);
	handled = true;
}

void
repeating_button::signal_handler_left_button_down(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	// If the timer isn't set it's the initial down event.
	if(!repeat_timer_) {

		// mimic the old gui and only play the sound once.
		sound::play_UI_sound(settings::sound_button_click);

		window* window = get_window();
		if(window) {
			repeat_timer_ = add_timer(settings::repeat_button_repeat_time,
									  [this, window](unsigned int) {
											window->fire(event::LEFT_BUTTON_DOWN, *this);
									  },true);

			window->mouse_capture();
		}

		set_state(PRESSED);
	}

	handled = true;
}

void repeating_button::signal_handler_left_button_up(const event::ui_event event,
													  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if(repeat_timer_) {
		remove_timer(repeat_timer_);
		repeat_timer_ = 0;
	}

	if(get_active()) {
		set_state(FOCUSED);
	}
	handled = true;
}

// }---------- DEFINITION ---------{

repeating_button_definition::repeating_button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing repeating button " << id;

	load_resolutions<resolution>(cfg);
}

repeating_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in
	// repeating_button.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("repeating_button_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("repeating_button_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_pressed", missing_mandatory_wml_tag("repeating_button_definition][resolution", "state_pressed")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("repeating_button_definition][resolution", "state_focused")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_repeating_button::builder_repeating_button(const config& cfg)
	: builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_repeating_button::build() const
{
	auto widget = std::make_unique<repeating_button>(*this);

	DBG_GUI_G << "Window builder: placed repeating button '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
