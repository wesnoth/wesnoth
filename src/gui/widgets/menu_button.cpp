/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/dialogs/drop_down_list.hpp"
#include "config_assign.hpp"
#include "sound.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(menu_button)

tmenu_button::tmenu_button()
	: tcontrol(COUNT)
	, tselectable_()
	, state_(ENABLED)
	, retval_(0)
	, values_()
	, selected_()
{
	values_.push_back(config_of("label", this->label()));

	connect_signal<event::MOUSE_ENTER>(
			std::bind(&tmenu_button::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(
			std::bind(&tmenu_button::signal_handler_mouse_leave, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&tmenu_button::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(
			std::bind(&tmenu_button::signal_handler_left_button_up, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&tmenu_button::signal_handler_left_button_click, this, _2, _3));
}

void tmenu_button::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool tmenu_button::get_active() const
{
	return state_ != DISABLED;
}

unsigned tmenu_button::get_state() const
{
	return state_;
}

void tmenu_button::set_state(const tstate state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

const std::string& tmenu_button::get_control_type() const
{
	static const std::string type = "menu_button";
	return type;
}

void tmenu_button::signal_handler_mouse_enter(const event::tevent event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void tmenu_button::signal_handler_mouse_leave(const event::tevent event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

void tmenu_button::signal_handler_left_button_down(const event::tevent event,
											  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	twindow* window = get_window();
	if(window) {
		window->mouse_capture();
	}

	set_state(PRESSED);
	handled = true;
}

void tmenu_button::signal_handler_left_button_up(const event::tevent event,
											bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void tmenu_button::signal_handler_left_button_click(const event::tevent event,
											   bool& handled)
{
	assert(get_window());
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	tdrop_down_list droplist(this->get_rectangle(), this->values_, this->selected_, this->get_use_markup());

	if(droplist.show(get_window()->video())) {
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
			if(twindow* window = get_window()) {
				window->set_retval(retval_);
				return;
			}
		}
	}

	handled = true;
}

void tmenu_button::set_values(const std::vector<::config>& values, int selected)
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
void tmenu_button::set_selected(int selected)
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

tmenu_button_definition::tmenu_button_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing menu_button " << id << '\n';

	load_resolutions<tresolution>(cfg);
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
tmenu_button_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
{
	// Note the order should be the same as the enum tstate in menu_button.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focused")));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{menu_button_description}
 *
 *        A menu_button is a control to choose an element from a list of elements.
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

tbuilder_menu_button::tbuilder_menu_button(const config& cfg)
	: tbuilder_control(cfg)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
	, options_()
{
	for(const auto& option : cfg.child_range("option")) {
		options_.push_back(option);
	}
}

twidget* tbuilder_menu_button::build() const
{
	tmenu_button* widget = new tmenu_button();

	init_control(widget);

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
