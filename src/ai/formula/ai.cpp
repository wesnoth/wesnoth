/*
   Copyright (C) 2008 - 2016 by David White <dave@whitevine.net>
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
 * Defines formula ai candidate actions - headers
 */

#include "ai/formula/ai.hpp"
#include "global.hpp"

#include "ai/formula/callable_objects.hpp"   // for unit_callable, etc
#include "chat_events.hpp"              // for chat_handler, etc
#include "display_chat_manager.hpp"
#include "formula/function.hpp"         // for formula_expression
#include "game_board.hpp"         // for game_board
#include "game_display.hpp"       // for game_display
#include "log.hpp"                // for LOG_STREAM, logger, etc
#include "map/map.hpp"                      // for gamemap
#include "menu_events.hpp"
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
#include "units/formula_manager.hpp"  // for unit_formula_manager
#include "units/ptr.hpp"                 // for unit_ptr
#include "units/types.hpp"
#include "formula/formula.hpp"  // for formula_error, formula, etc
#include "map/location.hpp"  // for map_location, etc
#include "ai/actions.hpp"               // for recall_result, etc
#include "ai/manager.hpp"               // for manager
#include "ai/composite/contexts.hpp"
#include "ai/composite/stage.hpp"  // for stage
#include "ai/default/contexts.hpp"  // for attack_analysis
#include "ai/formula/function_table.hpp"           // for ai_function_symbol_table
#include "ai/game_info.hpp"  // for move_result_ptr, move_map, etc
#include "ai/formula/candidates.hpp"               // for base_candidate_action, etc

#include <cassert>                     // for assert
#include <ctime>                       // for time
#include <map>                          // for multimap<>::const_iterator, etc
#include <sstream>                      // for operator<<, basic_ostream, etc
#include <stack>                        // for stack
#include <vector>                       // for vector, allocator, etc

static lg::log_domain log_formula_ai("ai/engine/fai");
#define DBG_AI LOG_STREAM(debug, log_formula_ai)
#define LOG_AI LOG_STREAM(info, log_formula_ai)
#define WRN_AI LOG_STREAM(warn, log_formula_ai)
#define ERR_AI LOG_STREAM(err, log_formula_ai)


using namespace game_logic;

namespace ai {

using ca_ptr = game_logic::candidate_action_ptr;

ca_ptr formula_ai::load_candidate_action_from_config(const config& rc_action)
{
	ca_ptr new_ca;
	const t_string &name = rc_action["name"];
	try {
		const t_string &type = rc_action["type"];

		if( type == "movement") {
			new_ca = ca_ptr(new move_candidate_action(name, type, rc_action, &function_table_));
		} else if( type == "attack") {
			new_ca = ca_ptr(new attack_candidate_action(name, type, rc_action, &function_table_));
		} else {
			ERR_AI << "Unknown candidate action type: " << type << std::endl;
		}
	} catch(formula_error& e) {
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
	game_logic::formula_callable(),
	ai_ptr_(nullptr),
	cfg_(cfg),
	recursion_counter_(context.get_recursion_count()),
	keeps_cache_(),
	infinite_loop_guardian_(),
	vars_(),
	function_table_(*this)
{
	init_readonly_context_proxy(context);
	LOG_AI << "creating new formula ai"<< std::endl;
}

void formula_ai::handle_exception(game_logic::formula_error& e) const
{
	handle_exception(e, "Error while parsing formula");
}

void formula_ai::handle_exception(game_logic::formula_error& e, const std::string& failed_operation) const
{
	LOG_AI << failed_operation << ": " << e.formula << std::endl;
	display_message(failed_operation + ": " + e.formula);
	//if line number = 0, don't display info about filename and line number
	if (e.line != 0) {
		LOG_AI << e.type << " in " << e.filename << ":" << e.line << std::endl;
		display_message(e.type + " in " + e.filename + ":" + std::to_string(e.line));
	} else {
		LOG_AI << e.type << std::endl;
		display_message(e.type);
	}
}

void formula_ai::display_message(const std::string& msg) const
{
	resources::screen->get_chat_manager().add_chat_message(time(nullptr), "wfl", get_side(), msg,
				events::chat_handler::MESSAGE_PUBLIC, false);

}

formula_ptr formula_ai::create_optional_formula(const std::string& formula_string){
	try{
		return game_logic::formula::create_optional_formula(formula_string, &function_table_);
	}
	catch(formula_error& e) {
		handle_exception(e);
		return game_logic::formula_ptr();
	}
}


void formula_ai::set_ai_context(ai_context *context)
{
	ai_ptr_ = context;
}


std::string formula_ai::evaluate(const std::string& formula_str)
{
	try{

		game_logic::formula f(formula_str, &function_table_);

		game_logic::map_formula_callable callable(this);

		//formula_debugger fdb;
		const variant v = f.evaluate(callable,nullptr);

		if (ai_ptr_) {
			variant var = execute_variant(v, *ai_ptr_, true );

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

variant formula_ai::make_action(game_logic::const_formula_ptr formula_, const game_logic::formula_callable& variables)
{
	if (!formula_) {
		throw formula_error("null formula passed to make_action","","formula",0);
	}
	LOG_AI << "do move...\n";
	const variant var = formula_->evaluate(variables);///@todo 1.9 add formula_debugger
	variant res;

	if (ai_ptr_) {
		res = execute_variant(var, *ai_ptr_, false);
	} else {
		ERR_AI << "skipped execution of action because ai context is not set correctly" << std::endl;
	}

	return res;
}

pathfind::plain_route formula_ai::shortest_path_calculator(const map_location &src,
	const map_location &dst, unit_map::iterator &unit_it,
	pathfind::teleport_map& allowed_teleports) const
{
    map_location destination = dst;

    unit_map &units_ = *resources::units;
    pathfind::shortest_path_calculator calc(*unit_it, current_team(), resources::gameboard->teams(), resources::gameboard->map());

    unit_map::const_iterator dst_un = units_.find(destination);

    map_location res;

    if( dst_un != units_.end() ) {
        //there is unit standing at dst, let's try to find free hex to move to
        const map_location::DIRECTION preferred = destination.get_relative_dir(src);

        int best_rating = 100;//smaller is better
        map_location adj[6];
        get_adjacent_tiles(destination,adj);

        for(size_t n = 0; n != 6; ++n) {
                if(resources::gameboard->map().on_board(adj[n]) == false) {
                        continue;
                }

                if(units_.find(adj[n]) != units_.end()) {
                        continue;
                }

                static const size_t NDIRECTIONS = map_location::NDIRECTIONS;
                unsigned int difference = std::abs(int(preferred - n));
                if(difference > NDIRECTIONS/2) {
                        difference = NDIRECTIONS - difference;
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

    pathfind::plain_route route = pathfind::a_star_search(src, destination, 1000.0, calc,
            resources::gameboard->map().w(), resources::gameboard->map().h(), &allowed_teleports);

    return route;
}

pathfind::teleport_map formula_ai::get_allowed_teleports(unit_map::iterator& unit_it) const
{
  return pathfind::get_teleport_locations(*unit_it, current_team(), true);
}

//commandline=true when we evaluate formula from commandline, false otherwise (default)
variant formula_ai::execute_variant(const variant& var, ai_context &ai_, bool commandline)
{
	std::stack<variant> vars;
	if(var.is_list()) {
		for(size_t n = 1; n <= var.num_elements() ; ++n) {
			vars.push(var[ var.num_elements() - n ]);
		}
	} else {
		vars.push(var);
	}

	std::vector<variant> made_moves;

	variant error;

	unit_map& units = *resources::units;

	while( !vars.empty() ) {

		if(vars.top().is_null()) {
			vars.pop();
			continue;
		}

		variant action = vars.top();
		vars.pop();

		game_logic::safe_call_callable* safe_call = try_convert_variant<game_logic::safe_call_callable>(action);

		if(safe_call) {
		    action = safe_call->get_main();
		}

		const move_callable* move = try_convert_variant<move_callable>(action);
		const move_partial_callable* move_partial = try_convert_variant<move_partial_callable>(action);
		const attack_callable* attack = try_convert_variant<attack_callable>(action);
		const attack_analysis* _attack_analysis = try_convert_variant<attack_analysis>(action);
		const recruit_callable* recruit_command = try_convert_variant<recruit_callable>(action);
		const recall_callable* recall_command = try_convert_variant<recall_callable>(action);
		const set_var_callable* set_var_command = try_convert_variant<set_var_callable>(action);
		const set_unit_var_callable* set_unit_var_command = try_convert_variant<set_unit_var_callable>(action);
		const fallback_callable* fallback_command = try_convert_variant<fallback_callable>(action);

		if( move || move_partial ) {
			move_result_ptr move_result;

			if(move)
				move_result = ai_.execute_move_action(move->src(), move->dst(), true);
			else
				move_result = ai_.execute_move_action(move_partial->src(), move_partial->dst(), false);

			if ( !move_result->is_ok() ) {
				if( move ) {
					LOG_AI << "ERROR #" << move_result->get_status() << " while executing 'move' formula function\n" << std::endl;

					if(safe_call) {
						//safe_call was called, prepare error information
						error = variant(new safe_call_result(move,
									move_result->get_status(), move_result->get_unit_location()));
					}
				} else {
					LOG_AI << "ERROR #" << move_result->get_status() << " while executing 'move_partial' formula function\n" << std::endl;

					if(safe_call) {
						//safe_call was called, prepare error information
						error = variant(new safe_call_result(move_partial,
									move_result->get_status(), move_result->get_unit_location()));
					}
				}
			}

			if( move_result->is_gamestate_changed() )
				made_moves.push_back(action);
		} else if(attack) {
			bool gamestate_changed = false;
			move_result_ptr move_result;

			if( attack->move_from() != attack->src() ) {
				move_result = ai_.execute_move_action(attack->move_from(), attack->src(), false);
				gamestate_changed |= move_result->is_gamestate_changed();

				if (!move_result->is_ok()) {
					//move part failed
					LOG_AI << "ERROR #" << move_result->get_status() << " while executing 'attack' formula function\n" << std::endl;

					if(safe_call) {
						//safe_call was called, prepare error information
						error = variant(new safe_call_result(attack,
								move_result->get_status(), move_result->get_unit_location()));
					}
				}
			}

			if (!move_result || move_result->is_ok() ) {
				//if move wasn't done at all or was done successfully
				attack_result_ptr attack_result = ai_.execute_attack_action(attack->src(), attack->dst(), attack->weapon() );
				gamestate_changed |= attack_result->is_gamestate_changed();
				if (!attack_result->is_ok()) {
					//attack failed

					LOG_AI << "ERROR #" << attack_result->get_status() << " while executing 'attack' formula function\n" << std::endl;

					if(safe_call) {
						//safe_call was called, prepare error information
						error = variant(new safe_call_result(attack, attack_result->get_status()));
					}
				}
			}

			if (gamestate_changed) {
			      made_moves.push_back(action);
			}
		} else if(_attack_analysis) {
			//If we get an attack analysis back we will do the first attack.
			//Then the AI can get run again and re-choose.
			assert(_attack_analysis->movements.empty() == false);

			//make sure that unit which has to attack is at given position and is able to attack
			unit_map::const_iterator unit = units.find(_attack_analysis->movements.front().first);
			if (!unit.valid() || unit->attacks_left() == 0)
				continue;

			const map_location& move_from = _attack_analysis->movements.front().first;
			const map_location& att_src = _attack_analysis->movements.front().second;
			const map_location& att_dst = _attack_analysis->target;

			//check if target is still valid
			unit = units.find(att_dst);
			if ( unit == units.end() )
				continue;

                        //check if we need to move
                        if( move_from != att_src ) {
                            //now check if location to which we want to move is still unoccupied
				unit = units.find(att_src);
				if ( unit != units.end() ) {
					continue;
				}

				ai_.execute_move_action(move_from, att_src);
                        }

			if(units.count(att_src)) {
				ai_.execute_attack_action(_attack_analysis->movements.front().second,_attack_analysis->target,-1);
			}
			made_moves.push_back(action);
		} else if(recall_command) {

			recall_result_ptr recall_result = ai_.check_recall_action(recall_command->id(), recall_command->loc());

			if( recall_result->is_ok() ) {
				recall_result->execute();
			}

			if (!recall_result->is_ok()) {

				if(safe_call) {
					//safe call was called, prepare error information
					error = variant(new safe_call_result(recall_command,
									recall_result->get_status()));

					LOG_AI << "ERROR #" <<recall_result->get_status() << " while executing 'recall' formula function\n"<<std::endl;
				} else {
					ERR_AI << "ERROR #" <<recall_result->get_status() << " while executing 'recall' formula function\n"<<std::endl;
				}
			}

			if( recall_result->is_gamestate_changed() ) {
				made_moves.push_back(action);
			}

		} else if(recruit_command) {
			recruit_result_ptr recruit_result = ai_.check_recruit_action(recruit_command->type(), recruit_command->loc());

			//is_ok()==true means that the action is successful (eg. no unexpected events)
			//is_ok() must be checked or the code will complain :)
			if( recruit_result->is_ok() )
				recruit_result->execute();

			if (!recruit_result->is_ok()) {

				if(safe_call) {
					//safe call was called, prepare error information
					error = variant(new safe_call_result(recruit_command,
									recruit_result->get_status()));

					LOG_AI << "ERROR #" <<recruit_result->get_status() << " while executing 'recruit' formula function\n"<<std::endl;
				} else {
					ERR_AI << "ERROR #" <<recruit_result->get_status() << " while executing 'recruit' formula function\n"<<std::endl;
				}
			}

			//is_gamestate_changed()==true means that the game state was somehow changed by action.
			//it is believed that during a turn, a game state can change only a finite number of times
			if( recruit_result->is_gamestate_changed() )
				made_moves.push_back(action);

		} else if(set_var_command) {
			if( infinite_loop_guardian_.set_var_check() ) {
				LOG_AI << "Setting variable: " << set_var_command->key() << " -> " << set_var_command->value().to_debug_string() << "\n";
				vars_.add(set_var_command->key(), set_var_command->value());
				made_moves.push_back(action);
			} else {
				//too many calls in a row - possible infinite loop
				ERR_AI << "ERROR #" << 5001 << " while executing 'set_var' formula function" << std::endl;

				if( safe_call )
					error = variant(new safe_call_result(set_var_command, 5001));
			}
		} else if(set_unit_var_command) {
			int status = 0;
			unit_map::iterator unit;

			if( !infinite_loop_guardian_.set_unit_var_check() ) {
			    status = 5001; //exceeded nmber of calls in a row - possible infinite loop
			} else if( (unit = units.find(set_unit_var_command->loc())) == units.end() ) {
			    status = 5002; //unit not found
			} else if (unit->side() != get_side()) {
			    status = 5003;//unit does not belong to our side
			}

			if( status == 0 ){
				LOG_AI << "Setting unit variable: " << set_unit_var_command->key() << " -> " << set_unit_var_command->value().to_debug_string() << "\n";
				unit->formula_manager().add_formula_var(set_unit_var_command->key(), set_unit_var_command->value());
				made_moves.push_back(action);
			} else {
				ERR_AI << "ERROR #" << status << " while executing 'set_unit_var' formula function" << std::endl;
				if(safe_call)
				    error = variant(new safe_call_result(set_unit_var_command,
									status));
			}

		} else if( action.is_string() && action.as_string() == "continue") {
			if( infinite_loop_guardian_.continue_check() ) {
				made_moves.push_back(action);
			} else {
				//too many calls in a row - possible infinite loop
				ERR_AI << "ERROR #" << 5001 << " while executing 'continue' formula keyword" << std::endl;

				if( safe_call )
					error = variant(new safe_call_result(nullptr, 5001));
			}
		} else if( action.is_string() && (action.as_string() == "end_turn" || action.as_string() == "end" )  ) {
			return variant();
		} else if(fallback_command) {
			if(get_recursion_count()<recursion_counter::MAX_COUNTER_VALUE) {
				//we want give control of the side to human for the rest of this turn
				throw fallback_ai_to_human_exception();
			}
			return variant();
		} else {
			//this information is unneded when evaluating formulas form commandline
			if (!commandline) {
				ERR_AI << "UNRECOGNIZED MOVE: " << action.to_debug_string() << std::endl;
			}
		}

		if( safe_call && (error != variant() || made_moves.empty() || made_moves.back() != action) ){
		    /*if we have safe_call formula and either error occurred, or current action
		     *was not reckognized, then evaluate backup formula from safe_call and execute it
		     *during the next loop
		     */

			game_logic::map_formula_callable callable(this);

			if(error != variant())
				callable.add("error", error);

			variant backup_result = safe_call->get_backup()->evaluate(callable);

			if(backup_result.is_list()) {
				for(size_t n = 1; n <= backup_result.num_elements() ; ++n) {
					vars.push(backup_result[ backup_result.num_elements() - n ]);
				}
			} else {
				vars.push(backup_result);
			}

			//store the result in safe_call_callable case we would like to display it to the user
			//for example if this formula was executed from commandline
			safe_call->set_backup_result(backup_result);

			error = variant();
		}
	}

	return variant(&made_moves);
}

void formula_ai::add_formula_function(const std::string& name, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& args)
{
	formula_function_ptr fcn(new user_formula_function(name,formula,precondition,args));
	function_table_.add_function(name, fcn);
}

namespace {
template<typename Container>
variant villages_from_set(const Container& villages,
				          const std::set<map_location>* exclude=nullptr) {
	std::vector<variant> vars;
	for(const map_location& loc : villages) {
		if(exclude && exclude->count(loc)) {
			continue;
		}
		vars.push_back(variant(new location_callable(loc)));
	}

	return variant(&vars);
}
}

variant formula_ai::get_value(const std::string& key) const
{
	const unit_map& units = *resources::units;

	if(key == "aggression")
	{
		return variant(get_aggression()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "attack_depth")
	{
		return variant(get_attack_depth());

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
		return variant(get_leader_ignores_keep());

	} else if(key == "leader_value")
	{
		return variant(get_leader_value()*1000,variant::DECIMAL_VARIANT);

	} else if(key == "passive_leader")
	{
		return variant(get_passive_leader());

	} else if(key == "passive_leader_shares_keep")
	{
		return variant(get_passive_leader_shares_keep());

	} else if(key == "recruitment_pattern")
	{
		const std::vector<std::string> &rp = get_recruitment_pattern();
		std::vector<variant> vars;
		for(const std::string &i : rp) {
			vars.push_back(variant(i));
		}
		return variant(&vars);

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
		return variant(new team_callable(resources::gameboard->teams()[get_side()-1]));

	} else if(key == "my_side_number")
	{
		return variant(get_side()-1);

	} else if(key == "teams")
	{
		std::vector<variant> vars;
		for(std::vector<team>::const_iterator i = resources::gameboard->teams().begin(); i != resources::gameboard->teams().end(); ++i) {
			vars.push_back(variant(new team_callable(*i)));
		}
		return variant(&vars);

	} else if(key == "allies")
	{
		std::vector<variant> vars;
		for( size_t i = 0; i < resources::gameboard->teams().size(); ++i) {
			if ( !current_team().is_enemy( i+1 ) )
				vars.push_back(variant( i ));
		}
		return variant(&vars);

	} else if(key == "enemies")
	{
		std::vector<variant> vars;
		for( size_t i = 0; i < resources::gameboard->teams().size(); ++i) {
			if ( current_team().is_enemy( i+1 ) )
				vars.push_back(variant( i ));
		}
		return variant(&vars);

	} else if(key == "my_recruits")
	{
		std::vector<variant> vars;

		unit_types.build_all(unit_type::FULL);

		const std::set<std::string>& recruits = current_team().recruits();
		if(recruits.empty()) {
			return variant( &vars );
		}
		for(std::set<std::string>::const_iterator i = recruits.begin(); i != recruits.end(); ++i)
		{
			const unit_type *ut = unit_types.find(*i);
			if (ut)
			{
				vars.push_back(variant(new unit_type_callable(*ut)));
			}
		}
		return variant( &vars );

	} else if(key == "recruits_of_side")
	{
		std::vector<variant> vars;
		std::vector< std::vector< variant> > tmp;

		unit_types.build_all(unit_type::FULL);

		for( size_t i = 0; i<resources::gameboard->teams().size(); ++i)
		{
			std::vector<variant> v;
			tmp.push_back( v );

			const std::set<std::string>& recruits = resources::gameboard->teams()[i].recruits();
			if(recruits.empty()) {
				continue;
			}
			for(std::set<std::string>::const_iterator str_it = recruits.begin(); str_it != recruits.end(); ++str_it)
			{
				const unit_type *ut = unit_types.find(*str_it);
				if (ut)
				{
					tmp[i].push_back(variant(new unit_type_callable(*ut)));
				}
			}
		}

		for( size_t i = 0; i<tmp.size(); ++i)
			vars.push_back( variant( &tmp[i] ));
		return variant(&vars);

	} else if(key == "units")
	{
		std::vector<variant> vars;
		for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
			vars.push_back(variant(new unit_callable(*i)));
		}
		return variant(&vars);

	} else if(key == "units_of_side")
	{
		std::vector<variant> vars;
		std::vector< std::vector< variant> > tmp;
		for( size_t i = 0; i<resources::gameboard->teams().size(); ++i)
		{
			std::vector<variant> v;
			tmp.push_back( v );
		}
		for(const unit &u : units) {
			tmp[u.side() - 1].push_back(variant(new unit_callable(u)));
		}
		for( size_t i = 0; i<tmp.size(); ++i)
			vars.push_back( variant( &tmp[i] ));
		return variant(&vars);

	} else if(key == "my_units")
	{
		std::vector<variant> vars;
		for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
			if (i->side() == get_side()) {
				vars.push_back(variant(new unit_callable(*i)));
			}
		}
		return variant(&vars);

	} else if(key == "enemy_units")
	{
		std::vector<variant> vars;
		for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
			if (current_team().is_enemy(i->side())) {
				if (!i->incapacitated()) {
					vars.push_back(variant(new unit_callable(*i)));
				}
			}
		}
		return variant(&vars);

	} else if(key == "my_moves")
	{
		return variant(new move_map_callable(get_srcdst(), get_dstsrc(), units));

	} else if(key == "my_attacks")
	{
		return variant(new attack_map_callable(*this, units));
	} else if(key == "enemy_moves")
	{
		return variant(new move_map_callable(get_enemy_srcdst(), get_enemy_dstsrc(), units));

	} else if(key == "my_leader")
	{
		unit_map::const_iterator i = units.find_leader(get_side());
		if(i == units.end()) {
			return variant();
		}
		return variant(new unit_callable(*i));

	} else if(key == "recall_list")
	{
		std::vector<variant> tmp;

		for(std::vector<unit_ptr >::const_iterator i = current_team().recall_list().begin(); i != current_team().recall_list().end(); ++i) {
			tmp.push_back( variant( new unit_callable(**i) ) );
		}

		return variant( &tmp );

	} else if(key == "vars")
	{
		return variant(&vars_);
	} else if(key == "keeps")
	{
		return get_keeps();
	} else if(key == "map")
	{
		return variant(new gamemap_callable(resources::gameboard->map()));
	} else if(key == "villages")
	{
		return villages_from_set(resources::gameboard->map().villages());
	} else if(key == "villages_of_side")
	{
		std::vector<variant> vars;
		for(size_t i = 0; i<resources::gameboard->teams().size(); ++i)
		{
			vars.push_back( variant() );
		}
		for(size_t i = 0; i<vars.size(); ++i)
		{
			vars[i] = villages_from_set(resources::gameboard->teams()[i].villages());
		}
		return variant(&vars);

	} else if(key == "my_villages")
	{
		return villages_from_set(current_team().villages());

	} else if(key == "enemy_and_unowned_villages")
	{
		return villages_from_set(resources::gameboard->map().villages(), &current_team().villages());
	}

	return variant();
}

void formula_ai::get_inputs(std::vector<formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("aggression", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("leader_aggression", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("caution", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("teams", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("turn", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("time_of_day", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("keeps", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("vars", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("allies", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemies", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("map", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_moves", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_leader", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_recruits", FORMULA_READ_ONLY));
	//inputs->push_back(game_logic::formula_input("recall_list", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("recruits_of_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("units_of_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("villages", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_villages", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("villages_of_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_and_unowned_villages", FORMULA_READ_ONLY));
}

variant formula_ai::get_keeps() const
{
	if(keeps_cache_.is_null()) {
		std::vector<variant> vars;
		for(size_t x = 0; x != size_t(resources::gameboard->map().w()); ++x) {
			for(size_t y = 0; y != size_t(resources::gameboard->map().h()); ++y) {
				const map_location loc(x,y);
				if(resources::gameboard->map().is_keep(loc)) {
					map_location adj[6];
					get_adjacent_tiles(loc,adj);
					for(size_t n = 0; n != 6; ++n) {
						if(resources::gameboard->map().is_castle(adj[n])) {
							vars.push_back(variant(new location_callable(loc)));
							break;
						}
					}
				}
			}
		}
		keeps_cache_ = variant(&vars);
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
		const t_string &name = func["name"];
		const t_string &inputs = func["inputs"];
		const t_string &formula_str = func["formula"];

		std::vector<std::string> args = utils::split(inputs);
		try {
			add_formula_function(name,
					     create_optional_formula(formula_str),
					     create_optional_formula(func["precondition"]),
					     args);
		}
		catch(game_logic::formula_error& e) {
			handle_exception(e, "Error while registering function '" + name + "'");
		}
	}


	vars_ = game_logic::map_formula_callable();
	if (const config &ai_vars = cfg_.child("vars"))
	{
		variant var;
		for(const config::attribute &i : ai_vars.attribute_range()) {
			var.serialize_from_string(i.second);
			vars_.add(i.first, var);
		}
	}


}


void formula_ai::evaluate_candidate_action(ca_ptr fai_ca)
{
	fai_ca->evaluate(this,*resources::units);

}

bool formula_ai::execute_candidate_action(ca_ptr fai_ca)
{
	game_logic::map_formula_callable callable(this);
	fai_ca->update_callable_map( callable );
	const_formula_ptr move_formula(fai_ca->get_action());
	return !make_action(move_formula, callable).is_empty();
}

formula_ai::gamestate_change_observer::gamestate_change_observer() :
	set_var_counter_(), set_unit_var_counter_(), continue_counter_()
{
	ai::manager::add_gamestate_observer(this);
}

formula_ai::gamestate_change_observer::~gamestate_change_observer() {
	ai::manager::remove_gamestate_observer(this);
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

config formula_ai::to_config() const
{
	if (!cfg_)
	{
		return config();
	}
	DBG_AI << "formula_ai::to_config(): "<< cfg_<<std::endl;
	config cfg = cfg_;

	//formula AI variables
	cfg.clear_children("vars");
	if (vars_.empty() == false) {
		config &ai_vars = cfg.add_child("vars");

		std::string str;
		for(game_logic::map_formula_callable::const_iterator i = vars_.begin(); i != vars_.end(); ++i)
		{
			try {
				i->second.serialize_to_string(str);
			} catch (type_error&) {
				WRN_AI << "variable ["<< i->first <<"] is not serializable - it will not be persisted across savegames"<<std::endl;
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
