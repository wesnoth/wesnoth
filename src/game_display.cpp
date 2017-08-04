/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "color.hpp"
#include "cursor.hpp"
#include "display_chat_manager.hpp"
#include "fake_unit_manager.hpp"
#include "fake_unit_ptr.hpp"
#include "floating_label.hpp"
#include "font/standard_colors.hpp"
#include "game_board.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "map/label.hpp"
#include "map/map.hpp"
#include "ogl/utils.hpp"
#include "preferences/game.hpp"
#include "reports.hpp"
#include "resources.hpp"
#include "sound.hpp"
#include "synced_context.hpp"
#include "terrain/type_data.hpp"
#include "tod_manager.hpp"
#include "units/drawer.hpp"
#include "units/unit.hpp"
#include "wesconfig.h"
#include "whiteboard/manager.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

std::map<map_location, fixed_t> game_display::debugHighlights_;

game_display::game_display(game_board& board, std::weak_ptr<wb::manager> wb,
		reports & reports_object,
		const config& theme_cfg,
		const config& level,
		bool)
	: display(&board, wb, reports_object, theme_cfg, level, false)
	, overlay_map_()
	, attack_indicator_src_()
	, attack_indicator_dst_()
	, hex_def_fl_labels_()
	, route_()
	, displayedUnitHex_()
	, sidebarScaling_(1.0)
	, first_turn_(true)
	, in_game_(false)
	, chat_man_(new display_chat_manager(*this))
	, mode_(RUNNING)
	, needs_rebuild_(false)
{
	replace_overlay_map(&overlay_map_);
#ifdef USE_GL_RENDERING
	gl::clear_screen();
#else
	video().clear_screen();
#endif
}

game_display::~game_display()
{
	try {
		// SDL_FreeSurface(minimap_);
		chat_man_->prune_chat_messages(true);
	} catch(...) {
	}
}

void game_display::new_turn()
{
	const time_of_day& tod = resources::tod_manager->get_time_of_day();

	if(!first_turn_) {
		const time_of_day& old_tod = resources::tod_manager->get_previous_time_of_day();

		if(old_tod.image_mask != tod.image_mask) {
			surface old_mask(image::get_image(old_tod.image_mask, image::SCALED_TO_HEX));
			surface new_mask(image::get_image(tod.image_mask, image::SCALED_TO_HEX));

			const int niterations = static_cast<int>(10 / turbo_speed());
			const int frame_time = 30;
			const int starting_ticks = SDL_GetTicks();
			for(int i = 0; i != niterations; ++i) {
				if(old_mask != nullptr) {
					const fixed_t proportion = ftofxp(1.0) - fxpdiv(i, niterations);
					adjust_surface_alpha(old_mask, proportion);
					tod_hex_mask1.assign(old_mask);
				}

				if(new_mask != nullptr) {
					const fixed_t proportion = fxpdiv(i, niterations);
					adjust_surface_alpha(new_mask, proportion);
					tod_hex_mask2.assign(new_mask);
				}


				const int cur_ticks = SDL_GetTicks();
				const int wanted_ticks = starting_ticks + i * frame_time;
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

	const unit* u = resources::gameboard->get_visible_unit(hex, dc_->teams()[viewing_team()], !dont_show_all_);
	if(u) {
		displayedUnitHex_ = hex;
		invalidate_unit();
	} else {
		u = resources::gameboard->get_visible_unit(mouseoverHex_, dc_->teams()[viewing_team()], !dont_show_all_);
		if(u) {
			// mouse moved from unit hex to non-unit hex
			if(dc_->units().count(selectedHex_)) {
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
	if(!hex.valid())
		return;

	wb::future_map_if future(!synced_context::is_synced()); /**< Lasts for whole method. */

	const unit* u = resources::gameboard->get_visible_unit(hex, dc_->teams()[viewing_team()], !dont_show_all_);
	if(u) {
		displayedUnitHex_ = hex;
		invalidate_unit();
	}
}

void game_display::invalidate_unit_after_move(const map_location& src, const map_location& dst)
{
	if(src == displayedUnitHex_) {
		displayedUnitHex_ = dst;
		invalidate_unit();
	}
}

void game_display::scroll_to_leader(int side, SCROLL_TYPE scroll_type, bool force)
{
	unit_map::const_iterator leader = dc_->units().find_leader(side);

	if(leader.valid()) {
		scroll_to_tile(leader->get_location(), scroll_type, true, force);
	}
}

void game_display::pre_draw()
{
	if(std::shared_ptr<wb::manager> w = wb_.lock()) {
		w->pre_draw();
	}

	/**
	 * @todo FIXME: must modify changed, but best to do it at the
	 * floating_label level
	 */
	chat_man_->prune_chat_messages();
}

void game_display::post_draw()
{
	if(std::shared_ptr<wb::manager> w = wb_.lock()) {
		w->post_draw();
	}
}

void game_display::draw_hex_cursor(const map_location& loc)
{
	if(!get_map().on_board(loc) || cursor::get() == cursor::WAIT) {
		return;
	}

	texture* cursor_bg = nullptr;
	texture* cursor_fg = nullptr;

	// image::SCALED_TO_HEX

	// TODO: either merge into one image or draw layered.

	// Default
	static texture default_cursor_bg(image::get_texture("misc/hover-hex-top.png~RC(magenta>gold)"));
	static texture default_cursor_fg(image::get_texture("misc/hover-hex-bottom.png~RC(magenta>gold)"));

	// Your troops
	static texture team_cursor_bg(image::get_texture("misc/hover-hex-top.png~RC(magenta>green)"));
	static texture team_cursor_fg(image::get_texture("misc/hover-hex-bottom.png~RC(magenta>green)"));

	// Allies
	static texture ally_cursor_bg(image::get_texture("misc/hover-hex-top.png~RC(magenta>lightblue)"));
	static texture ally_cursor_fg(image::get_texture("misc/hover-hex-bottom.png~RC(magenta>lightblue)"));

	// Enemies
	static texture enemy_cursor_bg(image::get_texture("misc/hover-hex-enemy-top.png~RC(magenta>red)"));
	static texture enemy_cursor_fg(image::get_texture("misc/hover-hex-enemy-bottom.png~RC(magenta>red)"));

	const team& t = dc_->teams()[currentTeam_];

	const unit* u = resources::gameboard->get_visible_unit(loc, dc_->teams()[viewing_team()]);

	if(u == nullptr) {
		cursor_bg = &default_cursor_bg;
		cursor_fg = &default_cursor_fg;
	} else if(t.is_enemy(u->side())) {
		cursor_bg = &enemy_cursor_bg;
		cursor_fg = &enemy_cursor_fg;
	} else if(t.side() == u->side()) {
		cursor_bg = &team_cursor_bg;
		cursor_fg = &team_cursor_fg;
	} else {
		cursor_bg = &ally_cursor_bg;
		cursor_fg = &ally_cursor_fg;
	}

	if(cursor_bg) {
		render_scaled_to_zoom(*cursor_bg, loc);
	}

	if(cursor_fg) {
		render_scaled_to_zoom(*cursor_fg, loc);
	}

	//
	// Draw accompanying defense ratings and turn reach numbers within the hex.
	//
	draw_movement_info();

	if(!game_config::images::selected.empty() && get_map().on_board(selectedHex_)) {
		static texture selected(image::get_texture(game_config::images::selected));

		render_scaled_to_zoom(selected, selectedHex_); // SCALED_TO_HEX
	}
}

void game_display::draw_hex_overlays()
{
	//
	// Render halos.
	//
	halo_man_->render();

	//
	// Mask on unreachable locations
	//
	if(!reach_map_.empty()) {
		for(const map_location& loc : get_visible_hexes()) {
			if(shrouded(loc) || loc == attack_indicator_dst_ || reach_map_.find(loc) != reach_map_.end()) {
				continue;
			}

			// was SCALED_TO_HEX
			static texture unreachable = image::get_texture(game_config::images::unreachable);

			render_scaled_to_zoom(unreachable, loc);
		}
	}

	//
	// Attack indicator - source
	//
	if(get_map().on_board(attack_indicator_src_)) {
		texture indicator = image::get_texture("misc/attack-indicator-src-" + attack_indicator_direction() + ".png");

		// was SCALED_TO_HEX
		render_scaled_to_zoom(indicator, attack_indicator_src_);
	}

	//
	// Attack indicator - target
	//
	if(get_map().on_board(attack_indicator_dst_)) {
		texture indicator = image::get_texture("misc/attack-indicator-dst-" + attack_indicator_direction() + ".png");

		// was SCALED_TO_HEX
		render_scaled_to_zoom(indicator, attack_indicator_dst_);
	}

	//
	// Draw route steps
	//
	if(std::shared_ptr<wb::manager> w = wb_.lock()) {
		// w->draw_hex(loc);

		if(!w->is_active() && !w->has_temp_move()) {
			draw_footstep_images();
		}
	}

	// Linger overlay unconditionally otherwise it might give glitches
	// so it's drawn over the shroud and fog.
	// FIXME: ^ split into seperate function so that happens.
	if(mode_ != RUNNING) {
		for(const map_location& loc : get_visible_hexes()) {
			static texture linger(image::get_texture(game_config::images::linger));

			// was TOD_COLORED
			render_scaled_to_zoom(linger, loc);
		}
	}

#if 0
	if(game_config::debug) {
		int debugH = debugHighlights_[loc];
		if (debugH) {
			std::string txt = std::to_string(debugH);
			draw_text_in_hex(loc, drawing_queue::LAYER_MOVE_INFO, txt, 18, font::BAD_COLOR);
		}
	}

	//simulate_delay += 1;
#endif
}

const time_of_day& game_display::get_time_of_day(const map_location& loc) const
{
	return resources::tod_manager->get_time_of_day(loc);
}

bool game_display::has_time_area() const
{
	return resources::tod_manager->has_time_area();
}

void game_display::draw_sidebar()
{
	if(!team_valid())
		return;

	refresh_report("report_clock");
	refresh_report("report_countdown");

	if(invalidateGameStatus_) {
		wb::future_map future; // start planned unit map scope

		// We display the unit the mouse is over if it is over a unit,
		// otherwise we display the unit that is selected.
		for(const std::string& name : reports_object_->report_list()) {
			refresh_report(name);
		}
		invalidateGameStatus_ = false;
	}
}

void game_display::set_game_mode(const game_mode mode)
{
	if(mode != mode_) {
		mode_ = mode;
	}
}

void game_display::draw_movement_info()
{
	// No route steps. Draw terrain defense based on selected hex.
	if(route_.steps.empty()) {
		if(selectedHex_.valid()) {
			const unit_map::const_iterator selectedUnit =
				resources::gameboard->find_visible_unit(selectedHex_, dc_->teams()[currentTeam_]);

			const unit_map::const_iterator mouseoveredUnit =
				resources::gameboard->find_visible_unit(mouseoverHex_, dc_->teams()[currentTeam_]);

#if 0
			if(selectedUnit != dc_->units().end() && mouseoveredUnit == dc_->units().end()) {
				// Display the def% of this terrain
				int def =  100 - selectedUnit->defense_modifier(get_map().get_terrain(mouseoverHex_));
				std::stringstream def_text;
				def_text << def << "%";

				color_t color = game_config::red_to_green(def, false);

				// Use small font
				static const int def_font = 16;
				draw_text_in_hex(mouseoverHex_, def_text.str(), def_font, color);
			}
#endif

			// LAYER_MOVE_INFO
			for(const auto& reach : reach_map_) {
				unsigned int num = reach.second;
				if(num > 1) {
					draw_text_in_hex(reach.first, std::to_string(num), 16, font::YELLOW_COLOR);
				}
			}
		}

		hex_def_fl_labels_.clear();
		return;
	}

	std::shared_ptr<wb::manager> wb = wb_.lock();

	// First step of the route should be a unit.
	const unit_map::const_iterator un = (wb && wb->get_temp_move_unit().valid())
		? wb->get_temp_move_unit()
		: dc_->units().find(route_.steps.front());

	const bool unit_at_start = (un != dc_->units().end());

	if(!unit_at_start) {
		// Remove all the defense labels.
		hex_def_fl_labels_.clear();

		return;
	}

	const unsigned int num_marks = route_.marks.size();

	if(hex_def_fl_labels_.size() != num_marks) {
		hex_def_fl_labels_.resize(num_marks);
	}

	int i = 0;

	for(const auto& mark : route_.marks) {
		const map_location& m_loc = mark.first;

		if(route_.steps.front() == m_loc || shrouded(m_loc) || !get_map().on_board(m_loc)) {
			continue;
		}

		//
		// Display the unit's defence on this terrain
		//
		const int def = 100 - un->defense_modifier(get_map().get_terrain(m_loc));

		std::stringstream def_text;
		def_text << def << "%";

		color_t color = game_config::red_to_green(def, false);

		// Simple mark (no turn point) uses smaller font.
		int def_font = mark.second.turns > 0 ? 18 : 16;

		int& marker_id = hex_def_fl_labels_[i].id;
		marker_id = draw_text_in_hex(m_loc, def_text.str(), def_font, color, marker_id);

		//
		// Draw special location markers
		//

		// TODO: do we want SCALED_TO_HEX?
		// LAYER_MOVE_INFO

		if(mark.second.invisible) {
			static texture hidden(image::get_texture("misc/hidden.png"));
			render_scaled_to_zoom(hidden, m_loc);
		}

		if(mark.second.zoc) {
			static texture zoc(image::get_texture("misc/zoc.png"));
			render_scaled_to_zoom(zoc, m_loc);
		}

		if(mark.second.capture) {
			static texture capture(image::get_texture("misc/capture.png"));
			render_scaled_to_zoom(capture, m_loc);
		}

		// We display turn info only if different from a simple last "1"
		if(mark.second.turns > 1 || (mark.second.turns == 1 && m_loc != route_.steps.back())) {
			std::stringstream turns_text;
			turns_text << mark.second.turns;
			// draw_text_in_hex(m_loc, turns_text.str(), 17, font::NORMAL_COLOR, 0.5, 0.8); TODO
		}

		++i;
	}
}

void game_display::draw_footstep_images() const
{
	// No real route.
	if(route_.steps.size() < 2) {
		return;
	}

	const unit_map::const_iterator u = dc_->units().find(route_.steps.front());
	const bool unit_at_start = u != dc_->units().end();

	for(auto iter = route_.steps.begin(); iter != route_.steps.end(); ++iter) {
		// Step location.
		const map_location& loc = *iter;

		// Check which footsteps image variant to use.
		int move_cost = 1;
		if(unit_at_start) {
			move_cost = u->movement_cost(dc_->map().get_terrain(loc));
		}

		const int image_number = std::min<int>(move_cost, game_config::foot_speed_prefix.size());
		if(image_number < 1) {
			continue; // Invalid movement cost or no images.
		}

		const std::string& foot_speed_prefix = game_config::foot_speed_prefix[image_number - 1];

		std::string teleport_image = "";

		// We draw 2 half-hex (with possibly different directions), but skip the first for the first step...
		const int first_half = (iter == route_.steps.begin()) ? 1 : 0;

		// ...and the second for the last step
		const int second_half = (iter + 1 == route_.steps.end()) ? 0 : 1;

		for(int h = first_half; h <= second_half; ++h) {
			const std::string sense(h == 0 ? "-in" : "-out");

			const map_location& loc_a = *(iter + (h - 1));
			const map_location& loc_b = *(iter +  h);

			// If we have a teleport image, record it and preceed to next step.
			if(!tiles_adjacent(loc_a, loc_b)) {
				teleport_image = (h == 0)
					? game_config::foot_teleport_enter
					: game_config::foot_teleport_exit;

				continue;
			}

			// In function of the half, use the incoming or outgoing direction
			map_location::DIRECTION dir = loc_a.get_relative_dir(loc_b);

			bool rotate = false;
			if(dir > map_location::SOUTH_EAST) {
				// No image, take the opposite direction and flag a 180 rotation.
				dir = map_location::get_opposite_dir(dir);
				rotate = true;
			}

			std::ostringstream ss;
			ss << foot_speed_prefix << sense << "-" << map_location::write_direction(dir) << ".png";

			// Pass rotate flag twice so we get both a horizontal and vertical flip (180 rotation).
			render_scaled_to_zoom(image::get_texture(ss.str()), loc, rotate, rotate); // SCALED_TO_HEX
		}

		// Render teleport image last, if any.
		if(!teleport_image.empty()) {
			render_scaled_to_zoom(image::get_texture(teleport_image), loc); // SCALED_TO_HEX
		}
	}
}

void game_display::highlight_reach(const pathfind::paths& paths_list)
{
	unhighlight_reach();
	highlight_another_reach(paths_list);
}

void game_display::highlight_another_reach(const pathfind::paths& paths_list)
{
	// Fold endpoints of routes into reachability map.
	for(const pathfind::paths::step& dest : paths_list.destinations) {
		reach_map_[dest.curr]++;
	}
}

bool game_display::unhighlight_reach()
{
	if(!reach_map_.empty()) {
		reach_map_.clear();
		return true;
	} else {
		return false;
	}
}

void game_display::set_route(const pathfind::marked_route* route)
{
	if(route != nullptr) {
		route_ = *route;
	} else {
		route_.steps.clear();
		route_.marks.clear();
	}
}

void game_display::float_label(const map_location& loc, const std::string& text, const color_t& color)
{
	if(preferences::show_floating_labels() == false || fogged(loc)) {
		return;
	}

	font::floating_label flabel(text);
	flabel.set_font_size(font::SIZE_XLARGE);
	flabel.set_color(color);
	flabel.set_position(get_location_x(loc) + zoom_ / 2, get_location_y(loc));
	flabel.set_move(0, -2 * turbo_speed());
	flabel.set_lifetime(60 / turbo_speed());
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
	if(attack_indicator_src_ != src || attack_indicator_dst_ != dst) {
		attack_indicator_src_ = src;
		attack_indicator_dst_ = dst;
	}
}

void game_display::clear_attack_indicator()
{
	set_attack_indicator(map_location::null_location(), map_location::null_location());
}

std::string game_display::current_team_name() const
{
	if(team_valid()) {
		return dc_->teams()[currentTeam_].team_name();
	}
	return std::string();
}

void game_display::begin_game()
{
	in_game_ = true;
	create_buttons();
}

void game_display::needs_rebuild(bool b)
{
	if(b) {
		needs_rebuild_ = true;
	}
}

bool game_display::maybe_rebuild()
{
	if(needs_rebuild_) {
		needs_rebuild_ = false;
		recalculate_minimap();
		rebuild_all();
		return true;
	}
	return false;
}
