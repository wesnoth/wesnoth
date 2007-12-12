/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
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

#include "display.hpp"
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
	// Clear the screen contents
	surface const disp(screen_.getSurface());
	SDL_Rect area = screen_area();
	SDL_FillRect(disp,&area,SDL_MapRGB(disp->format,0,0,0));
}

void editor_display::draw(bool update,bool force)
{
	bool changed = display::draw_init();

	//int simulate_delay = 0;
	if(!map_.empty() && !invalidated_.empty()) {
		changed = true;

		SDL_Rect clip_rect = map_outside_area();
		surface const dst(screen_.getSurface());
		clip_rect_setter set_clip_rect(dst, clip_rect);

		std::set<gamemap::location>::const_iterator it;
		for(it = invalidated_.begin(); it != invalidated_.end(); ++it) {
			image::TYPE image_type = image::SCALED_TO_HEX;

			if(*it == mouseoverHex_ && map_.on_board(mouseoverHex_, true)) {
				image_type = image::BRIGHTENED;
			}
			else if (highlighted_locations_.find(*it) != highlighted_locations_.end()) {
				image_type = image::SEMI_BRIGHTENED;
			}

			if(screen_.update_locked()) {
				continue;
			}

			int xpos = int(get_location_x(*it));
			int ypos = int(get_location_y(*it));

			if(xpos >= clip_rect.x + clip_rect.w || ypos >= clip_rect.y + clip_rect.h ||
			   xpos + zoom_ < clip_rect.x || ypos + zoom_ < clip_rect.y) {
				continue;
			}

			tile_stack_clear();

			const std::string nodarken = "morning";
			tile_stack_append(get_terrain_images(*it,nodarken,image_type,ADJACENT_BACKGROUND));
			tile_stack_append(get_terrain_images(*it,nodarken,image_type,ADJACENT_FOREGROUND));

			// Draw the grid, if it has been enabled
			if(grid_ && map_.on_board(*it)) {
				tile_stack_append(image::get_image(game_config::grid_image, image::SCALED_TO_HEX));
			}

			// Paint selection and mouseover overlays
			if(*it == selectedHex_ && map_.on_board(selectedHex_, true) && selected_hex_overlay_ != NULL)
				tile_stack_append(selected_hex_overlay_);
			if(*it == mouseoverHex_ && map_.on_board(mouseoverHex_, true) && mouseover_hex_overlay_ != NULL)
				tile_stack_append(mouseover_hex_overlay_);

			tile_stack_render(xpos, ypos);
		}

		invalidated_.clear();
	} else if (!map_.empty()) {
		assert(invalidated_.empty());
	}

	// Fill in the terrain report
	if(map_.on_board(mouseoverHex_, true)) {
		const t_translation::t_letter terrain = map_.get_terrain(mouseoverHex_);
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

	{
	  std::stringstream str3;
	  str3 << map_.villages().size();
	  refresh_report(reports::VILLAGES, reports::report(str3.str()));
	}

	display::draw_wrap(update, force, changed);
}


