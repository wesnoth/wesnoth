/* $Id$ */
/*
   copyright (C) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/toggle_panel.hpp"

#include "foreach.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

void ttoggle_panel::set_child_members(const std::map<std::string /* widget id */, string_map>& data)
{
	// typedef boost problem work around.
	typedef std::pair<std::string, string_map> hack ;
	foreach(const hack& item, data) {
		tcontrol* control = dynamic_cast<tcontrol*>(find_widget(item.first, false));
		if(control) {
			control->set_members(item.second);
		}
	}
}

void ttoggle_panel::mouse_enter(tevent_handler&)
{
	DBG_G_E << "Toggle panel: mouse enter.\n";

	if(get_value()) {
		set_state(FOCUSSED_SELECTED);
	} else {
		set_state(FOCUSSED);
	}
}

void ttoggle_panel::mouse_leave(tevent_handler&)
{
	DBG_G_E << "Toggle panel: mouse leave.\n";

	if(get_value()) {
		set_state(ENABLED_SELECTED);
	} else {
		set_state(ENABLED);
	}
}

void ttoggle_panel::mouse_left_button_click(tevent_handler&)
{
	DBG_G_E << "Toggle panel: left mouse button click.\n";

	if(get_value()) {
		set_state(ENABLED);
	} else {
		set_state(ENABLED_SELECTED);
	}

	if(callback_state_change_) {
		callback_state_change_(this);
	}
}

void ttoggle_panel::mouse_left_button_double_click(tevent_handler&)
{
	DBG_G_E << "Toggle panel: left mouse button double click.\n";

	assert(retval_ != 0);

	twindow* window = get_window();
	assert(window);

	window->set_retval(retval_);
}

void ttoggle_panel::set_active(const bool active)
{
	if(active) {
		if(get_value()) {
			set_state(ENABLED_SELECTED);
		} else {
			set_state(ENABLED);
		}
	} else {
		if(get_value()) {
			set_state(DISABLED_SELECTED);
		} else {
			set_state(DISABLED);
		}
	}
}

SDL_Rect ttoggle_panel::get_client_rect() const
{
	boost::intrusive_ptr<const ttoggle_panel_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const ttoggle_panel_definition::tresolution>(config());
	assert(conf);

	SDL_Rect result = get_rect();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

tpoint ttoggle_panel::border_space() const
{
	boost::intrusive_ptr<const ttoggle_panel_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const ttoggle_panel_definition::tresolution>(config());
	assert(conf);

	return tpoint(conf->left_border + conf->right_border,
		conf->top_border + conf->bottom_border);
}

void ttoggle_panel::set_value(const bool selected)
{
	if(selected == get_value()) {
		return;
	}

	if(selected) {
		set_state(static_cast<tstate>(state_ + ENABLED_SELECTED));
	} else {
		set_state(static_cast<tstate>(state_ - ENABLED_SELECTED));
	}
}

void ttoggle_panel::set_retval(const int retval)
{
	if(retval == retval_) {
		return;
	}

	retval_ = retval;
	set_wants_mouse_left_double_click(retval_ != 0);
}

void ttoggle_panel::set_state(const tstate state)
{
	if(state == state_) {
		return;
	}

	state_ = state;
	set_dirty(true);

	boost::intrusive_ptr<const ttoggle_panel_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const ttoggle_panel_definition::tresolution>(config());
	assert(conf);
}

} // namespace gui2


