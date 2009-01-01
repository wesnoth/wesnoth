/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/panel.hpp"


namespace gui2 {

SDL_Rect tpanel::get_client_rect() const
{
	boost::intrusive_ptr<const tpanel_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const tpanel_definition::tresolution>(config());
	assert(conf);

	SDL_Rect result = get_rect();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

void tpanel::impl_draw_background(surface& frame_buffer)
{
   	canvas(0).draw();
	SDL_Rect rect = get_screen_rect();
	SDL_BlitSurface(canvas(0).surf(), NULL, frame_buffer, &rect);
}

void tpanel::impl_draw_foreground(surface& frame_buffer)
{
   	canvas(1).draw();
	SDL_Rect rect = get_screen_rect();
	SDL_BlitSurface(canvas(1).surf(), NULL, frame_buffer, &rect);
}

tpoint tpanel::border_space() const
{
	boost::intrusive_ptr<const tpanel_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const tpanel_definition::tresolution>(config());
	assert(conf);

	return tpoint(conf->left_border + conf->right_border,
		conf->top_border + conf->bottom_border);
}

} // namespace gui2

