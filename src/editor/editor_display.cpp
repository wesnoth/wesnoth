/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file editor/editor_display.cpp 
//! Draw the screen for the map-editor.

#include "global.hpp"

#include "editor_display.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "theme.hpp"
#include "tooltips.hpp"
#include "util.hpp"

#include "SDL_image.h"

#include <cassert>

editor_display::editor_display(CVideo& video, const gamemap& map,
		const config& theme_cfg, const config& cfg,
		const config& level) :
	display(video, map, theme_cfg, cfg, level)
{
    clear_screen();
}

void editor_display::pre_draw()
{
	//!@todo FIXME: this should be done from the preferences dialog as well,
	//! to reflect changes for the user before closing it
	this->update_light_levels();
	image::set_colour_adjustment(lr_, lg_, lb_);
}

image::TYPE editor_display::get_image_type(const gamemap::location& loc)
{
    if(loc == mouseoverHex_ && map_.on_board_with_border(mouseoverHex_)) {
        return image::BRIGHTENED;
    } else if (highlighted_locations_.find(loc) != highlighted_locations_.end()) {
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
		const t_translation::t_terrain terrain = map_.get_terrain(mouseoverHex_);
		const t_translation::t_list& underlying = map_.underlying_union_terrain(terrain);

		std::stringstream str;
		str << map_.get_terrain_info(terrain).name();
		if(underlying.size() != 1 || underlying.front() != terrain) {
			str << " (";

			for(t_translation::t_list::const_iterator i =
					underlying.begin(); i != underlying.end(); ++i) {

			str << map_.get_terrain_info(*i).name();
				if(i+1 != underlying.end()) {
					str << ",";
				}
			}
			str << ")";
		}
		refresh_report(reports::TERRAIN, reports::report(str.str()));

		std::stringstream str2;
		str2 << mouseoverHex_;
		refresh_report(reports::POSITION, reports::report(str2.str()));
	}
	refresh_report(reports::VILLAGES, reports::report(lexical_cast<std::string>(map_.villages().size())));
}

void editor_display::update_light_levels(void)
{
	this->lr_ = preferences::editor_r();
	this->lg_ = preferences::editor_g();
	this->lb_ = preferences::editor_b();
}
