/* $Id$ */
/*
   Copyright (C) 2004 - 2008 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file tools/dummy_video.cpp
//!

#include "../video.hpp"

surface CVideo::getSurface()
{
	return NULL;
}

void update_rect(const SDL_Rect&)
{
}

surface display_format_alpha(surface)
{
	return NULL;
}

surface get_video_surface()
{
	return NULL;
}

