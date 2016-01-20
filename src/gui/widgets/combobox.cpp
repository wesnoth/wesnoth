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

#include "gui/widgets/combobox.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/combobox.hpp"
#include "gui/auxiliary/window_builder/combobox.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/dialogs/drop_down_list.hpp"
#include "sound.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

REGISTER_WIDGET(combobox)

tcombobox::tcombobox() 
	: tcontrol(COUNT)
	, tclickable_()
	, state_(ENABLED)
	, retval_(0)
	, values_()
	, selected_()
{
	values_.push_back(this->label());

	connect_signal<event::MOUSE_ENTER>(
			boost::bind(&tcombobox::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(
			boost::bind(&tcombobox::signal_handler_mouse_leave, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_DOWN>(boost::bind(
			&tcombobox::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(
			boost::bind(&tcombobox::signal_handler_left_button_up, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_CLICK>(boost::bind(
			&tcombobox::signal_handler_left_button_click, this, _2, _3));
}

void tcombobox::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool tcombobox::get_active() const
{
	return state_ != DISABLED;
}

unsigned tcombobox::get_state() const
{
	return state_;
}

void tcombobox::set_state(const tstate state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

const std::string& tcombobox::get_control_type() const
{
	static const std::string type = "button";
	return type;
}

void tcombobox::signal_handler_mouse_enter(const event::tevent event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void tcombobox::signal_handler_mouse_leave(const event::tevent event,
										 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

void tcombobox::signal_handler_left_button_down(const event::tevent event,
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

void tcombobox::signal_handler_left_button_up(const event::tevent event,
											bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void tcombobox::signal_handler_left_button_click(const event::tevent event,
											   bool& handled)
{
	assert(get_window());
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	tdrop_down_list droplist(this->get_rectangle(), this->values_, this->selected_, this->get_use_markup());

	if(droplist.show(get_window()->video())) {
		selected_ = droplist.selected_item();
		this->set_label(values_[selected_]);
		if(selected_callback_) {
			selected_callback_(*this);
		}
		if(retval_ != 0) {
			twindow* window = get_window();
			if(window) {
				window->set_retval(retval_);
				return;
			}
		}
	}

	handled = true;
}

void tcombobox::set_values(const std::vector<std::string>& values, int selected)
{
	assert(static_cast<size_t>(selected) < values.size());
	assert(static_cast<size_t>(selected_) < values_.size());
	if(values[selected] != values_[selected_]) {
		set_is_dirty(true);
	}
	values_ = values;
	selected_ = selected;
	set_label(values_[selected_]);

}
void tcombobox::set_selected(int selected)
{
	assert(static_cast<size_t>(selected) < values_.size());
	assert(static_cast<size_t>(selected_) < values_.size());
	if(selected != selected_) {
		set_is_dirty(true);
	}
	selected_ = selected;
	set_label(values_[selected_]);
}
} // namespace gui2
