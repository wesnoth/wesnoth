/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/label.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/label.hpp"
#include "gui/auxiliary/window_builder/label.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "desktop/open.hpp"
#include "gettext.hpp"

#include <boost/bind.hpp>
#include <string>
#include <sstream>

namespace gui2
{

REGISTER_WIDGET(label)

tlabel::tlabel()
		: tcontrol(COUNT)
		, state_(ENABLED)
		, can_wrap_(false)
		, characters_per_line_(0)
{
	connect_signal<event::LEFT_BUTTON_CLICK>(boost::bind(&tlabel::signal_handler_left_button_click, this, _2, _3));
}

bool tlabel::can_wrap() const
{
	return can_wrap_ || characters_per_line_ != 0;
}

unsigned tlabel::get_characters_per_line() const
{
	return characters_per_line_;
}

void tlabel::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool tlabel::get_active() const
{
	return state_ != DISABLED;
}

unsigned tlabel::get_state() const
{
	return state_;
}

bool tlabel::disable_click_dismiss() const
{
	return false;
}

void tlabel::set_characters_per_line(const unsigned characters_per_line)
{
	characters_per_line_ = characters_per_line;
}

void tlabel::set_state(const tstate state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

const std::string& tlabel::get_control_type() const
{
	static const std::string type = "label";
	return type;
}

static bool looks_like_url(const std::string & str)
{
	return (str.substr(0,7) == "http://") || (str.substr(0,8) == "https://");
}

void tlabel::signal_handler_left_button_click(const event::tevent /* event */, bool & handled)
{
	DBG_GUI_E << "label click" << std::endl;

	get_window()->mouse_capture();

	tpoint mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	std::string delim = " \n\r\t";

	std::string token = get_label_token(mouse, delim.c_str());

	if (token.length() == 0) {
		return ; // without marking event as "handled"
	}

	DBG_GUI_E << "Clicked Token:\"" << token << "\"\n";

	bool url = looks_like_url(token);

	DBG_GUI_E << ( url ? "Looks like a url" : "Doesn't look like a url") << std::endl;

	if ( url ) {
		const int res = gui2::show_message(get_window()->video(), "", _("Do you want to open this link?") + std::string("\n") + token, gui2::tmessage::yes_no_buttons);
		if(res != gui2::twindow::CANCEL) {
			desktop::open_object(token);
		}
	}

	handled = true;
}

} // namespace gui2
