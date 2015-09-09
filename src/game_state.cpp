/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
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

#include "game_board.hpp"
#include "game_data.hpp"
#include "game_events/manager.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"
#include "play_controller.hpp"
#include "game_preferences.hpp"
#include "random_new_deterministic.hpp"
#include "reports.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "teambuilder.hpp"
#include "unit.hpp"
#include "whiteboard/manager.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <SDL_timer.h>

#include <algorithm>
#include <set>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

static void no_op() {}

game_state::game_state(const config & level, play_controller & pc, const tdata_cache & tdata) :
	gamedata_(level),
	board_(tdata, level),
	tod_manager_(level),
	pathfind_manager_(),
	reports_(new reports()),
	lua_kernel_(),
	events_manager_(),
	player_number_(level["playing_team"].to_int() + 1),
	init_side_done_(level["init_side_done"].to_bool(false)),
	start_event_fired_(!level["playing_team"].empty()),
	first_human_team_(-1)
{
	init(pc.ticks(), pc, level);
}
game_state::game_state(const config & level, play_controller & pc, game_board& board) :
	gamedata_(level),
	board_(board),
	tod_manager_(level),
	pathfind_manager_(new pathfind::manager(level)),
	reports_(new reports()),
	lua_kernel_(new game_lua_kernel(NULL, *this, pc, *reports_)),
	events_manager_(),
	player_number_(level["playing_team"].to_int() + 1),
	init_side_done_(level["init_side_done"].to_bool(false)),
	first_human_team_(-1)
{
	init(pc.ticks(), pc, level);

	game_events_resources_ = boost::make_shared<game_events::t_context>(lua_kernel_.get(), this, static_cast<game_display*>(NULL), &gamedata_, &board_.units_, &no_op, boost::bind(&play_controller::current_side, &pc));

	events_manager_.reset(new game_events::manager(level, game_events_resources_));

}


game_state::~game_state() {}

static int placing_score(const config& side, const gamemap& map, const map_location& pos)
{
	int positions = 0, liked = 0;
	const t_translation::t_list terrain = t_translation::read_list(side["terrain_liked"]);

	for(int i = pos.x-8; i != pos.x+8; ++i) {
		for(int j = pos.y-8; j != pos.y+8; ++j) {
			const map_location pos(i,j);
			if(map.on_board(pos)) {
				++positions;
				if(std::count(terrain.begin(),terrain.end(),map[pos])) {
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
	BOOST_FOREACH(const config &side, level.child_range("side"))
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

void game_state::init(const int ticks, play_controller & pc, const config& level)
{
	int ticks1 = SDL_GetTicks();
	if (level["modify_placing"].to_bool()) {
		LOG_NG << "modifying placing..." << std::endl;
		place_sides_in_preferred_locations(level);
	}

	LOG_NG << "initialized time of day regions... "    << (SDL_GetTicks() - ticks) << std::endl;
	BOOST_FOREACH(const config &t, level.child_range("time_area")) {
		tod_manager_.add_time_area(board_.map(),t);
	}

	LOG_NG << "initialized teams... "    << (SDL_GetTicks() - ticks) << std::endl;
	//loadscreen::start_stage("init teams");

	board_.teams_.resize(level.child_count("side"));

	std::vector<team_builder_ptr> team_builders;

	int team_num = 0;
	BOOST_FOREACH(const config &side, level.child_range("side"))
	{
		if (first_human_team_ == -1) {
			const std::string &controller = side["controller"];
			if (controller == "human") {
				first_human_team_ = team_num;
			}
		}
		team_builder_ptr tb_ptr = create_team_builder(side,
			board_.teams_, level, board_);
		++team_num;
		build_team_stage_one(tb_ptr);
		team_builders.push_back(tb_ptr);
	}
	{
		//sync traits of start units and the random start time.
		random_new::set_random_determinstic deterministic(gamedata_.rng());

		tod_manager_.resolve_random(*random_new::generator);

		BOOST_FOREACH(team_builder_ptr tb_ptr, team_builders)
		{
			build_team_stage_two(tb_ptr);
		}
		for(size_t i = 0; i < board_.teams_.size(); i++) {
			// Labels from players in your ignore list default to hidden
			if(preferences::is_ignored(board_.teams_[i].current_player())) {
				std::string label_cat = "side:" + str_cast(i + 1);
				board_.hidden_label_categories_ref().push_back(label_cat);
			}
		}
	}
	
	std::cerr << "Initilizing teams took " << SDL_GetTicks() - ticks1 << " ticks.\n";

	pathfind_manager_.reset(new pathfind::manager(level));

	lua_kernel_.reset(new game_lua_kernel(NULL, *this, pc, *reports_));

	game_events_resources_ = boost::make_shared<game_events::t_context>(lua_kernel_.get(), this, static_cast<game_display*>(NULL), &gamedata_, &board_.units_, &no_op, boost::bind(&play_controller::current_side, &pc));

	events_manager_.reset(new game_events::manager(level, game_events_resources_));
	
	std::cerr << "Initilizing total took " << SDL_GetTicks() - ticks1 << " ticks.\n";


}

void game_state::bind(wb::manager * whiteboard, game_display * gd)
{
	if (whiteboard) {
		game_events_resources_->on_gamestate_change = boost::bind(&wb::manager::on_gamestate_change, whiteboard);
	} else {
		game_events_resources_->on_gamestate_change = &no_op;
	}
	set_game_display(gd);
}

void game_state::set_game_display(game_display * gd)
{
	lua_kernel_->set_game_display(gd);
	game_events_resources_->screen = gd;
}

void game_state::write(config& cfg) const
{
	cfg["init_side_done"] = init_side_done_;
	if(gamedata_.phase() == game_data::PLAY) {
		cfg["playing_team"] = player_number_ - 1;
	}

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

	return pathfind::find_vacant_tile(leader_loc, pathfind::VACANT_CASTLE, NULL,
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
	const team & view_team = board_.teams()[side-1];

	if ( view_team.shrouded(recruit_loc) )
		return false;

	if ( board_.has_visible_unit(recruit_loc, view_team) )
		return false;

	castle_cost_calculator calc(map, view_team);
	// The limit computed in the third argument is more than enough for
	// any convex castle on the map. Strictly speaking it could be
	// reduced to sqrt(map.w()**2 + map.h()**2).
	pathfind::plain_route rt =
		pathfind::a_star_search(leader_loc, recruit_loc, map.w()+map.h(), &calc,
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

game_events::wmi_container& game_state::get_wml_menu_items()
{
	return this->events_manager_->wml_menu_items_;
}

const game_events::wmi_container& game_state::get_wml_menu_items() const
{
	return this->events_manager_->wml_menu_items_;
}
