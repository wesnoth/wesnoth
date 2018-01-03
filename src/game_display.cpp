/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * During a game, show map & info-panels at top+right.
 */

#include "game_display.hpp"

#include "gettext.hpp"
#include "wesconfig.h"

#include "cursor.hpp"
#include "display_chat_manager.hpp"
#include "fake_unit_manager.hpp"
#include "fake_unit_ptr.hpp"
#include "floating_label.hpp"
#include "game_board.hpp"
#include "preferences/game.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/label.hpp"
#include "font/standard_colors.hpp"
#include "reports.hpp"
#include "resources.hpp"
#include "tod_manager.hpp"
#include "color.hpp"
#include "sound.hpp"
#include "synced_context.hpp"
#include "terrain/type_data.hpp"
#include "units/unit.hpp"
#include "units/drawer.hpp"
#include "whiteboard/manager.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

std::map<map_location,fixed_t> game_display::debugHighlights_;

/**
 * Function to return 2 half-hex footsteps images for the given location.
 * Only loc is on the current route set by set_route.
 *
 * This function is only used internally by game_display so I have moved it out of the header into the compilaton unit.
 */
std::vector<surface> footsteps_images(const map_location& loc, const pathfind::marked_route & route_, const display_context * dc_);

game_display::game_display(game_board& board, std::weak_ptr<wb::manager> wb,
		reports & reports_object,
		const tod_manager& tod,
		const config& theme_cfg,
		const config& level,
		bool) :
		display(&board, wb, reports_object, theme_cfg, level, false),
		overlay_map_(),
		attack_indicator_src_(),
		attack_indicator_dst_(),
		route_(),
		tod_manager_(&tod),
		displayedUnitHex_(),
		sidebarScaling_(1.0),
		first_turn_(true),
		in_game_(false),
		chat_man_(new display_chat_manager(*this)),
		mode_(RUNNING),
		needs_rebuild_(false)
{
	replace_overlay_map(&overlay_map_);
	video().clear_screen();
}

game_display::~game_display()
{
	try {
	// SDL_FreeSurface(minimap_);
	chat_man_->prune_chat_messages(true);
	} catch (...) {}
}

void game_display::new_turn()
{
	const time_of_day& tod = tod_manager_->get_time_of_day();

	if( !first_turn_) {
		const time_of_day& old_tod = tod_manager_->get_previous_time_of_day();

		if(old_tod.image_mask != tod.image_mask) {
			surface old_mask(image::get_image(old_tod.image_mask,image::SCALED_TO_HEX));
			surface new_mask(image::get_image(tod.image_mask,image::SCALED_TO_HEX));

			const int niterations = static_cast<int>(10/turbo_speed());
			const int frame_time = 30;
			const int starting_ticks = SDL_GetTicks();
			for(int i = 0; i != niterations; ++i) {

				if(old_mask != nullptr) {
					const fixed_t proportion = ftofxp(1.0) - fxpdiv(i,niterations);
					adjust_surface_alpha(old_mask, proportion);
					tod_hex_mask1.assign(old_mask);
				}

				if(new_mask != nullptr) {
					const fixed_t proportion = fxpdiv(i,niterations);
					adjust_surface_alpha(new_mask, proportion);
					tod_hex_mask2.assign(new_mask);
				}

				invalidate_all();

				const int cur_ticks = SDL_GetTicks();
				const int wanted_ticks = starting_ticks + i*frame_time;
				if(cur_ticks < wanted_ticks) {
					SDL_Delay(wanted_ticks - cur_ticks);
				}
			}
		}

		tod_hex_mask1.assign(nullptr);
		tod_hex_mask2.assign(nullptr);
	}

	first_turn_ = false;

	display::update_tod();

	invalidate_all();
}

void game_display::select_hex(map_location hex)
{
	if(hex.valid() && fogged(hex)) {
		return;
	}
	display::select_hex(hex);

	display_unit_hex(hex);
}

void game_display::highlight_hex(map_location hex)
{
	wb::future_map_if future(!synced_context::is_synced()); /**< Lasts for whole method. */

	const unit *u = resources::gameboard->get_visible_unit(hex, dc_->teams()[viewing_team()], !dont_show_all_);
	if (u) {
		displayedUnitHex_ = hex;
		invalidate_unit();
	} else {
		u = resources::gameboard->get_visible_unit(mouseoverHex_, dc_->teams()[viewing_team()], !dont_show_all_);
		if (u) {
			// mouse moved from unit hex to non-unit hex
			if (dc_->units().count(selectedHex_)) {
				displayedUnitHex_ = selectedHex_;
				invalidate_unit();
			}
		}
	}

	display::highlight_hex(hex);
	invalidate_game_status();
}


void game_display::display_unit_hex(map_location hex)
{
	if (!hex.valid())
		return;

	wb::future_map_if future(!synced_context::is_synced()); /**< Lasts for whole method. */

	const unit *u = resources::gameboard->get_visible_unit(hex, dc_->teams()[viewing_team()], !dont_show_all_);
	if (u) {
		displayedUnitHex_ = hex;
		invalidate_unit();
	}
}

void game_display::invalidate_unit_after_move(const map_location& src, const map_location& dst)
{
	if (src == displayedUnitHex_) {
		displayedUnitHex_ = dst;
		invalidate_unit();
	}
}

void game_display::scroll_to_leader(int side, SCROLL_TYPE scroll_type,bool force)
{
	unit_map::const_iterator leader = dc_->units().find_leader(side);

	if(leader.valid()) {
		scroll_to_tile(leader->get_location(), scroll_type, true, force);
	}
}

void game_display::pre_draw() {
	if (std::shared_ptr<wb::manager> w = wb_.lock()) {
		w->pre_draw();
	}
	process_reachmap_changes();
	/**
	 * @todo FIXME: must modify changed, but best to do it at the
	 * floating_label level
	 */
	chat_man_->prune_chat_messages();
}


void game_display::post_draw() {
	if (std::shared_ptr<wb::manager> w = wb_.lock()) {
		w->post_draw();
	}
}

void game_display::draw_invalidated()
{
	halo_man_->unrender(invalidated_);
	display::draw_invalidated();
	if (fake_unit_man_->empty()) {
		return;
	}
	unit_drawer drawer = unit_drawer(*this);

	for (const unit* temp_unit : *fake_unit_man_) {
		const map_location& loc = temp_unit->get_location();
		exclusive_unit_draw_requests_t::iterator request = exclusive_unit_draw_requests_.find(loc);
		if (invalidated_.find(loc) != invalidated_.end()
				&& (request == exclusive_unit_draw_requests_.end() || request->second == temp_unit->id()))
			drawer.redraw_unit(*temp_unit);
	}
}

void game_display::post_commit()
{
	halo_man_->render();
}

void game_display::draw_hex(const map_location& loc)
{
	const bool on_map = get_map().on_board(loc);
	const bool is_shrouded = shrouded(loc);
//	const bool is_fogged = fogged(loc);
	const int xpos = get_location_x(loc);
	const int ypos = get_location_y(loc);

//	image::TYPE image_type = get_image_type(loc);

	display::draw_hex(loc);

	if(cursor::get() == cursor::WAIT) {
		// Interaction is disabled, so we don't need anything else
		return;
	}

	if(on_map && loc == mouseoverHex_) {
		drawing_layer hex_top_layer = LAYER_MOUSEOVER_BOTTOM;
		const unit *u = resources::gameboard->get_visible_unit(loc, dc_->teams()[viewing_team()] );
		if( u != nullptr ) {
			hex_top_layer = LAYER_MOUSEOVER_TOP;
		}
		if(u == nullptr) {
			drawing_buffer_add( hex_top_layer, loc, xpos, ypos,
					image::get_image("misc/hover-hex-top.png~RC(magenta>gold)", image::SCALED_TO_HEX));
			drawing_buffer_add(LAYER_MOUSEOVER_BOTTOM, loc, xpos, ypos,
					image::get_image("misc/hover-hex-bottom.png~RC(magenta>gold)", image::SCALED_TO_HEX));
		} else if(dc_->teams()[currentTeam_].is_enemy(u->side())) {
			drawing_buffer_add( hex_top_layer, loc, xpos, ypos,
					image::get_image("misc/hover-hex-enemy-top.png~RC(magenta>red)", image::SCALED_TO_HEX));
			drawing_buffer_add(LAYER_MOUSEOVER_BOTTOM, loc, xpos, ypos,
					image::get_image("misc/hover-hex-enemy-bottom.png~RC(magenta>red)", image::SCALED_TO_HEX));
		} else if(dc_->teams()[currentTeam_].side() == u->side()) {
			drawing_buffer_add( hex_top_layer, loc, xpos, ypos,
					image::get_image("misc/hover-hex-top.png~RC(magenta>green)", image::SCALED_TO_HEX));
			drawing_buffer_add(LAYER_MOUSEOVER_BOTTOM, loc, xpos, ypos,
					image::get_image("misc/hover-hex-bottom.png~RC(magenta>green)", image::SCALED_TO_HEX));
		} else {
			drawing_buffer_add( hex_top_layer, loc, xpos, ypos,
					image::get_image("misc/hover-hex-top.png~RC(magenta>lightblue)", image::SCALED_TO_HEX));
			drawing_buffer_add(LAYER_MOUSEOVER_BOTTOM, loc, xpos, ypos,
					image::get_image("misc/hover-hex-bottom.png~RC(magenta>lightblue)", image::SCALED_TO_HEX));
		}
	}



	// Draw reach_map information.
	// We remove the reachability mask of the unit
	// that we want to attack.
	if (!is_shrouded && !reach_map_.empty()
			&& reach_map_.find(loc) == reach_map_.end() && loc != attack_indicator_dst_) {
		static const image::locator unreachable(game_config::images::unreachable);
		drawing_buffer_add(LAYER_REACHMAP, loc, xpos, ypos,
				image::get_image(unreachable,image::SCALED_TO_HEX));
	}

	if (std::shared_ptr<wb::manager> w = wb_.lock()) {
		w->draw_hex(loc);

		if (!(w->is_active() && w->has_temp_move()))
		{
			std::vector<surface> footstepImages = footsteps_images(loc, route_, dc_);
			if (!footstepImages.empty()) {
				drawing_buffer_add(LAYER_FOOTSTEPS, loc, xpos, ypos, footsteps_images(loc, route_, dc_));
			}
		}
	}
	// Draw the attack direction indicator
	if(on_map && loc == attack_indicator_src_) {
		drawing_buffer_add(LAYER_ATTACK_INDICATOR, loc, xpos, ypos,
			image::get_image("misc/attack-indicator-src-" + attack_indicator_direction() + ".png", image::SCALED_TO_HEX));
	} else if (on_map && loc == attack_indicator_dst_) {
		drawing_buffer_add(LAYER_ATTACK_INDICATOR, loc, xpos, ypos,
			image::get_image("misc/attack-indicator-dst-" + attack_indicator_direction() + ".png", image::SCALED_TO_HEX));
	}

	// Linger overlay unconditionally otherwise it might give glitches
	// so it's drawn over the shroud and fog.
	if(mode_ != RUNNING) {
		static const image::locator linger(game_config::images::linger);
		drawing_buffer_add(LAYER_LINGER_OVERLAY, loc, xpos, ypos,
			image::get_image(linger, image::TOD_COLORED));
	}

	if(on_map && loc == selectedHex_ && !game_config::images::selected.empty()) {
		static const image::locator selected(game_config::images::selected);
		drawing_buffer_add(LAYER_SELECTED_HEX, loc, xpos, ypos,
				image::get_image(selected, image::SCALED_TO_HEX));
	}

	// Show def% and turn to reach info
	if(!is_shrouded && on_map) {
		draw_movement_info(loc);
	}

	if(game_config::debug) {
		int debugH = debugHighlights_[loc];
		if (debugH) {
			std::string txt = std::to_string(debugH);
			draw_text_in_hex(loc, LAYER_MOVE_INFO, txt, 18, font::BAD_COLOR);
		}
	}
	//simulate_delay += 1;
}

const time_of_day& game_display::get_time_of_day(const map_location& loc) const
{
	return tod_manager_->get_time_of_day(loc);
}

bool game_display::has_time_area() const
{
	return tod_manager_->has_time_area();
}

void game_display::draw_sidebar()
{
	if ( !team_valid() )
		return;

	refresh_report("report_clock");
	refresh_report("report_countdown");

	if (invalidateGameStatus_)
	{
		wb::future_map future; // start planned unit map scope

		// We display the unit the mouse is over if it is over a unit,
		// otherwise we display the unit that is selected.
		for (const std::string &name : reports_object_->report_list()) {
			refresh_report(name);
		}
		invalidateGameStatus_ = false;
	}
}


void game_display::set_game_mode(const game_mode mode)
{
	if(mode != mode_) {
		mode_ = mode;
		invalidate_all();
	}
}

void game_display::draw_movement_info(const map_location& loc)
{
	// Search if there is a mark here
	pathfind::marked_route::mark_map::iterator w = route_.marks.find(loc);

	std::shared_ptr<wb::manager> wb = wb_.lock();

	// Don't use empty route or the first step (the unit will be there)
	if(w != route_.marks.end()
				&& !route_.steps.empty() && route_.steps.front() != loc) {
		const unit_map::const_iterator un =
				(wb && wb->get_temp_move_unit().valid()) ?
						wb->get_temp_move_unit() : dc_->units().find(route_.steps.front());
		if(un != dc_->units().end()) {
			// Display the def% of this terrain
			int move_cost = un->movement_cost(get_map().get_terrain(loc));
			int def = (move_cost == movetype::UNREACHABLE ?
						0 : 100 - un->defense_modifier(get_map().get_terrain(loc)));
			std::stringstream def_text;
			def_text << def << "%";

			color_t color = game_config::red_to_green(def, false);

			// simple mark (no turn point) use smaller font
			int def_font = w->second.turns > 0 ? 18 : 16;
			draw_text_in_hex(loc, LAYER_MOVE_INFO, def_text.str(), def_font, color);

			int xpos = get_location_x(loc);
			int ypos = get_location_y(loc);
			if (w->second.invisible) {
				drawing_buffer_add(LAYER_MOVE_INFO, loc, xpos, ypos,
					image::get_image("misc/hidden.png", image::SCALED_TO_HEX));
			}

			if (w->second.zoc) {
				drawing_buffer_add(LAYER_MOVE_INFO, loc, xpos, ypos,
					image::get_image("misc/zoc.png", image::SCALED_TO_HEX));
			}

			if (w->second.capture) {
				drawing_buffer_add(LAYER_MOVE_INFO, loc, xpos, ypos,
					image::get_image("misc/capture.png", image::SCALED_TO_HEX));
			}

			//we display turn info only if different from a simple last "1"
			if (w->second.turns > 1 || (w->second.turns == 1 && loc != route_.steps.back())) {
				std::stringstream turns_text;
				turns_text << w->second.turns;
				draw_text_in_hex(loc, LAYER_MOVE_INFO, turns_text.str(), 17, font::NORMAL_COLOR, 0.5,0.8);
			}

			// The hex is full now, so skip the "show enemy moves"
			return;
		}
	}
	// When out-of-turn, it's still interesting to check out the terrain defs of the selected unit
	else if (selectedHex_.valid() && loc == mouseoverHex_)
	{
		const unit_map::const_iterator selectedUnit = resources::gameboard->find_visible_unit(selectedHex_,dc_->teams()[currentTeam_]);
		const unit_map::const_iterator mouseoveredUnit = resources::gameboard->find_visible_unit(mouseoverHex_,dc_->teams()[currentTeam_]);
		if(selectedUnit != dc_->units().end() && mouseoveredUnit == dc_->units().end()) {
			// Display the def% of this terrain
			int move_cost = selectedUnit->movement_cost(get_map().get_terrain(loc));
			int def = (move_cost == movetype::UNREACHABLE ?
						0 : 100 - selectedUnit->defense_modifier(get_map().get_terrain(loc)));
			std::stringstream def_text;
			def_text << def << "%";

			color_t color = game_config::red_to_green(def, false);

			// use small font
			int def_font = 16;
			draw_text_in_hex(loc, LAYER_MOVE_INFO, def_text.str(), def_font, color);
		}
	}

	if (!reach_map_.empty()) {
		reach_map::iterator reach = reach_map_.find(loc);
		if (reach != reach_map_.end() && reach->second > 1) {
			const std::string num = std::to_string(reach->second);
			draw_text_in_hex(loc, LAYER_MOVE_INFO, num, 16, font::YELLOW_COLOR);
		}
	}
}

std::vector<surface> footsteps_images(const map_location& loc, const pathfind::marked_route & route_, const display_context * dc_)
{
	std::vector<surface> res;

	if (route_.steps.size() < 2) {
		return res; // no real "route"
	}

	std::vector<map_location>::const_iterator i =
	         std::find(route_.steps.begin(),route_.steps.end(),loc);

	if( i == route_.steps.end()) {
		return res; // not on the route
	}

	// Check which footsteps images of game_config we will use
	int move_cost = 1;
	const unit_map::const_iterator u = dc_->units().find(route_.steps.front());
	if(u != dc_->units().end()) {
		move_cost = u->movement_cost(dc_->map().get_terrain(loc));
	}
	int image_number = std::min<int>(move_cost, game_config::foot_speed_prefix.size());
	if (image_number < 1) {
		return res; // Invalid movement cost or no images
	}
	const std::string foot_speed_prefix = game_config::foot_speed_prefix[image_number-1];

	surface teleport = nullptr;

	// We draw 2 half-hex (with possibly different directions),
	// but skip the first for the first step.
	const int first_half = (i == route_.steps.begin()) ? 1 : 0;
	// and the second for the last step
	const int second_half = (i+1 == route_.steps.end()) ? 0 : 1;

	for (int h = first_half; h <= second_half; ++h) {
		const std::string sense( h==0 ? "-in" : "-out" );

		if (!tiles_adjacent(*(i+(h-1)), *(i+h))) {
			std::string teleport_image =
			h==0 ? game_config::foot_teleport_enter : game_config::foot_teleport_exit;
			teleport = image::get_image(teleport_image, image::SCALED_TO_HEX);
			continue;
		}

		// In function of the half, use the incoming or outgoing direction
		map_location::DIRECTION dir = (i+(h-1))->get_relative_dir(*(i+h));

		std::string rotate;
		if (dir > map_location::SOUTH_EAST) {
			// No image, take the opposite direction and do a 180 rotation
			dir = i->get_opposite_dir(dir);
			rotate = "~FL(horiz)~FL(vert)";
		}

		const std::string image = foot_speed_prefix
			+ sense + "-" + i->write_direction(dir)
			+ ".png" + rotate;

		res.push_back(image::get_image(image, image::SCALED_TO_HEX));
	}

	// we draw teleport image (if any) in last
	if (teleport != nullptr) res.push_back(teleport);

	return res;
}



void game_display::highlight_reach(const pathfind::paths &paths_list)
{
	unhighlight_reach();
	highlight_another_reach(paths_list);
}

void game_display::highlight_another_reach(const pathfind::paths &paths_list)
{
	// Fold endpoints of routes into reachability map.
	for (const pathfind::paths::step &dest : paths_list.destinations) {
		reach_map_[dest.curr]++;
	}
	reach_map_changed_ = true;
}

bool game_display::unhighlight_reach()
{
	if(!reach_map_.empty()) {
		reach_map_.clear();
		reach_map_changed_ = true;
		return true;
	} else {
		return false;
	}
}

void game_display::invalidate_route()
{
	for(std::vector<map_location>::const_iterator i = route_.steps.begin();
	    i != route_.steps.end(); ++i) {
		invalidate(*i);
	}
}

void game_display::set_route(const pathfind::marked_route *route)
{
	invalidate_route();

	if(route != nullptr) {
		route_ = *route;
	} else {
		route_.steps.clear();
		route_.marks.clear();
	}

	invalidate_route();
}

void game_display::float_label(const map_location& loc, const std::string& text, const color_t& color)
{
	if(preferences::show_floating_labels() == false || fogged(loc)) {
		return;
	}

	font::floating_label flabel(text);
	flabel.set_font_size(font::SIZE_XLARGE);
	flabel.set_color(color);
	flabel.set_position(get_location_x(loc)+zoom_/2, get_location_y(loc));
	flabel.set_move(0, -2 * turbo_speed());
	flabel.set_lifetime(60/turbo_speed());
	flabel.set_scroll_mode(font::ANCHOR_LABEL_MAP);

	font::add_floating_label(flabel);
}

int& game_display::debug_highlight(const map_location& loc)
{
	assert(game_config::debug);
	return debugHighlights_[loc];
}

void game_display::set_attack_indicator(const map_location& src, const map_location& dst)
{
	if (attack_indicator_src_ != src || attack_indicator_dst_ != dst) {
		invalidate(attack_indicator_src_);
		invalidate(attack_indicator_dst_);

		attack_indicator_src_ = src;
		attack_indicator_dst_ = dst;

		invalidate(attack_indicator_src_);
		invalidate(attack_indicator_dst_);
	}
}

void game_display::clear_attack_indicator()
{
	set_attack_indicator(map_location::null_location(), map_location::null_location());
}

std::string game_display::current_team_name() const
{
	if (team_valid())
	{
		return dc_->teams()[currentTeam_].team_name();
	}
	return std::string();
}


void game_display::set_playing_team(size_t teamindex)
{
	assert(teamindex < dc_->teams().size());
	activeTeam_ = teamindex;
	invalidate_game_status();
}

void game_display::begin_game()
{
	in_game_ = true;
	create_buttons();
	invalidate_all();
}

void game_display::needs_rebuild(bool b) {
	if (b) {
		needs_rebuild_ = true;
	}
}

bool game_display::maybe_rebuild() {
	if (needs_rebuild_) {
		needs_rebuild_ = false;
		recalculate_minimap();
		invalidate_all();
		rebuild_all();
		return true;
	}
	return false;
}
