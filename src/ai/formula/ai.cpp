/*
	Copyright (C) 2008 - 2024
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

/**
 * @file
 * Defines formula ai candidate actions - headers
 */

#include "ai/formula/ai.hpp"

#include "ai/formula/callable_objects.hpp"   // for unit_callable, etc
#include "chat_events.hpp"              // for chat_handler, etc
#include "display_chat_manager.hpp"
#include "formula/function.hpp"         // for formula_expression
#include "game_board.hpp"         // for game_board
#include "game_display.hpp"       // for game_display
#include "log.hpp"                // for LOG_STREAM, logger, etc
#include "map/map.hpp"                      // for gamemap
#include "pathfind/pathfind.hpp"  // for plain_route, etc
#include "pathfind/teleport.hpp"  // for get_teleport_locations, etc
#include "recall_list_manager.hpp"      // for recall_list_manager
#include "resources.hpp"          // for gameboard, teams, units, etc
#include "serialization/string_utils.hpp"  // for split
#include "team.hpp"                     // for team
#include "terrain/filter.hpp"     // for terrain_filter
#include "time_of_day.hpp"              // for time_of_day
#include "tod_manager.hpp"        // for tod_manager
#include "tstring.hpp"                  // for t_string, operator+
#include "units/unit.hpp"               // for unit
#include "units/ptr.hpp"                 // for unit_ptr
#include "units/types.hpp"
#include "formula/formula.hpp"  // for formula_error, formula, etc
#include "map/location.hpp"  // for map_location, etc
#include "ai/formula/function_table.hpp"           // for ai_function_symbol_table
#include "ai/game_info.hpp"  // for move_result_ptr, move_map, etc
#include "ai/formula/candidates.hpp"               // for base_candidate_action, etc
#include "utils/variant.hpp"

#include <ctime>                       // for time
#include <vector>                       // for vector, allocator, etc

static lg::log_domain log_formula_ai("ai/engine/fai");
#define DBG_AI LOG_STREAM(debug, log_formula_ai)
#define LOG_AI LOG_STREAM(info, log_formula_ai)
#define WRN_AI LOG_STREAM(warn, log_formula_ai)
#define ERR_AI LOG_STREAM(err, log_formula_ai)

using namespace wfl;

namespace ai {

using ca_ptr = wfl::candidate_action_ptr;

ca_ptr formula_ai::load_candidate_action_from_config(const config& rc_action)
{
	ca_ptr new_ca;
	const std::string name = rc_action["name"];
	try {
		const std::string& type = rc_action["type"];

		if( type == "movement") {
			new_ca = std::make_shared<move_candidate_action>(name, type, rc_action, &function_table_);
		} else if( type == "attack") {
			new_ca = std::make_shared<attack_candidate_action>(name, type, rc_action, &function_table_);
		} else {
			ERR_AI << "Unknown candidate action type: " << type;
		}
	} catch(const formula_error& e) {
		handle_exception(e, "Error while registering candidate action '" + name + "'");
	}
	return new_ca;
}

int formula_ai::get_recursion_count() const{
	return recursion_counter_.get_count();
}

formula_ai::formula_ai(readonly_context &context, const config &cfg)
	:
	readonly_context_proxy(),
	formula_callable(),
	ai_ptr_(nullptr),
	cfg_(cfg),
	recursion_counter_(context.get_recursion_count()),
	keeps_cache_(),
	attacks_callable(*this, resources::gameboard->units()),
//	infinite_loop_guardian_(),
	vars_(),
	function_table_(*this)
{
	init_readonly_context_proxy(context);
	LOG_AI << "creating new formula ai";
}

void formula_ai::handle_exception(const formula_error& e) const
{
	handle_exception(e, "Error while parsing formula");
}

void formula_ai::handle_exception(const formula_error& e, const std::string& failed_operation) const
{
	LOG_AI << failed_operation << ": " << e.formula;
	display_message(failed_operation + ": " + e.formula);
	//if line number = 0, don't display info about filename and line number
	if (e.line != 0) {
		LOG_AI << e.type << " in " << e.filename << ":" << e.line;
		display_message(e.type + " in " + e.filename + ":" + std::to_string(e.line));
	} else {
		LOG_AI << e.type;
		display_message(e.type);
	}
}

void formula_ai::display_message(const std::string& msg) const
{
	game_display::get_singleton()->get_chat_manager().add_chat_message(std::time(nullptr), "wfl", get_side(), msg,
				events::chat_handler::MESSAGE_PUBLIC, false);

}

formula_ptr formula_ai::create_optional_formula(const std::string& formula_string) const {
	try{
		return formula::create_optional_formula(formula_string, &function_table_);
	}
	catch(const formula_error& e) {
		handle_exception(e);
		return wfl::formula_ptr();
	}
}

void formula_ai::set_ai_context(ai_context *context)
{
	ai_ptr_ = context;
}

std::string formula_ai::evaluate(const std::string& formula_str)
{
	try{

		formula f(formula_str, &function_table_);

		map_formula_callable callable(fake_ptr());

		//formula_debugger fdb;
		const variant v = f.evaluate(callable,nullptr);

		if (ai_ptr_) {
			variant var = variant(this->fake_ptr()).execute_variant(v);

			if (  !var.is_empty() ) {
				return "Made move: " + var.to_debug_string();
			}
		}

		return v.to_debug_string();
	}
	catch(formula_error& e) {
		e.line = 0;
		handle_exception(e);
		throw;
	}
}

wfl::variant formula_ai::make_action(const wfl::const_formula_ptr& formula_, const wfl::formula_callable& variables)
{
	if (!formula_) {
		throw formula_error("null formula passed to make_action","","formula",0);
	}
	LOG_AI << "do move...";
	const variant var = formula_->evaluate(variables);
	variant res;

	if (ai_ptr_) {
		res = variant(this->fake_ptr()).execute_variant(var);
	} else {
		ERR_AI << "skipped execution of action because ai context is not set correctly";
	}

	return res;
}

pathfind::plain_route formula_ai::shortest_path_calculator(const map_location &src,
	const map_location &dst, unit_map::iterator &unit_it,
	pathfind::teleport_map& allowed_teleports) const
{
	map_location destination = dst;

	unit_map &units_ = resources::gameboard->units();
	pathfind::shortest_path_calculator calc(*unit_it, current_team(), resources::gameboard->teams(), resources::gameboard->map());

	unit_map::const_iterator dst_un = units_.find(destination);

	map_location res;

	if( dst_un != units_.end() ) {
		//there is unit standing at dst, let's try to find free hex to move to
		const map_location::direction preferred = destination.get_relative_dir(src);

		int best_rating = 100;//smaller is better
		const auto adj = get_adjacent_tiles(destination);

		for(std::size_t n = 0; n < adj.size(); ++n) {
			if(resources::gameboard->map().on_board(adj[n]) == false) {
				continue;
			}

			if(units_.find(adj[n]) != units_.end()) {
				continue;
			}

			static constexpr std::size_t ndirections = static_cast<int>(map_location::direction::indeterminate);
			unsigned int difference = std::abs(static_cast<int>(static_cast<int>(preferred) - n));
			if(difference > ndirections/2) {
				difference = ndirections - difference;
			}

			const int rating = difference * 2;
			if(rating < best_rating || res.valid() == false) {
				best_rating = rating;
				res = adj[n];
			}
		}
	}

	if( res != map_location() ) {
		destination = res;
	}

	pathfind::plain_route route = pathfind::a_star_search(src, destination,
		1000.0, calc, resources::gameboard->map().w(),
		resources::gameboard->map().h(), &allowed_teleports);

	return route;
}

pathfind::teleport_map formula_ai::get_allowed_teleports(unit_map::iterator& unit_it) const
{
	return pathfind::get_teleport_locations(*unit_it, current_team(), true);
}

void formula_ai::add_formula_function(const std::string& name, const const_formula_ptr& formula, const const_formula_ptr& precondition, const std::vector<std::string>& args)
{
	function_table_.add_function(name, std::make_shared<user_formula_function>(name, formula, precondition, args));
}

namespace
{
template<typename Container>
variant villages_from_set(const Container& villages, const std::set<map_location>* exclude = nullptr)
{
	std::vector<variant> vars;
	for(const map_location& loc : villages) {
		if(exclude && exclude->count(loc)) {
			continue;
		}
		vars.emplace_back(std::make_shared<location_callable>(loc));
	}

	return variant(vars);
}

// TODO: I have no damn idea what to name this function
variant visit_helper(const utils::variant<bool, std::vector<std::string>>& input)
{
	return utils::visit(
		[](const auto& v) {
			if constexpr(utils::decayed_is_same<bool, decltype(v)>) {
				return variant(v);
			} else {
				const std::vector<variant> vars(v.begin(), v.end());
				return variant(vars);
			}
		},
		input);
}
} // namespace

variant formula_ai::get_value(const std::string& key) const
{
	const unit_map& units = resources::gameboard->units();

	if(key == "aggression")
	{
		return variant(get_aggression()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "allow_ally_villages")
	{
		return visit_helper(get_allow_ally_villages());

	} else if(key == "avoid")
	{
		std::set<map_location> av_locs;
		get_avoid().get_locations(av_locs);
		return villages_from_set(av_locs);

	} else if(key == "caution")
	{
		return variant(get_caution()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "grouping")
	{
		return variant(get_grouping());

	} else if(key == "leader_aggression")
	{
		return variant(get_leader_aggression()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "leader_ignores_keep")
	{
		return visit_helper(get_leader_ignores_keep());

	} else if(key == "leader_value")
	{
		return variant(get_leader_value()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "passive_leader")
	{
		return visit_helper(get_passive_leader());

	} else if(key == "passive_leader_shares_keep")
	{
		return visit_helper(get_passive_leader_shares_keep());

	} else if(key == "recruitment_pattern")
	{
		const std::vector<std::string> &rp = get_recruitment_pattern();
		std::vector<variant> vars;
		for(const std::string &i : rp) {
			vars.emplace_back(i);
		}
		return variant(vars);

	} else if(key == "retreat_enemy_weight")
	{
		return variant(get_retreat_enemy_weight()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "retreat_factor")
	{
		return variant(get_retreat_factor()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "scout_village_targeting")
	{
		return variant(get_scout_village_targeting()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "support_villages")
	{
		return variant(get_support_villages());

	} else if(key == "village_value")
	{
		return variant(get_village_value()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "villages_per_scout")
	{
		return variant(get_villages_per_scout());

	} else if(key == "attacks")
	{
		return get_attacks_as_variant();

	} else if(key == "turn")
	{
		return variant(resources::tod_manager->turn());

	} else if(key == "time_of_day")
	{
		return variant(resources::tod_manager->get_time_of_day().id);

	} else if(key == "my_side")
	{
		return variant(std::make_shared<team_callable>(resources::gameboard->get_team(get_side())));

	} else if(key == "my_side_number")
	{
		return variant(get_side()-1);

	} else if(key == "teams")
	{
		std::vector<variant> vars;
		for(const team& t : resources::gameboard->teams()) {
			vars.emplace_back(std::make_shared<team_callable>(t));
		}
		return variant(vars);

	} else if(key == "allies")
	{
		std::vector<variant> vars;
		for( std::size_t i = 0; i < resources::gameboard->teams().size(); ++i) {
			if ( !current_team().is_enemy( i+1 ) )
				vars.emplace_back(i);
		}
		return variant(vars);

	} else if(key == "enemies")
	{
		std::vector<variant> vars;
		for( std::size_t i = 0; i < resources::gameboard->teams().size(); ++i) {
			if ( current_team().is_enemy( i+1 ) )
				vars.emplace_back(i);
		}
		return variant(vars);

	} else if(key == "my_recruits")
	{
		std::vector<variant> vars;

		unit_types.build_all(unit_type::FULL);

		for(const std::string& recruit : current_team().recruits()) {
			if(const unit_type* ut = unit_types.find(recruit)) {
				vars.emplace_back(std::make_shared<unit_type_callable>(*ut));
			}
		}
		return variant(vars);

	} else if(key == "recruits_of_side")
	{
		std::vector<variant> vars;
		std::vector<std::vector<variant>> tmp(resources::gameboard->teams().size());

		unit_types.build_all(unit_type::FULL);

		for(std::size_t i = 0; i < tmp.size(); ++i) {
			for(const std::string& recruit : resources::gameboard->teams()[i].recruits()) {
				if(const unit_type* ut = unit_types.find(recruit)) {
					tmp[i].emplace_back(std::make_shared<unit_type_callable>(*ut));
				}
			}
		}

		for( std::size_t i = 0; i<tmp.size(); ++i)
			vars.emplace_back(tmp[i]);
		return variant(vars);

	} else if(key == "units")
	{
		std::vector<variant> vars;
		for(const unit& u : units) {
			vars.emplace_back(std::make_shared<unit_callable>(u));
		}
		return variant(vars);

	} else if(key == "units_of_side")
	{
		std::vector<variant> vars;
		std::vector<std::vector<variant>> tmp(resources::gameboard->teams().size());
		for(const unit &u : units) {
			tmp[u.side() - 1].emplace_back(std::make_shared<unit_callable>(u));
		}
		for( std::size_t i = 0; i<tmp.size(); ++i)
			vars.emplace_back(tmp[i]);
		return variant(vars);

	} else if(key == "my_units")
	{
		std::vector<variant> vars;
		for(const unit& u : units) {
			if(u.side() == get_side()) {
				vars.emplace_back(std::make_shared<unit_callable>(u));
			}
		}
		return variant(vars);

	} else if(key == "enemy_units")
	{
		std::vector<variant> vars;
		for(const unit& u : units) {
			if(current_team().is_enemy(u.side()) && !u.incapacitated()) {
				vars.emplace_back(std::make_shared<unit_callable>(u));
			}
		}
		return variant(vars);

	} else if(key == "my_moves")
	{
		return variant(std::make_shared<move_map_callable>(get_srcdst(), get_dstsrc(), units));

	} else if(key == "my_attacks")
	{
		return variant(attacks_callable.fake_ptr());
	} else if(key == "enemy_moves")
	{
		return variant(std::make_shared<move_map_callable>(get_enemy_srcdst(), get_enemy_dstsrc(), units));

	} else if(key == "my_leader")
	{
		unit_map::const_iterator i = units.find_leader(get_side());
		if(i == units.end()) {
			return variant();
		}
		return variant(std::make_shared<unit_callable>(*i));

	} else if(key == "recall_list")
	{
		std::vector<variant> tmp;
		for(const unit_ptr& ptr : current_team().recall_list()) {
			tmp.emplace_back(std::make_shared<unit_callable>(*ptr));
		}

		return variant(tmp);

	} else if(key == "vars")
	{
		return variant(vars_.fake_ptr());
	} else if(key == "keeps")
	{
		return get_keeps();
	} else if(key == "map")
	{
		return variant(std::make_shared<gamemap_callable>(*resources::gameboard));
	} else if(key == "villages")
	{
		return villages_from_set(resources::gameboard->map().villages());
	} else if(key == "villages_of_side")
	{
		std::vector<variant> vars;
		for(const team& t : resources::gameboard->teams()) {
			vars.push_back(villages_from_set(t.villages()));
		}
		return variant(vars);

	} else if(key == "my_villages")
	{
		return villages_from_set(current_team().villages());

	} else if(key == "enemy_and_unowned_villages")
	{
		return villages_from_set(resources::gameboard->map().villages(), &current_team().villages());
	}
	else if(key == "world") {
		return variant(std::make_shared<gamestate_callable>());
	}

	return variant();
}

void formula_ai::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "aggression");
	add_input(inputs, "leader_aggression");
	add_input(inputs, "caution");
	add_input(inputs, "attacks");
	add_input(inputs, "my_side");
	add_input(inputs, "teams");
	add_input(inputs, "turn");
	add_input(inputs, "time_of_day");
	add_input(inputs, "keeps");
	add_input(inputs, "vars");
	add_input(inputs, "allies");
	add_input(inputs, "enemies");
	add_input(inputs, "map");
	add_input(inputs, "my_attacks");
	add_input(inputs, "enemy_moves");
	add_input(inputs, "my_leader");
	add_input(inputs, "my_recruits");
	//add_input(inputs, "recall_list");
	add_input(inputs, "recruits_of_side");
	add_input(inputs, "units");
	add_input(inputs, "units_of_side");
	add_input(inputs, "my_units");
	add_input(inputs, "enemy_units");
	add_input(inputs, "villages");
	add_input(inputs, "my_villages");
	add_input(inputs, "villages_of_side");
	add_input(inputs, "enemy_and_unowned_villages");
	add_input(inputs, "world");
}

void formula_ai::set_value(const std::string& key, const variant& value) {
	vars_.mutate_value(key, value);
}

variant formula_ai::get_keeps() const
{
	if(keeps_cache_.is_null()) {
		std::vector<variant> vars;
		for(std::size_t x = 0; x != std::size_t(resources::gameboard->map().w()); ++x) {
			for(std::size_t y = 0; y != std::size_t(resources::gameboard->map().h()); ++y) {
				const map_location loc(x,y);
				if(resources::gameboard->map().is_keep(loc)) {
					for(const map_location& adj : get_adjacent_tiles(loc)) {
						if(resources::gameboard->map().is_castle(adj)) {
							vars.emplace_back(std::make_shared<location_callable>(loc));
							break;
						}
					}
				}
			}
		}
		keeps_cache_ = variant(vars);
	}

	return keeps_cache_;
}

bool formula_ai::can_reach_unit(map_location unit_A, map_location unit_B) const {
	if (tiles_adjacent(unit_A,unit_B)) {
		return true;
	}
	move_map::const_iterator i;
	std::pair<move_map::const_iterator,
			  move_map::const_iterator> unit_moves;

	unit_moves = get_srcdst().equal_range(unit_A);
	for(i = unit_moves.first; i != unit_moves.second; ++i) {
		if (tiles_adjacent((*i).second,unit_B)) {
			return true;
		}
	}
	return false;
}

void formula_ai::on_create(){
	//make sure we don't run out of refcount

	for(const config &func : cfg_.child_range("function"))
	{
		const std::string name = func["name"];
		const std::string inputs = func["inputs"];
		const std::string formula_str = func["formula"];

		std::vector<std::string> args = utils::split(inputs);
		try {
			add_formula_function(name,
					     create_optional_formula(formula_str),
					     create_optional_formula(func["precondition"]),
					     args);
		}
		catch(const formula_error& e) {
			handle_exception(e, "Error while registering function '" + name + "'");
		}
	}

	vars_ = map_formula_callable();
	if (const auto ai_vars = cfg_.optional_child("vars"))
	{
		variant var;
		for(const auto& [key, value] : ai_vars->attribute_range()) {
			var.serialize_from_string(value);
			vars_.add(key, var);
		}
	}

}

void formula_ai::evaluate_candidate_action(const ca_ptr& fai_ca)
{
	fai_ca->evaluate(this,resources::gameboard->units());

}

bool formula_ai::execute_candidate_action(const ca_ptr& fai_ca)
{
	map_formula_callable callable(fake_ptr());
	fai_ca->update_callable_map( callable );
	const_formula_ptr move_formula(fai_ca->get_action());
	return !make_action(move_formula, callable).is_empty();
}

#if 0
formula_ai::gamestate_change_observer::gamestate_change_observer() :
	set_var_counter_(), set_unit_var_counter_(), continue_counter_()
{
	ai::manager::get_singleton().add_gamestate_observer(this);
}

formula_ai::gamestate_change_observer::~gamestate_change_observer() {
	ai::manager::get_singleton().remove_gamestate_observer(this);
}

void formula_ai::gamestate_change_observer::handle_generic_event(const std::string& /*event_name*/) {
	set_var_counter_ = 0;
	set_unit_var_counter_ = 0;
	continue_counter_ = 0;
}

//return false if number of calls exceeded MAX_CALLS
bool formula_ai::gamestate_change_observer::set_var_check() {
	if(set_var_counter_ >= MAX_CALLS)
	    return false;

	set_var_counter_++;
	return true;
}

bool formula_ai::gamestate_change_observer::set_unit_var_check() {
	if(set_unit_var_counter_ >= MAX_CALLS)
	    return false;

	set_unit_var_counter_++;
	return true;
}

bool formula_ai::gamestate_change_observer::continue_check() {
	if(continue_counter_ >= MAX_CALLS)
	    return false;

	continue_counter_++;
	return true;
}
#endif

config formula_ai::to_config() const
{
	DBG_AI << "formula_ai::to_config(): "<< cfg_;
	config cfg = cfg_;

	//formula AI variables
	cfg.clear_children("vars");
	if (vars_.empty() == false) {
		config &ai_vars = cfg.add_child("vars");

		std::string str;
		for(map_formula_callable::const_iterator i = vars_.begin(); i != vars_.end(); ++i)
		{
			try {
				str = i->second.serialize_to_string();
			} catch(const type_error&) {
				WRN_AI << "variable ["<< i->first <<"] is not serializable - it will not be persisted across savegames";
				continue;
			}
				if (!str.empty())
				{
					ai_vars[i->first] = str;
					str.clear();
				}
		}
	}

	return cfg;
}

} // end of namespace ai
