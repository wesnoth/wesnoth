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

#include "gui/widgets/panel.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/panel.hpp"
#include "gui/auxiliary/window_builder/panel.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

REGISTER_WIDGET(panel)

SDL_Rect tpanel::get_client_rect() const
{
	boost::intrusive_ptr<const tpanel_definition::tresolution> conf
			= boost::dynamic_pointer_cast<const tpanel_definition::tresolution>(
					config());
	assert(conf);

	SDL_Rect result = get_rectangle();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

bool tpanel::get_active() const
{
	return true;
}

unsigned tpanel::get_state() const
{
	return 0;
}

void tpanel::impl_draw_background(surface& frame_buffer, int x_offset, int y_offset)
{
	DBG_GUI_D << LOG_HEADER << " size " << get_rectangle() << ".\n";

	canvas(0).blit(frame_buffer,
				   calculate_blitting_rectangle(x_offset, y_offset));
}

void tpanel::impl_draw_foreground(surface& frame_buffer, int x_offset, int y_offset)
{
	canvas(1).blit(frame_buffer,
				   calculate_blitting_rectangle(x_offset, y_offset));
}

tpoint tpanel::border_space() const
{
	boost::intrusive_ptr<const tpanel_definition::tresolution> conf
			= boost::dynamic_pointer_cast<const tpanel_definition::tresolution>(
					config());
	assert(conf);

	return tpoint(conf->left_border + conf->right_border,
				  conf->top_border + conf->bottom_border);
}

const std::string& tpanel::get_control_type() const
{
	static const std::string type = "panel";
	return type;
}

void tpanel::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

} // namespace gui2
