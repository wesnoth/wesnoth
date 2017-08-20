/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/button.hpp"

#include "gui/core/log.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"

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

REGISTER_WIDGET(button)

button::button(const implementation::builder_button& builder)
	: styled_widget(builder, get_control_type())
	, clickable_item()
	, state_(ENABLED)
	, retval_(0)
{
	connect_signal<event::MOUSE_ENTER>(
			std::bind(&button::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(
			std::bind(&button::signal_handler_mouse_leave, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&button::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(
			std::bind(&button::signal_handler_left_button_up, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&button::signal_handler_left_button_click, this, _2, _3));
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
		set_is_dirty(true);
	}
}

const std::string& button::get_control_type() const
{
	static const std::string type = "button";
	return type;
}

void button::signal_handler_mouse_enter(const event::ui_event event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void button::signal_handler_mouse_leave(const event::ui_event event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

void button::signal_handler_left_button_down(const event::ui_event event,
											  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

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
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void button::signal_handler_left_button_click(const event::ui_event event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	if(retval_ != 0) {
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
	DBG_GUI_P << "Parsing button " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_button
 *
 * == Button ==
 *
 * @macro = button_description
 *
 * The following states exist:
 * * state_enabled, the button is enabled.
 * * state_disabled, the button is disabled.
 * * state_pressed, the left mouse button is down.
 * * state_focused, the mouse is over the button.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="button_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
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
 * @end{tag}{name="button_definition"}
 * @end{parent}{name="gui/"}
 */
button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in button.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
	state.emplace_back(cfg.child("state_pressed"));
	state.emplace_back(cfg.child("state_focused"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{button_description}
 *
 *        A button is a styled_widget that can be pushed to start an action or close
 *        a dialog.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_button
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="button"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Button ==
 *
 * @macro = button_description
 *
 * Instance of a button. When a button has a return value it sets the
 * return value for the window. Normally this closes the window and returns
 * this value to the caller. The return value can either be defined by the
 * user or determined from the id of the button. The return value has a
 * higher precedence as the one defined by the id. (Of course it's weird to
 * give a button an id and then override its return value.)
 *
 * When the button doesn't have a standard id, but you still want to use the
 * return value of that id, use return_value_id instead. This has a higher
 * precedence as return_value.
 *
 * List with the button specific variables:
 * @begin{table}{config}
 *     return_value_id & string & "" &   The return value id. $
 *     return_value & int & 0 &          The return value. $
 *
 * @end{table}
 * @end{tag}{name="button"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_button::builder_button(const config& cfg)
	: builder_styled_widget(cfg)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
{
}

widget* builder_button::build() const
{
	button* widget = new button(*this);

	widget->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed button '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
