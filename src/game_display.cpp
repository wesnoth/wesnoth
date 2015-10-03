/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file game_display.cpp
 * During a game, show map & info-panels at top+right.
 */

#include "global.hpp"

#include "game_display.hpp"

#include "wesconfig.h"

#ifdef HAVE_LIBDBUS
#include <dbus/dbus.h>
#endif

#ifdef HAVE_GROWL
#include <Growl/GrowlApplicationBridge-Carbon.h>
#include <Carbon/Carbon.h>
Growl_Delegate growl_obj;
#endif

#include "actions.hpp"
#include "foreach.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "marked-up_text.hpp"
#include "game_preferences.hpp"
#include "tod_manager.hpp"
#include "sound.hpp"

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

std::map<map_location,fixed_t> game_display::debugHighlights_;

game_display::game_display(unit_map& units, CVideo& video, const gamemap& map,
		const tod_manager& tod, const std::vector<team>& t,
		const config& theme_cfg, const config& level) :
		display(video, &map, theme_cfg, level),
		units_(units),
		temp_unit_(NULL),
		attack_indicator_src_(),
		attack_indicator_dst_(),
		energy_bar_rects_(),
		route_(),
		tod_manager_(tod),
		teams_(t),
		level_(level),
		invalidateUnit_(true),
		displayedUnitHex_(),
		overlays_(),
		currentTeam_(0),
		activeTeam_(0),
		sidebarScaling_(1.0),
		first_turn_(true),
		in_game_(false),
		observers_(),
		chat_messages_(),
		tod_hex_mask1(NULL),
		tod_hex_mask2(NULL),
		reach_map_(),
		reach_map_old_(),
		reach_map_changed_(true),
		game_mode_(RUNNING),
		flags_()
{
	singleton_ = this;

	// Inits the flag list and the team colors used by ~TC
	flags_.reserve(teams_.size());

	std::vector<std::string> side_colors;
	side_colors.reserve(teams_.size());

	for(size_t i = 0; i != teams_.size(); ++i) {
		std::string side_color = team::get_side_colour_index(i+1);
		side_colors.push_back(side_color);
		std::string flag = teams_[i].flag();
		std::string old_rgb = game_config::flag_rgb;
		std::string new_rgb = side_color;

		if(flag.empty()) {
			flag = game_config::flag_image;
		}

		LOG_DP << "Adding flag for team " << i << " from animation " << flag << "\n";

		// Must recolor flag image
		animated<image::locator> temp_anim;

		std::vector<std::string> items = utils::split(flag);
		std::vector<std::string>::const_iterator itor = items.begin();
		for(; itor != items.end(); ++itor) {
			const std::vector<std::string>& items = utils::split(*itor, ':');
			std::string str;
			int time;

			if(items.size() > 1) {
				str = items.front();
				time = atoi(items.back().c_str());
			} else {
				str = *itor;
				time = 100;
			}
			std::stringstream temp;
			temp << str << "~RC(" << old_rgb << ">"<< new_rgb << ")";
			image::locator flag_image(temp.str());
			temp_anim.add_frame(time, flag_image);
		}
		flags_.push_back(temp_anim);

		flags_.back().start_animation(rand()%flags_.back().get_end_time(), true);
	}
	image::set_team_colors(&side_colors);
	clear_screen();
}

game_display* game_display::create_dummy_display(CVideo& video)
{
	static unit_map dummy_umap;
	static config dummy_cfg;
	static gamemap dummy_map(dummy_cfg, "");
	static tod_manager dummy_tod(dummy_cfg, 0);
	static std::vector<team> dummy_teams;
	return new game_display(dummy_umap, video, dummy_map, dummy_tod,
			dummy_teams, dummy_cfg, dummy_cfg);
}

game_display::~game_display()
{
	// SDL_FreeSurface(minimap_);
	prune_chat_messages(true);
	singleton_ = NULL;
}

void game_display::new_turn()
{
	const time_of_day& tod = tod_manager_.get_time_of_day();

	if( !first_turn_) {
		const time_of_day& old_tod = tod_manager_.get_previous_time_of_day();

		if(old_tod.image_mask != tod.image_mask) {
			const surface old_mask(image::get_image(old_tod.image_mask,image::UNMASKED));
			const surface new_mask(image::get_image(tod.image_mask,image::UNMASKED));

			const int niterations = static_cast<int>(10/turbo_speed());
			const int frame_time = 30;
			const int starting_ticks = SDL_GetTicks();
			for(int i = 0; i != niterations; ++i) {

				if(old_mask != NULL) {
					const fixed_t proportion = ftofxp(1.0) - fxpdiv(i,niterations);
					tod_hex_mask1.assign(adjust_surface_alpha(old_mask,proportion));
				}

				if(new_mask != NULL) {
					const fixed_t proportion = fxpdiv(i,niterations);
					tod_hex_mask2.assign(adjust_surface_alpha(new_mask,proportion));
				}

				invalidate_all();
				draw();

				const int cur_ticks = SDL_GetTicks();
				const int wanted_ticks = starting_ticks + i*frame_time;
				if(cur_ticks < wanted_ticks) {
					SDL_Delay(wanted_ticks - cur_ticks);
				}
			}
		}

		tod_hex_mask1.assign(NULL);
		tod_hex_mask2.assign(NULL);
	}

	first_turn_ = false;

	image::set_colour_adjustment(tod.red,tod.green,tod.blue);

	invalidate_all();
	draw();
}

void game_display::adjust_colours(int r, int g, int b)
{
	const time_of_day& tod = tod_manager_.get_time_of_day();
	image::set_colour_adjustment(tod.red+r,tod.green+g,tod.blue+b);
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
	const unit *u = get_visible_unit(units_, hex, teams_[viewing_team()], !viewpoint_);
	if (u) {
		displayedUnitHex_ = hex;
		invalidate_unit();
	} else {
		u = get_visible_unit(units_, mouseoverHex_, teams_[viewing_team()], !viewpoint_);
		if (u) {
			// mouse moved from unit hex to non-unit hex
			if (units_.count(selectedHex_)) {
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
	const unit *u = get_visible_unit(units_, hex, teams_[viewing_team()], !viewpoint_);
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

void game_display::scroll_to_leader(unit_map& units, int side, SCROLL_TYPE scroll_type,bool force)
{
	unit_map::const_iterator leader = units.find_leader(side);

	if(leader != units_.end()) {
		// YogiHH: I can't see why we need another key_handler here,
		// therefore I will comment it out :
		/*
		const hotkey::basic_handler key_events_handler(gui_);
		*/
		scroll_to_tile(leader->first, scroll_type,true,force);
	}
}

void game_display::pre_draw() {
	process_reachmap_changes();
	/**
	 * @todo FIXME: must modify changed, but best to do it at the
	 * floating_label level
	 */
	prune_chat_messages();
}

std::vector<map_location> game_display::get_invalidated_unit_locations() {
	std::vector<map_location> unit_locations;
	BOOST_FOREACH (const map_location& loc, invalidated_) {
		if ((temp_unit_ && temp_unit_->get_location() == loc) ||
		    units_.find(loc) != units_.end())
		{
			unit_locations.push_back(loc);
		}
	}
	//sorted according to a drawing ordering object in order to render correctly
	std::sort(unit_locations.begin(), unit_locations.end(), ordered_draw());
	return unit_locations;
}

image::TYPE game_display::get_image_type(const map_location& loc) {
	// We highlight hex under the mouse, or under a selected unit.
	if (get_map().on_board(loc)) {
		if (loc == mouseoverHex_ || loc == attack_indicator_src_) {
			return image::BRIGHTENED;
		} else if (loc == selectedHex_) {
			const unit *un = get_visible_unit(units_, loc, teams_[currentTeam_], !viewpoint_);
			if (un && !un->get_hidden()) {
				return image::BRIGHTENED;
			}
		}
	}
	return image::SCALED_TO_HEX;
}


void game_display::draw_invalidated()
{
	halo::unrender(invalidated_);
	display::draw_invalidated();
	std::vector<map_location> unit_invals = get_invalidated_unit_locations();
	redraw_units(unit_invals);
}

void game_display::post_commit()
{
	halo::render();
}

void game_display::draw_hex(const map_location& loc)
{
	const bool on_map = get_map().on_board(loc);
	const bool is_shrouded = shrouded(loc);
	const bool is_fogged = fogged(loc);
	int xpos = get_location_x(loc);
	int ypos = get_location_y(loc);
	tblit blit(xpos, ypos);

	image::TYPE image_type = get_image_type(loc);

	display::draw_hex(loc);

	if(!is_shrouded) {
		typedef overlay_map::const_iterator Itor;
		std::pair<Itor,Itor> overlays = overlays_.equal_range(loc);
		for( ; overlays.first != overlays.second; ++overlays.first) {
			if ((overlays.first->second.team_name == "" ||
			overlays.first->second.team_name.find(teams_[playing_team()].team_name()) != std::string::npos)
			&& !(is_fogged && !overlays.first->second.visible_in_fog))
			{
				drawing_buffer_add(LAYER_TERRAIN_BG, loc, tblit(xpos, ypos,
					image::get_image(overlays.first->second.image,image_type)));
			}
		}
		// village-control flags.
		drawing_buffer_add(LAYER_TERRAIN_BG, loc, tblit(xpos, ypos, get_flag(loc)));
	}

	// Draw the time-of-day mask on top of the terrain in the hex.
	// tod may differ from tod if hex is illuminated.
	const std::string& tod_hex_mask = tod_manager_.get_time_of_day(0, loc).image_mask;
	if(tod_hex_mask1 != NULL || tod_hex_mask2 != NULL) {
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, tblit(xpos, ypos, tod_hex_mask1));
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, tblit(xpos, ypos, tod_hex_mask2));
	} else if(!tod_hex_mask.empty()) {
		drawing_buffer_add(LAYER_TERRAIN_FG, loc, tblit(xpos, ypos,
			image::get_image(tod_hex_mask,image::UNMASKED)));
	}

	// Draw reach_map information.
	// We remove the reachability mask of the unit
	// that we want to attack.
	if (!is_shrouded && !reach_map_.empty()
			&& reach_map_.find(loc) == reach_map_.end() && loc != attack_indicator_dst_) {
		drawing_buffer_add(LAYER_REACHMAP, loc, tblit(xpos, ypos,
			image::get_image(game_config::unreachable_image,image::UNMASKED)));
	}

	// Footsteps indicating a movement path
	drawing_buffer_add(LAYER_TERRAIN_TMP_BG, loc, tblit(xpos, ypos, footsteps_images(loc)));
	// Draw the attack direction indicator
	if(on_map && loc == attack_indicator_src_) {
		drawing_buffer_add(LAYER_ATTACK_INDICATOR, loc, tblit(xpos, ypos,
			image::get_image("misc/attack-indicator-src-" + attack_indicator_direction() + ".png", image::UNMASKED)));
	} else if (on_map && loc == attack_indicator_dst_) {
		drawing_buffer_add(LAYER_ATTACK_INDICATOR, loc, tblit(xpos, ypos,
			image::get_image("misc/attack-indicator-dst-" + attack_indicator_direction() + ".png", image::UNMASKED)));
	}

	// Linger overlay unconditionally otherwise it might give glitches
	// so it's drawn over the shroud and fog.
	if(game_mode_ != RUNNING) {
		blit.surf.push_back(image::get_image(game_config::linger_image, image::SCALED_TO_HEX));
		drawing_buffer_add(LAYER_LINGER_OVERLAY, loc, blit);
		blit.surf.clear();
	}

	// Show def% and turn to reach infos
	if(!is_shrouded && on_map) {
		draw_movement_info(loc);
	}

	if(game_config::debug) {
		int debugH = debugHighlights_[loc];
		if (debugH) {
			std::string txt = lexical_cast<std::string>(debugH);
			draw_text_in_hex(loc, LAYER_MOVE_INFO, txt, 18, font::BAD_COLOUR);
		}
	}
	//simulate_delay += 1;
}

void game_display::update_time_of_day()
{
	tod_ = tod_manager_.get_time_of_day();
}

void game_display::redraw_units(const std::vector<map_location>& invalidated_unit_locations)
{
	// Units can overlap multiple hexes, so we need
	// to redraw them last and in the good sequence.
	BOOST_FOREACH (map_location loc, invalidated_unit_locations) {
		unit_map::iterator u_it = units_.find(loc);
		if (u_it != units_.end()) {
			u_it->second.redraw_unit();
			//simulate_delay += 1;
		}
		if (temp_unit_ && temp_unit_->get_location() == loc) {
			temp_unit_->redraw_unit();
			//simulate_delay += 1;
		}
	}
}

void game_display::draw_report(reports::TYPE report_num)
{
	if(!team_valid()) {
		return;
	}

	reports::report report = reports::generate_report(report_num, report_,
							  teams_[viewing_team()],
							  size_t(currentTeam_+1),size_t(activeTeam_+1),
							  selectedHex_, mouseoverHex_, displayedUnitHex_,
		observers_, level_, !viewpoint_);

	refresh_report(report_num, report);
}

void game_display::draw_game_status()
{
	if(teams_.empty()) {
		return;
	}

	for(size_t r = reports::STATUS_REPORTS_BEGIN; r != reports::STATUS_REPORTS_END; ++r) {
		draw_report(reports::TYPE(r));
	}
}

void game_display::draw_sidebar()
{
	draw_report(reports::REPORT_CLOCK);
	draw_report(reports::REPORT_COUNTDOWN);

	if(teams_.empty()) {
		return;
	}

	if(invalidateUnit_) {
		// We display the unit the mouse is over if it is over a unit,
		// otherwise we display the unit that is selected.
		for(size_t r = reports::UNIT_REPORTS_BEGIN; r != reports::UNIT_REPORTS_END; ++r) {
			draw_report(reports::TYPE(r));
		}

		invalidateUnit_ = false;
	}

	if(invalidateGameStatus_) {
		draw_game_status();
		invalidateGameStatus_ = false;
	}
}

void game_display::draw_minimap_units()
{
	double xscaling = 1.0 * minimap_location_.w / get_map().w();
	double yscaling = 1.0 * minimap_location_.h / get_map().h();

	for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
		if(fogged(u->first) ||
				(teams_[currentTeam_].is_enemy(u->second.side()) &&
				u->second.invisible(u->first,units_,teams_))) {
			continue;
		}

		const int side = u->second.side();
		const SDL_Color col = team::get_minimap_colour(side);
		const Uint32 mapped_col = SDL_MapRGB(video().getSurface()->format,col.r,col.g,col.b);

		double u_x = u->first.x * xscaling;
		double u_y = (u->first.y + (is_odd(u->first.x) ? 1 : -1)/4.0) * yscaling;
 		// use 4/3 to compensate the horizontal hexes imbrication
		double u_w = 4.0 / 3.0 * xscaling;
		double u_h = yscaling;

		SDL_Rect r = { minimap_location_.x + round_double(u_x),
                       minimap_location_.y + round_double(u_y),
                       round_double(u_w), round_double(u_h) };

		SDL_FillRect(video().getSurface(), &r, mapped_col);
	}
}

void game_display::draw_bar(const std::string& image, int xpos, int ypos,
		const map_location& loc, size_t height, double filled,
		const SDL_Color& col, fixed_t alpha)
{

	filled = std::min<double>(std::max<double>(filled,0.0),1.0);
	height = static_cast<size_t>(height*get_zoom_factor());
#ifdef USE_TINY_GUI
	height /= 2;
#endif

	surface surf(image::get_image(image,image::UNMASKED));

	// We use UNSCALED because scaling (and bilinear interpolaion)
	// is bad for calculate_energy_bar.
	// But we will do a geometric scaling later.
	surface bar_surf(image::get_image(image));
	if(surf == NULL || bar_surf == NULL) {
		return;
	}

	// calculate_energy_bar returns incorrect results if the surface colors
	// have changed (for example, due to bilinear interpolaion)
	const SDL_Rect& unscaled_bar_loc = calculate_energy_bar(bar_surf);

	SDL_Rect bar_loc;
	if (surf->w == bar_surf->w && surf->h == bar_surf->h)
	  bar_loc = unscaled_bar_loc;
	else {
	  const fixed_t xratio = fxpdiv(surf->w,bar_surf->w);
	  const fixed_t yratio = fxpdiv(surf->h,bar_surf->h);
	  const SDL_Rect scaled_bar_loc = {fxptoi(unscaled_bar_loc. x * xratio),
					   fxptoi(unscaled_bar_loc. y * yratio + 127),
					   fxptoi(unscaled_bar_loc. w * xratio + 255),
					   fxptoi(unscaled_bar_loc. h * yratio + 255)};
	  bar_loc = scaled_bar_loc;
	}

	if(height > bar_loc.h) {
		height = bar_loc.h;
	}

	//if(alpha != ftofxp(1.0)) {
	//	surf.assign(adjust_surface_alpha(surf,alpha));
	//	if(surf == NULL) {
	//		return;
	//	}
	//}

	const size_t skip_rows = bar_loc.h - height;

	SDL_Rect top = {0,0,surf->w,bar_loc.y};
	SDL_Rect bot = {0,bar_loc.y+skip_rows,surf->w,0};
	bot.h = surf->w - bot.y;

	drawing_buffer_add(LAYER_UNIT_BAR, loc, tblit(xpos, ypos, surf, top));
	drawing_buffer_add(LAYER_UNIT_BAR, loc, tblit(xpos, ypos + top.h, surf, bot));

	size_t unfilled = static_cast<size_t>(height * (1.0 - filled));

	if(unfilled < height && alpha >= ftofxp(0.3)) {
		const Uint8 r_alpha = std::min<unsigned>(unsigned(fxpmult(alpha,255)),255);
		surface filled_surf = create_compatible_surface(bar_surf, bar_loc.w, height - unfilled);
		SDL_Rect filled_area = {0, 0, bar_loc.w, height-unfilled};
		SDL_FillRect(filled_surf,&filled_area,SDL_MapRGBA(bar_surf->format,col.r,col.g,col.b, r_alpha));
		drawing_buffer_add(LAYER_UNIT_BAR, loc, tblit(xpos + bar_loc.x, ypos + bar_loc.y + unfilled, filled_surf));
	}
}

void game_display::set_game_mode(const tgame_mode game_mode)
{
	if(game_mode != game_mode_) {
		game_mode_ = game_mode;
		invalidate_all();
	}
}

void game_display::draw_movement_info(const map_location& loc)
{
	// Search if there is a mark here
	pathfind::marked_route::mark_map::iterator w = route_.marks.find(loc);

	// Don't use empty route or the first step (the unit will be there)
	if(w != route_.marks.end()
				&& !route_.steps.empty() && route_.steps.front() != loc) {
		const unit_map::const_iterator un = units_.find(route_.steps.front());
		if(un != units_.end()) {
			// Display the def% of this terrain
			const int def =  100 - un->second.defense_modifier(get_map().get_terrain(loc));
			std::stringstream def_text;
			def_text << def << "%";

			SDL_Color color = int_to_color(game_config::red_to_green(def, false));

			// simple mark (no turn point) use smaller font
			int def_font = w->second.turns > 0 ? 18 : 16;
			draw_text_in_hex(loc, LAYER_MOVE_INFO, def_text.str(), def_font, color);

			int xpos = get_location_x(loc);
			int ypos = get_location_y(loc);

            if (w->second.invisible) {
				drawing_buffer_add(LAYER_MOVE_INFO, loc, tblit(xpos, ypos,
					image::get_image("misc/hidden.png", image::UNMASKED)));
			}

			if (w->second.zoc) {
				drawing_buffer_add(LAYER_MOVE_INFO, loc, tblit(xpos, ypos,
					image::get_image("misc/zoc.png", image::UNMASKED)));
			}

			if (w->second.capture) {
				drawing_buffer_add(LAYER_MOVE_INFO, loc, tblit(xpos, ypos,
					image::get_image("misc/capture.png", image::UNMASKED)));
			}

			if (w->second.pass_here) {
				drawing_buffer_add(LAYER_MOVE_INFO, loc, tblit(xpos, ypos,
					image::get_image("misc/waypoint.png", image::UNMASKED)));
			}

			//we display turn info only if different from a simple last "1"
			if (w->second.turns > 1 || (w->second.turns == 1 && loc != route_.steps.back())) {
				std::stringstream turns_text;
				turns_text << w->second.turns;
				draw_text_in_hex(loc, LAYER_MOVE_INFO, turns_text.str(), 17, font::NORMAL_COLOUR, 0.5,0.8);
			}

			// The hex is full now, so skip the "show enemy moves"
			return;
		}
	}

	if (!reach_map_.empty()) {
		reach_map::iterator reach = reach_map_.find(loc);
		if (reach != reach_map_.end() && reach->second > 1) {
			const std::string num = lexical_cast<std::string>(reach->second);
			draw_text_in_hex(loc, LAYER_MOVE_INFO, num, 16, font::YELLOW_COLOUR);
		}
	}
}

std::vector<surface> game_display::footsteps_images(const map_location& loc)
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
	const unit_map::const_iterator u = units_.find(route_.steps.front());
	if(u != units_.end()) {
			move_cost = u->second.movement_cost(get_map().get_terrain(loc));
	}
	int image_number = std::min<int>(move_cost, game_config::foot_speed_prefix.size());
	if (image_number < 1) {
		return res; // Invalid movement cost or no images
	}
	const std::string foot_speed_prefix = game_config::foot_speed_prefix[image_number-1];

	surface teleport = NULL;

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
			teleport = image::get_image(teleport_image, image::UNMASKED);
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

		res.push_back(image::get_image(image, image::UNMASKED));
	}

	// we draw teleport image (if any) in last
	if (teleport != NULL) res.push_back(teleport);

	return res;
}

surface game_display::get_flag(const map_location& loc)
{
	t_translation::t_terrain terrain = get_map().get_terrain(loc);

	if(!get_map().is_village(terrain)) {
		return surface(NULL);
	}

	for(size_t i = 0; i != teams_.size(); ++i) {
		if(teams_[i].owns_village(loc) &&
		  (!fogged(loc) || !teams_[currentTeam_].is_enemy(i+1)))
		{
			flags_[i].update_last_draw_time();
			const image::locator& image_flag = preferences::animate_map() ?
				flags_[i].get_current_frame() : flags_[i].get_first_frame();
			return image::get_image(image_flag, image::SCALED_TO_HEX);
		}
	}

	return surface(NULL);
}

void game_display::highlight_reach(const pathfind::paths &paths_list)
{
	unhighlight_reach();
	highlight_another_reach(paths_list);
}

void game_display::highlight_another_reach(const pathfind::paths &paths_list)
{
	// Fold endpoints of routes into reachability map.
	BOOST_FOREACH (const pathfind::paths::step &dest, paths_list.destinations) {
		reach_map_[dest.curr]++;
	}
	reach_map_changed_ = true;
}

void game_display::unhighlight_reach()
{
	reach_map_ = reach_map();
	reach_map_changed_ = true;
}

void game_display::process_reachmap_changes()
{
	if (!reach_map_changed_) return;
	if (reach_map_.empty() != reach_map_old_.empty()) {
		// Invalidate everything except the non-darkened tiles
		reach_map &full = reach_map_.empty() ? reach_map_old_ : reach_map_;

		rect_of_hexes hexes = get_visible_hexes();
		rect_of_hexes::iterator i = hexes.begin(), end = hexes.end();
		for (;i != end; ++i) {
			reach_map::iterator reach = full.find(*i);
			if (reach == full.end()) {
				// Location needs to be darkened or brightened
				invalidate(*i);
			} else if (reach->second != 1) {
				// Number needs to be displayed or cleared
				invalidate(*i);
			}
		}
	} else if (!reach_map_.empty()) {
		// Invalidate only changes
		reach_map::iterator reach, reach_old;
		for (reach = reach_map_.begin(); reach != reach_map_.end(); ++reach) {
			reach_old = reach_map_old_.find(reach->first);
			if (reach_old == reach_map_old_.end()) {
				invalidate(reach->first);
			} else {
				if (reach_old->second != reach->second) {
					invalidate(reach->first);
				}
				reach_map_old_.erase(reach_old);
			}
		}
		for (reach_old = reach_map_old_.begin(); reach_old != reach_map_old_.end(); ++reach_old) {
			invalidate(reach_old->first);
		}
	}
	reach_map_old_ = reach_map_;
	reach_map_changed_ = false;
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

	if(route != NULL) {
		route_ = *route;
	} else {
		route_.steps.clear();
		route_.marks.clear();
	}

	invalidate_route();
}

void game_display::float_label(const map_location& loc, const std::string& text,
						  int red, int green, int blue)
{
	if(preferences::show_floating_labels() == false || fogged(loc)) {
		return;
	}

	const SDL_Color color = {red,green,blue,255};
	int lifetime = static_cast<int>(60/turbo_speed());
	font::add_floating_label(text,font::SIZE_XLARGE,color,get_location_x(loc)+zoom_/2,get_location_y(loc),
	                         0,-2*turbo_speed(),lifetime,screen_area(),font::CENTER_ALIGN,NULL,0,font::ANCHOR_LABEL_MAP);
}

struct is_energy_colour {
	bool operator()(Uint32 colour) const { return (colour&0xFF000000) > 0x10000000 &&
	                                              (colour&0x00FF0000) < 0x00100000 &&
												  (colour&0x0000FF00) < 0x00001000 &&
												  (colour&0x000000FF) < 0x00000010; }
};

const SDL_Rect& game_display::calculate_energy_bar(surface surf)
{
	const std::map<surface,SDL_Rect>::const_iterator i = energy_bar_rects_.find(surf);
	if(i != energy_bar_rects_.end()) {
		return i->second;
	}

	int first_row = -1, last_row = -1, first_col = -1, last_col = -1;

	surface image(make_neutral_surface(surf));

	surface_lock image_lock(image);
	const Uint32* const begin = image_lock.pixels();

	for(int y = 0; y != image->h; ++y) {
		const Uint32* const i1 = begin + image->w*y;
		const Uint32* const i2 = i1 + image->w;
		const Uint32* const itor = std::find_if(i1,i2,is_energy_colour());
		const int count = std::count_if(itor,i2,is_energy_colour());

		if(itor != i2) {
			if(first_row == -1) {
				first_row = y;
			}

			first_col = itor - i1;
			last_col = first_col + count;
			last_row = y;
		}
	}

	const SDL_Rect res = {first_col,first_row,last_col-first_col,last_row+1-first_row};
	energy_bar_rects_.insert(std::pair<surface,SDL_Rect>(surf,res));
	return calculate_energy_bar(surf);
}

void game_display::invalidate_animations_location(const map_location& loc) {
	if (get_map().is_village(loc)) {
		const int owner = player_teams::village_owner(loc);
		if (owner >= 0 && flags_[owner].need_update()
		&& (!fogged(loc) || !teams_[currentTeam_].is_enemy(owner+1))) {
			invalidate(loc);
		}
	}
}

void game_display::invalidate_animations()
{
	new_animation_frame();
	display::invalidate_animations();
	unit_map::iterator unit;
	for(unit=units_.begin() ; unit != units_.end() ; ++unit)
		unit->second.refresh();
	if (temp_unit_ )
		temp_unit_->refresh();
	bool new_inval = true;
	while(new_inval) {
		new_inval = false;
		for(unit=units_.begin() ; unit != units_.end() ; ++unit) {
			new_inval |= unit->second.invalidate(unit->second.get_location());
		}
		if (temp_unit_ ) {
			//new_inval |=invalidate(temp_unit_loc_);
			new_inval |=temp_unit_->invalidate(temp_unit_->get_location());
		}
	}
}

void game_display::debug_highlight(const map_location& loc, fixed_t amount)
{
	assert(game_config::debug);
	debugHighlights_[loc] += amount;
}

void game_display::place_temporary_unit(unit &u)
{
	temp_unit_ = &u;
	invalidate(u.get_location());
}

void game_display::remove_temporary_unit()
{
	if(!temp_unit_) return;

	invalidate(temp_unit_->get_location());
	// Redraw with no location to get rid of haloes
	temp_unit_->clear_haloes();
	temp_unit_ = NULL;
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
	set_attack_indicator(map_location::null_location, map_location::null_location);
}

void game_display::add_overlay(const map_location& loc, const std::string& img, const std::string& halo,const std::string& team_name, bool visible_under_fog)
{
	const int halo_handle = halo::add(get_location_x(loc) + hex_size() / 2,
			get_location_y(loc) + hex_size() / 2, halo, loc);

	const overlay item(img, halo, halo_handle, team_name, visible_under_fog);
	overlays_.insert(overlay_map::value_type(loc,item));
}

void game_display::remove_overlay(const map_location& loc)
{
	typedef overlay_map::const_iterator Itor;
	std::pair<Itor,Itor> itors = overlays_.equal_range(loc);
	while(itors.first != itors.second) {
		halo::remove(itors.first->second.halo_handle);
		++itors.first;
	}

	overlays_.erase(loc);
}

void game_display::remove_single_overlay(const map_location& loc, const std::string& toDelete)
{
	//Iterate through the values with key of loc
	typedef overlay_map::iterator Itor;
	overlay_map::iterator iteratorCopy;
	std::pair<Itor,Itor> itors = overlays_.equal_range(loc);
	while(itors.first != itors.second) {
		//If image or halo of overlay struct matches toDelete, remove the overlay
		if(itors.first->second.image == toDelete || itors.first->second.halo == toDelete) {
			iteratorCopy = itors.first;
			++itors.first;
			halo::remove(iteratorCopy->second.halo_handle);
			overlays_.erase(iteratorCopy);
		}
		else {
			++itors.first;
		}
	}
}

void game_display::write_overlays(config& cfg) const
{
	for(overlay_map::const_iterator i = overlays_.begin(); i != overlays_.end(); ++i) {
		config& item = cfg.add_child("item");
		i->first.write(item);
		item["image"] = i->second.image;
		item["halo"] = i->second.halo;
		item["team_name"] = i->second.team_name;
		item["visible_in_fog"] = i->second.visible_in_fog ? "yes" : "no";
	}
}

void game_display::parse_team_overlays()
{
	const team& curr_team = teams_[playing_team()];
	const team& prev_team = teams_[playing_team()-1 < teams_.size() ? playing_team()-1 : teams_.size()-1];
	BOOST_FOREACH (const game_display::overlay_map::value_type i, overlays_) {
		const overlay& ov = i.second;
		if (!ov.team_name.empty() &&
			((ov.team_name.find(curr_team.team_name()) + 1) != 0) !=
			((ov.team_name.find(prev_team.team_name()) + 1) != 0))
		{
			invalidate(i.first);
		}
	}
}

std::string game_display::current_team_name() const
{
	if (team_valid())
	{
		return teams_[currentTeam_].team_name();
	}
	return std::string();
}

#ifdef HAVE_LIBDBUS
/** Use KDE 4 notifications. */
static bool kde_style = false;

struct wnotify
{
	uint32_t id;
	std::string owner;
	std::string message;
};

static std::list<wnotify> notifications;

static DBusHandlerResult filter_dbus_signal(DBusConnection *, DBusMessage *buf, void *)
{
	if (!dbus_message_is_signal(buf, "org.freedesktop.Notifications", "NotificationClosed")) {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	uint32_t id;
	dbus_message_get_args(buf, NULL,
		DBUS_TYPE_UINT32, &id,
		DBUS_TYPE_INVALID);

	std::list<wnotify>::iterator i = notifications.begin(),
		i_end = notifications.end();
	while (i != i_end && i->id != id) ++i;
	if (i != i_end)
		notifications.erase(i);

	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusConnection *get_dbus_connection()
{
	static bool initted = false;
	static DBusConnection *connection = NULL;
	if (!initted)
	{
		initted = true;
		if (utils::string_bool(preferences::get("disable_notifications"))) {
			return NULL;
		}
		if (getenv("KDE_SESSION_VERSION")) {
			// This variable is defined for KDE 4 only.
			kde_style = true;
		}
		DBusError err;
		dbus_error_init(&err);
		connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
		if (!connection) {
			ERR_DP << "Failed to open DBus session: " << err.message << '\n';
			dbus_error_free(&err);
			return NULL;
		}
		dbus_connection_add_filter(connection, filter_dbus_signal, NULL, NULL);
	}
	if (connection) {
		dbus_connection_read_write(connection, 0);
		while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS) {}
	}
	return connection;
}

static uint32_t send_dbus_notification(DBusConnection *connection, uint32_t replaces_id,
	const std::string &owner, const std::string &message)
{
	DBusMessage *buf = dbus_message_new_method_call(
		kde_style ? "org.kde.VisualNotifications" : "org.freedesktop.Notifications",
		kde_style ? "/VisualNotifications" : "/org/freedesktop/Notifications",
		kde_style ? "org.kde.VisualNotifications" : "org.freedesktop.Notifications",
		"Notify");
	const char *app_name = "Battle for Wesnoth";
	dbus_message_append_args(buf,
		DBUS_TYPE_STRING, &app_name,
		DBUS_TYPE_UINT32, &replaces_id,
		DBUS_TYPE_INVALID);
	if (kde_style) {
		const char *event_id = "";
		dbus_message_append_args(buf,
			DBUS_TYPE_STRING, &event_id,
			DBUS_TYPE_INVALID);
	}
	std::string app_icon_ = game_config::path + "/images/wesnoth-icon-small.png";
	const char *app_icon = app_icon_.c_str();
	const char *summary = owner.c_str();
	const char *body = message.c_str();
	const char **actions = NULL;
	dbus_message_append_args(buf,
		DBUS_TYPE_STRING, &app_icon,
		DBUS_TYPE_STRING, &summary,
		DBUS_TYPE_STRING, &body,
		DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &actions, 0,
		DBUS_TYPE_INVALID);
	DBusMessageIter iter, hints;
	dbus_message_iter_init_append(buf, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &hints);
	dbus_message_iter_close_container(&iter, &hints);
	int expire_timeout = kde_style ? 5000 : -1;
	dbus_message_append_args(buf,
		DBUS_TYPE_INT32, &expire_timeout,
		DBUS_TYPE_INVALID);
	DBusError err;
	dbus_error_init(&err);
	DBusMessage *ret = dbus_connection_send_with_reply_and_block(connection, buf, 1000, &err);
	dbus_message_unref(buf);
	if (!ret) {
		ERR_DP << "Failed to send visual notification: " << err.message << '\n';
		dbus_error_free(&err);
		if (kde_style) {
			ERR_DP << " Retrying with the freedesktop protocol.\n";
			kde_style = false;
			return send_dbus_notification(connection, replaces_id, owner, message);
		}
		return 0;
	}
	uint32_t id;
	dbus_message_get_args(ret, NULL,
		DBUS_TYPE_UINT32, &id,
		DBUS_TYPE_INVALID);
	dbus_message_unref(ret);
	// TODO: remove once closing signals for KDE are handled in filter_dbus_signal.
	if (kde_style) return 0;
	return id;
}
#endif

#if defined(HAVE_LIBDBUS) || defined(HAVE_GROWL)
void game_display::send_notification(const std::string& owner, const std::string& message)
#else
void game_display::send_notification(const std::string& /*owner*/, const std::string& /*message*/)
#endif
{
#if defined(HAVE_LIBDBUS) || defined(HAVE_GROWL)
	Uint8 app_state = SDL_GetAppState();

	// Do not show notifications when the window is visible...
	if ((app_state & SDL_APPACTIVE) != 0)
	{
		// ... and it has a focus.
		if ((app_state & (SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) != 0) {
			return;
		}
	}
#endif

#ifdef HAVE_LIBDBUS
	DBusConnection *connection = get_dbus_connection();
	if (!connection) return;

	std::list<wnotify>::iterator i = notifications.begin(),
		i_end = notifications.end();
	while (i != i_end && i->owner != owner) ++i;

	if (i != i_end) {
		i->message += "\n";
		i->message += message;
		send_dbus_notification(connection, i->id, owner, i->message);
		return;
	}

	uint32_t id = send_dbus_notification(connection, 0, owner, message);
	if (!id) return;
	wnotify visual;
	visual.id = id;
	visual.owner = owner;
	visual.message = message;
	notifications.push_back(visual);
#endif

#ifdef HAVE_GROWL
	CFStringRef app_name = CFStringCreateWithCString(NULL, "Wesnoth", kCFStringEncodingUTF8);
	CFStringRef cf_owner = CFStringCreateWithCString(NULL, owner.c_str(), kCFStringEncodingUTF8);
	CFStringRef cf_message = CFStringCreateWithCString(NULL, message.c_str(), kCFStringEncodingUTF8);
	//Should be changed as soon as there are more than 2 types of notifications
	CFStringRef cf_note_name = CFStringCreateWithCString(NULL, owner == "Turn changed" ? "Turn changed" : "Chat message", kCFStringEncodingUTF8);

	growl_obj.applicationName = app_name;
	growl_obj.registrationDictionary = NULL;
	growl_obj.applicationIconData = NULL;
	growl_obj.growlIsReady = NULL;
	growl_obj.growlNotificationWasClicked = NULL;
	growl_obj.growlNotificationTimedOut = NULL;

	Growl_SetDelegate(&growl_obj);
	Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(cf_owner, cf_message, cf_note_name, NULL, NULL, NULL, NULL);

	CFRelease(app_name);
	CFRelease(cf_owner);
	CFRelease(cf_message);
	CFRelease(cf_note_name);
#endif
}

void game_display::set_team(size_t teamindex, bool show_everything)
{
	assert(teamindex < teams_.size());
	currentTeam_ = teamindex;
	if (!show_everything)
	{
		labels().set_team(&teams_[teamindex]);
		viewpoint_ = &teams_[teamindex];
	}
	else
	{
		labels().set_team(0);
		viewpoint_ = NULL;
	}
	labels().recalculate_labels();
}

void game_display::set_playing_team(size_t teamindex)
{
	assert(teamindex < teams_.size());
	activeTeam_ = teamindex;
	invalidate_game_status();
}

void game_display::begin_game()
{
	in_game_ = true;
	create_buttons();
	invalidate_all();
}

namespace {
	const int chat_message_border = 5;
	const int chat_message_x = 10;
	const int chat_message_y = 10;
	const SDL_Color chat_message_colour = {255,255,255,255};
	const SDL_Color chat_message_bg     = {0,0,0,140};
}

void game_display::add_chat_message(const time_t& time, const std::string& speaker,
		int side, const std::string& message, events::chat_handler::MESSAGE_TYPE type,
		bool bell)
{
	const bool whisper = speaker.find("whisper: ") == 0;
	std::string sender = speaker;
	if (whisper) {
		sender.assign(speaker, 9, speaker.size());
	}
	if (!preferences::parse_should_show_lobby_join(sender, message)) return;
	if (preferences::is_ignored(sender)) return;

	preferences::parse_admin_authentication(sender, message);

	if (bell) {
		if ((type == events::chat_handler::MESSAGE_PRIVATE && (!is_observer() || whisper))
			|| utils::word_match(message, preferences::login())) {
			sound::play_UI_sound(game_config::sounds::receive_message_highlight);
		} else if (preferences::is_friend(sender)) {
			sound::play_UI_sound(game_config::sounds::receive_message_friend);
		} else if (sender == "server") {
			sound::play_UI_sound(game_config::sounds::receive_message_server);
		} else {
			sound::play_UI_sound(game_config::sounds::receive_message);
		}
	}

	bool action = false;

	std::string msg;

	if (message.find("/me ") == 0) {
		msg.assign(message, 4, message.size());
		action = true;
	} else {
		msg += message;
	}

	try {
		// We've had a joker who send an invalid utf-8 message to crash clients
		// so now catch the exception and ignore the message.
		msg = font::word_wrap_text(msg,font::SIZE_SMALL,map_outside_area().w*3/4);
	} catch (utils::invalid_utf8_exception&) {
		ERR_NG << "Invalid utf-8 found, chat message is ignored.\n";
		return;
	}

	int ypos = chat_message_x;
	for(std::vector<chat_message>::const_iterator m = chat_messages_.begin(); m != chat_messages_.end(); ++m) {
		ypos += std::max(font::get_floating_label_rect(m->handle).h,
			font::get_floating_label_rect(m->speaker_handle).h);
	}
	SDL_Color speaker_colour = {255,255,255,255};
	if(side >= 1) {
		speaker_colour = int_to_color(team::get_side_color_range(side).mid());
	}

	SDL_Color message_colour = chat_message_colour;
	std::stringstream str;
	std::stringstream message_str;

	if(type ==  events::chat_handler::MESSAGE_PUBLIC) {
		if(action) {
			str << "<" << speaker << " " << msg << ">";
			message_colour = speaker_colour;
			message_str << " ";
		} else {
			if (!speaker.empty())
				str << "<" << speaker << ">";
			message_str << msg;
		}
	} else {
		if(action) {
			str << "*" << speaker << " " << msg << "*";
			message_colour = speaker_colour;
			message_str << " ";
		} else {
			if (!speaker.empty())
				str << "*" << speaker << "*";
			message_str << msg;
		}
	}

	// Prepend message with timestamp.
	std::stringstream message_complete;
	message_complete << preferences::get_chat_timestamp(time) << str.str();

	const SDL_Rect rect = map_outside_area();
	int speaker_handle = font::add_floating_label(message_complete.str(),
		font::SIZE_SMALL, speaker_colour, rect.x + chat_message_x,
		rect.y + ypos, 0, 0, -1, rect, font::LEFT_ALIGN, &chat_message_bg,
		chat_message_border, font::ANCHOR_LABEL_SCREEN, false);

	int message_handle = font::add_floating_label(message_str.str(),
		font::SIZE_SMALL, message_colour,
		rect.x + chat_message_x + font::get_floating_label_rect(speaker_handle).w,
		rect.y + ypos, 0, 0, -1, rect, font::LEFT_ALIGN, &chat_message_bg,
		chat_message_border, font::ANCHOR_LABEL_SCREEN, false);

	// Send system notification if appropriate.
	send_notification(speaker, message);

	chat_messages_.push_back(chat_message(speaker_handle,message_handle));

	prune_chat_messages();
}

void game_display::prune_chat_messages(bool remove_all)
{
	unsigned message_ttl = remove_all ? 0 : 1200000;
	unsigned max_chat_messages = preferences::chat_lines();
	int movement = 0;

	while (!chat_messages_.empty() &&
	       (chat_messages_.front().created_at + message_ttl < SDL_GetTicks() ||
	        chat_messages_.size() > max_chat_messages))
	{
		const chat_message &old = chat_messages_.front();
		movement += font::get_floating_label_rect(old.handle).h;
		font::remove_floating_label(old.speaker_handle);
		font::remove_floating_label(old.handle);
		chat_messages_.erase(chat_messages_.begin());
	}

	BOOST_FOREACH (const chat_message &cm, chat_messages_) {
		font::move_floating_label(cm.speaker_handle, 0, - movement);
		font::move_floating_label(cm.handle, 0, - movement);
	}
}

game_display *game_display::singleton_ = NULL;

