/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * Implementations of action WML tags, other than those implemented in Lua, and
 * excluding conditional action WML.
 */

#include "game_events/action_wml.hpp"
#include "game_events/conditional_wml.hpp"
#include "game_events/pump.hpp"

#include "actions/attack.hpp"
#include "actions/create.hpp"
#include "actions/move.hpp"
#include "actions/vision.hpp"
#include "ai/manager.hpp"
#include "fake_unit_ptr.hpp"
#include "filesystem.hpp"
#include "game_classification.hpp"
#include "game_display.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "map/label.hpp"
#include "pathfind/teleport.hpp"
#include "pathfind/pathfind.hpp"
#include "persist_var.hpp"
#include "play_controller.hpp"
#include "recall_list_manager.hpp"
#include "replay.hpp"
#include "random.hpp"
#include "mouse_handler_base.hpp" // for events::commands_disabled
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "side_filter.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "synced_context.hpp"
#include "synced_user_choice.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "units/udisplay.hpp"
#include "units/filter.hpp"
#include "wml_exception.hpp"
#include "whiteboard/manager.hpp"

#include <boost/regex.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

static lg::log_domain log_wml("wml");
#define LOG_WML LOG_STREAM(info, log_wml)
#define WRN_WML LOG_STREAM(warn, log_wml)
#define ERR_WML LOG_STREAM(err, log_wml)

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)


// This file is in the game_events namespace.
namespace game_events
{

// This must be defined before any WML actions are.
// (So keep it at the rop of this file?)
wml_action::map wml_action::registry_;

namespace { // Support functions

	/**
	 * Converts a vconfig to a location (based on x,y=).
	 * The default parameter values cause the default return value (if neither
	 * x nor y is specified) to equal map_location::null_location().
	 */
	map_location cfg_to_loc(const vconfig& cfg, int defaultx = -999, int defaulty = -999)
	{
		return map_location(cfg["x"].to_int(defaultx), cfg["y"].to_int(defaulty), wml_loc());
	}

	fake_unit_ptr create_fake_unit(const vconfig& cfg)
	{
		std::string type = cfg["type"];
		std::string variation = cfg["variation"];
		std::string img_mods = cfg["image_mods"];

		size_t side_num = cfg["side"].to_int(1);
		if (!resources::gameboard->has_team(side_num)) {
			side_num = 1;
		}

		unit_race::GENDER gender = string_gender(cfg["gender"]);
		const unit_type *ut = unit_types.find(type);
		if (!ut) return fake_unit_ptr();
		fake_unit_ptr fake = fake_unit_ptr(unit_ptr(new unit(*ut, side_num, false, gender)));

		if(!variation.empty()) {
			config mod;
			config &effect = mod.add_child("effect");
			effect["apply_to"] = "variation";
			effect["name"] = variation;
			fake->add_modification("variation",mod);
		}

		if(!img_mods.empty()) {
			config mod;
			config &effect = mod.add_child("effect");
			effect["apply_to"] = "image_mod";
			effect["add"] = img_mods;
			fake->add_modification("image_mod",mod);
		}

		return fake;
	}

	std::vector<map_location> fake_unit_path(const unit& fake_unit, const std::vector<std::string>& xvals, const std::vector<std::string>& yvals)
	{
		const gamemap *game_map = & resources::gameboard->map();
		std::vector<map_location> path;
		map_location src;
		map_location dst;
		for(size_t i = 0; i != std::min(xvals.size(),yvals.size()); ++i) {
			if(i==0){
				try {
					src.set_wml_x(std::stoi(xvals[i]));
					src.set_wml_y(std::stoi(yvals[i]));
				} catch(std::invalid_argument) {
					ERR_CF << "Invalid move_unit_fake source: " << xvals[i] << ", " << yvals[i] << '\n';
					continue;
				}
				if (!game_map->on_board(src)) {
					ERR_CF << "Invalid move_unit_fake source: " << src << '\n';
					break;
				}
				path.push_back(src);
				continue;
			}
			pathfind::shortest_path_calculator calc(fake_unit,
					resources::gameboard->get_team(fake_unit.side()),
					resources::gameboard->teams(),
					*game_map);

			try {
				dst.set_wml_x(std::stoi(xvals[i]));
				dst.set_wml_y(std::stoi(yvals[i]));
			} catch(std::invalid_argument) {
				ERR_CF << "Invalid move_unit_fake destination: " << xvals[i] << ", " << yvals[i] << '\n';
			}
			if (!game_map->on_board(dst)) {
				ERR_CF << "Invalid move_unit_fake destination: " << dst << '\n';
				break;
			}

			pathfind::plain_route route = pathfind::a_star_search(src, dst, 10000, calc,
				game_map->w(), game_map->h());

			if (route.steps.empty()) {
				WRN_NG << "Could not find move_unit_fake route from " << src << " to " << dst << ": ignoring complexities" << std::endl;
				pathfind::emergency_path_calculator emergency_calc(fake_unit, *game_map);

				route = pathfind::a_star_search(src, dst, 10000, emergency_calc,
						game_map->w(), game_map->h());
				if(route.steps.empty()) {
					// This would occur when trying to do a MUF of a unit
					// over locations which are unreachable to it (infinite movement
					// costs). This really cannot fail.
					WRN_NG << "Could not find move_unit_fake route from " << src << " to " << dst << ": ignoring terrain" << std::endl;
					pathfind::dummy_path_calculator dummy_calc(fake_unit, *game_map);
					route = a_star_search(src, dst, 10000, dummy_calc, game_map->w(), game_map->h());
					assert(!route.steps.empty());
				}
			}
			// we add this section to the end of the complete path
			// skipping section's head because already included
			// by the previous iteration
			path.insert(path.end(),
					route.steps.begin()+1, route.steps.end());

			src = dst;
		}
		return path;
	}
} // end anonymous namespace (support functions)

/**
 * Using this constructor for a static object outside action_wml.cpp
 * will likely lead to a static initialization fiasco.
 * @param[in]  tag       The WML tag for this action.
 * @param[in]  function  The callback for this action.
 */
wml_action::wml_action(const std::string & tag, handler function)
{
	registry_[tag] = function;
}


/**
 * WML_HANDLER_FUNCTION macro handles auto registration for wml handlers
 *
 * @param pname wml tag name
 * @param pei the variable name of the queued_event object inside the function
 * @param pcfg the variable name of the config object inside the function
 *
 * You are warned! This is evil macro magic!
 *
 * The following code registers a [foo] tag:
 * \code
 * // comment out unused parameters to prevent compiler warnings
 * WML_HANDLER_FUNCTION(foo, event_info, cfg)
 * {
 *    // code for foo
 * }
 * \endcode
 *
 * Generated code looks like this:
 * \code
 * void wml_func_foo(...);
 * static wml_action wml_action_foo("foo", &wml_func_foo);
 * void wml_func_foo(...)
 * {
 *    // code for foo
 * }
 * \endcode
 */
#define WML_HANDLER_FUNCTION(pname, pei, pcfg) \
	static void wml_func_##pname(const queued_event &pei, const vconfig &pcfg); \
	static wml_action wml_action_##pname(#pname, &wml_func_##pname);  \
	static void wml_func_##pname(const queued_event& pei, const vconfig& pcfg)


/// Experimental data persistence
/// @todo Finish experimenting.
WML_HANDLER_FUNCTION(clear_global_variable,,pcfg)
{
	if (!resources::controller->is_replay())
		verify_and_clear_global_variable(pcfg);
}

static void on_replay_error(const std::string& message, bool /*b*/)
{
	ERR_NG << "Error via [do_command]:" << std::endl;
	ERR_NG << message << std::endl;
}

// This tag exposes part of the code path used to handle [command]'s in replays
// This allows to perform scripting in WML that will use the same code path as player actions, for example.
WML_HANDLER_FUNCTION(do_command,, cfg)
{
	// Doing this in a whiteboard applied context will cause bugs
	// Note that even though game_events::wml_event_pump() will always apply the real unit map
	// It is still possible get a wml commands to run in a whiteboard applied context
	// With the theme_items lua callbacks
	if(resources::whiteboard->has_planned_unit_map())
	{
		ERR_NG << "[do_command] called while whiteboard is applied, ignoring" << std::endl;
		return;
	}

	static const std::set<std::string> allowed_tags {"attack", "move", "recruit", "recall", "disband", "fire_event", "lua_ai"};

	const bool is_too_early = resources::gamedata->phase() != game_data::START && resources::gamedata->phase() != game_data::PLAY;
	const bool is_unsynced_too_early = resources::gamedata->phase() != game_data::PLAY;
	const bool is_unsynced = synced_context::get_synced_state() == synced_context::UNSYNCED;
	if(is_too_early)
	{
		ERR_NG << "[do_command] called too early, only allowed at START or later" << std::endl;
		return;
	}
	if(is_unsynced && resources::controller->is_lingering())
	{
		ERR_NG << "[do_command] cannot be used in linger mode" << std::endl;
		return;
	}
	if(is_unsynced && resources::controller->gamestate().init_side_done())
	{
		ERR_NG << "[do_command] cannot be used before the turn has started" << std::endl;
		return;
	}
	if(is_unsynced && is_unsynced_too_early)
	{
		ERR_NG << "[do_command] called too early" << std::endl;
		return;
	}
	if(is_unsynced && events::commands_disabled)
	{
		ERR_NG << "[do_command] cannot invoke synced commands while commands are blocked" << std::endl;
		return;
	}
	if(is_unsynced && !resources::controller->current_team().is_local())
	{
		ERR_NG << "[do_command] can only be used from clients that control the currently playing side" << std::endl;
		return;
	}
	for(vconfig::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++i)
	{
		if(allowed_tags.find( i.get_key()) == allowed_tags.end()) {
			ERR_NG << "unsupported tag [" << i.get_key() << "] in [do_command]" << std::endl;
			std::stringstream o;
			std::copy(allowed_tags.begin(), allowed_tags.end(), std::ostream_iterator<std::string>(o, " "));
			ERR_NG << "allowed tags: " << o.str() << std::endl;
			continue;
		}
		// TODO: afaik run_in_synced_context_if_not_already thows exceptions when the executed action end the scenario or the turn.
		//       This could cause problems, specially when its unclear whether that excetion is caught by lua or not...

		//Note that this fires related events and everthing else that also happen normally.
		//have to watch out with the undo stack, therefore forbid [auto_shroud] and [update_shroud] here...
		synced_context::run_in_synced_context_if_not_already(
			/*commandname*/ i.get_key(),
			/*data*/ i.get_child().get_parsed_config(),
			/*use_undo*/ true,
			/*show*/ true,
			/*error_handler*/ &on_replay_error
		);
	}
}

/// Experimental data persistence
/// @todo Finish experimenting.
WML_HANDLER_FUNCTION(get_global_variable,,pcfg)
{
	verify_and_get_global_variable(pcfg);
}

WML_HANDLER_FUNCTION(modify_turns,, cfg)
{
	config::attribute_value value = cfg["value"];
	std::string add = cfg["add"];
	config::attribute_value current = cfg["current"];
	tod_manager& tod_man = *resources::tod_manager;
	if(!add.empty()) {
		tod_man.modify_turns_by_wml(add);
	} else if(!value.empty()) {
		tod_man.set_number_of_turns_by_wml(value.to_int(-1));
	}
	// change current turn only after applying mods
	if(!current.empty()) {
		const unsigned int current_turn_number = tod_man.turn();
		int new_turn_number = current.to_int(current_turn_number);
		const unsigned int new_turn_number_u = static_cast<unsigned int>(new_turn_number);
		if(new_turn_number_u < 1 || (new_turn_number > tod_man.number_of_turns() && tod_man.number_of_turns() != -1)) {
			ERR_NG << "attempted to change current turn number to one out of range (" << new_turn_number << ")" << std::endl;
		} else if(new_turn_number_u != current_turn_number) {
			tod_man.set_turn_by_wml(new_turn_number_u, resources::gamedata);
			resources::screen->new_turn();
		}
	}
}

/// Moving a 'unit' - i.e. a dummy unit
/// that is just moving for the visual effect
WML_HANDLER_FUNCTION(move_unit_fake,, cfg)
{
	fake_unit_ptr dummy_unit(create_fake_unit(cfg));
	if(!dummy_unit.get())
		return;

	const bool force_scroll = cfg["force_scroll"].to_bool(true);

	const std::string x = cfg["x"];
	const std::string y = cfg["y"];

	const std::vector<std::string> xvals = utils::split(x);
	const std::vector<std::string> yvals = utils::split(y);

	const std::vector<map_location>& path = fake_unit_path(*dummy_unit, xvals, yvals);
	if (!path.empty()) {
		// Always scroll.
		unit_display::move_unit(path, dummy_unit.get_unit_ptr(), true, map_location::NDIRECTIONS, force_scroll);
	}
}

WML_HANDLER_FUNCTION(move_units_fake,, cfg)
{
	LOG_NG << "Processing [move_units_fake]\n";

	const vconfig::child_list unit_cfgs = cfg.get_children("fake_unit");
	size_t num_units = unit_cfgs.size();
	std::vector<fake_unit_ptr > units;
	units.reserve(num_units);
	std::vector<std::vector<map_location> > paths;
	paths.reserve(num_units);

	LOG_NG << "Moving " << num_units << " units\n";

	size_t longest_path = 0;

	for (const vconfig& config : unit_cfgs) {
		const std::vector<std::string> xvals = utils::split(config["x"]);
		const std::vector<std::string> yvals = utils::split(config["y"]);
		int skip_steps = config["skip_steps"];
		fake_unit_ptr u = create_fake_unit(config);
		units.push_back(u);
		paths.push_back(fake_unit_path(*u, xvals, yvals));
		if(skip_steps > 0)
			paths.back().insert(paths.back().begin(), skip_steps, paths.back().front());
		longest_path = std::max(longest_path, paths.back().size());
		DBG_NG << "Path " << paths.size() - 1 << " has length " << paths.back().size() << '\n';

		u->set_location(paths.back().front());
		u.place_on_fake_unit_manager(resources::fake_units);
	}

	LOG_NG << "Units placed, longest path is " << longest_path << " long\n";

	std::vector<map_location> path_step(2);
	path_step.resize(2);
	for(size_t step = 1; step < longest_path; ++step) {
		DBG_NG << "Doing step " << step << "...\n";
		for(size_t un = 0; un < num_units; ++un) {
			if(step >= paths[un].size() || paths[un][step - 1] == paths[un][step])
				continue;
			DBG_NG << "Moving unit " << un << ", doing step " << step << '\n';
			path_step[0] = paths[un][step - 1];
			path_step[1] = paths[un][step];
			unit_display::move_unit(path_step, units[un].get_unit_ptr());
			units[un]->set_location(path_step[1]);
			units[un]->anim_comp().set_standing();
		}
	}

	LOG_NG << "Units moved\n";
}

/// If we should recall units that match a certain description.
// If you change attributes specific to [recall] (that is, not a Standard Unit Filter)
// be sure to update data/lua/wml_tag, auto_recall feature for [role] to reflect your changes.
WML_HANDLER_FUNCTION(recall,, cfg)
{
	LOG_NG << "recalling unit...\n";
	config temp_config(cfg.get_config());
	// Prevent the recall unit filter from using the location as a criterion

	/**
	 * @todo FIXME: we should design the WML to avoid these types of
	 * collisions; filters should be named consistently and always have a
	 * distinct scope.
	 */
	temp_config["x"] = "recall";
	temp_config["y"] = "recall";
	vconfig unit_filter_cfg(temp_config);
	const vconfig & leader_filter = cfg.child("secondary_unit");

	for(int index = 0; index < int(resources::gameboard->teams().size()); ++index) {
		LOG_NG << "for side " << index + 1 << "...\n";
		const std::string player_id = resources::gameboard->teams()[index].save_id();

		if(resources::gameboard->teams()[index].recall_list().size() < 1) {
			DBG_NG << "recall list is empty when trying to recall!\n"
				   << "player_id: " << player_id << " side: " << index+1 << "\n";
			continue;
		}

		recall_list_manager & avail = resources::gameboard->teams()[index].recall_list();
		std::vector<unit_map::unit_iterator> leaders = resources::gameboard->units().find_leaders(index + 1);

		const unit_filter ufilt(unit_filter_cfg, resources::filter_con);
		const unit_filter lfilt(leader_filter, resources::filter_con); // Note that if leader_filter is null, this correctly gives a null filter that matches all units.
		for(std::vector<unit_ptr>::iterator u = avail.begin(); u != avail.end(); ++u) {
			DBG_NG << "checking unit against filter...\n";
			scoped_recall_unit auto_store("this_unit", player_id, u - avail.begin());
			if (ufilt(*(*u), map_location())) {
				DBG_NG << (*u)->id() << " matched the filter...\n";
				const unit_ptr to_recruit = *u;
				const unit* pass_check = to_recruit.get();
				if(!cfg["check_passability"].to_bool(true)) pass_check = nullptr;
				const map_location cfg_loc = cfg_to_loc(cfg);

				/// @todo fendrin: comment this monster
				for (unit_map::const_unit_iterator leader : leaders) {
					DBG_NG << "...considering " + leader->id() + " as the recalling leader...\n";
					map_location loc = cfg_loc;
					if ( lfilt(*leader)  &&
					     unit_filter(vconfig(leader->recall_filter()), resources::filter_con).matches( *(*u),map_location() ) ) {
						DBG_NG << "...matched the leader filter and is able to recall the unit.\n";
						if(!resources::gameboard->map().on_board(loc))
							loc = leader->get_location();
						if(pass_check || (resources::gameboard->units().count(loc) > 0))
							loc = pathfind::find_vacant_tile(loc, pathfind::VACANT_ANY, pass_check);
						if(resources::gameboard->map().on_board(loc)) {
							DBG_NG << "...valid location for the recall found. Recalling.\n";
							avail.erase(u);	// Erase before recruiting, since recruiting can fire more events
							actions::place_recruit(to_recruit, loc, leader->get_location(), 0, true,
							                       map_location::parse_direction(cfg["facing"]),
							                       cfg["show"].to_bool(true), cfg["fire_event"].to_bool(false),
							                       true, true);
							return;
						}
					}
				}
				if (resources::gameboard->map().on_board(cfg_loc)) {
					map_location loc = cfg_loc;
					if(pass_check || (resources::gameboard->units().count(loc) > 0))
						loc = pathfind::find_vacant_tile(loc, pathfind::VACANT_ANY, pass_check);
					// Check if we still have a valid location
					if (resources::gameboard->map().on_board(loc)) {
						DBG_NG << "No usable leader found, but found usable location. Recalling.\n";
						avail.erase(u);	// Erase before recruiting, since recruiting can fire more events
						map_location null_location = map_location::null_location();
						actions::place_recruit(to_recruit, loc, null_location, 0, true,
						                       map_location::parse_direction(cfg["facing"]),
						                       cfg["show"].to_bool(true), cfg["fire_event"].to_bool(false),
						                       true, true);
						return;
					}
				}
			}
		}
	}
	LOG_WML << "A [recall] tag with the following content failed:\n" << cfg.get_config().debug();
}

namespace {
	struct map_choice : public mp_sync::user_choice
	{
		map_choice(const std::string& filename) : filename_(filename) {}
		std::string filename_;
		virtual config query_user(int /*side*/) const
		{
			//Do a regex check for the file format to prevent sending aribitary files to other clients.
			//Note: this allows only the new format.
			static const std::string s_simple_terrain = R"""([A-Za-z\\|/]{1,4})""";
			static const std::string s_terrain = s_simple_terrain + R"""((\^)""" + s_simple_terrain + ")?";
			static const std::string s_sep = "(, |\\n)";
			static const std::string s_prefix = R"""((\d+ )?)""";
			static const std::string s_all = "(" + s_prefix + s_terrain + s_sep + ")+";
			static const boost::regex r_all(s_all);

			const std::string& mapfile = filesystem::get_wml_location(filename_);
			std::string res = "";
			if(filesystem::file_exists(mapfile)) {
				res = filesystem::read_file(mapfile);
			}
			config retv;
			if(boost::regex_match(res, r_all))
			{
				retv["map_data"] = res;
			}
			return retv;
		}
		virtual config random_choice(int /*side*/) const
		{
			return config();
		}
		virtual std::string description() const
		{
			return "Map Data";
		}

	};
}

/// Experimental map replace
/// @todo Finish experimenting.
WML_HANDLER_FUNCTION(replace_map,, cfg)
{
	/*
	 * When a hex changes from a village terrain to a non-village terrain, and
	 * a team owned that village it loses that village. When a hex changes from
	 * a non-village terrain to a village terrain and there is a unit on that
	 * hex it does not automatically capture the village. The reason for not
	 * capturing villages it that there are too many choices to make; should a
	 * unit loose its movement points, should capture events be fired. It is
	 * easier to do this as wanted by the author in WML.
	 */

	const gamemap * game_map = & resources::gameboard->map();
	gamemap map(*game_map);

	try {
		if(!cfg["map_file"].empty()) {
			config file_cfg = mp_sync::get_user_choice("map_data", map_choice(cfg["map_file"].str()));
			map.read(file_cfg["map_data"].str(), false);
		} else {
			map.read(cfg["map"], false);
		}
	} catch(incorrect_map_format_error&) {
		const std::string log_map_name = cfg["map"].empty() ? cfg["file"] : std::string("from inline data");
		lg::wml_error() << "replace_map: Unable to load map " << log_map_name << std::endl;
		return;
	} catch(wml_exception& e) {
		e.show(resources::screen->video());
		return;
	}

	if (map.total_width() > game_map->total_width()
	|| map.total_height() > game_map->total_height()) {
		if (!cfg["expand"].to_bool()) {
			lg::wml_error() << "replace_map: Map dimension(s) increase but expand is not set" << std::endl;
			return;
		}
	}

	if (map.total_width() < game_map->total_width()
	|| map.total_height() < game_map->total_height()) {
		if (!cfg["shrink"].to_bool()) {
			lg::wml_error() << "replace_map: Map dimension(s) decrease but shrink is not set" << std::endl;
			return;
		}
	}

	boost::optional<std::string> errmsg = resources::gameboard->replace_map(map);

	if (errmsg) {
		lg::wml_error() << *errmsg << std::endl;
	}

	resources::screen->reload_map();
	resources::screen->needs_rebuild(true);
	ai::manager::raise_map_changed();
}

/// Experimental data persistence
/// @todo Finish experimenting.
WML_HANDLER_FUNCTION(set_global_variable,,pcfg)
{
	if (!resources::controller->is_replay())
		verify_and_set_global_variable(pcfg);
}

WML_HANDLER_FUNCTION(set_variables,, cfg)
{
	const t_string& name = cfg["name"];
	variable_access_create dest = resources::gamedata->get_variable_access_write(name);
	if(name.empty()) {
		ERR_NG << "trying to set a variable with an empty name:\n" << cfg.get_config().debug();
		return;
	}

	std::vector<config> data;
	if(cfg.has_attribute("to_variable"))
	{
		try
		{
			variable_access_const tovar = resources::gamedata->get_variable_access_read(cfg["to_variable"]);
			for (const config& c : tovar.as_array())
			{
				data.push_back(c);
			}
		}
		catch(const invalid_variablename_exception&)
		{
			ERR_NG << "Cannot do [set_variables] with invalid to_variable variable: " << cfg["to_variable"] << " with " << cfg.get_config().debug() << std::endl;
		}
	} else {
		typedef std::pair<std::string, vconfig> vchild;
		for (const vchild& p : cfg.all_ordered()) {
			if(p.first == "value") {
				data.push_back(p.second.get_parsed_config());
			} else if(p.first == "literal") {
				data.push_back(p.second.get_config());
			} else if(p.first == "split") {
				const vconfig & split_element = p.second;

				std::string split_string=split_element["list"];
				std::string separator_string=split_element["separator"];
				std::string key_name=split_element["key"];
				if(key_name.empty())
				{
					key_name="value";
				}

				bool remove_empty = split_element["remove_empty"].to_bool();

				char* separator = separator_string.empty() ? nullptr : &separator_string[0];

				std::vector<std::string> split_vector;

				//if no separator is specified, explode the string
				if(separator == nullptr)
				{
					for(std::string::iterator i=split_string.begin(); i!=split_string.end(); ++i)
					{
						split_vector.push_back(std::string(1, *i));
					}
				}
				else {
					split_vector=utils::split(split_string, *separator, remove_empty ? utils::REMOVE_EMPTY | utils::STRIP_SPACES : utils::STRIP_SPACES);
				}

				for(std::vector<std::string>::iterator i=split_vector.begin(); i!=split_vector.end(); ++i)
				{
					data.emplace_back(config {key_name, *i});
				}
			}
		}
	}
	try
	{
		const std::string& mode = cfg["mode"];
		if(mode == "merge")
		{
			if(dest.explicit_index() && data.size() > 1)
			{
				//merge children into one
				config merged_children;
				for (const config &ch : data) {
					merged_children.append(ch);
				}
				data = {merged_children};
			}
			dest.merge_array(data);
		}
		else if(mode == "insert")
		{
			dest.insert_array(data);
		}
		else if(mode == "append")
		{
			dest.append_array(data);
		}
		else /*default if(mode == "replace")*/
		{
			dest.replace_array(data);
		}
	}
	catch(const invalid_variablename_exception&)
	{
		ERR_NG << "Cannot do [set_variables] with invalid destination variable: " << name << " with " << cfg.get_config().debug() << std::endl;
	}
}

/// Store the relative direction from one hex to another in a WML variable.
/// This is mainly useful as a diagnostic tool, but could be useful
/// for some kind of scenario.
WML_HANDLER_FUNCTION(store_relative_direction,, cfg)
{
	if (!cfg.child("source")) {
		WRN_NG << "No source in [store_relative_direction]" << std::endl;
		return;
	}
	if (!cfg.child("destination")) {
		WRN_NG << "No destination in [store_relative_direction]" << std::endl;
		return;
	}
	if (!cfg.has_attribute("variable")) {
		WRN_NG << "No variable in [store_relative_direction]" << std::endl;
		return;
	}

	const map_location src = cfg_to_loc(cfg.child("source"));
	const map_location dst = cfg_to_loc(cfg.child("destination"));

	std::string variable = cfg["variable"];
	map_location::RELATIVE_DIR_MODE mode = static_cast<map_location::RELATIVE_DIR_MODE> (cfg["mode"].to_int(0));
	try
	{
		variable_access_create store = resources::gamedata->get_variable_access_write(variable);

		store.as_scalar() = map_location::write_direction(src.get_relative_dir(dst,mode));
	}
	catch(const invalid_variablename_exception&)
	{
		ERR_NG << "Cannot do [store_relative_direction] with invalid destination variable: " << variable << " with " << cfg.get_config().debug() << std::endl;
	}
}

/// Store the rotation of one hex around another in a WML variable.
/// In increments of 60 degrees, clockwise.
/// This is mainly useful as a diagnostic tool, but could be useful
/// for some kind of scenario.
WML_HANDLER_FUNCTION(store_rotate_map_location,, cfg)
{
	if (!cfg.child("source")) {
		WRN_NG << "No source in [store_rotate_map_location]" << std::endl;
		return;
	}
	if (!cfg.child("destination")) {
		WRN_NG << "No destination in [store_rotate_map_location]" << std::endl;
		return;
	}
	if (!cfg.has_attribute("variable")) {
		WRN_NG << "No variable in [store_rotate_map_location]" << std::endl;
		return;
	}

	const map_location src = cfg_to_loc(cfg.child("source"));
	const map_location dst = cfg_to_loc(cfg.child("destination"));

	std::string variable = cfg["variable"];
	int angle = cfg["angle"].to_int(1);

	try
	{
		variable_access_create store = resources::gamedata->get_variable_access_write(variable);

		dst.rotate_right_around_center(src,angle).write(store.as_container());
	}
	catch(const invalid_variablename_exception&)
	{
		ERR_NG << "Cannot do [store_rotate_map_location] with invalid destination variable: " << variable << " with " << cfg.get_config().debug() << std::endl;
	}
}


/// Store time of day config in a WML variable. This is useful for those who
/// are too lazy to calculate the corresponding time of day for a given turn,
/// or if the turn / time-of-day sequence mutates in a scenario.
WML_HANDLER_FUNCTION(store_time_of_day,, cfg)
{
	const map_location loc = cfg_to_loc(cfg);
	int turn = cfg["turn"];
	// using 0 will use the current turn
	const time_of_day& tod = resources::tod_manager->get_time_of_day(loc,turn);

	std::string variable = cfg["variable"];
	if(variable.empty()) {
		variable = "time_of_day";
	}
	try
	{
		variable_access_create store = resources::gamedata->get_variable_access_write(variable);
		tod.write(store.as_container());
	}
	catch(const invalid_variablename_exception&)
	{
		ERR_NG << "Found invalid variablename " << variable << " in [store_time_of_day] with " << cfg.get_config().debug() << "\n";
	}
}

/// Creating a mask of the terrain
WML_HANDLER_FUNCTION(terrain_mask,, cfg)
{
	map_location loc = cfg_to_loc(cfg, 1, 1);

	gamemap mask_map(resources::gameboard->map().tdata(), "");

	try {
		if(!cfg["mask_file"].empty()) {
			const std::string& maskfile = filesystem::get_wml_location(cfg["mask_file"].str());

			if(filesystem::file_exists(maskfile)) {
				mask_map.read(filesystem::read_file(maskfile), false);
			} else {
				throw incorrect_map_format_error("Invalid file path");
			}
		} else {
			mask_map.read(cfg["mask"], false);
		}
	} catch(incorrect_map_format_error&) {
		ERR_NG << "terrain mask is in the incorrect format, and couldn't be applied" << std::endl;
		return;
	} catch(wml_exception& e) {
		e.show(resources::screen->video());
		return;
	}

	if (!cfg["border"].to_bool(true)) {
		mask_map.add_fog_border();
	}

	resources::gameboard->overlay_map(mask_map, cfg.get_parsed_config(), loc);
	resources::screen->needs_rebuild(true);
}

WML_HANDLER_FUNCTION(tunnel,, cfg)
{
	const bool remove = cfg["remove"].to_bool(false);
	const bool delay = cfg["delayed_variable_substitution"].to_bool(true);
	if (remove) {
		const std::vector<std::string> ids = utils::split(cfg["id"]);
		for (const std::string &id : ids) {
			resources::tunnels->remove(id);
		}
	} else if (cfg.get_children("source").empty() ||
		cfg.get_children("target").empty() ||
		cfg.get_children("filter").empty()) {
		ERR_WML << "[tunnel] is missing a mandatory tag:\n"
			 << cfg.get_config().debug();
	} else {
		pathfind::teleport_group tunnel(delay ? cfg : vconfig(cfg.get_parsed_config()), false);
		resources::tunnels->add(tunnel);

		if(cfg["bidirectional"].to_bool(true)) {
			tunnel = pathfind::teleport_group(delay ? cfg : vconfig(cfg.get_parsed_config()), true);
			resources::tunnels->add(tunnel);
		}
	}
}

/// If we should spawn a new unit on the map somewhere
WML_HANDLER_FUNCTION(unit,, cfg)
{
	config parsed_cfg = cfg.get_parsed_config();

	config::attribute_value to_variable = cfg["to_variable"];
	if (!to_variable.blank())
	{
		parsed_cfg.remove_attribute("to_variable");
		unit new_unit(parsed_cfg, true, &cfg);
		try
		{
			config &var = resources::gamedata->get_variable_cfg(to_variable);
			var.clear();
			new_unit.write(var);
			if (const config::attribute_value *v = parsed_cfg.get("x")) var["x"] = *v;
			if (const config::attribute_value *v = parsed_cfg.get("y")) var["y"] = *v;
		}
		catch(const invalid_variablename_exception&)
		{
			ERR_NG << "Cannot do [unit] with invalid to_variable:  " << to_variable << " with " << cfg.get_config().debug() << std::endl;
		}
		return;

	}

	int side = parsed_cfg["side"].to_int(1);


	if ((side<1)||(side > static_cast<int>(resources::gameboard->teams().size()))) {
		ERR_NG << "wrong side in [unit] tag - no such side: "<<side<<" ( number of teams :"<<resources::gameboard->teams().size()<<")"<<std::endl;
		DBG_NG << parsed_cfg.debug();
		return;
	}
	team &tm = resources::gameboard->get_team(side);

	unit_creator uc(tm,resources::gameboard->map().starting_position(side));

	uc
		.allow_add_to_recall(true)
		.allow_discover(true)
		.allow_get_village(true)
		.allow_invalidate(true)
		.allow_rename_side(true)
		.allow_show(true);

	uc.add_unit(parsed_cfg, &cfg);

}

WML_HANDLER_FUNCTION(on_undo, event_info, cfg)
{
	if(cfg["delayed_variable_substitution"].to_bool(false)) {
		synced_context::add_undo_commands(cfg.get_config(), event_info);
	} else {
		synced_context::add_undo_commands(cfg.get_parsed_config(), event_info);
	}
}

WML_HANDLER_FUNCTION(on_redo, , )
{
}

} // end namespace game_events

