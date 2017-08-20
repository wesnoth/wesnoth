/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/repeating_button.hpp"

#include "gui/core/log.hpp"
#include "gui/core/timer.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(repeating_button)

repeating_button::repeating_button(const implementation::builder_repeating_button& builder)
	: styled_widget(builder, get_control_type())
	, clickable_item()
	, state_(ENABLED)
	, repeat_timer_(0)
{
	connect_signal<event::MOUSE_ENTER>(std::bind(
			&repeating_button::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(std::bind(
			&repeating_button::signal_handler_mouse_leave, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&repeating_button::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(std::bind(
			&repeating_button::signal_handler_left_button_up, this, _2, _3));
}

repeating_button::~repeating_button()
{
	if(repeat_timer_) {
		remove_timer(repeat_timer_);
	}
}

void repeating_button::connect_signal_mouse_left_down(
		const event::signal_function& signal)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(signal);
}

void repeating_button::disconnect_signal_mouse_left_down(
		const event::signal_function& signal)
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
		set_is_dirty(true);

		if(state_ == DISABLED && repeat_timer_) {
			remove_timer(repeat_timer_);
			repeat_timer_ = 0;
		}
	}
}

const std::string& repeating_button::get_control_type() const
{
	static const std::string type = "repeating_button";
	return type;
}

void repeating_button::signal_handler_mouse_enter(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void repeating_button::signal_handler_mouse_leave(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

void
repeating_button::signal_handler_left_button_down(const event::ui_event event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

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
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

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
	DBG_GUI_P << "Parsing repeating button " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_repeating_button
 *
 * == Repeating button ==
 *
 * @macro = repeating_button_description
 *
 * The following states exist:
 * * state_enabled, the repeating_button is enabled.
 * * state_disabled, the repeating_button is disabled.
 * * state_pressed, the left mouse repeating_button is down.
 * * state_focused, the mouse is over the repeating_button.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="repeating_button_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_pressed"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_pressed"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="repeating_button_definition"}
 * @end{parent}{name="gui/"}
 */
repeating_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in
	// repeating_button.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
	state.emplace_back(cfg.child("state_pressed"));
	state.emplace_back(cfg.child("state_focused"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{repeating_button_description}
 *
 *        A repeating_button is a styled_widget that can be pushed down and repeat a
 *        certain action. Once the button is down every x milliseconds it is
 *        down a new down event is triggered.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_repeating_button
 *
 * == Repeating button ==
 *
 * @macro = repeating_button_description
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="repeating_button"}{min=0}{max=-1}{super="gui/window/resolution/grid/row/column/button"}
 * @end{tag}{name="repeating_button"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_repeating_button::builder_repeating_button(const config& cfg)
	: builder_styled_widget(cfg)
{
}

widget* builder_repeating_button::build() const
{
	repeating_button* widget = new repeating_button(*this);

	DBG_GUI_G << "Window builder: placed repeating button '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
