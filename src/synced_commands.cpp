/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "synced_commands.hpp"
#include <cassert>

#include "log.hpp"
#include "resources.hpp"
#include "map_location.hpp"
#include "gamestatus.hpp"
#include "unit.hpp"
#include "team.hpp"
#include "play_controller.hpp"
#include "actions/create.hpp"
#include "actions/attack.hpp"
#include "actions/move.hpp"
#include "actions/undo.hpp"
#include "preferences.hpp"
#include "game_preferences.hpp"
#include "game_events/pump.hpp"
#include "dialogs.hpp"
#include "unit_helper.hpp"
#include "replay.hpp" //user choice
#include "resources.hpp"
#include <boost/foreach.hpp>

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)


/**
 * @param[in]  tag       The replay tag for this action.
 * @param[in]  function  The callback for this action.
 */
synced_command::synced_command(const std::string & tag, handler function)
{
	assert(registry().find( tag ) == registry().end());
	registry()[tag] = function;
}

synced_command::map& synced_command::registry()
{
	static map* instance = new map();
	return *instance;
}


SYNCED_COMMAND_HANDLER_FUNCTION(recruit, child, use_undo, show, error_handler)
{
	int current_team_num = resources::controller->current_side();
	team &current_team = (*resources::teams)[current_team_num - 1];

	map_location loc(child, resources::gamedata);
	map_location from(child.child_or_empty("from"), resources::gamedata);
	// Validate "from".
	if ( !from.valid() ) {
		// This will be the case for AI recruits in replays saved
		// before 1.11.2, so it is not more severe than a warning.
		// EDIT:  we borke compability with 1.11.2 anyway so we should give an error.
		error_handler("Missing leader location for recruitment.\n", false);
	}
	else if ( resources::units->find(from) == resources::units->end() ) {
		// Sync problem?
		std::stringstream errbuf;
		errbuf << "Recruiting leader not found at " << from << ".\n";
		error_handler(errbuf.str(), false);
	}

	// Get the unit_type ID.
	std::string type_id = child["type"];
	if ( type_id.empty() ) {
		error_handler("Recruitment is missing a unit type.", true);
		return false;
	}

	const unit_type *u_type = unit_types.find(type_id);
	if (!u_type) {
		std::stringstream errbuf;
		errbuf << "Recruiting illegal unit: '" << type_id << "'.\n";
		error_handler(errbuf.str(), true);
		return false;
	}

	const std::string res = actions::find_recruit_location(current_team_num, loc, from, type_id);
	if(!res.empty())
	{
		std::stringstream errbuf;
		errbuf << "cannot recruit unit: " << res << "\n";
		error_handler(errbuf.str(), true);
		return false;
		//we are already oos because the unit wasn't created, no need to keep the bookkeeping right...
	}
	const int beginning_gold = current_team.gold();



	if ( u_type->cost() > beginning_gold ) {
		std::stringstream errbuf;
		errbuf << "unit '" << type_id << "' is too expensive to recruit: "
			<< u_type->cost() << "/" << beginning_gold << "\n";
		error_handler(errbuf.str(), false);
	}

	actions::recruit_unit(*u_type, current_team_num, loc, from, show, use_undo);

	LOG_REPLAY << "recruit: team=" << current_team_num << " '" << type_id << "' at (" << loc
		<< ") cost=" << u_type->cost() << " from gold=" << beginning_gold << ' '
		<< "-> " << current_team.gold() << "\n";
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(recall, child, use_undo, show, error_handler)
{

	int current_team_num = resources::controller->current_side();
	team &current_team = (*resources::teams)[current_team_num - 1];

	const std::string& unit_id = child["value"];
	map_location loc(child, resources::gamedata);
	map_location from(child.child_or_empty("from"), resources::gamedata);

	if ( !actions::recall_unit(unit_id, current_team, loc, from, show, use_undo) ) {
		error_handler("illegal recall: unit_id '" + unit_id + "' could not be found within the recall list.\n", true);
		//when recall_unit returned false nothing happend so we can safety return false;
		return false;
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(attack, child, /*use_undo*/, show, error_handler)
{
	const config &destination = child.child("destination");
	const config &source = child.child("source");
	//check_checksums(*cfg);

	if (!destination) {
		error_handler("no destination found in attack\n", true);
		return false;
	}

	if (!source) {
		error_handler("no source found in attack \n", true);
		return false;
	}

	//we must get locations by value instead of by references, because the iterators
	//may become invalidated later
	const map_location src(source, resources::gamedata);
	const map_location dst(destination, resources::gamedata);

	int weapon_num = child["weapon"];
	int def_weapon_num = child["defender_weapon"].to_int(-2);
	if (def_weapon_num == -2) {
		// Let's not gratuitously destroy backwards compatibility.
		LOG_REPLAY << "Old data, having to guess weapon\n";
		def_weapon_num = -1;
	}

	unit_map::iterator u = resources::units->find(src);
	if (!u.valid()) {
		error_handler("unfound location for source of attack\n", true);
		return false;
	}

	if (child.has_attribute("attacker_type")) {
		const std::string &att_type_id = child["attacker_type"];
		if (u->type_id() != att_type_id) {
			WRN_REPLAY << "unexpected attacker type: " << att_type_id << "(game_state gives: " << u->type_id() << ")\n";
		}
	}

	if (size_t(weapon_num) >= u->attacks().size()) {
		error_handler("illegal weapon type in attack\n", true);
		return false;
	}

	unit_map::const_iterator tgt = resources::units->find(dst);

	if (!tgt.valid()) {
		std::stringstream errbuf;
		errbuf << "unfound defender for attack: " << src << " -> " << dst << '\n';
		error_handler(errbuf.str(), true);
		return false;
	}

	if (child.has_attribute("defender_type")) {
		const std::string &def_type_id = child["defender_type"];
		if (tgt->type_id() != def_type_id) {
			WRN_REPLAY << "unexpected defender type: " << def_type_id << "(game_state gives: " << tgt->type_id() << ")\n";
		}
	}

	if (def_weapon_num >= static_cast<int>(tgt->attacks().size())) {

		error_handler("illegal defender weapon type in attack\n", true);
		return false;
	}

	DBG_REPLAY << "Attacker XP (before attack): " << u->experience() << "\n";
	
	resources::undo_stack->clear();
	attack_unit_and_advance(src, dst, weapon_num, def_weapon_num, show);
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(disband, child, /*use_undo*/, /*show*/, error_handler)
{
	
	int current_team_num = resources::controller->current_side();
	team &current_team = (*resources::teams)[current_team_num - 1];

	const std::string& unit_id = child["value"];
	std::vector<unit>::iterator disband_unit =
		find_if_matches_id(current_team.recall_list(), unit_id);

	if(disband_unit != current_team.recall_list().end()) {
		current_team.recall_list().erase(disband_unit);
	} else {
		error_handler("illegal disband\n", true);
		return false;
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(move, child,  use_undo, show, error_handler)
{
	int current_team_num = resources::controller->current_side();
	team &current_team = (*resources::teams)[current_team_num - 1];

	const std::string& x = child["x"];
	const std::string& y = child["y"];
	const std::vector<map_location> steps = parse_location_range(x,y);

	if(steps.empty()) 
	{
		WRN_REPLAY << "Warning: Missing path data found in [move]\n";
		return false;
	}

	const map_location& src = steps.front();
	const map_location& dst = steps.back();

	if (src == dst) {
		WRN_REPLAY << "Warning: Move with identical source and destination. Skipping...\n";
		return false;
	}
	
	// The nominal destination should appear to be unoccupied.
	unit_map::iterator u = find_visible_unit(dst, current_team);
	if ( u.valid() ) {
		WRN_REPLAY << "Warning: Move destination " << dst << " appears occupied.\n";
		// We'll still proceed with this movement, though, since
		// an event might intervene.
		// 'event' doesnt mean wml event but rather it means 'hidden' units form the movers point of view.
	}

	u = resources::units->find(src);
	if (!u.valid()) {
		std::stringstream errbuf;
		errbuf << "unfound location for source of movement: "
			<< src << " -> " << dst << '\n';
		error_handler(errbuf.str(), true);
		return false;
	}
	bool skip_sighted = false;
	bool skip_ally_sighted = false;
	if(child["skip_sighed"] == "all")
	{
		skip_sighted = true;
	}
	else if(child["skip_sighted"] == "only_ally")
	{
		skip_ally_sighted = true;
	}

	bool show_move = show;
	if ( current_team.is_ai() || current_team.is_network_ai() )
	{
		show_move = show_move && preferences::show_ai_moves();
	}
	actions::move_unit_from_replay(steps, use_undo ? resources::undo_stack : NULL, skip_sighted, skip_ally_sighted, show_move);
	
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(fire_event, child,  /*use_undo*/, /*show*/, /*error_handler*/)
{
	//i don't know the reason for the following three lines.
	//TODO: find out wheter we can delete them. I think this code was introduced in bbfdfcf9ed6ca44f01da32bf74c39d5fa9a75c37
	BOOST_FOREACH(const config &v, child.child_range("set_variable")) {
		resources::gamedata->set_variable(v["name"], v["value"]);
	}
	bool undoable = true;

	if(const config &last_select = child.child("last_select"))
	{
		//the select event cannot clear the undo stack.
		game_events::fire("select", map_location(last_select, resources::gamedata));
	}
	const std::string &event_name = child["raise"];
	if (const config &source = child.child("source")) {
		undoable = undoable & !game_events::fire(event_name, map_location(source, resources::gamedata));
	} else {
		undoable = undoable & !game_events::fire(event_name);
	}
	if ( !undoable)
		resources::undo_stack->clear();
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(lua_ai, child,  /*use_undo*/, /*show*/, /*error_handler*/)
{
	const std::string &lua_code = child["code"];
	game_events::run_lua_commands(lua_code.c_str());
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(auto_shroud, child,  use_undo, /*show*/, /*error_handler*/)
{
	assert(use_undo);
	int current_team_num = resources::controller->current_side();
	team &current_team = (*resources::teams)[current_team_num - 1];

	bool active = child["active"].to_bool();
	// Turning on automatic shroud causes vision to be updated.
	if ( active )
		resources::undo_stack->commit_vision();

	current_team.set_auto_shroud_updates(active);
	return true;
}

/** from resources::undo_stack->commit_vision(bool is_replay):
 * Updates fog/shroud based on the undo stack, then updates stack as needed.
 * Call this when "updating shroud now".
 * This may fire events and change the game state.
 * @param[in]  is_replay  Set to true when this is called during a replay.
 *
 * this means it ia synced command liek any other.
 */

SYNCED_COMMAND_HANDLER_FUNCTION(update_shroud, /*child*/,  use_undo, /*show*/, /*error_handler*/)
{
	assert(use_undo);
	resources::undo_stack->commit_vision();
	return true;
}
