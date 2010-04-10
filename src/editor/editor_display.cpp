/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor_display.hpp"
#include "builder.hpp"
#include "resources.hpp"

namespace editor {

editor_display::editor_display(CVideo& video, editor_map* map,
		const config& theme_cfg, const config& level)
	: game_display(&map->get_units(), video, map, &map->get_tod_manager(), &map->get_teams(), theme_cfg, level)
	, brush_locations_()
	, toolbar_hint_()
{
	clear_screen();
}

void editor_display::add_brush_loc(const map_location& hex)
{
	brush_locations_.insert(hex);
	invalidate(hex);
}

void editor_display::set_brush_locs(const std::set<map_location>& hexes)
{
	invalidate(brush_locations_);
	brush_locations_ = hexes;
	invalidate(brush_locations_);
}

void editor_display::clear_brush_locs()
{
	invalidate(brush_locations_);
	brush_locations_.clear();
}

void editor_display::remove_brush_loc(const map_location& hex)
{
	brush_locations_.erase(hex);
	invalidate(hex);
}

void editor_display::rebuild_terrain(const map_location &loc) {
	builder_->rebuild_terrain(loc);
}

//TODO clean up.
//void editor_display::pre_draw()
//{
//}

void editor_display::change_map(editor_map* m)
{
	map_ = m;
	teams_ = &m->get_teams();
	units_ = &m->get_units();
	tod_manager_ = &m->get_tod_manager();

	builder_->change_map(m);

	resources::game_map = map_;
	resources::teams = &m->get_teams();
	resources::tod_manager = &m->get_tod_manager();
	resources::units = &m->get_units();
	resources::state_of_game = &m->get_state();

	load_flags();
	set_playing_team(0);
	set_team(0, true);
}



image::TYPE editor_display::get_image_type(const map_location& loc)
{
	if (brush_locations_.find(loc) != brush_locations_.end()) {
		return image::BRIGHTENED;
	} else if (map().in_selection(loc)) {
		return image::SEMI_BRIGHTENED;
	}
	return image::SCALED_TO_HEX;
}

//void editor_display::draw_hex(const map_location& loc)
//{
//	int xpos = get_location_x(loc);
//	int ypos = get_location_y(loc);
//	tblit blit(xpos, ypos);
//	display::draw_hex(loc);
//	if (map().on_board_with_border(loc)) {
//		if (map().in_selection(loc)) {
//			drawing_buffer_add(LAYER_FOG_SHROUD, loc, tblit(xpos, ypos,
//				image::get_image("editor/selection-overlay.png", image::SCALED_TO_HEX)));
//		}
//	}
//}

const SDL_Rect& editor_display::get_clip_rect()
{
	return map_outside_area();
}

void editor_display::draw_sidebar()
{
	// Fill in the terrain report
	if(get_map().on_board_with_border(mouseoverHex_)) {
		refresh_report(reports::TERRAIN, reports::report(get_map().get_terrain_string(mouseoverHex_)));
		refresh_report(reports::POSITION, reports::report(lexical_cast<std::string>(mouseoverHex_)));
	}
//TODO cleanup.
//	const team_data data = calculate_team_data(current_team, current_side, units_);
//		if (current_side != playing_side)
//			str << span_color(font::GRAY_COLOUR);
//		str << data.villages << '/';
//		if (current_team.uses_shroud()) {
//			int unshrouded_villages = 0;
//			std::vector<map_location>::const_iterator i = map.villages().begin();
//			for (; i != map.villages().end(); ++i) {
//				if (!current_team.shrouded(*i))
//					++unshrouded_villages;
//			}
//			str << unshrouded_villages;
//		} else {
//			str << map.villages().size();
//		}
//		if (current_side != playing_side)
//			str << naps;

	std::ostringstream village_report;

	village_report << lexical_cast<std::string>(map().get_teams()[viewing_side() -1].villages().size())
				<< "/" << lexical_cast<std::string>(get_map().villages().size());

	refresh_report(reports::VILLAGES, reports::report(village_report.str()));

	std::ostringstream unit_report;

	unit_report << lexical_cast<std::string>(side_units(map().get_units(), viewing_side()))
	    << "/" << lexical_cast<std::string>(map().get_units().size());

//TODO cleanup.
//	case NUM_UNITS: {
//		if (current_side != playing_side)
//			str << span_color(font::GRAY_COLOUR);
//		str << side_units(units, current_side);
//		if (current_side != playing_side)
//			str << naps;
//		break;
	refresh_report(reports::NUM_UNITS, reports::report(unit_report.str()));
	refresh_report(reports::EDITOR_TOOL_HINT, reports::report(toolbar_hint_));
}

} //end namespace editor
