/*
	Copyright (C) 2014 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "preferences/preferences.hpp"
#include "game_events/pump.hpp"
#include "map/map.hpp"
#include "recall_list_manager.hpp"
#include "resources.hpp"
#include "savegame.hpp"
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
	//Use a pointer to ensure that this object is not destructed when the program ends.
	static map* instance = new map();
	return *instance;
}


SYNCED_COMMAND_HANDLER_FUNCTION(recruit, child, spectator)
{
	int current_team_num = resources::controller->current_side();
	team &current_team = resources::gameboard->get_team(current_team_num);

	map_location loc(child, resources::gamedata);
	map_location from(child.child_or_empty("from"), resources::gamedata);
	// Validate "from".
	if ( !from.valid() ) {
		// This will be the case for AI recruits in replays saved
		// before 1.11.2, so it is not more severe than a warning.
		// EDIT: we broke compatibility with 1.11.2 anyway so we should give an error.
		spectator.error("Missing leader location for recruitment.\n");
	}
	else if ( resources::gameboard->units().find(from) == resources::gameboard->units().end() ) {
		// Sync problem?
		std::stringstream errbuf;
		errbuf << "Recruiting leader not found at " << from << ".\n";
		spectator.error(errbuf.str());
	}

	// Get the unit_type ID.
	std::string type_id = child["type"];
	if ( type_id.empty() ) {
		spectator.error("Recruitment is missing a unit type.");
		return false;
	}

	const unit_type *u_type = unit_types.find(type_id);
	if (!u_type) {
		std::stringstream errbuf;
		errbuf << "Recruiting illegal unit: '" << type_id << "'.\n";
		spectator.error(errbuf.str());
		return false;
	}

	const std::string res = actions::find_recruit_location(current_team_num, loc, from, type_id);
	if(!res.empty())
	{
		std::stringstream errbuf;
		errbuf << "cannot recruit unit: " << res << "\n";
		spectator.error(errbuf.str());
		return false;
		//we are already oos because the unit wasn't created, no need to keep the bookkeeping right...
	}
	const int beginning_gold = current_team.gold();



	if ( u_type->cost() > beginning_gold ) {
		std::stringstream errbuf;
		errbuf << "unit '" << type_id << "' is too expensive to recruit: "
			<< u_type->cost() << "/" << beginning_gold << "\n";
		spectator.error(errbuf.str());
	}

	actions::recruit_unit(*u_type, current_team_num, loc, from);

	LOG_REPLAY << "recruit: team=" << current_team_num << " '" << type_id << "' at (" << loc
		<< ") cost=" << u_type->cost() << " from gold=" << beginning_gold << ' '
		<< "-> " << current_team.gold();
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(recall, child, spectator)
{
	int current_team_num = resources::controller->current_side();
	team &current_team = resources::gameboard->get_team(current_team_num);

	const std::string& unit_id = child["value"];
	map_location loc(child, resources::gamedata);
	map_location from(child.child_or_empty("from"), resources::gamedata);

	if(!actions::recall_unit(unit_id, current_team, loc, from, map_location::direction::indeterminate)) {
		spectator.error("illegal recall: unit_id '" + unit_id + "' could not be found within the recall list.\n");
		//when recall_unit returned false nothing happened so we can safety return false;
		return false;
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(attack, child, spectator)
{
	const auto destination = child.optional_child("destination");
	const auto source = child.optional_child("source");
	//check_checksums(*cfg);

	if (!destination) {
		spectator.error("no destination found in attack\n");
		return false;
	}

	if (!source) {
		spectator.error("no source found in attack \n");
		return false;
	}

	//we must get locations by value instead of by references, because the iterators
	//may become invalidated later
	const map_location src(source.value(), resources::gamedata);
	const map_location dst(destination.value(), resources::gamedata);

	int weapon_num = child["weapon"].to_int();
	// having defender_weapon in the replay fixes a bug (OOS) where one player (or observer) chooses a different defensive weapon.
	// Xan pointed out this was a possibility: we calculate defense weapon
	// now based on attack_prediction code, but this uses floating point
	// calculations, which means that in the case where results are close,
	// rounding differences can mean that both ends choose different weapons.
	int def_weapon_num = child["defender_weapon"].to_int(-2);
	if (def_weapon_num == -2) {
		// Let's not gratuitously destroy backwards compatibility.
		LOG_REPLAY << "Old data, having to guess weapon";
		def_weapon_num = -1;
	}

	unit_map::iterator u = resources::gameboard->units().find(src);
	if (!u.valid()) {
		spectator.error("unfound location for source of attack\n");
		return false;
	}

	if (child.has_attribute("attacker_type")) {
		const std::string &att_type_id = child["attacker_type"];
		if (u->type_id() != att_type_id) {
			WRN_REPLAY << "unexpected attacker type: " << att_type_id << "(game state gives: " << u->type_id() << ")";
		}
	}

	if (static_cast<unsigned>(weapon_num) >= u->attacks().size()) {
		spectator.error("illegal weapon type in attack\n");
		return false;
	}

	unit_map::const_iterator tgt = resources::gameboard->units().find(dst);

	if (!tgt.valid()) {
		std::stringstream errbuf;
		errbuf << "unfound defender for attack: " << src << " -> " << dst << '\n';
		spectator.error(errbuf.str());
		return false;
	}

	if (child.has_attribute("defender_type")) {
		const std::string &def_type_id = child["defender_type"];
		if (tgt->type_id() != def_type_id) {
			WRN_REPLAY << "unexpected defender type: " << def_type_id << "(game state gives: " << tgt->type_id() << ")";
		}
	}

	if (def_weapon_num >= static_cast<int>(tgt->attacks().size())) {

		spectator.error("illegal defender weapon type in attack\n");
		return false;
	}

	DBG_REPLAY << "Attacker XP (before attack): " << u->experience();

	synced_context::block_undo();
	bool show = !resources::controller->is_skipping_replay();
	attack_unit_and_advance(src, dst, weapon_num, def_weapon_num, show);
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(disband, child, spectator)
{

	team& current_team = resources::controller->current_team();

	const std::string& unit_id = child["value"];
	std::size_t old_size = current_team.recall_list().size();

	// Find the unit in the recall list.
	unit_ptr dismissed_unit = current_team.recall_list().find_if_matches_id(unit_id);
	if (!dismissed_unit) {
		spectator.error("illegal disband\n");
		return false;
	}
	//add dismissal to the undo stack
	resources::undo_stack->add_dismissal(dismissed_unit);

	current_team.recall_list().erase_if_matches_id(unit_id);

	if (old_size == current_team.recall_list().size()) {
		spectator.error("illegal disband\n");
		return false;
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(move, child, spectator)
{
	team& current_team = resources::controller->current_team();

	std::vector<map_location> steps;

	try {
		read_locations(child,steps);
	} catch (const std::invalid_argument&) {
		WRN_REPLAY << "Warning: Path data contained something which could not be parsed to a sequence of locations:" << "\n config = " << child.debug();
		return false;
	}

	if(steps.empty())
	{
		WRN_REPLAY << "Warning: Missing path data found in [move]";
		return false;
	}

	const map_location& src = steps.front();
	const map_location& dst = steps.back();

	if (src == dst) {
		WRN_REPLAY << "Warning: Move with identical source and destination. Skipping...";
		return false;
	}

	// The nominal destination should appear to be unoccupied.
	unit_map::iterator u = resources::gameboard->find_visible_unit(dst, current_team);
	if ( u.valid() ) {
		WRN_REPLAY << "Warning: Move destination " << dst << " appears occupied.";
		// We'll still proceed with this movement, though, since
		// an event might intervene.
		// 'event' doesn't mean wml event but rather it means 'hidden' units form the movers point of view.
	}

	u = resources::gameboard->units().find(src);
	if (!u.valid()) {
		std::stringstream errbuf;
		errbuf << "unfound location for source of movement: "
			<< src << " -> " << dst << '\n';
		spectator.error(errbuf.str());
		return false;
	}

	bool skip_sighted = child["skip_sighted"] == "all";
	bool skip_ally_sighted = child["skip_sighted"] == "only_ally";

	actions::execute_move_unit(steps, skip_sighted, skip_ally_sighted, dynamic_cast<actions::move_unit_spectator*>(&spectator));

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(fire_event, child, /*spectator*/)
{
	if(const auto last_select = child.optional_child("last_select"))
	{
		//the select event cannot clear the undo stack.
		resources::game_events->pump().fire("select", map_location(last_select.value(), resources::gamedata));
	}
	const std::string &event_name = child["raise"];
	if (const auto source = child.optional_child("source")) {
		synced_context::block_undo(std::get<0>(resources::game_events->pump().fire(event_name, map_location(source.value(), resources::gamedata))));
	} else {
		synced_context::block_undo(std::get<0>(resources::game_events->pump().fire(event_name)));
	}

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(custom_command, child, /*spectator*/)
{
	assert(resources::lua_kernel);
	resources::lua_kernel->custom_command(child["name"], child.child_or_empty("data"));

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(auto_shroud, child, /*spectator*/)
{
	team &current_team = resources::controller->current_team();

	bool active = child["active"].to_bool();
	if(active && !current_team.auto_shroud_updates()) {
		resources::undo_stack->commit_vision();
	}
	current_team.set_auto_shroud_updates(active);
	if(resources::undo_stack->can_undo()) {
		resources::undo_stack->add_auto_shroud(active);
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(update_shroud, /*child*/, spectator)
{
	// When "updating shroud now" is used.
	// Updates fog/shroud based on the undo stack, then updates stack as needed.
	// This may fire events and change the game state.

	team &current_team = resources::controller->current_team();
	if(current_team.auto_shroud_updates()) {
		spectator.error("Team has DSU disabled but we found an explicit shroud update");
	}
	bool res = resources::undo_stack->commit_vision();
	if(res) {
		synced_context::block_undo();
	}
	return true;
}

namespace
{
	void debug_notification(const std::string& text, bool message_is_command = false)
	{
		auto& controller = *resources::controller;
		auto& current_team = controller.current_team();
		static bool ignore = false;
		bool show_long_message = controller.is_replay() || !current_team.is_local();

		std::string message;
		utils::string_map i18n_vars = {{ "player", current_team.current_player() }};

		if(i18n_vars["player"].empty()) {
			i18n_vars["player"] = _("(unknown player)");
		}

		if(message_is_command) {
			i18n_vars["command"] = text;
			message = VGETTEXT("The :$command debug command was used during $player’s turn", i18n_vars);
		} else {
			message = VGETTEXT(text.c_str(), i18n_vars);
		}

		if(show_long_message && !ignore) {
			play_controller::scoped_savegame_snapshot snapshot(controller);
			std::stringstream sbuilder;
			sbuilder << _("A player used a debug command during the game. If this is unexpected, it is possible the player in question is cheating.")
			         << "\n\n"
			         << _("Details:") << "\n"
			         << message
			         << "\n\n"
			         << _("Do you wish to save the game before continuing?");
			savegame::oos_savegame save(controller.get_saved_game(), ignore);
			save.set_title(_("Debug Command Used"));
			save.save_game_interactive(sbuilder.str(), savegame::savegame::YES_NO); // can throw quit_game_exception
		}
		else {
			display::announce_options announce_options;
			display::get_singleton()->announce(message, font::NORMAL_COLOR, announce_options);
		}
	}

	void debug_cmd_notification(const std::string& command)
	{
		debug_notification(command, true);
	}
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_terrain, child, /*spectator*/)
{
	synced_context::block_undo();
	debug_cmd_notification("terrain");

	map_location loc(child);
	const std::string& terrain_type = child["terrain_type"];
	const std::string& mode_str = child["mode_str"];

	bool result = resources::gameboard->change_terrain(loc, terrain_type, mode_str, false);
	if(result) {
		display::get_singleton()->invalidate(loc);
		game_display::get_singleton()->needs_rebuild(result);
		game_display::get_singleton()->maybe_rebuild();
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_unit, child, /*spectator*/)
{
	synced_context::block_undo();
	debug_cmd_notification("unit");
	map_location loc(child);
	const std::string name = child["name"];
	const std::string value = child["value"];

	unit_map::iterator i = resources::gameboard->units().find(loc);
	if (i == resources::gameboard->units().end()) {
		return false;
	}
	if (name == "advances" ) {
		int int_value = 0;
		try {
			int_value = std::stoi(value);
		} catch (const std::invalid_argument&) {
			WRN_REPLAY << "Warning: Invalid unit advancement argument: " << value;
			return false;
		}
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
			unit_ptr new_u = unit::create(cfg, true);
			new_u->set_location(loc);
			// Don't remove the unit until after we've verified there are no errors in creating the new one,
			// or else the unit would simply be removed from the map with no replacement.
			resources::gameboard->units().erase(loc);
			resources::whiteboard->on_kill_unit();
			resources::gameboard->units().insert(new_u);
		} catch(const unit_type::error& e) {
			ERR_REPLAY << e.what(); // TODO: more appropriate error message log
			return false;
		}
	}
	if (name == "fail") { //testcase for bug #18488
		assert(i.valid());
	}
	display::get_singleton()->invalidate(loc);
	game_display::get_singleton()->invalidate_unit();

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_create_unit, child, spectator)
{
	synced_context::block_undo();

	debug_notification(N_("A unit was created using debug mode during $player’s turn"));
	map_location loc(child);
	resources::whiteboard->on_kill_unit();
	const std::string& variation = child["variation"].str();
	const unit_race::GENDER gender = string_gender(child["gender"], unit_race::NUM_GENDERS);
	const unit_type *u_type = unit_types.find(child["type"]);
	if (!u_type) {
		spectator.error("Invalid unit type");
		return false;
	}

	const int side_num = resources::controller
			? resources::controller->current_side() : 1;

	// Create the unit.
	unit_ptr created = unit::create(*u_type, side_num, true, gender, variation);
	created->new_turn();

	unit_map::unit_iterator unit_it;

	// Add the unit to the board.
	std::tie(unit_it, std::ignore) = resources::gameboard->units().replace(loc, created);

	game_display::get_singleton()->invalidate_unit();
	resources::game_events->pump().fire("unit_placed", loc);
	unit_display::unit_recruited(loc);

	// Village capture?
	if ( resources::gameboard->map().is_village(loc) )
		actions::get_village(loc, created->side());

	// Update fog/shroud.
	// Not checking auto_shroud_updates() here since :create is not undoable. (#2196)
	actions::shroud_clearer clearer;
	clearer.clear_unit(loc, *created);
	clearer.fire_events();
	if (unit_it.valid() ) // In case sighted events messed with the unit.
		actions::actor_sighted(*unit_it);

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_lua, child, /*spectator*/)
{
	synced_context::block_undo();
	debug_cmd_notification("lua");
	resources::lua_kernel->run(child["code"].str().c_str(), "debug command");
	resources::controller->pump().flush_messages();

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_teleport, child, /*spectator*/)
{
	synced_context::block_undo();
	debug_cmd_notification("teleport");

	const map_location teleport_from(child["teleport_from_x"].to_int(), child["teleport_from_y"].to_int(), wml_loc());
	const map_location teleport_to(child["teleport_to_x"].to_int(), child["teleport_to_y"].to_int(), wml_loc());

	const unit_map::iterator unit_iter = resources::gameboard->units().find(teleport_from);
	if(unit_iter != resources::gameboard->units().end()) {
		if(unit_iter.valid()) {
			actions::teleport_unit_from_replay({teleport_from, teleport_to}, false, false, false);
		}
		display::get_singleton()->redraw_minimap();
	}

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_kill, child, /*spectator*/)
{
	synced_context::block_undo();
	debug_cmd_notification("kill");

	const map_location loc(child["x"].to_int(), child["y"].to_int(), wml_loc());
	const unit_map::iterator i = resources::gameboard->units().find(loc);
	if (i != resources::gameboard->units().end()) {
		const int dying_side = i->side();
		resources::controller->pump().fire("last_breath", loc, loc);
		if (i.valid()) {
			unit_display::unit_die(loc, *i);
		}
		display::get_singleton()->redraw_minimap();
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

SYNCED_COMMAND_HANDLER_FUNCTION(debug_next_level, child, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("next_level");

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

SYNCED_COMMAND_HANDLER_FUNCTION(debug_turn_limit, child, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("turn_limit");

	resources::tod_manager->set_number_of_turns(child["turn_limit"].to_int(-1));
	display::get_singleton()->queue_rerender();
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_turn, child, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("turn");

	resources::tod_manager->set_turn(child["turn"].to_int(1), resources::gamedata);

	game_display::get_singleton()->new_turn();
	display::get_singleton()->queue_rerender();

	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_set_var, child, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("set_var");

	try {
		resources::gamedata->set_variable(child["name"],child["value"]);
	}
	catch(const invalid_variablename_exception&) {
	//	command_failed(_("Variable not found"));
		return false;
	}
	return true;
}

SYNCED_COMMAND_HANDLER_FUNCTION(debug_gold, child, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("gold");

	resources::controller->current_team().spend_gold(-child["gold"].to_int(0));
	display::get_singleton()->queue_rerender();
	return true;
}


SYNCED_COMMAND_HANDLER_FUNCTION(debug_event, child, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("throw");

	resources::controller->pump().fire(child["eventname"]);
	display::get_singleton()->queue_rerender();

	return true;
}


SYNCED_COMMAND_HANDLER_FUNCTION(debug_fog, /*child*/, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("fog");

	team& current_team = resources::controller->current_team();
	current_team.set_fog(!current_team.uses_fog());
	actions::recalculate_fog(current_team.side());

	display::get_singleton()->recalculate_minimap();
	display::get_singleton()->queue_rerender();

	return true;
}


SYNCED_COMMAND_HANDLER_FUNCTION(debug_shroud, /*child*/, /*spectator*/)
{
	synced_context::block_undo();

	debug_cmd_notification("shroud");

	team& current_team = resources::controller->current_team();
	current_team.set_shroud(!current_team.uses_shroud());
	actions::clear_shroud(current_team.side());

	display::get_singleton()->recalculate_minimap();
	display::get_singleton()->queue_rerender();

	return true;
}
