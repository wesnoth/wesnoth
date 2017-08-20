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

#include "gui/widgets/toggle_button.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(toggle_button)

toggle_button::toggle_button(const implementation::builder_toggle_button& builder)
	: styled_widget(builder, get_control_type())
	, state_(ENABLED)
	, state_num_(0)
	, retval_(0)
	, callback_state_change_()
	, icon_name_()
{
	connect_signal<event::MOUSE_ENTER>(std::bind(
			&toggle_button::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(std::bind(
			&toggle_button::signal_handler_mouse_leave, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&toggle_button::signal_handler_left_button_click, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(std::bind(
			&toggle_button::signal_handler_left_button_double_click,
			this,
			_2,
			_3));
}

unsigned toggle_button::num_states() const
{
	std::div_t res = std::div(this->config()->state.size(), COUNT);
	assert(res.rem == 0);
	assert(res.quot > 0);
	return res.quot;
}

void toggle_button::set_members(const string_map& data)
{
	// Inherit
	styled_widget::set_members(data);

	string_map::const_iterator itor = data.find("icon");
	if(itor != data.end()) {
		set_icon_name(itor->second);
	}
}

void toggle_button::set_active(const bool active)
{
	if(active) {
		set_state(ENABLED);
	} else {
		set_state(DISABLED);
	}
}

bool toggle_button::get_active() const
{
	return state_ != DISABLED;
}

unsigned toggle_button::get_state() const
{
	return state_ +  COUNT * state_num_;
}

void toggle_button::update_canvas()
{
	// Inherit.
	styled_widget::update_canvas();

	// set icon in canvases
	std::vector<canvas>& canvases = styled_widget::get_canvases();
	for(auto & canvas : canvases)
	{
		canvas.set_variable("icon", wfl::variant(icon_name_));
	}

	set_is_dirty(true);
}

void toggle_button::set_value(const unsigned selected)
{
	if(selected == get_value()) {
		return;
	}
	state_num_ = selected % num_states();
	set_is_dirty(true);

	// Check for get_window() is here to prevent the callback from
	// being called when the initial value is set.
	if(callback_state_change_ && get_window() != nullptr) {
		callback_state_change_(*this);
	}
}

void toggle_button::set_retval(const int retval)
{
	if(retval == retval_) {
		return;
	}

	retval_ = retval;
	set_wants_mouse_left_double_click(retval_ != 0);
}

void toggle_button::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

void toggle_button::signal_handler_mouse_enter(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";
	set_state(FOCUSED);
	handled = true;
}

void toggle_button::signal_handler_mouse_leave(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";
	set_state(ENABLED);
	handled = true;
}

void toggle_button::signal_handler_left_button_click(const event::ui_event event,
													  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	sound::play_UI_sound(settings::sound_toggle_button_click);

	set_value(get_value() + 1);

	fire(event::NOTIFY_MODIFIED, *this, nullptr);

	handled = true;
}

void toggle_button::signal_handler_left_button_double_click(
		const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(retval_ == 0) {
		return;
	}

	window* window = get_window();
	assert(window);

	window->set_retval(retval_);

	handled = true;
}

// }---------- DEFINITION ---------{

toggle_button_definition::toggle_button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle button " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_toggle_button
 *
 * == Toggle button ==
 *
 * The definition of a toggle button.
 *
 * The following states exist:
 * * state_enabled, the button is enabled and not selected.
 * * state_disabled, the button is disabled and not selected.
 * * state_focused, the mouse is over the button and not selected.
 *
 * * state_enabled_selected, the button is enabled and selected.
 * * state_disabled_selected, the button is disabled and selected.
 * * state_focused_selected, the mouse is over the button and selected.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="oggle_button_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @begin{tag}{name="state_enabled_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled_selected"}
 * @begin{tag}{name="state_disabled_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled_selected"}
 * @begin{tag}{name="state_focused_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused_selected"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="oggle_button_definition"}
 * @end{parent}{name="gui/"}
 */
toggle_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in
	// toggle_button.hpp.
	for(const auto& c : cfg.child_range("state"))
	{
		state.emplace_back(c.child("enabled"));
		state.emplace_back(c.child("disabled"));
		state.emplace_back(c.child("focused"));
	}
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_toggle_button
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="toggle_button"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Toggle button ==
 *
 * @begin{table}{config}
 *     icon & f_string & "" &          The name of the icon file to show. $
 *     return_value_id & string & "" & The return value id, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 *     return_value & int & 0 &        The return value, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 * @end{table}
 * @end{tag}{name="toggle_button"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_toggle_button::builder_toggle_button(const config& cfg)
	: builder_styled_widget(cfg)
	, icon_name_(cfg["icon"])
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
{
}

widget* builder_toggle_button::build() const
{
	toggle_button* widget = new toggle_button(*this);

	widget->set_icon_name(icon_name_);
	widget->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed toggle button '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
