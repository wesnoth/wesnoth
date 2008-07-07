/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR2_EDITOR_DISPLAY_HPP_INCLUDED
#define EDITOR2_EDITOR_DISPLAY_HPP_INCLUDED

#include "editor_map.hpp"
#include "../display.hpp"

namespace editor2 {
	
class editor_display : public display
{
public:
	editor_display(CVideo& video, const editor_map& map, const config& theme_cfg,
			const config& cfg, const config& level);

	bool in_editor() const { return true; }

protected:
	const editor_map& map() { return static_cast<const editor_map&>(map_); }
	void pre_draw();
	/**
	* The editor uses different rules for terrain highligting (e.g. selections)
	*/
	image::TYPE get_image_type(const gamemap::location& loc);
	const SDL_Rect& get_clip_rect();
	void draw_sidebar();
};

} //end namespace editor2
#endif
