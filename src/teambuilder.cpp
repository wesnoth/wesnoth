/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "teambuilder.hpp"

#include "actions/create.hpp"
#include "config.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "resources.hpp"
#include "gettext.hpp"
#include "game_errors.hpp"
#include "formula/string_utils.hpp"

#include <deque>
#include <vector>

static lg::log_domain log_engine_tc("engine/team_construction");
#define ERR_NG_TC LOG_STREAM(err, log_engine_tc)
#define WRN_NG_TC LOG_STREAM(warn, log_engine_tc)
#define LOG_NG_TC LOG_STREAM(info, log_engine_tc)
#define DBG_NG_TC LOG_STREAM(debug, log_engine_tc)

class team_builder {
public:
	team_builder(const config& side_cfg, std::vector<team>& teams,
		     const config& level, game_board& board, int num)
		: gold_info_ngold_(0)
		, leader_configs_()
		, level_(level)
		, board_(board)
		, player_exists_(false)
		, seen_ids_()
		, side_(num)
		, side_cfg_(side_cfg)
		, t_(nullptr)
		, teams_(teams)
		, unit_configs_()
	{
	}

	void build_team_stage_one()
	{
		//initialize the context variables and flags, find relevant tags, set up everything
		init();

		//find out the correct qty of gold and handle gold carryover.
		gold();

		//create a new instance of team and push it to back of resources::gameboard->teams() vector
		new_team();

		assert(t_!=nullptr);

		//set team objectives if necessary
		objectives();

		// If the game state specifies additional units that can be recruited by the player, add them.
		previous_recruits();

		//place leader
		leader();

		//prepare units, populate obvious recall lists elements
		prepare_units();

	}


	void build_team_stage_two()
	{
		//place units
		//this is separate stage because we need to place units only after every other team is constructed
		place_units();

	}

protected:

	int gold_info_ngold_;
	std::deque<config> leader_configs_;
	//only used for objectives
	const config &level_;
	game_board &board_;
	//only used for debug message
	bool player_exists_;
	std::set<std::string> seen_ids_;
	int side_;
	const config &side_cfg_;
	team *t_;
	std::vector<team> &teams_;
	std::vector<const config*> unit_configs_;

	void log_step(const char *s) const
	{
		LOG_NG_TC << "team "<<side_<<" construction: "<< s << std::endl;
	}


	void init()
	{
		if (side_cfg_["side"].to_int(side_) != side_) {
			ERR_NG_TC << "found invalid side=" << side_cfg_["side"].to_int(side_) << " in definition of side number " << side_ << std::endl;
		}
		t_ = &teams_[side_ - 1];
		log_step("init");

		//track whether a [player] tag with persistence information exists (in addition to the [side] tag)
		player_exists_ = false;

		if(board_.map().empty()) {
			throw game::load_game_failed("Map not found");
		}

		DBG_NG_TC << "snapshot: " << utils::bool_string(player_exists_) <<std::endl;

		unit_configs_.clear();
		seen_ids_.clear();

	}


	void gold()
	{
		log_step("gold");

		gold_info_ngold_ = side_cfg_["gold"];

		DBG_NG_TC << "set gold to '" << gold_info_ngold_ << "'\n";
	}


	void new_team()
	{
		log_step("new team");
		t_->build(side_cfg_, board_.map(), gold_info_ngold_);
	}


	void objectives()
	{
		log_step("objectives");
		// If this team has no objectives, set its objectives
		// to the level-global "objectives"
		// this is only used by the default mp 'Defeat enemy leader' objectives
		if (t_->objectives().empty())
			t_->set_objectives(level_["objectives"], false);
	}


	void previous_recruits()
	{
		log_step("previous recruits");
		// If the game state specifies units that
		// can be recruited for the player, add them.
		if (!side_cfg_) return;
		if (const config::attribute_value *v = side_cfg_.get("previous_recruits")) {
			for (const std::string &rec : utils::split(*v)) {
				DBG_NG_TC << "adding previous recruit: " << rec << '\n';
				t_->add_recruit(rec);
			}
		}
	}




	void handle_unit(const config &u, const char *origin)
	{
		DBG_NG_TC
			<< "unit from "<<origin
			<< ": type=["<<u["type"]
			<< "] id=["<<u["id"]
			<< "] placement=["<<u["placement"]
			<< "] x=["<<u["x"]
			<< "] y=["<<u["y"]
			<<"]"<< std::endl;

		if (u["type"].empty()) {
			WRN_NG_TC << "warning: when building level, skipping a unit (id=[" << u["id"] << "]) from " << origin
			<< " with no type information,\n"
			<< "for side:\n" << side_cfg_.debug() << std::endl;

			return ;
		}

		const std::string &id = u["id"];
		if (!id.empty()) {
			if ( seen_ids_.find(id)!=seen_ids_.end() ) {
				//seen before
				config u_tmp = u;
				u_tmp["side"] = std::to_string(side_);
				t_->recall_list().add(unit_ptr(new unit(u_tmp,true)));
			} else {
				//not seen before
				unit_configs_.push_back(&u);
				seen_ids_.insert(id);
			}

		} else {
			unit_configs_.push_back(&u);
		}
	}

	void handle_leader(const config &leader)
	{
		// Make a persistent copy of the config.
		leader_configs_.push_back(leader);
		config & stored = leader_configs_.back();

		// Remove the attributes used to define a side.
		for (const std::string & attr : team::attributes) {
			stored.remove_attribute(attr);
		}

		// Provide some default values, if not specified.
		config::attribute_value &a1 = stored["canrecruit"];
		if (a1.blank()) a1 = true;
		config::attribute_value &a2 = stored["placement"];
		if (a2.blank()) a2 = "map,leader";

		// Add the leader to the list of units to create.
		handle_unit(stored, "leader_cfg");
	}

	void leader()
	{
		log_step("leader");
		// If this side tag describes the leader of the side, we can simply add it to front of unit queue
		// there was a hack: if this side tag describes the leader of the side,
		// we may replace the leader with someone from recall list who can recruit, but take positioning from [side]
		// this hack shall be removed, since it messes up with 'multiple leaders'

		// If this side tag describes the leader of the side
		if (side_cfg_.has_attribute("type") && side_cfg_["type"] != "null" ) {
			handle_leader(side_cfg_);
		}
		for (const config &l : side_cfg_.child_range("leader")) {
			handle_leader(l);
		}
	}


	void prepare_units()
	{
		//if this is a start-of-scenario save then  playcampaign.cpp merged
		//units in [replay_start][side] merged with [side] already
		//units that are in '[scenario][side]' are 'first'

		//for create-or-recall semantics to work: for each unit with non-empty
		//id, unconditionally put OTHER, later, units with same id directly to
		//recall list, not including them in unit_configs_
		for (const config &su : side_cfg_.child_range("unit")) {
			handle_unit(su, "side_cfg");
		}
	}


	void place_units()
	{
		log_step("place units");
		unit_creator uc(*t_, board_.map().starting_position(side_), &board_);
		uc
			.allow_add_to_recall(true)
			.allow_discover(true)
			.allow_get_village(false)
			.allow_invalidate(false)
			.allow_rename_side(true)
			.allow_show(false);

		for (const config *u : unit_configs_) {
			try {
				uc.add_unit(*u);
			}
			catch (const unit_type::error& e) {
				ERR_NG_TC << e.what() << "\n";
			}
		}
	}

};

team_builder_ptr create_team_builder(const config& side_cfg,
					 std::vector<team>& teams,
					 const config& level, game_board& board, int num)
{
	return team_builder_ptr(new team_builder(side_cfg, teams, level, board, num));
}

void build_team_stage_one(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_one();
}

void build_team_stage_two(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_two();
}
