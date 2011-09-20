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

#include "gamestatus.hpp"

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

DEFAULT_TOKEN_BODY(z_NORMAL_default, "NORMAL");
DEFAULT_TOKEN_BODY(z_label_default, "label")
DEFAULT_TOKEN_BODY(z_parent_default, "parent")
DEFAULT_TOKEN_BODY(z_version_default, "version")
DEFAULT_TOKEN_BODY(z_campaign_type_default, "campaign_type")
DEFAULT_TOKEN_BODY(z_scenario_default, "scenario")
DEFAULT_TOKEN_BODY(z_campaign_define_default, "campaign_define")
DEFAULT_TOKEN_BODY(z_campaign_extra_defines_default, "campaign_extra_defines")
DEFAULT_TOKEN_BODY(z_campaign_default, "campaign")
DEFAULT_TOKEN_BODY(z_history_default, "history")
DEFAULT_TOKEN_BODY(z_abbrev_default, "abbrev")
DEFAULT_TOKEN_BODY(z_next_scenario_default, "next_scenario")
DEFAULT_TOKEN_BODY(z_completion_default, "completion")
DEFAULT_TOKEN_BODY(z_end_text_default, "end_text")
DEFAULT_TOKEN_BODY(z_end_text_duration_default, "end_text_duration")
DEFAULT_TOKEN_BODY(z_difficulty_default, "difficulty")

game_classification::game_classification():
	savegame_config(),
	label(),
	parent(),
	version(),
	campaign_type(),
	campaign_define(),
	campaign_xtra_defines(),
	campaign(),
	history(),
	abbrev(),
	scenario(),
	next_scenario(),
	completion(),
	end_text(),
	end_text_duration(),
	difficulty(z_NORMAL_default())
	{}

game_classification::game_classification(const config& cfg):
	savegame_config(),
	label(cfg[z_label_default()]),
	parent(cfg[z_parent_default()]),
	version(cfg[z_version_default()]),
	campaign_type(cfg[z_campaign_type_default()].empty() ? z_scenario_default() : cfg[z_campaign_type_default()].str()),
	campaign_define(cfg[z_campaign_define_default()]),
	campaign_xtra_defines(utils::split(cfg[z_campaign_extra_defines_default()])),
	campaign(cfg[z_campaign_default()]),
	history(cfg[z_history_default()]),
	abbrev(cfg[z_abbrev_default()]),
	scenario(cfg[z_scenario_default()]),
	next_scenario(cfg[z_next_scenario_default()]),
	completion(cfg[z_completion_default()]),
	end_text(cfg[z_end_text_default()]),
	end_text_duration(cfg[z_end_text_duration_default()]),
	difficulty(cfg[z_difficulty_default()].empty() ? z_NORMAL_default() : cfg[z_difficulty_default()].str())
	{}

game_classification::game_classification(const game_classification& gc):
	savegame_config(),
	label(gc.label),
	parent(gc.parent),
	version(gc.version),
	campaign_type(gc.campaign_type),
	campaign_define(gc.campaign_define),
	campaign_xtra_defines(gc.campaign_xtra_defines),
	campaign(gc.campaign),
	history(gc.history),
	abbrev(gc.abbrev),
	scenario(gc.scenario),
	next_scenario(gc.next_scenario),
	completion(gc.completion),
	end_text(gc.end_text),
	end_text_duration(gc.end_text_duration),
	difficulty(gc.difficulty)
{
}

config game_classification::to_config() const
{
	static const config::t_token z_label("label", false);
	static const config::t_token z_parent("parent", false);
	static const config::t_token z_version("version", false);
	static const config::t_token z_campaign_type("campaign_type", false);
	static const config::t_token z_campaign_define("campaign_define", false);
	static const config::t_token z_campaign_extra_defines("campaign_extra_defines", false);
	static const config::t_token z_campaign("campaign", false);
	static const config::t_token z_history("history", false);
	static const config::t_token z_abbrev("abbrev", false);
	static const config::t_token z_scenario("scenario", false);
	static const config::t_token z_next_scenario("next_scenario", false);
	static const config::t_token z_completion("completion", false);
	static const config::t_token z_end_text("end_text", false);
	static const config::t_token z_end_text_duration("end_text_duration", false);
	static const config::t_token z_difficulty("difficulty", false);

	config cfg;

	cfg[z_label] = label;
	cfg[z_parent] = parent;
	cfg[z_version] = game_config::version;
	cfg[z_campaign_type] = campaign_type;
	cfg[z_campaign_define] = campaign_define;
	cfg[z_campaign_extra_defines] = utils::join(campaign_xtra_defines);
	cfg[z_campaign] = campaign;
	cfg[z_history] = history;
	cfg[z_abbrev] = abbrev;
	cfg[z_scenario] = scenario;
	cfg[z_next_scenario] = next_scenario;
	cfg[z_completion] = completion;
	cfg[z_end_text] = end_text;
	cfg[z_end_text_duration] = str_cast<unsigned int>(end_text_duration);
	cfg[z_difficulty] = difficulty;

	return cfg;
}

game_state::game_state()  :
		scoped_variables(),
		wml_menu_items(),
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(map_location::null_location),
		rng_(),
		variables_(),
		temporaries_(),
		generator_setter_(&recorder),
		classification_(),
		mp_settings_(),
		phase_(INITIAL),
		can_end_turn_(true)
		{}

void write_players(game_state& gamestate, config& cfg, const bool use_snapshot, const bool merge_side)
{
	static const config::t_token z_player("player", false);
	static const config::t_token z_side("side", false);
	static const config::t_token z_save_id("save_id", false);
	static const config::t_token z_id("id", false);
	static const config::t_token z_gold("gold", false);
	static const config::t_token z_gold_add("gold_add", false);
	static const config::t_token z_previous_recruits("previous_recruits", false);
	static const config::t_token z_can_recruit("can_recruit", false);
	static const config::t_token z_name("name", false);
	static const config::t_token z_current_player("current_player", false);
	static const config::t_token z_color("color", false);
	static const config::t_token z_unit("unit", false);

	// If there is already a player config available it means we are loading
	// from a savegame. Don't do anything then, the information is already there
	config::child_itors player_cfg = cfg.child_range(z_player);
	if (player_cfg.first != player_cfg.second)
		return;

	config *source = NULL;
	if (use_snapshot) {
		source = &gamestate.snapshot;
	} else {
		source = &gamestate.starting_pos;
	}

	if (merge_side) {
		//merge sides/players from starting pos with the scenario cfg
		std::vector<config::t_token> tags;
		tags.push_back(z_side);
		tags.push_back(z_player); //merge [player] tags for backwards compatibility of saves

		foreach (const config::t_token& side_tag, tags)
		{
			foreach (config &carryover_side, source->child_range(side_tag))
			{
				config *scenario_side = NULL;

				//TODO: use the player_id instead of the save_id for that
				if (config& c = cfg.find_child(z_side, z_save_id, carryover_side[z_save_id])) {
					scenario_side = &c;
				} else if (config& c = cfg.find_child(z_side, z_id, carryover_side[z_save_id])) {
					scenario_side = &c;
				}

				if (scenario_side == NULL) {
					//no matching side in the current scenario, we add the persistent information in a [player] tag
					cfg.add_child(z_player, carryover_side);
					continue;
				}

				//we have a matching side in the current scenario

				//sort carryover gold
				int ngold = (*scenario_side)[z_gold].to_int(100);
				int player_gold = carryover_side[z_gold];
				if (carryover_side[z_gold_add].to_bool()) {
					ngold += player_gold;
				} else if (player_gold >= ngold) {
					ngold = player_gold;
				}
				carryover_side[z_gold] = str_cast(ngold);
				if (const config::attribute_value *v = scenario_side->get(z_gold_add)) {
					carryover_side[z_gold_add] = *v;
				}
				//merge player information into the scenario cfg
				(*scenario_side)[z_save_id] = carryover_side[z_save_id];
				(*scenario_side)[z_gold] = ngold;
				(*scenario_side)[z_gold_add] = carryover_side[z_gold_add];
				if (const config::attribute_value *v = carryover_side.get(z_previous_recruits)) {
					(*scenario_side)[z_previous_recruits] = *v;
				} else {
					(*scenario_side)[z_previous_recruits] = carryover_side[z_can_recruit];
				}
				(*scenario_side)[z_name] = carryover_side[z_name];
				(*scenario_side)[z_current_player] = carryover_side[z_current_player];

				(*scenario_side)[z_color] = carryover_side[z_color];

				//add recallable units
				foreach (const config &u, carryover_side.child_range(z_unit)) {
					scenario_side->add_child(z_unit, u);
				}
			}
		}
	} else {
		foreach(const config &snapshot_side, source->child_range(z_side)) {
			//take all side tags and add them as players (assuming they only contain carryover information)
			cfg.add_child(z_player, snapshot_side);
		}
	}
}

game_state::game_state(const config& cfg, bool show_replay) :
		scoped_variables(),
		wml_menu_items(),
		replay_data(),
		starting_pos(),
		snapshot(),
		last_selected(map_location::null_location),
		rng_(cfg),
		variables_(),
		temporaries_(),
		generator_setter_(&recorder),
		classification_(cfg),
		mp_settings_(cfg),
		phase_(INITIAL),
		can_end_turn_(true)
{
	static const config::t_token z_next_underlying_unit_id("next_underlying_unit_id", false);
	static const config::t_token z_read_game("read_game", false);
	static const config::t_token z_snapshot("snapshot", false);
	static const config::t_token z_replay_start("replay_start", false);
	static const config::t_token z_random_calls("random_calls", false);
	static const config::t_token z_can_end_turn("can_end_turn", false);
	static const config::t_token z_variables("variables", false);
	static const config::t_token z_menu_item("menu_item", false);
	static const config::t_token z_replay("replay", false);
	static const config::t_token z_player("player", false);
	static const config::t_token z_statistics("statistics", false);

	n_unit::id_manager::instance().set_save_id(cfg[z_next_underlying_unit_id]);
	log_scope(z_read_game);

	const config &snapshot = cfg.child(z_snapshot);
	const config &replay_start = cfg.child(z_replay_start);
	// We're loading a snapshot if we have it and the user didn't request a replay.
	bool load_snapshot = !show_replay && snapshot && !snapshot.empty();

	if (load_snapshot) {
		this->snapshot = snapshot;

		rng_.seed_random(snapshot[z_random_calls]);
	} else {
		assert(replay_start);
	}

	can_end_turn_ = cfg[z_can_end_turn].to_bool(true);

	LOG_NG << "scenario: '" << classification_.scenario << "'\n";
	LOG_NG << "next_scenario: '" << classification_.next_scenario << "'\n";

	//priority of populating wml variables:
	//snapshot -> replay_start -> root
	if (load_snapshot) {
		if (const config &vars = snapshot.child(z_variables)) {
			set_variables(vars);
		} else if (const config &vars = cfg.child(z_variables)) {
			set_variables(vars);
		}
	}
	else if (const config &vars = replay_start.child(z_variables)) {
		set_variables(vars);
	}
	else if (const config &vars = cfg.child(z_variables)) {
		set_variables(vars);
	}
	set_menu_items(cfg.child_range(z_menu_item));

	if (const config &replay = cfg.child(z_replay)) {
		replay_data = replay;
	}

	if (replay_start) {
		starting_pos = replay_start;
		//This is a quick hack to make replays for campaigns work again:
		//The [player] information needs to be stored somewhere within the gamestate,
		//because we need it later on when creating the replay savegame.
		//We therefore put it inside the starting_pos, so it doesn't get lost.
		//See also playcampaign::play_game, where after finishing the scenario the replay
		//will be saved.
		if(!starting_pos.empty()) {
			foreach (const config &p, cfg.child_range(z_player)) {
				config& cfg_player = starting_pos.add_child(z_player);
				cfg_player.merge_with(p);
			}
		}
	}

	if (const config &stats = cfg.child(z_statistics)) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}
}

void game_state::write_snapshot(config& cfg) const
{
	static const config::t_token z_write_game("write_game", false);
	static const config::t_token z_label("label", false);
	static const config::t_token z_history("history", false);
	static const config::t_token z_abbrev("abbrev", false);
	static const config::t_token z_version("version", false);
	static const config::t_token z_scenario("scenario", false);
	static const config::t_token z_next_scenario("next_scenario", false);
	static const config::t_token z_completion("completion", false);
	static const config::t_token z_campaign("campaign", false);
	static const config::t_token z_campaign_type("campaign_type", false);
	static const config::t_token z_difficulty("difficulty", false);
	static const config::t_token z_campaign_define("campaign_define", false);
	static const config::t_token z_campaign_extra_defines("campaign_extra_defines", false);
	static const config::t_token z_next_underlying_unit_id("next_underlying_unit_id", false);
	static const config::t_token z_can_end_turn("can_end_turn", false);
	static const config::t_token z_random_seed("random_seed", false);
	static const config::t_token z_random_calls("random_calls", false);
	static const config::t_token z_end_text("end_text", false);
	static const config::t_token z_end_text_duration("end_text_duration", false);
	static const config::t_token z_variables("variables", false);
	static const config::t_token z_id("id", false);
	static const config::t_token z_image("image", false);
	static const config::t_token z_description("description", false);
	static const config::t_token z_needs_select("needs_select", false);
	static const config::t_token z_show_if("show_if", false);
	static const config::t_token z_filter_location("filter_location", false);
	static const config::t_token z_command("command", false);
	static const config::t_token z_menu_item("menu_item", false);

	log_scope(z_write_game);
	cfg[z_label] = classification_.label;
	cfg[z_history] = classification_.history;
	cfg[z_abbrev] = classification_.abbrev;
	cfg[z_version] = game_config::version;

	cfg[z_scenario] = classification_.scenario;
	cfg[z_next_scenario] = classification_.next_scenario;

	cfg[z_completion] = classification_.completion;

	cfg[z_campaign] = classification_.campaign;
	cfg[z_campaign_type] = classification_.campaign_type;
	cfg[z_difficulty] = classification_.difficulty;

	cfg[z_campaign_define] = classification_.campaign_define;
	cfg[z_campaign_extra_defines] = utils::join(classification_.campaign_xtra_defines);
	cfg[z_next_underlying_unit_id] = str_cast(n_unit::id_manager::instance().get_save_id());
	cfg[z_can_end_turn] = can_end_turn_;

	cfg[z_random_seed] = rng_.get_random_seed();
	cfg[z_random_calls] = rng_.get_random_calls();

	cfg[z_end_text] = classification_.end_text;
	cfg[z_end_text_duration] = str_cast<unsigned int>(classification_.end_text_duration);

	cfg.add_child(z_variables, variables_);

	for(std::map<std::string, wml_menu_item *>::const_iterator j=wml_menu_items.begin();
	    j!=wml_menu_items.end(); ++j) {
		config new_cfg;
		new_cfg[z_id]=j->first;
		new_cfg[z_image]=j->second->image;
		new_cfg[z_description]=j->second->description;
		new_cfg[z_needs_select] = j->second->needs_select;
		if(!j->second->show_if.empty())
			new_cfg.add_child(z_show_if, j->second->show_if);
		if(!j->second->filter_location.empty())
			new_cfg.add_child(z_filter_location, j->second->filter_location);
		if(!j->second->command.empty())
			new_cfg.add_child(z_command, j->second->command);
		cfg.add_child(z_menu_item, new_cfg);
	}
}

void extract_summary_from_config(config& cfg_save, config& cfg_summary)
{
	static const config::t_token z_snapshot("snapshot", false);
	static const config::t_token z_replay_start("replay_start", false);
	static const config::t_token z_replay("replay", false);
	static const config::t_token z_side("side", false);
	static const config::t_token z_label("label", false);
	static const config::t_token z_parent("parent", false);
	static const config::t_token z_campaign_type("campaign_type", false);
	static const config::t_token z_scenario("scenario", false);
	static const config::t_token z_campaign("campaign", false);
	static const config::t_token z_difficulty("difficulty", false);
	static const config::t_token z_version("version", false);
	static const config::t_token z_corrupt("corrupt", false);
	static const config::t_token z_turn("turn", false);
	static const config::t_token z_turn_at("turn_at", false);
	static const config::t_token z_turns("turns", false);
	static const config::t_token z_controller("controller", false);
	static const config::t_token z_human("human", false);
	static const config::t_token z_shroud("shroud", false);
	static const config::t_token z_canrecruit("canrecruit", false);
	static const config::t_token z_id("id", false);
	static const config::t_token z_image("image", false);
	static const config::t_token z_unit("unit", false);
	static const config::t_token z_leader("leader", false);
	static const config::t_token z_leader_image("leader_image", false);
	static const config::t_token z_map_data("map_data", false);
	static const config::t_token z_yes("yes", false);

	const config &cfg_snapshot = cfg_save.child(z_snapshot);
	const config &cfg_replay_start = cfg_save.child(z_replay_start);

	const config &cfg_replay = cfg_save.child(z_replay);
	const bool has_replay = cfg_replay && !cfg_replay.empty();
	const bool has_snapshot = cfg_snapshot && cfg_snapshot.child(z_side);

	cfg_summary[z_replay] = has_replay;
	cfg_summary[z_snapshot] = has_snapshot;

	cfg_summary[z_label] = cfg_save[z_label];
	cfg_summary[z_parent] = cfg_save[z_parent];
	cfg_summary[z_campaign_type] = cfg_save[z_campaign_type];
	cfg_summary[z_scenario] = cfg_save[z_scenario];
	cfg_summary[z_campaign] = cfg_save[z_campaign];
	cfg_summary[z_difficulty] = cfg_save[z_difficulty];
	cfg_summary[z_version] = cfg_save[z_version];
	cfg_summary[z_corrupt] = n_token::t_token::z_empty();

	if(has_snapshot) {
		cfg_summary[z_turn] = cfg_snapshot[z_turn_at];
		if (cfg_snapshot[z_turns] != "-1") {
			cfg_summary[z_turn] = cfg_summary[z_turn].str() + "/" + cfg_snapshot[z_turns].str();
		}
	}

	// Find the first human leader so we can display their icon in the load menu.

	/** @todo Ideally we should grab all leaders if there's more than 1 human player? */
	config::t_token leader;
	config::t_token leader_image;

	//foreach (const config &p, cfg_save.child_range(z_player))
	//{
	//	if (utils::string_bool(p[z_canrecruit], false)) {
	//		leader = p[z_save_id];
	//	}
	//}

	bool shrouded = false;

	//if (!leader.empty())
	//{
		if (const config &snapshot = *(has_snapshot ? &cfg_snapshot : &cfg_replay_start))
		{
			foreach (const config &side, snapshot.child_range(z_side))
			{
				if (side[z_controller] != z_human) {
					continue;
				}

				if (side[z_shroud].to_bool()) {
					shrouded = true;
				}

				if (side[z_canrecruit].to_bool())
				{
					leader = side[z_id].token();
					leader_image = side[z_image].token();
					break;
				}

				foreach (const config &u, side.child_range(z_unit))
				{
					if (u[z_canrecruit].to_bool()) {
						leader = u[z_id].token();
						leader_image = u[z_image].token();
						break;
					}
				}
			}
		}
	//}

	cfg_summary[z_leader] = leader;
	cfg_summary[z_leader_image] = leader_image;
	cfg_summary[z_map_data] = n_token::t_token::z_empty();

	if(!shrouded) {
		if(has_snapshot) {
			if (!cfg_snapshot.find_child(z_side, z_shroud, z_yes)) {
				cfg_summary[z_map_data] = cfg_snapshot[z_map_data];
			}
		} else if(has_replay) {
			if (!cfg_replay_start.find_child(z_side,z_shroud,z_yes)) {
				cfg_summary[z_map_data] = cfg_replay_start[z_map_data];
			}
		}
	}
}

config::attribute_value &game_state::get_variable(const config::t_token& key) {
	return variable_info(key, true, variable_info::TYPE_SCALAR).as_scalar(); }

config::attribute_value game_state::get_variable_const(const config::t_token &key) const {

	variable_info to_get(key, false, variable_info::TYPE_SCALAR);
	if (!to_get.is_valid()) {
		config::attribute_value &to_return = temporaries_[key];
		std::string const & skey(key);
		if (skey.size() > 7 && skey.substr(skey.size() - 7) == ".length") {
			// length is a special attribute, so guarantee its correctness
			to_return = 0;
		}
		return to_return;
	}
	return to_get.as_scalar();
}

config& game_state::get_variable_cfg(const config::t_token& key) {
	return variable_info(key, true, variable_info::TYPE_CONTAINER).as_container();
}

void game_state::set_variable(const config::t_token& key, const t_string& value) {
	get_variable(key) = value;
}

config& game_state::add_variable_cfg(const config::t_token& key, const config& value) {
	variable_info to_add(key, true, variable_info::TYPE_ARRAY);
	return to_add.vars->add_child(to_add.key, value);
}

void game_state::clear_variable_cfg(const config::t_token& varname) {
	variable_info to_clear(varname, false, variable_info::TYPE_CONTAINER);
	if(!to_clear.is_valid()) return;
	if(to_clear.is_explicit_index()) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
	}
}

void game_state::clear_variable(const config::t_token& varname) {
	variable_info to_clear(varname, false);
	if(!to_clear.is_valid()) return;
	if(to_clear.is_explicit_index()) {
		to_clear.vars->remove_child(to_clear.key, to_clear.index);
	} else {
		to_clear.vars->clear_children(to_clear.key);
		to_clear.vars->remove_attribute(to_clear.key);
	}
}


config::attribute_value &game_state::get_variable(const config::attribute_value& key) {
	return get_variable(key.token());}
config::attribute_value game_state::get_variable_const(const config::attribute_value &key) const {
	return get_variable_const(key.token());}
config& game_state::get_variable_cfg(const config::attribute_value& key) {
	return get_variable_cfg(key.token());}
void game_state::set_variable(const config::attribute_value& key, const t_string& value) {
	return set_variable(key.token(), value); }
config& game_state::add_variable_cfg(const config::attribute_value& key, const config& value) {
	return add_variable_cfg(key.token(), value); }
void game_state::clear_variable_cfg(const config::attribute_value& varname) {
	clear_variable_cfg(varname.token()); }
void game_state::clear_variable(const config::attribute_value& varname) {
	clear_variable(varname.token()); }


config::attribute_value &game_state::get_variable(const std::string& key) {
	return get_variable(config::t_token(key));}
config::attribute_value game_state::get_variable_const(const std::string &key) const {
	return get_variable_const(config::t_token(key));}
config& game_state::get_variable_cfg(const std::string& key) {
	return get_variable_cfg(config::t_token(key));}
void game_state::set_variable(const std::string& key, const t_string& value) {
	return set_variable(config::t_token(key), value); }
config& game_state::add_variable_cfg(const std::string& key, const config& value) {
	return add_variable_cfg(config::t_token(key), value); }
void game_state::clear_variable_cfg(const std::string& varname) {
	clear_variable_cfg(config::t_token(varname)); }
void game_state::clear_variable(const std::string& varname) {
	clear_variable(config::t_token(varname)); }


static void clear_wmi(std::map<std::string, wml_menu_item *> &gs_wmi) {
	for (std::map<std::string, wml_menu_item *>::iterator i = gs_wmi.begin(),
	     i_end = gs_wmi.end(); i != i_end; ++i)
	{
		delete i->second;
	}
	gs_wmi.clear();
}

game_state::game_state(const game_state& state) :
	variable_set(), // Not sure why empty, copied from old code
	scoped_variables(state.scoped_variables),
	wml_menu_items(),
	replay_data(state.replay_data),
	starting_pos(state.starting_pos),
	snapshot(state.snapshot),
	last_selected(state.last_selected),
	rng_(state.rng_),
	variables_(state.variables_),
	temporaries_(), // Not sure why empty, copied from old code
	generator_setter_(state.generator_setter_),
	classification_(state.classification_),
	mp_settings_(state.mp_settings_),
	phase_(state.phase_),
	can_end_turn_(state.can_end_turn_)
{
	clear_wmi(wml_menu_items);
	std::map<std::string, wml_menu_item*>::const_iterator itor;
	for (itor = state.wml_menu_items.begin(); itor != state.wml_menu_items.end(); ++itor) {
		wml_menu_item*& mref = wml_menu_items[itor->first];
		mref = new wml_menu_item(*(itor->second));
	}
}

game_state& game_state::operator=(const game_state& state)
{
	// Use copy constructor to make sure we are coherant
	if (this != &state) {
		this->~game_state();
		new (this) game_state(state) ;
	}
	return *this ;
}

game_state::~game_state() {
	clear_wmi(wml_menu_items);
}

void game_state::set_variables(const config& vars) {
	variables_ = vars;
}


team_builder_ptr game_state::create_team_builder(const config& side_cfg,
					 std::string save_id, t_teams& teams,
					 const config& level, gamemap& map, unit_map& units,
					 bool snapshot)
{
	return team_builder_ptr(new team_builder(side_cfg,save_id,teams,level,map,units,snapshot,starting_pos));
}

void game_state::build_team_stage_one(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_one();
}

void game_state::build_team_stage_two(team_builder_ptr tb_ptr)
{
	tb_ptr->build_team_stage_two();
}

void game_state::set_menu_items(const config::const_child_itors &menu_items)
{
	static const config::t_token z_id("id", false);

	clear_wmi(wml_menu_items);
	foreach (const config &item, menu_items)
	{
		std::string id = item[z_id];
		wml_menu_item*& mref = wml_menu_items[id];
		if(mref == NULL) {
			mref = new wml_menu_item(id, &item);
		} else {
			WRN_NG << "duplicate menu item (" << id << ") while loading gamestate\n";
		}
	}
}

void game_state::write_config(config_writer& out, bool write_variables) const
{
	static const config::t_token z_multiplayer("multiplayer", false);
	static const config::t_token z_random_seed("random_seed", false);
	static const config::t_token z_random_calls("random_calls", false);
	static const config::t_token z_variables("variables", false);
	static const config::t_token z_menu_item("menu_item", false);
	static const config::t_token z_id("id", false);
	static const config::t_token z_image("image", false);
	static const config::t_token z_description("description", false);
	static const config::t_token z_needs_select("needs_select", false);
	static const config::t_token z_yes("yes", false);
	static const config::t_token z_no("no", false);
	static const config::t_token z_show_if("show_if", false);
	static const config::t_token z_filter_location("filter_location", false);
	static const config::t_token z_command("command", false);
	static const config::t_token z_replay("replay", false);
	static const config::t_token z_replay_start("replay_start", false);

	out.write(classification_.to_config());
	if (classification_.campaign_type == z_multiplayer)
		out.write_child(z_multiplayer, mp_settings_.to_config());
	out.write_key_val(z_random_seed, lexical_cast<std::string>(rng_.get_random_seed()));
	out.write_key_val(z_random_calls, lexical_cast<std::string>(rng_.get_random_calls()));
	if (write_variables) {
		out.write_child(z_variables, variables_);
	}

	for(std::map<std::string, wml_menu_item *>::const_iterator j = wml_menu_items.begin();
	    j != wml_menu_items.end(); ++j) {
		out.open_child(z_menu_item);
		out.write_key_val(z_id, j->first);
		out.write_key_val(z_image, j->second->image);
		out.write_key_val(z_description, j->second->description);
		out.write_key_val(z_needs_select, (j->second->needs_select) ? z_yes : z_no);
		if(!j->second->show_if.empty())
			out.write_child(z_show_if, j->second->show_if);
		if(!j->second->filter_location.empty())
			out.write_child(z_filter_location, j->second->filter_location);
		if(!j->second->command.empty())
			out.write_child(z_command, j->second->command);
		out.close_child(z_menu_item);
	}

	if (!replay_data.child(z_replay)) {
		out.write_child(z_replay, replay_data);
	}

	out.write_child(z_replay_start,starting_pos);
}

wml_menu_item::wml_menu_item(const std::string& id, const config* cfg) :
		name(),
		image(),
		description(),
		needs_select(false),
		show_if(),
		filter_location(),
		command()

{
	std::stringstream temp;
	temp << "menu item";
	if(!id.empty()) {
		temp << ' ' << id;
	}
	name = config::t_token(temp.str());
	static const config::t_token z_image("image", false);
	static const config::t_token z_description("description", false);
	static const config::t_token z_needs_select("needs_select", false);
	static const config::t_token z_show_if("show_if", false);
	static const config::t_token z_filter_location("filter_location", false);
	static const config::t_token z_command("command", false);

	if(cfg != NULL) {
		image = (*cfg)[z_image].str();
		description = (*cfg)[z_description].t_str();
		needs_select = (*cfg)[z_needs_select].to_bool();
		if (const config &c = cfg->child(z_show_if)) show_if = c;
		if (const config &c = cfg->child(z_filter_location)) filter_location = c;
		if (const config &c = cfg->child(z_command)) command = c;
	}
}
