/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
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
#include "map/location.hpp"
#include "game_data.hpp"
#include "units/unit.hpp"
#include "team.hpp"
#include "play_controller.hpp"
#include "actions/create.hpp"
#include "actions/advancement.hpp"
#include "actions/attack.hpp"
#include "actions/move.hpp"
#include "actions/undo.hpp"
#include "preferences/general.hpp"
#include "preferences/game.hpp"
#include "game_events/pump.hpp"
#include "map/map.hpp"
#include "units/helper.hpp"
#include "recall_list_manager.hpp"
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "formula/string_utils.hpp"
#include "units/types.hpp"
#include "units/udisplay.hpp"
#include "whiteboard/manager.hpp"
#include "font/standard_colors.hpp"

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
	//Use a pointer to ensure that this object is not destructed when the programm ends.
	static map* instance = new map();
	return *instance;
}


SYNCED_COMMAND_HANDLER_FUNCTION(recruit, child, use_undo, show, error_handler)
{
	int current_team_num = resources::controller->current_side();
	team &current_team = resources::gameboard->get_team(current_team_num);

	map_location loc(child, resources::gamedata);
	map_location from(child.child_or_empty("from"), resources::gamedata);
	// Validate "from".
	if ( !from.valid() ) {
		// This will be the case for AI recruits in replays saved
		// before 1.11.2, so it is not more severe than a warning.
		// EDIT:  we borke compability with 1.11.2 anyway so we should give an error.
		error_handler("Missing leader location for recruitment.\n", false);
	}
	else if ( resources::gameboard->units().find(from) == resources::gameboard->units().end() ) {
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
	team &current_team = resources::gameboard->get_team(current_team_num);

	const std::string& unit_id = child["value"];
	map_location loc(child, resources::gamedata);
	map_location from(child.child_or_empty("from"), resources::gamedata);

	if ( !actions::recall_unit(unit_id, current_team, loc, from, map_location::NDIRECTIONS, show, use_undo) ) {
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
	// having defender_weapon in the replay fixes a bug (OOS) where one player (or observer) chooses a different defensive weapon.
	// Xan pointed out this was a possibility: we calculate defense weapon
	// now based on attack_prediction code, but this uses floating point
	// calculations, which means that in the case where results are close,
	// rounding differences can mean that both ends choose different weapons.
	int def_weapon_num = child["defender_weapon"].to_int(-2);
	if (def_weapon_num == -2) {
		// Let's not gratuitously destroy backwards compatibility.
		LOG_REPLAY << "Old data, having to guess weapon\n";
		def_weapon_num = -1;
	}

	unit_map::iterator u = resources::gameboard->units().find(src);
	if (!u.valid()) {
		error_handler("unfound location for source of attack\n", true);
		return false;
	}

	if (child.has_attribute("attacker_type")) {
		const std::string &att_type_id = child["attacker_type"];
		if (u->type_id() != att_type_id) {
			WRN_REPLAY << "unexpected attacker type: " << att_type_id << "(game state gives: " << u->type_id() << ")" << std::endl;
		}
	}

	if (static_cast<unsigned>(weapon_num) >= u->attacks().size()) {
		error_handler("illegal weapon type in attack\n", true);
		return false;
	}

	unit_map::const_iterator tgt = resources::gameboard->units().find(dst);

	if (!tgt.valid()) {
		std::stringstream errbuf;
		errbuf << "unfound defender for attack: " << src << " -> " << dst << '\n';
		error_handler(errbuf.str(), true);
		return false;
	}

	if (child.has_attribute("defender_type")) {
		const std::string &def_type_id = child["defender_type"];
		if (tgt->type_id() != def_type_id) {
			WRN_REPLAY << "unexpected defender type: " << def_type_id << "(game state gives: " << tgt->type_id() << ")" << std::endl;
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
	team &current_team = resources::gameboard->get_team(current_team_num);

	const std::string& unit_id = child["value"];
	size_t old_size = current_team.recall_list().size();

	// Find the unit in the recall list.
	unit_ptr dismissed_unit = current_team.recall_list().find_if_matches_id(unit_id);
	assert(dismissed_unit);
	//add dismissal to the undo stack
	resources::undo_stack->add_dismissal(dismissed_unit);

	current_team.recall_list().erase_if_matches_id(unit_id);

	if (old_size == current_team.recall_list().size()) {
		error_handler("illegal disband\n", true);
		return false;
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(move, child,  use_undo, show, error_handler)
{
	int current_team_num = resources::controller->current_side();
	team &current_team = resources::gameboard->get_team(current_team_num);

	std::vector<map_location> steps;

	try {
		read_locations(child,steps);
	} catch (std::invalid_argument&) {
		WRN_REPLAY << "Warning: Path data contained something which could not be parsed to a sequence of locations:" << "\n config = " << child.debug() << std::endl;
		return false;
	}

	if(steps.empty())
	{
		WRN_REPLAY << "Warning: Missing path data found in [move]" << std::endl;
		return false;
	}

	const map_location& src = steps.front();
	const map_location& dst = steps.back();

	if (src == dst) {
		WRN_REPLAY << "Warning: Move with identical source and destination. Skipping..." << std::endl;
		return false;
	}

	// The nominal destination should appear to be unoccupied.
	unit_map::iterator u = resources::gameboard->find_visible_unit(dst, current_team);
	if ( u.valid() ) {
		WRN_REPLAY << "Warning: Move destination " << dst << " appears occupied." << std::endl;
		// We'll still proceed with this movement, though, since
		// an event might intervene.
		// 'event' doesn't mean wml event but rather it means 'hidden' units form the movers point of view.
	}

	u = resources::gameboard->units().find(src);
	if (!u.valid()) {
		std::stringstream errbuf;
		errbuf << "unfound location for source of movement: "
			<< src << " -> " << dst << '\n';
		error_handler(errbuf.str(), true);
		return false;
	}
	bool skip_sighted = false;
	bool skip_ally_sighted = false;
	if(child["skip_sighted"] == "all")
	{
		skip_sighted = true;
	}
	else if(child["skip_sighted"] == "only_ally")
	{
		skip_ally_sighted = true;
	}

	bool show_move = show;
	if ( current_team.is_local_ai() || current_team.is_network_ai())
	{
		show_move = show_move && !preferences::skip_ai_moves();
	}
	actions::move_unit_from_replay(steps, use_undo ? resources::undo_stack : nullptr, skip_sighted, skip_ally_sighted, show_move);

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(fire_event, child,  use_undo, /*show*/, /*error_handler*/)
{
	bool undoable = true;

	if(const config &last_select = child.child("last_select"))
	{
		//the select event cannot clear the undo stack.
		resources::game_events->pump().fire("select", map_location(last_select, resources::gamedata));
	}
	const std::string &event_name = child["raise"];
	if (const config &source = child.child("source")) {
		undoable = undoable & !std::get<0>(resources::game_events->pump().fire(event_name, map_location(source, resources::gamedata)));
	} else {
		undoable = undoable & !std::get<0>(resources::game_events->pump().fire(event_name));
	}

	// Not clearing the undo stack here casues OOS because we added an entry to the replay but no entry to the undo stack.
	if(use_undo) {
		if(!undoable) {
			resources::undo_stack->clear();
		} else {
			resources::undo_stack->add_dummy();
		}
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(lua_ai, child,  /*use_undo*/, /*show*/, /*error_handler*/)
{
	const std::string &lua_code = child["code"];
	assert(resources::lua_kernel);
	resources::lua_kernel->run(lua_code.c_str());
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(auto_shroud, child,  use_undo, /*show*/, /*error_handler*/)
{
	assert(use_undo);
	team &current_team = resources::controller->current_team();

	bool active = child["active"].to_bool();
	// We cannot update shroud here like 'if(active) resources::undo_stack->commit_vision();'.
	// Becasue the undo.cpp code assumes exactly 1 entry in the undo stack per entry in the replay.
	// And doing so would create a second entry in the undo stack for this 'auto_shroud' entry.
	current_team.set_auto_shroud_updates(active);
	resources::undo_stack->add_auto_shroud(active);
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

SYNCED_COMMAND_HANDLER_FUNCTION(update_shroud, /*child*/,  use_undo, /*show*/, error_handler)
{
	assert(use_undo);
	team &current_team = resources::controller->current_team();
	if(current_team.auto_shroud_updates()) {
		error_handler("Team has DSU disabled but we found an explicit shroud update", false);
	}
	resources::undo_stack->commit_vision();
	resources::undo_stack->add_update_shroud();
	return true;
}
namespace
{
	void debug_notification(const char* message)
	{
		utils::string_map symbols;
		symbols["player"] = resources::controller->current_team().current_player();
		display::announce_options announce_options;
		announce_options.lifetime = 1000;
		resources::screen->announce(vgettext(message, symbols), font::NORMAL_COLOR, announce_options);
	}
}
SYNCED_COMMAND_HANDLER_FUNCTION(debug_unit, child,  use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}
	debug_notification(":unit debug command was used during turn of $player");
	map_location loc(child);
	const std::string name = child["name"];
	const std::string value = child["value"];

	unit_map::iterator i = resources::gameboard->units().find(loc);
	if (i == resources::gameboard->units().end()) {
		return false;
	}
	if (name == "advances" ) {
		int int_value = std::stoi(value);
		for (int levels=0; levels<int_value; levels++) {
			i->set_experience(i->max_experience());

			advance_unit_at(advance_unit_params(loc).force_dialog(true));
			i = resources::gameboard->units().find(loc);
			if (!i.valid()) {
				break;
			}
		}
	} else if (name == "status" ) {
		for (std::string status : utils::split(value)) {
			bool add = true;
			if (status.length() >= 1 && status[0] == '-') {
				add = false;
				status = status.substr(1);
			}
			if (status.empty()) {
				continue;
			}
			i->set_state(status, add);
		}
	} else {
		config cfg;
		i->write(cfg);
		cfg[name] = value;

		// Attempt to create a new unit. If there are error (such an invalid type key), exit.
		try{
			unit_ptr new_u(new unit(cfg, true));
			new_u->set_location(loc);
			// Don't remove the unit until after we've verified there are no errors in creating the new one,
			// or else the unit would simply be removed from the map with no replacement.
			resources::gameboard->units().erase(loc);
			resources::whiteboard->on_kill_unit();
			resources::gameboard->units().insert(new_u);
		} catch(unit_type::error& e) {
			ERR_REPLAY << e.what() << std::endl; // TODO: more appropriate error message log
			return false;
		}
	}
	if (name == "fail") { //testcase for bug #18488
		assert(i.valid());
	}
	resources::screen->invalidate(loc);
	resources::screen->invalidate_unit();

	return true;
}
SYNCED_COMMAND_HANDLER_FUNCTION(debug_create_unit, child,  use_undo, /*show*/, error_handler)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification("A unit was created using debug command during turn of $player");
	map_location loc(child);
	resources::whiteboard->on_kill_unit();
	const unit_race::GENDER gender = string_gender(child["gender"], unit_race::NUM_GENDERS);
	const unit_type *u_type = unit_types.find(child["type"]);
	if (!u_type) {
		error_handler("Invalid unit type", true);
		return false;
	}

	const int side_num = resources::controller
			? resources::controller->current_side() : 1;

	// Create the unit.
	unit_ptr created(new unit(*u_type, side_num, true, gender));
	created->new_turn();
	// Add the unit to the board.
	std::pair<unit_map::iterator, bool> add_result = resources::gameboard->units().replace(loc, created);
	resources::screen->invalidate_unit();
	resources::game_events->pump().fire("unit_placed", loc);
	unit_display::unit_recruited(loc);

	// Village capture?
	if ( resources::gameboard->map().is_village(loc) )
		actions::get_village(loc, created->side());

	// Update fog/shroud.
	actions::shroud_clearer clearer;
	clearer.clear_unit(loc, *created);
	clearer.fire_events();
	if ( add_result.first.valid() ) // In case sighted events messed with the unit.
		actions::actor_sighted(*add_result.first);

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_lua, child, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}
	debug_notification(":lua debug command was used during turn of $player");
	resources::lua_kernel->run(child["code"].str().c_str());
	resources::controller->pump().flush_messages();

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_kill, child, use_undo, /*show*/, /*error_handler*/)
{
	if (use_undo) {
		resources::undo_stack->clear();
	}
	debug_notification("kill debug command was used during turn of $player");

	const map_location loc(child["x"].to_int(), child["y"].to_int(), wml_loc());
	const unit_map::iterator i = resources::gameboard->units().find(loc);
	if (i != resources::gameboard->units().end()) {
		const int dying_side = i->side();
		resources::controller->pump().fire("last_breath", loc, loc);
		if (i.valid()) {
			unit_display::unit_die(loc, *i);
		}
		resources::screen->redraw_minimap();
		if (i.valid()) {
			i->set_hitpoints(0);
		}
		resources::controller->pump().fire("die", loc, loc);
		if (i.valid()) {
			resources::gameboard->units().erase(i);
		}
		resources::whiteboard->on_kill_unit();
		actions::recalculate_fog(dying_side);
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_next_level, child, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":next_level debug command was used during turn of $player");

	std::string next_level = child["next_level"];
	if (!next_level.empty())
		resources::gamedata->set_next_scenario(next_level);
	end_level_data e;
	e.transient.carryover_report = false;
	e.prescenario_save = true;
	e.transient.linger_mode = false;
	e.proceed_to_next_level = true;
	e.is_victory = true;

	resources::controller->set_end_level_data(e);
	resources::controller->force_end_turn();

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_turn_limit, child, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":turn_limit debug command was used during turn of $player");

	resources::tod_manager->set_number_of_turns(child["turn_limit"].to_int(-1));
	resources::screen->redraw_everything();
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_turn, child, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":turn debug command was used during turn of $player");

	resources::tod_manager->set_turn(child["turn"].to_int(1), resources::gamedata);

	resources::screen->new_turn();
	resources::screen->redraw_everything();

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_set_var, child, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":set_var debug command was used during turn of $player");

	try {
		resources::gamedata->set_variable(child["name"],child["value"]);
	}
	catch(const invalid_variablename_exception&) {
	//	command_failed(_("Variable not found"));
		return false;
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_gold, child, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":gold debug command was used during turn of $player");

	resources::controller->current_team().spend_gold(-child["gold"].to_int(0));
	resources::screen->redraw_everything();
	return true;
}


SYNCED_COMMAND_HANDLER_FUNCTION(debug_event, child, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":throw debug command was used during turn of $player");

	resources::controller->pump().fire(child["eventname"]);
	resources::screen->redraw_everything();

	return true;
}


SYNCED_COMMAND_HANDLER_FUNCTION(debug_fog, /*child*/, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":fog debug command was used during turn of $player");

	team& current_team = resources::controller->current_team();
	current_team.set_fog(!current_team.uses_fog());
	actions::recalculate_fog(current_team.side());

	resources::screen->recalculate_minimap();
	resources::screen->redraw_everything();

	return true;
}


SYNCED_COMMAND_HANDLER_FUNCTION(debug_shroud, /*child*/, use_undo, /*show*/, /*error_handler*/)
{
	if(use_undo) {
		resources::undo_stack->clear();
	}

	debug_notification(":shroud debug command was used during turn of $player");

	team& current_team = resources::controller->current_team();
	current_team.set_shroud(!current_team.uses_shroud());
	actions::clear_shroud(current_team.side());

	resources::screen->recalculate_minimap();
	resources::screen->redraw_everything();

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(surrender, child, /*use_undo*/, /*show*/, /*error_handler*/)
{
	std::vector<team>& teams = resources::gameboard->teams();
	int side = child.get("side_number")->to_int();
	for(std::vector<team>::iterator i = teams.begin(); i != teams.end(); ++i) {
		if(i->side() == side) {
			(*i).set_defeat_condition(team::DEFEAT_CONDITION::ALWAYS);
		}
	}
	resources::controller->check_victory();
	return true;
}

