/*
   Copyright (C) 2010 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/progress_bar.hpp"

#include "gui/auxiliary/widget_definition/progress_bar.hpp"
#include "gui/auxiliary/window_builder/progress_bar.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

REGISTER_WIDGET(progress_bar)

void tprogress_bar::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tprogress_bar::get_active() const
{
	return true;
}

unsigned tprogress_bar::get_state() const
{
	return ENABLED;
}

void tprogress_bar::set_percentage(unsigned percentage)
{
	percentage = std::min<unsigned>(percentage, 100);

	if(percentage_ != percentage) {
		percentage_ = percentage;

		FOREACH(AUTO & c, canvas())
		{
			c.set_variable("percentage", variant(percentage));
		}

		set_is_dirty(true);
	}
}

bool tprogress_bar::disable_click_dismiss() const
{
	return false;
}

const std::string& tprogress_bar::get_control_type() const
{
	static const std::string type = "progress_bar";
	return type;
}

} // namespace gui2
