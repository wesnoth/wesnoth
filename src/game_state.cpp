/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_state.hpp"

#include "actions/undo.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "game_events/manager.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"
#include "play_controller.hpp"
#include "preferences/game.hpp"
#include "random_deterministic.hpp"
#include "reports.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "teambuilder.hpp"
#include "units/unit.hpp"
#include "whiteboard/manager.hpp"
#include "gui/dialogs/loading_screen.hpp"

#include "utils/functional.hpp"
#include <SDL_timer.h>

#include <algorithm>
#include <set>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

game_state::game_state(const config & level, play_controller & pc, const ter_data_cache & tdata) :
	gamedata_(level),
	board_(tdata, level),
	tod_manager_(level),
	pathfind_manager_(new pathfind::manager(level)),
	reports_(new reports()),
	lua_kernel_(new game_lua_kernel(*this, pc, *reports_)),
	ai_manager_(),
	events_manager_(new game_events::manager()),
	//TODO: this construct units (in dimiss undo action) but resrouces:: are not available yet,
	//      so we might want to move the innitialisation of undo_stack_ to game_state::init
	undo_stack_(new actions::undo_list(level.child("undo_stack"))),
	player_number_(level["playing_team"].to_int() + 1),
	init_side_done_(level["init_side_done"].to_bool(false)),
	start_event_fired_(!level["playing_team"].empty()),
	server_request_number_(level["server_request_number"].to_int()),
	first_human_team_(-1)
{
	lua_kernel_->load_core();
	if(const config& endlevel_cfg = level.child("end_level_data")) {
		end_level_data el_data;
		el_data.read(endlevel_cfg);
		el_data.transient.carryover_report = false;
		end_level_data_ = el_data;
	}
}

game_state::game_state(const config & level, play_controller & pc, game_board& board) :
	gamedata_(level),
	board_(board),
	tod_manager_(level),
	pathfind_manager_(new pathfind::manager(level)),
	reports_(new reports()),
	lua_kernel_(new game_lua_kernel(*this, pc, *reports_)),
	ai_manager_(),
	events_manager_(new game_events::manager()),
	player_number_(level["playing_team"].to_int() + 1),
	end_level_data_(),
	init_side_done_(level["init_side_done"].to_bool(false)),
	start_event_fired_(!level["playing_team"].empty()),
	server_request_number_(level["server_request_number"].to_int()),
	first_human_team_(-1)
{
	lua_kernel_->load_core();
	events_manager_->read_scenario(level);
	if(const config& endlevel_cfg = level.child("end_level_data")) {
		end_level_data el_data;
		el_data.read(endlevel_cfg);
		el_data.transient.carryover_report = false;
		end_level_data_ = el_data;
	}
}


game_state::~game_state() {}

static int placing_score(const config& side, const gamemap& map, const map_location& pos)
{
	int positions = 0, liked = 0;
	const t_translation::ter_list terrain = t_translation::read_list(side["terrain_liked"]);

	for(int i = -8; i != 8; ++i) {
		for(int j = -8; j != +8; ++j) {
			const map_location pos2  = pos.plus(i, j);
			if(map.on_board(pos2)) {
				++positions;
				if(std::count(terrain.begin(),terrain.end(),map[pos2])) {
					++liked;
				}
			}
		}
	}

	return (100*liked)/positions;
}

struct placing_info {

	placing_info() :
		side(0),
		score(0),
		pos()
	{
	}

	int side, score;
	map_location pos;
};

static bool operator<(const placing_info& a, const placing_info& b) { return a.score > b.score; }


void game_state::place_sides_in_preferred_locations(const config& level)
{
	std::vector<placing_info> placings;

	int num_pos = board_.map().num_valid_starting_positions();

	int side_num = 1;
	for(const config &side : level.child_range("side"))
	{
		for(int p = 1; p <= num_pos; ++p) {
			const map_location& pos = board_.map().starting_position(p);
			int score = placing_score(side, board_.map(), pos);
			placing_info obj;
			obj.side = side_num;
			obj.score = score;
			obj.pos = pos;
			placings.push_back(obj);
		}
		++side_num;
	}

	std::stable_sort(placings.begin(),placings.end());
	std::set<int> placed;
	std::set<map_location> positions_taken;

	for (std::vector<placing_info>::const_iterator i = placings.begin(); i != placings.end() && int(placed.size()) != side_num - 1; ++i) {
		if(placed.count(i->side) == 0 && positions_taken.count(i->pos) == 0) {
			placed.insert(i->side);
			positions_taken.insert(i->pos);
			board_.map_->set_starting_position(i->side,i->pos);
			LOG_NG << "placing side " << i->side << " at " << i->pos << std::endl;
		}
	}
}

void game_state::init(const config& level, play_controller & pc)
{
	events_manager_->read_scenario(level);
	gui2::dialogs::loading_screen::progress(loading_stage::init_teams);
	if (level["modify_placing"].to_bool()) {
		LOG_NG << "modifying placing..." << std::endl;
		place_sides_in_preferred_locations(level);
	}

	LOG_NG << "initialized time of day regions... "    << (SDL_GetTicks() - pc.ticks()) << std::endl;
	for (const config &t : level.child_range("time_area")) {
		tod_manager_.add_time_area(board_.map(),t);
	}

	LOG_NG << "initialized teams... "    << (SDL_GetTicks() - pc.ticks()) << std::endl;

	board_.teams_.resize(level.child_count("side"));

	std::vector<team_builder_ptr> team_builders;

	int team_num = 0;
	for (const config &side : level.child_range("side"))
	{
		if (first_human_team_ == -1) {
			const std::string &controller = side["controller"];
			if (controller == "human" && side["is_local"].to_bool(true)) {
				first_human_team_ = team_num;
			}
		}
		++team_num;
		team_builder_ptr tb_ptr = create_team_builder(side,
			board_.teams_, level, board_, team_num);
		build_team_stage_one(tb_ptr);
		team_builders.push_back(tb_ptr);
	}
	//Initilize the lua kernel before the units are created.
	lua_kernel_->initialize(level);

	{
		//sync traits of start units and the random start time.
		randomness::set_random_determinstic deterministic(gamedata_.rng());

		tod_manager_.resolve_random(*randomness::generator);

		for(team_builder_ptr tb_ptr : team_builders)
		{
			build_team_stage_two(tb_ptr);
		}
		for(size_t i = 0; i < board_.teams_.size(); i++) {
			// Labels from players in your ignore list default to hidden
			if(preferences::is_ignored(board_.teams_[i].current_player())) {
				std::string label_cat = "side:" + std::to_string(i + 1);
				board_.hidden_label_categories_ref().push_back(label_cat);
			}
		}
	}
}

void game_state::set_game_display(game_display * gd)
{
	lua_kernel_->set_game_display(gd);
}

void game_state::write(config& cfg) const
{
	cfg["init_side_done"] = init_side_done_;
	if(gamedata_.phase() == game_data::PLAY) {
		cfg["playing_team"] = player_number_ - 1;
	}
	cfg["server_request_number"] = server_request_number_;
	//Call the lua save_game functions
	lua_kernel_->save_game(cfg);

	//Write the game events.
	events_manager_->write_events(cfg);

	//Write the map, unit_map, and teams info
	board_.write_config(cfg);

	//Write the tod manager, and time areas
	cfg.merge_with(tod_manager_.to_config());

	//write out the current state of the map
	cfg.merge_with(pathfind_manager_->to_config());

	//Write the game data, including wml vars
	gamedata_.write_snapshot(cfg);

	// Preserve the undo stack so that fog/shroud clearing is kept accurate.
	undo_stack_->write(cfg.add_child("undo_stack"));

	if(end_level_data_.get_ptr() != nullptr) {
		end_level_data_->write(cfg.add_child("end_level_data"));
	}
}

namespace {
	struct castle_cost_calculator : pathfind::cost_calculator
	{
		castle_cost_calculator(const gamemap& map, const team & view_team) :
			map_(map),
			viewer_(view_team),
			use_shroud_(view_team.uses_shroud())
		{}

		virtual double cost(const map_location& loc, const double) const
		{
			if(!map_.is_castle(loc))
				return 10000;

			if ( use_shroud_ && viewer_.shrouded(loc) )
				return 10000;

			return 1;
		}

	private:
		const gamemap& map_;
		const team& viewer_;
		const bool use_shroud_; // Allows faster checks when shroud is disabled.
	};
}//anonymous namespace


/**
 * Checks to see if a leader at @a leader_loc could recruit somewhere.
 * This takes into account terrain, shroud (for side @a side), and the presence
 * of visible units.
 * The behavior for an invalid @a side is subject to change for future needs.
 */
bool game_state::can_recruit_from(const map_location& leader_loc, int side) const
{
	const gamemap& map = board_.map();

	if( !map.is_keep(leader_loc) )
		return false;

	if ( side < 1  ||  board_.teams().size() < static_cast<size_t>(side) ) {
		// Invalid side specified.
		// Currently this cannot happen, but it could conceivably be used in
		// the future to request that shroud and visibility be ignored. Until
		// that comes to pass, just return.
 		return false;
	}

	return pathfind::find_vacant_tile(leader_loc, pathfind::VACANT_CASTLE, nullptr,
	                                  &(board_.teams())[side-1])
	       != map_location::null_location();
}

bool game_state::can_recruit_from(const unit& leader) const
{
	return can_recruit_from(leader.get_location(), leader.side());
}


/**
 * Checks to see if a leader at @a leader_loc could recruit on @a recruit_loc.
 * This takes into account terrain, shroud (for side @a side), and whether or
 * not there is already a visible unit at recruit_loc.
 * The behavior for an invalid @a side is subject to change for future needs.
 */
bool game_state::can_recruit_on(const map_location& leader_loc, const map_location& recruit_loc, int side) const
{
	const gamemap& map = board_.map();

	if( !map.is_castle(recruit_loc) )
		return false;

	if( !map.is_keep(leader_loc) )
		return false;

	if ( side < 1  ||  board_.teams().size() < static_cast<size_t>(side) ) {
		// Invalid side specified.
		// Currently this cannot happen, but it could conceivably be used in
		// the future to request that shroud and visibility be ignored. Until
		// that comes to pass, just return.
		return false;
	}
	const team & view_team = board_.get_team(side);

	if ( view_team.shrouded(recruit_loc) )
		return false;

	if ( board_.has_visible_unit(recruit_loc, view_team) )
		return false;

	castle_cost_calculator calc(map, view_team);
	// The limit computed in the third argument is more than enough for
	// any convex castle on the map. Strictly speaking it could be
	// reduced to sqrt(map.w()**2 + map.h()**2).
	pathfind::plain_route rt =
		pathfind::a_star_search(leader_loc, recruit_loc, map.w()+map.h(), calc,
		                        map.w(), map.h());
	return !rt.steps.empty();
}

bool game_state::can_recruit_on(const unit& leader, const map_location& recruit_loc) const
{
	return can_recruit_on(leader.get_location(), recruit_loc, leader.side());
}

bool game_state::side_can_recruit_on(int side, map_location hex) const
{
	unit_map::const_iterator leader = board_.units().find(hex);
	if ( leader != board_.units().end() ) {
		return leader->can_recruit() && leader->side() == side && can_recruit_from(*leader);
	} else {
		// Look for a leader who can recruit on last_hex.
		for ( leader = board_.units().begin(); leader != board_.units().end(); ++leader) {
			if ( leader->can_recruit() && leader->side() == side && can_recruit_on(*leader, hex) ) {
				return true;
			}
		}
	}
	// No leader found who can recruit at last_hex.
	return false;
}

game_events::wmi_manager& game_state::get_wml_menu_items()
{
	return this->events_manager_->wml_menu_items();
}

const game_events::wmi_manager& game_state::get_wml_menu_items() const
{
	return this->events_manager_->wml_menu_items();
}
