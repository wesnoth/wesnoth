/*
	Copyright (C) 2023 - 2023
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

#include "gui/widgets/spinbox.hpp"

#include "gui/core/log.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "sound.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <iostream>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(spinbox)

spinbox::spinbox(const implementation::builder_spinbox& builder)
	: styled_widget(builder, type())
	  , state_(ENABLED)
	  , minimum_value_(0)
	  , step_size_(1)
{
	connect_signal<event::SDL_KEY_DOWN>(std::bind(
				&spinbox::signal_handler_sdl_key_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6));

	connect_signal<event::MOUSE_ENTER>(
		std::bind(&spinbox::signal_handler_mouse_enter, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::MOUSE_LEAVE>(
		std::bind(&spinbox::signal_handler_mouse_leave, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&spinbox::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_UP>(
		std::bind(&spinbox::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_CLICK>(
		std::bind(&spinbox::signal_handler_left_button_click, this, std::placeholders::_2, std::placeholders::_3));

	set_value(0);
}

void spinbox::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool spinbox::get_active() const
{
	return state_ != DISABLED;
}

void spinbox::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		queue_redraw();
	}
}


unsigned spinbox::get_state() const
{
	return state_;
}

void spinbox::signal_handler_sdl_key_down(const event::ui_event event, bool& handled, const SDL_Keycode key, SDL_Keymod /*modifier*/)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;

	std::cout << "Event key down" << std::endl;

	switch(key) {
	case SDLK_DOWN:
	case SDLK_LEFT:
		prev();
		queue_redraw();
		break;

	case SDLK_UP:
	case SDLK_RIGHT:
		next();
		queue_redraw();
		break;

	default:
		break;
	}
}

void spinbox::signal_handler_mouse_enter(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if(state_ != FOCUSED) {
		set_state(HOVERED);
	}

	handled = true;
}

void spinbox::signal_handler_mouse_leave(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if(state_ != FOCUSED) {
		set_state(ENABLED);
	}

	handled = true;
}

void spinbox::signal_handler_left_button_down(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	window* window = get_window();
	if(window) {
		window->mouse_capture();
		window->keyboard_capture(this);
	}

	/* get_x() is the left border
	 * this->get_size().x is the size of this widget
	 * so get_x() + this->get_size().x is the right border
	 * 25 is the size of the left/right arrow icons. */

	int x = get_x() + this->get_size().x;
	int mx = get_mouse_position().x;
	std::cout << x << "," << mx << std::endl;
	if ((mx <= x) && (mx >= x-25)) {
		next();
	} else if ((mx <= get_x()+25) && (mx >= get_x())) {
		prev();
	}

	set_state(FOCUSED);
	handled = true;
}

void spinbox::signal_handler_left_button_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void spinbox::signal_handler_left_button_click(const event::ui_event event, bool& handled)
{
	assert(get_window());
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	sound::play_UI_sound(settings::sound_button_click);

	handled = true;
}


// }---------- DEFINITION ---------{

spinbox_definition::spinbox_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing spinbox " << id;

	load_resolutions<resolution>(cfg);
}

spinbox_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in spinbox.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for spinbox")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", _("Missing required state for spinbox")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", _("Missing required state for spinbox")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_hovered", _("Missing required state for spinbox")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_spinbox::builder_spinbox(const config& cfg)
	: builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_spinbox::build() const
{
	auto widget = std::make_unique<spinbox>(*this);

	DBG_GUI_G << "Window builder: placed spinbox '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
