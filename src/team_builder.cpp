//* $Id$ */
/*
  Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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
 * Maintain status of a game, load&save games.
 */

#include "global.hpp"
#include "config.hpp"

#include "actions.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "game_preferences.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "map.hpp"
#include "pathfind/pathfind.hpp"
#include "whiteboard/side_actions.hpp"

#include "team_builder.hpp"

#include <boost/bind.hpp>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_engine_tc("engine/team_construction");
#define ERR_NG_TC LOG_STREAM(err, log_engine_tc)
#define WRN_NG_TC LOG_STREAM(warn, log_engine_tc)
#define LOG_NG_TC LOG_STREAM(info, log_engine_tc)
#define DBG_NG_TC LOG_STREAM(debug, log_engine_tc)


team_builder::team_builder(const config& side_cfg,
						   const std::string &save_id, t_teams& teams,
						   const config& level, gamemap& map, unit_map& units,
						   bool snapshot, const config &starting_pos)
	: gold_info_ngold_(0)
	, gold_info_add_(false)
	, leader_configs_()
	, level_(level)
	, map_(map)
	, player_cfg_(NULL)
	, player_exists_(false)
	, save_id_(save_id)
	, seen_ids_()
	, side_(0)
	, side_cfg_(side_cfg)
	, snapshot_(snapshot)
	, starting_pos_(starting_pos)
	, t_(NULL)
	, teams_(teams)
	, unit_configs_()
	, units_(units)
{
}

void team_builder::build_team_stage_one()
{
	//initialize the context variables and flags, find relevant tags, set up everything
	init();

	//find out the correct qty of gold and handle gold carryover.
	gold();

	//create a new instance of team and push it to back of resources::teams vector
	new_team();

	assert(t_!=NULL);

	//set team objectives if necessary
	objectives();

	// If the game state specifies additional units that can be recruited by the player, add them.
	previous_recruits();

	//place leader
	leader();

	//prepare units, populate obvious recall lists elements
	prepare_units();

}


void team_builder::build_team_stage_two()
{
	//place units
	//this is separate stage because we need to place units only after every other team is constructed
	place_units();

}

void team_builder::log_step(const char *s) const
{
	LOG_NG_TC << "team "<<side_<<" construction: "<< s << std::endl;
}


void team_builder::init()
{

	static const config::t_token & z_side( generate_safe_static_const_t_interned(n_token::t_token("side")) );
	static const config::t_token & z_controller( generate_safe_static_const_t_interned(n_token::t_token("controller")) );
	static const config::t_token & z_human( generate_safe_static_const_t_interned(n_token::t_token("human")) );
	static const config::t_token & z_network( generate_safe_static_const_t_interned(n_token::t_token("network")) );
	static const config::t_token & z_network_ai( generate_safe_static_const_t_interned(n_token::t_token("network_ai")) );
	static const config::t_token & z_human_ai( generate_safe_static_const_t_interned(n_token::t_token("human_ai")) );
	static const config::t_token & z_persistent( generate_safe_static_const_t_interned(n_token::t_token("persistent")) );
	static const config::t_token & z_player( generate_safe_static_const_t_interned(n_token::t_token("player")) );
	static const config::t_token & z_save_id( generate_safe_static_const_t_interned(n_token::t_token("save_id")) );
	static const config::t_token & z_true( generate_safe_static_const_t_interned(n_token::t_token("true")) );
	static const config::t_token & z_false( generate_safe_static_const_t_interned(n_token::t_token("false")) );

	side_ = side_cfg_[z_side].to_int(1);
	if (unsigned(side_ - 1) >= teams_.size() || teams_[side_ - 1].side() != 0)
		throw config::error("Invalid side number.");
	t_ = &teams_[side_ - 1];

	log_step("init");

	player_cfg_ = NULL;
	//track whether a [player] tag with persistence information exists (in addition to the [side] tag)
	player_exists_ = false;

	if(map_.empty()) {
		throw game::load_game_failed("Map not found");
	}

	if(side_cfg_[z_controller] == z_human ||
	   side_cfg_[z_controller] == z_network ||
	   side_cfg_[z_controller] == z_network_ai ||
	   side_cfg_[z_controller] == z_human_ai ||
	   side_cfg_[z_persistent].to_bool())
		{
			player_exists_ = true;

			//if we have a snapshot, level contains team information
			//else, we look for [side] or [player] (deprecated) tags in starting_pos
			///@deprecated r37519 [player] instead of [side] in starting_pos
			if (snapshot_) {
				if (const config &c = level_.find_child(z_player,z_save_id,save_id_))  {
					player_cfg_ = &c;
				}
			} else {
				//at the start of scenario, get the persistence information from starting_pos
				if (const config &c =  starting_pos_.find_child(z_player,z_save_id,save_id_))  {
					player_cfg_ = &c;
				} else if (const config &c =  starting_pos_.find_child(z_side,z_save_id,save_id_))  {
					player_cfg_ = &c;
					player_exists_ = false; //there is only a [side] tag for this save_id in starting_pos
				} else {
					player_cfg_ = NULL;
					player_exists_ = false;
				}
			}
		}

	DBG_NG_TC << "save id: "<< save_id_ <<std::endl;
	DBG_NG_TC << "snapshot: "<< (player_exists_ ? z_true : z_false) <<std::endl;
	DBG_NG_TC << "player_cfg: "<< (player_cfg_==NULL ? "is null" : "is not null") <<std::endl;
	DBG_NG_TC << "player_exists: "<< (player_exists_ ? z_true : z_false) <<std::endl;

	unit_configs_.clear();
	seen_ids_.clear();

}

bool team_builder::use_player_cfg() const
{
	return (player_cfg_ != NULL) && (!snapshot_);
}

void team_builder::gold()
{
	static const config::t_token & z_gold( generate_safe_static_const_t_interned(n_token::t_token("gold")) );
	static const config::t_token & z_gold_add( generate_safe_static_const_t_interned(n_token::t_token("gold_add")) );
	static const config::t_token & z_default_gold_qty( generate_safe_static_const_t_interned(n_token::t_token("100")) );

	log_step("gold");

	n_token::t_token gold = side_cfg_[z_gold].token();
	if(gold.empty()) {
		gold = z_default_gold_qty;
	}

	DBG_NG_TC << "found gold: '" << *gold << "'\n";

	gold_info_ngold_ = lexical_cast_default<int>(*gold);

	/* This is the gold carry-over mechanism for subsequent campaign
	   scenarios. Snapshots and replays are loaded from savegames and
	   got their own gold information, which must not be altered here
	*/

	//true  - carryover gold is added to the start_gold.
	//false - the max of the two is taken as start_gold.
	gold_info_add_ = side_cfg_[z_gold_add].to_bool();

	if (use_player_cfg()) {
		try {
			int player_gold = (*player_cfg_)[z_gold];
			if (!player_exists_) {
				//if we get the persistence information from [side], carryover gold is already sorted
				gold_info_ngold_ = player_gold;
				gold_info_add_ = (*player_cfg_)[z_gold_add].to_bool();
			} else if ((*player_cfg_)[z_gold_add].to_bool()) {
				gold_info_ngold_ +=  player_gold;
				gold_info_add_ = true;
			} else if(player_gold >= gold_info_ngold_) {
				gold_info_ngold_ = player_gold;
			}
		} catch (config::error&) {
			ERR_NG_TC << "player tag for " << save_id_ << " does not have gold information\n";
		}
	}

	DBG_NG_TC << "set gold to '" << gold_info_ngold_ << "'\n";
	DBG_NG_TC << "set gold add flag to '" << gold_info_add_ << "'\n";
}


void team_builder::new_team()
{
	log_step("new team");
	t_->build(side_cfg_, map_, gold_info_ngold_);
	t_->set_gold_add(gold_info_add_);
}


void team_builder::objectives()
{
	static const config::t_token & z_objectives( generate_safe_static_const_t_interned(n_token::t_token("objectives")) );

	log_step("objectives");
	// If this team has no objectives, set its objectives
	// to the level-global z_objectives
	if (t_->objectives().empty())
		t_->set_objectives(level_[z_objectives].t_str(), false);
}


void team_builder::previous_recruits()
{
	static const config::t_token & z_previous_recruits( generate_safe_static_const_t_interned(n_token::t_token("previous_recruits")) );

	log_step("previous recruits");
	// If the game state specifies units that
	// can be recruited for the player, add them.
	if (!player_cfg_) return;
	if (const config::attribute_value *v = player_cfg_->get(z_previous_recruits)) {
		foreach (const config::t_token &rec, utils::split_token(v->token())) {
			DBG_NG_TC << "adding previous recruit: " << rec << '\n';
			t_->add_recruit(rec);
		}
	}
}




void team_builder::handle_unit(const config &u, const char *origin)
{
	static const config::t_token & z_type( generate_safe_static_const_t_interned(n_token::t_token("type")) );
	static const config::t_token & z_id( generate_safe_static_const_t_interned(n_token::t_token("id")) );
	static const config::t_token & z_placement( generate_safe_static_const_t_interned(n_token::t_token("placement")) );
	static const config::t_token & z_x( generate_safe_static_const_t_interned(n_token::t_token("x")) );
	static const config::t_token & z_y( generate_safe_static_const_t_interned(n_token::t_token("y")) );
	static const config::t_token & z_side( generate_safe_static_const_t_interned(n_token::t_token("side")) );

	DBG_NG_TC
		<< "unit from "<<origin
		<< ": type=["<<u[z_type]
		<< "] id=["<<u[z_id]
		<< "] placement=["<<u[z_placement]
		<< "] x=["<<u[z_x]
		<< "] y=["<<u[z_y]
		<<"]"<< std::endl;
	const config::attribute_value &id = u[z_id];
	if (!id.empty()) {
		if ( seen_ids_.find(id)!=seen_ids_.end() ) {
			//seen before
			config u_tmp = u;
			u_tmp[z_side] = str_cast(side_);
			unit new_unit(u_tmp, true);
			t_->recall_list().push_back(new_unit);
		} else {
			//not seen before
			unit_configs_.push_back(&u);
			seen_ids_.insert(id);
		}

	} else {
		unit_configs_.push_back(&u);
	}
}

void team_builder::handle_leader(const config &leader)
{
	static const config::t_token & z_canrecruit( generate_safe_static_const_t_interned(n_token::t_token("canrecruit")) );
	static const config::t_token & z_placement( generate_safe_static_const_t_interned(n_token::t_token("placement")) );

	leader_configs_.push_back(leader);

	config::attribute_value &a1 = leader_configs_.back()[z_canrecruit];
	if (a1.empty()) a1 = true;
	config::attribute_value &a2 = leader_configs_.back()[z_placement];
	if (a2.empty()) a2 = "map,leader";

	handle_unit(leader_configs_.back(), "leader_cfg");
}

void team_builder::leader()
{
	static const config::t_token & z_no_leader( generate_safe_static_const_t_interned(n_token::t_token("no_leader")) );
	static const config::t_token & z_controller( generate_safe_static_const_t_interned(n_token::t_token("controller")) );
	static const config::t_token & z_null( generate_safe_static_const_t_interned(n_token::t_token("null")) );
	static const config::t_token & z_leader( generate_safe_static_const_t_interned(n_token::t_token("leader")) );

	log_step("leader");
	// If this side tag describes the leader of the side, we can simply add it to front of unit queue
	// there was a hack: if this side tag describes the leader of the side,
	// we may replace the leader with someone from recall list who can recruit, but take positioning from [side]
	// this hack shall be removed, since it messes up with 'multiple leaders'

	// If this side tag describes the leader of the side
	if (!side_cfg_[z_no_leader].to_bool() && side_cfg_[z_controller] != z_null) {
		handle_leader(side_cfg_);
	}
	foreach (const config &l, side_cfg_.child_range(z_leader)) {
		handle_leader(l);
	}
}


void team_builder::prepare_units()
{
	static const config::t_token & z_unit( generate_safe_static_const_t_interned(n_token::t_token("unit")) );

	log_step("prepare units");
	if (use_player_cfg()) {
		//units in [replay_start][side] merged with [side]
		//only relevant in start-of-scenario saves, that's why !shapshot
		//units that are in '[scenario][side]' are 'first'
		//for create-or-recall semantics to work: for each unit with non-empty id, unconditionally put OTHER, later, units with same id directly to recall list, not including them in unit_configs_
		foreach(const config &u, (*player_cfg_).child_range(z_unit)) {
			handle_unit(u,"player_cfg");
		}

	} else {
		//units in [side]
		foreach (const config &su, side_cfg_.child_range(z_unit)) {
			handle_unit(su, "side_cfg");
		}
	}
}


void team_builder::place_units()
{
	static const config::t_token & z_income( generate_safe_static_const_t_interned(n_token::t_token("income")) );
	static const config::t_token & z_team_name( generate_safe_static_const_t_interned(n_token::t_token("team_name")) );
	static const config::t_token & z_user_team_name( generate_safe_static_const_t_interned(n_token::t_token("user_team_name")) );
	static const config::t_token & z_save_id( generate_safe_static_const_t_interned(n_token::t_token("save_id")) );
	static const config::t_token & z_current_player( generate_safe_static_const_t_interned(n_token::t_token("current_player")) );
	static const config::t_token & z_countdown_time( generate_safe_static_const_t_interned(n_token::t_token("countdown_time")) );
	static const config::t_token & z_action_bonus_count( generate_safe_static_const_t_interned(n_token::t_token("action_bonus_count")) );
	static const config::t_token & z_flag( generate_safe_static_const_t_interned(n_token::t_token("flag")) );
	static const config::t_token & z_flag_icon( generate_safe_static_const_t_interned(n_token::t_token("flag_icon")) );
	static const config::t_token & z_objectives( generate_safe_static_const_t_interned(n_token::t_token("objectives")) );
	static const config::t_token & z_objectives_changed( generate_safe_static_const_t_interned(n_token::t_token("objectives_changed")) );
	static const config::t_token & z_disallow_observers( generate_safe_static_const_t_interned(n_token::t_token("disallow_observers")) );
	static const config::t_token & z_allow_player( generate_safe_static_const_t_interned(n_token::t_token("allow_player")) );
	static const config::t_token & z_no_leader( generate_safe_static_const_t_interned(n_token::t_token("no_leader")) );
	static const config::t_token & z_hidden( generate_safe_static_const_t_interned(n_token::t_token("hidden")) );
	static const config::t_token & z_music( generate_safe_static_const_t_interned(n_token::t_token("music")) );
	static const config::t_token & z_color( generate_safe_static_const_t_interned(n_token::t_token("color")) );
	static const config::t_token & z_colour( generate_safe_static_const_t_interned(n_token::t_token("colour")) );
	static const config::t_token & z_ai_config( generate_safe_static_const_t_interned(n_token::t_token("ai_config")) );
	static const config::t_token & z_gold( generate_safe_static_const_t_interned(n_token::t_token("gold")) );
	static const config::t_token & z_start_gold( generate_safe_static_const_t_interned(n_token::t_token("start_gold")) );
	static const config::t_token & z_team_rgb( generate_safe_static_const_t_interned(n_token::t_token("team_rgb")) );
	static const config::t_token & z_village_gold( generate_safe_static_const_t_interned(n_token::t_token("village_gold")) );
	static const config::t_token & z_recall_cost( generate_safe_static_const_t_interned(n_token::t_token("recall_cost")) );
	static const config::t_token & z_controller( generate_safe_static_const_t_interned(n_token::t_token("controller")) );
	static const config::t_token & z_persistent( generate_safe_static_const_t_interned(n_token::t_token("persistent")) );
	static const config::t_token & z_share_view( generate_safe_static_const_t_interned(n_token::t_token("share_view")) );
	static const config::t_token & z_share_maps( generate_safe_static_const_t_interned(n_token::t_token("share_maps")) );
	static const config::t_token & z_recruit( generate_safe_static_const_t_interned(n_token::t_token("recruit")) );
	static const config::t_token & z_fog( generate_safe_static_const_t_interned(n_token::t_token("fog")) );
	static const config::t_token & z_shroud( generate_safe_static_const_t_interned(n_token::t_token("shroud")) );
	static const config::t_token & z_shroud_data( generate_safe_static_const_t_interned(n_token::t_token("shroud_data")) );
	static const config::t_token & z_scroll_to_leader( generate_safe_static_const_t_interned(n_token::t_token("scroll_to_leader")) );
	static const config::t_token & z_income_lock( generate_safe_static_const_t_interned(n_token::t_token("income_lock")) );
	static const config::t_token & z_gold_lock( generate_safe_static_const_t_interned(n_token::t_token("gold_lock")) );
	static const config::t_token & z_color_lock( generate_safe_static_const_t_interned(n_token::t_token("color_lock")) );
	static const config::t_token & z_team_lock( generate_safe_static_const_t_interned(n_token::t_token("team_lock")) );
	static const config::t_token & z_leader( generate_safe_static_const_t_interned(n_token::t_token("leader")) );
	static const config::t_token & z_random_leader( generate_safe_static_const_t_interned(n_token::t_token("random_leader")) );
	static const config::t_token & z_terrain_liked( generate_safe_static_const_t_interned(n_token::t_token("terrain_liked")) );
	static const config::t_token & z_allow_changes( generate_safe_static_const_t_interned(n_token::t_token("allow_changes")) );
	static const config::t_token & z_faction_name( generate_safe_static_const_t_interned(n_token::t_token("faction_name")) );
	static const config::t_token & z_user_description( generate_safe_static_const_t_interned(n_token::t_token("user_description")) );
	static const config::t_token & z_faction( generate_safe_static_const_t_interned(n_token::t_token("faction")) );

	static n_token::t_token const side_attrs[] = {
		z_income, z_team_name, z_user_team_name, z_save_id,
		z_current_player, z_countdown_time, z_action_bonus_count,
		z_flag, z_flag_icon, z_objectives, z_objectives_changed,
		z_disallow_observers, z_allow_player, z_no_leader,
		z_hidden, z_music, z_color, z_colour, z_ai_config, z_gold,
		z_start_gold, z_team_rgb, z_village_gold, z_recall_cost,
		z_controller, z_persistent, z_share_view,
		z_share_maps, z_recruit, z_fog, z_shroud, z_shroud_data,
		z_scroll_to_leader,
		// Multiplayer attributes.
		z_income_lock, z_gold_lock, z_color_lock, z_team_lock, z_leader,
		z_random_leader, z_terrain_liked,
		z_allow_changes, z_faction_name, z_user_description, z_faction };

	log_step("place units");
	foreach (const config *u, unit_configs_) {
		unit_creator uc(*t_,map_.starting_position(side_));
		uc
			.allow_add_to_recall(true)
			.allow_discover(true)
			.allow_get_village(true)
			.allow_invalidate(false)
			.allow_rename_side(true)
			.allow_show(false);

		config cfg = *u;
		foreach (const config::t_token & attr, side_attrs) {
			cfg.remove_attribute(attr);
		}
		uc.add_unit(cfg);

	}

	// Find the first leader and use its name as the player name.
	unit_map::iterator u = resources::units->find_first_leader(t_->side());
	if ((u != resources::units->end()) && t_->current_player().empty())
		t_->set_current_player(u->name().token());

}


