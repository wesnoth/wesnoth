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

#include "editor_display.hpp"
#include "editor_common.hpp"
#include <cassert>

namespace editor2 {
	
editor_display::editor_display(CVideo& video, const editor_map& map,
		const config& theme_cfg, const config& cfg,
		const config& level) :
	display(video, map, theme_cfg, cfg, level)
{
    clear_screen();
}

void editor_display::pre_draw()
{
}

image::TYPE editor_display::get_image_type(const gamemap::location& loc)
{
    if(loc == mouseoverHex_ && map_.on_board_with_border(mouseoverHex_)) {
        return image::BRIGHTENED;
    } else if (highlighted_locations_.find(loc) != highlighted_locations_.end()) {
        return image::SEMI_BRIGHTENED;
    } else if (map().in_selection(loc)) {
		return image::SEMI_BRIGHTENED;
	}
    return image::SCALED_TO_HEX;
}

const SDL_Rect& editor_display::get_clip_rect()
{
    return map_outside_area();
}

void editor_display::draw_sidebar()
{
	// Fill in the terrain report
	if(map_.on_board_with_border(mouseoverHex_)) {
		refresh_report(reports::TERRAIN, reports::report(map_.get_terrain_string(mouseoverHex_)));
		refresh_report(reports::POSITION, reports::report(lexical_cast<std::string>(mouseoverHex_)));
	}
	refresh_report(reports::VILLAGES, reports::report(lexical_cast<std::string>(map_.villages().size())));
}


} //end namespace editor2
