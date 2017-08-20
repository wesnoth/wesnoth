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

#include "gui/widgets/menu_button.hpp"

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

REGISTER_WIDGET(menu_button)

menu_button::menu_button(const implementation::builder_menu_button& builder)
	: styled_widget(builder, get_control_type())
	, selectable_item()
	, state_(ENABLED)
	, retval_(0)
	, values_()
	, selected_()
	, keep_open_(false)
	, droplist_(nullptr)
{
	values_.emplace_back(::config {"label", this->get_label()});

	connect_signal<event::MOUSE_ENTER>(
			std::bind(&menu_button::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(
			std::bind(&menu_button::signal_handler_mouse_leave, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&menu_button::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(
			std::bind(&menu_button::signal_handler_left_button_up, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&menu_button::signal_handler_left_button_click, this, _2, _3));
}

void menu_button::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool menu_button::get_active() const
{
	return state_ != DISABLED;
}

unsigned menu_button::get_state() const
{
	return state_;
}

void menu_button::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

void menu_button::signal_handler_mouse_enter(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void menu_button::signal_handler_mouse_leave(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

void menu_button::signal_handler_left_button_down(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	window* window = get_window();
	if(window) {
		window->mouse_capture();
	}

	set_state(PRESSED);
	handled = true;
}

void menu_button::signal_handler_left_button_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void menu_button::signal_handler_left_button_click(const event::ui_event event, bool& handled)
{
	assert(get_window());
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	dialogs::drop_down_menu droplist(this->get_rectangle(), this->values_, this->selected_, this->get_use_markup(), this->keep_open_,
		nullptr);

	droplist_ = &droplist;

	if(droplist.show(get_window()->video())) {
		droplist_ = nullptr;

		const int selected = droplist.selected_item();

		// Saftey check. If the user clicks a selection in the dropdown and moves their mouse away too
		// quickly, selected_ could be set to -1. This returns in that case, preventing crashes.
		if(selected < 0) {
			return;
		}

		selected_ = selected;

		this->set_label(values_[selected_]["label"]);

		fire(event::NOTIFY_MODIFIED, *this, nullptr);

		if(callback_state_change_) {
			callback_state_change_(*this);
		}

		if(retval_ != 0) {
			if(window* window = get_window()) {
				window->set_retval(retval_);
				return;
			}
		}
	}

	droplist_ = nullptr;

	handled = true;
}

void menu_button::set_values(const std::vector<::config>& values, int selected)
{
	assert(static_cast<size_t>(selected) < values.size());
	assert(static_cast<size_t>(selected_) < values_.size());

	if(values[selected]["label"] != values_[selected_]["label"]) {
		set_is_dirty(true);
	}

	values_ = values;
	selected_ = selected;

	set_label(values_[selected_]["label"]);
}

void menu_button::set_selected(int selected)
{
	assert(static_cast<size_t>(selected) < values_.size());
	assert(static_cast<size_t>(selected_) < values_.size());

	if(selected != selected_) {
		set_is_dirty(true);
	}

	selected_ = selected;

	set_label(values_[selected_]["label"]);
}

// }---------- DEFINITION ---------{

menu_button_definition::menu_button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing menu_button " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_menu_button
 *
 * == menu_button ==
 *
 * @macro = menu_button_description
 *
 * The following states exist:
 * * state_enabled, the menu_button is enabled.
 * * state_disabled, the menu_button is disabled.
 * * state_pressed, the left mouse menu_button is down.
 * * state_focused, the mouse is over the menu_button.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="menu_button_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
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
 * @end{tag}{name="menu_button_definition"}
 * @end{parent}{name="gui/"}
 */
menu_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in menu_button.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
	state.emplace_back(cfg.child("state_pressed"));
	state.emplace_back(cfg.child("state_focused"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{menu_button_description}
 *
 *        A menu_button is a styled_widget to choose an element from a list of elements.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_menu_button
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="menu_button"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == menu_button ==
 *
 * @macro = menu_button_description
 *
 * Instance of a menu_button. When a menu_button has a return value it sets the
 * return value for the window. Normally this closes the window and returns
 * this value to the caller. The return value can either be defined by the
 * user or determined from the id of the menu_button. The return value has a
 * higher precedence as the one defined by the id. (Of course it's weird to
 * give a menu_button an id and then override its return value.)
 *
 * When the menu_button doesn't have a standard id, but you still want to use the
 * return value of that id, use return_value_id instead. This has a higher
 * precedence as return_value.
 *
 * List with the menu_button specific variables:
 * @begin{table}{config}
 *     return_value_id & string & "" &   The return value id. $
 *     return_value & int & 0 &          The return value. $
 *
 * @end{table}
 * @end{tag}{name="menu_button"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_menu_button::builder_menu_button(const config& cfg)
	: builder_styled_widget(cfg)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
	, options_()
{
	for(const auto& option : cfg.child_range("option")) {
		options_.push_back(option);
	}
}

widget* builder_menu_button::build() const
{
	menu_button* widget = new menu_button(*this);

	widget->set_retval(get_retval(retval_id_, retval_, id));
	if(!options_.empty()) {
		widget->set_values(options_);
	}

	DBG_GUI_G << "Window builder: placed menu_button '" << id
	          << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
