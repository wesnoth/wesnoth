/*
   Copyright (C) 2009 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#include <queue>
#include <set>
#include <utility>
#include <vector>

#include "ai.hpp"
#include "callable_objects.hpp"
#include "function_table.hpp"

#include "../../attack_prediction.hpp"
#include "../../filesystem.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"
#include "../../map_label.hpp"
#include "../../menu_events.hpp"
#include "../../replay.hpp"
#include "../../unit.hpp"

static lg::log_domain log_formula_ai("ai/formula_ai");
#define LOG_AI LOG_STREAM(info, log_formula_ai)
#define WRN_AI LOG_STREAM(warn, log_formula_ai)
#define ERR_AI LOG_STREAM(err, log_formula_ai)

namespace game_logic {

namespace {

class distance_between_function : public function_expression {
public:
	explicit distance_between_function(const args_list& args)
	  : function_expression("distance_between", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables) const {
		const args_list& arguments = args();
		const expression_ptr& exp_p = arguments[0];
		variant my_variant = exp_p->evaluate(variables);
		const map_location loc1 = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		const map_location loc2 = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		return variant(distance_between(loc1, loc2));
	}
};


class distance_to_nearest_unowned_village_function : public function_expression {
public:
	distance_to_nearest_unowned_village_function(const args_list& args, const formula_ai& ai)
	  : function_expression("distance_to_nearest_unowned_village", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		int best = 1000000;
		const std::vector<map_location>& villages = ai_.get_info().map.villages();
		const std::set<map_location>& my_villages = ai_.current_team().villages();
		for(std::vector<map_location>::const_iterator i = villages.begin(); i != villages.end(); ++i) {
			int distance = distance_between(loc, *i);
			if(distance < best) {
				if(my_villages.count(*i) == 0) {
					best = distance;
				}
			}
		}

		return variant(best);
	}

	const formula_ai& ai_;
};


class nearest_loc_function : public function_expression {
public:
	nearest_loc_function(const args_list& args, const formula_ai& ai)
	  : function_expression("nearest_loc", args, 2, 2), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		variant items = args()[1]->evaluate(variables);
		int best = 1000000;
		int best_i = -1;

		for(size_t i = 0; i < items.num_elements(); ++i) {

			const map_location move_loc = convert_variant<location_callable>(items[i])->loc();
			int distance = distance_between(loc, move_loc);

			if(distance < best) {
					best = distance;
					best_i = i;
			}
		}

		if( best_i != -1)
			return variant(new location_callable(convert_variant<location_callable>(items[best_i])->loc()));
		else
			return variant();
	}

	const formula_ai& ai_;
};


class adjacent_locs_function : public function_expression {
public:
	adjacent_locs_function(const args_list& args, const formula_ai& ai)
	  : function_expression("adjacent_locs", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		map_location adj[6];
		get_adjacent_tiles(loc, adj);

		std::vector<variant> v;
		for(int n = 0; n != 6; ++n) {
                        if (ai_.get_info().map.on_board(adj[n]) )
                            v.push_back(variant(new location_callable(adj[n])));
		}

		return variant(&v);
	}

	const formula_ai& ai_;
};


class locations_in_radius_function : public function_expression {
public:
	locations_in_radius_function(const args_list& args, const formula_ai& ai)
	  : function_expression("locations_in_radius", args, 2, 2), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();

		int range = args()[1]->evaluate(variables).as_int();

		if( range < 0 )
			return variant();

		if(!range)
			return variant(new location_callable(loc));

		std::vector<map_location> res;

		get_tiles_in_radius( loc, range, res);

		std::vector<variant> v;
		v.reserve(res.size()+1);
		v.push_back(variant(new location_callable(loc)));

		for(size_t n = 0; n != res.size(); ++n) {
                        if (ai_.get_info().map.on_board(res[n]) )
                            v.push_back(variant(new location_callable(res[n])));
		}

		return variant(&v);
	}

	const formula_ai& ai_;
};


/** FormulaAI function to run fai script from file. Usable from in-game console.
*   arguments[0] - required file name, follows the usual wml convention
*/
class run_file_function : public function_expression {
public:
	explicit run_file_function(const args_list& args, formula_ai& ai)
	    :  function_expression("run_file", args, 1, 1), ai_(ai)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const args_list& arguments = args();
		const variant var0 = arguments[0]->evaluate(variables);
		const std::string filename = var0.string_cast();

		//NOTE: get_wml_location also filters file path to ensure it doesn't contain things like "../../top/secret"
		std::string path = get_wml_location(filename);
		if(path.empty()) {
			return variant(); //no suitable file
		}

		std::string formula_string = read_file(path);
		//need to get function_table from somewhere or delegate to someone who has access to it
		formula_ptr parsed_formula = ai_.create_optional_formula(formula_string);
		if(parsed_formula == game_logic::formula_ptr()) {
			return variant(); //was unable to create a formula from file
		}
		return parsed_formula->execute(variables);
	}

	formula_ai& ai_;
};


class castle_locs_function : public function_expression {
public:
	castle_locs_function(const args_list& args, const formula_ai& ai)
	  : function_expression("castle_locs", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const map_location starting_loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();

                std::set< map_location > visited_locs;
                std::queue< map_location > queued_locs;

                queued_locs.push(starting_loc);

                while( !queued_locs.empty() ) {
                   const map_location loc = queued_locs.front();
                   queued_locs.pop();

                   if ( visited_locs.find( loc ) != visited_locs.end() )
                       continue;

                   visited_locs.insert(loc);

                   map_location adj[6];
                   get_adjacent_tiles(loc, adj);

                   for(int n = 0; n != 6; ++n) {
                        if ( ai_.get_info().map.on_board(adj[n]) && visited_locs.find( adj[n] ) == visited_locs.end() ) {
                            if ( ai_.get_info().map.get_terrain_info(adj[n]).is_keep() ||
                                    ai_.get_info().map.get_terrain_info(adj[n]).is_castle() ) {
                                queued_locs.push(adj[n]);
                            }
                        }
                   }
                }

                if ( !ai_.get_info().map.get_terrain_info(starting_loc).is_keep() &&
                     !ai_.get_info().map.get_terrain_info(starting_loc).is_castle() )
                    visited_locs.erase(starting_loc);

                std::vector<variant> res;
                foreach( const map_location ml, visited_locs) {
                    res.push_back( variant(new location_callable( ml ) ) );
                }

		return variant(&res);
	}

	const formula_ai& ai_;
};


/**
 * timeofday_modifer formula function. Returns combat modifier, taking
 * alignment, illuminate, time of day and fearless trait into account.
 * 'leadership' and 'slowed' are not taken into account.
 * arguments[0] - unit
 * arguments[1] - location (optional, defaults to unit's current location.
 */
class timeofday_modifier_function : public function_expression {
public:
	timeofday_modifier_function(const args_list& args, const formula_ai& ai)
	  : function_expression("timeofday_modifier", args, 1, 2), ai_(ai) {
	}
private:
	variant execute(const formula_callable& variables) const {
		variant u = args()[0]->evaluate(variables);

		if( u.is_null() ) {
			return variant();
		}

		const unit_callable* u_call = try_convert_variant<unit_callable>(u);

		if (u_call == NULL) {
			return variant();
		}

		const unit& un = u_call->get_unit();

		map_location const* loc = NULL;

		if(args().size()==2) {
			loc = &convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		}

		if (loc == NULL) {
			loc = &u_call->get_location();
		}

		return variant(combat_modifier(ai_.get_info().tod_manager_, ai_.get_info().units, *loc, un.alignment(), un.is_fearless(), ai_.get_info().map));
	}

	const formula_ai& ai_;
};


class nearest_keep_function : public function_expression {
public:
	nearest_keep_function(const args_list& args, const formula_ai& ai)
	  : function_expression("nearest_keep", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		int best = 1000000;
		int best_i = -1;

		ai_.get_keeps();
		int size = ai_.get_keeps_cache().num_elements();

		for( int i = 0 ; i < size; ++i) {
			int distance = distance_between(loc, convert_variant<location_callable>(ai_.get_keeps_cache()[i])->loc() );
			if(distance < best)
			{
					best = distance;
					best_i = i;
			}
		}

		if( best_i != -1)
			return variant(new location_callable(convert_variant<location_callable>(ai_.get_keeps_cache()[best_i])->loc()));
		else
			return variant();
	}

	const formula_ai& ai_;
};


/**
* Find suitable keep for unit at location
* arguments[0] - location for unit on which the suitable keep is to be found
*/
class suitable_keep_function : public function_expression {
public:
	suitable_keep_function(const args_list& args, formula_ai& ai)
	  : function_expression("suitable_keep", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		if (ai_.get_info().units.find(loc)==ai_.get_info().units.end()){
			return variant();
		}
		const paths unit_paths(ai_.get_info().map, ai_.get_info().units, loc ,ai_.get_info().teams, false, false, ai_.current_team());
		return variant(new location_callable(ai_.suitable_keep(loc,unit_paths)));
	}

	formula_ai& ai_;
};


class find_shroud_function : public function_expression {
public:
	find_shroud_function(const args_list& args, const formula_ai& ai)
		: function_expression("find_shroud", args, 0, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> vars;
		int w,h;

		if(args().size()==1) {
			const gamemap& m = convert_variant<gamemap_callable>(args()[0]->evaluate(variables))->get_gamemap();
			w = m.w();
			h = m.h();
		} else {
			w = ai_.get_info().map.w();
			h = ai_.get_info().map.h();
		}

		for(int i = 0; i < w; ++i)
			for(int j = 0; j < h; ++j) {
				if(ai_.current_team().shrouded(map_location(i,j)))
					vars.push_back(variant(new location_callable(i,j)));
			}

		return variant(&vars);
	}

	const formula_ai& ai_;
};


class close_enemies_function : public function_expression {
public:
	close_enemies_function(const args_list& args, const formula_ai& ai)
	  : function_expression("close_enemies", args, 2, 2), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> vars;
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		int range_s = args()[1]->evaluate(variables).as_int();
		if (range_s < 0) {
			WRN_AI << "close_enemies_function: range is negative (" << range_s << ")\n";
			range_s = 0;
		}
		size_t range = static_cast<size_t>(range_s);
		unit_map::const_iterator un = ai_.get_info().units.begin();
		unit_map::const_iterator end = ai_.get_info().units.end();
		while (un != end) {
			if (distance_between(loc, un->first) <= range) {
				if (un->second.side() != ai_.get_side()) {
					vars.push_back(variant(new unit_callable(*un)));
				}
			}
			++un;
		}
		return variant(&vars);
	}

	const formula_ai& ai_;
};


class calculate_outcome_function : public function_expression {
public:
	calculate_outcome_function(const args_list& args, const formula_ai& ai)
	  : function_expression("calculate_outcome", args, 3, 4), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> vars;
		int weapon;
		if (args().size() > 3) weapon = args()[3]->evaluate(variables).as_int();
		else weapon = -1;

		map_location attacker_location =
			convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		if(ai_.get_info().units.count(attacker_location) == 0) {
			ERR_AI << "Performing calculate_outcome() with non-existent attacker at (" <<
				attacker_location.x+1 << "," << attacker_location.y+1 << ")\n";
			return variant();
		}

		map_location defender_location =
			convert_variant<location_callable>(args()[2]->evaluate(variables))->loc();
		if(ai_.get_info().units.count(defender_location) == 0) {
			ERR_AI << "Performing calculate_outcome() with non-existent defender at (" <<
				defender_location.x+1 << "," << defender_location.y+1 << ")\n";
			return variant();
		}

		battle_context bc(ai_.get_info().map, ai_.get_info().teams, ai_.get_info().units,
			ai_.get_info().tod_manager_, convert_variant<location_callable>(args()[1]->evaluate(variables))->loc(),
			defender_location, weapon, -1, 1.0, NULL, &ai_.get_info().units.find(attacker_location)->second);
		std::vector<double> hp_dist = bc.get_attacker_combatant().hp_dist;
		std::vector<double>::iterator it = hp_dist.begin();
		int i = 0;
		std::vector<variant> hitLeft;
		std::vector<variant> prob;
		while (it != hp_dist.end()) {
			if (*it != 0) {
				hitLeft.push_back(variant(i));
				prob.push_back(variant(int(*it*10000)));
			}
			++it;
			++i;
		}
		std::vector<variant> status;
		if (bc.get_attacker_combatant().poisoned != 0)
			status.push_back(variant("Poisoned"));
		if (bc.get_attacker_combatant().slowed != 0)
			status.push_back(variant("Slowed"));
		if (bc.get_defender_stats().petrifies && static_cast<unsigned int>(hitLeft[0].as_int()) != bc.get_attacker_stats().hp)
			status.push_back(variant("Stoned"));
		if (bc.get_defender_stats().plagues && hitLeft[0].as_int() == 0)
			status.push_back(variant("Zombiefied"));
		vars.push_back(variant(new outcome_callable(hitLeft, prob, status)));
		hitLeft.clear();
		prob.clear();
		status.clear();
		hp_dist = bc.get_defender_combatant().hp_dist;
		it = hp_dist.begin();
		i = 0;
		while (it != hp_dist.end()) {
			if (*it != 0) {
				hitLeft.push_back(variant(i));
				prob.push_back(variant(int(*it*10000)));
			}
			++it;
			++i;
		}
		if (bc.get_defender_combatant().poisoned != 0)
			status.push_back(variant("Poisoned"));
		if (bc.get_defender_combatant().slowed != 0)
			status.push_back(variant("Slowed"));
		if (bc.get_attacker_stats().petrifies && static_cast<unsigned int>(hitLeft[0].as_int()) != bc.get_attacker_stats().hp)
			status.push_back(variant("Stoned"));
		if (bc.get_attacker_stats().plagues && hitLeft[0].as_int() == 0)
			status.push_back(variant("Zombiefied"));
		vars.push_back(variant(new outcome_callable(hitLeft, prob, status)));
		return variant(&vars);
	}

	const formula_ai& ai_;
};


class outcomes_function : public function_expression {
public:
	outcomes_function(const args_list& args, const formula_ai& ai)
	  : function_expression("outcomes", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		variant attack = args()[0]->evaluate(variables);
		ai::attack_analysis* analysis = convert_variant<ai::attack_analysis>(attack);
		unit_map units_with_moves(ai_.get_info().units);
		typedef std::pair<map_location, map_location> mv;
		foreach (const mv &m, analysis->movements) {
			units_with_moves.move(m.first, m.second);
		}

		std::vector<variant> vars;
		if(analysis->chance_to_kill > 0.0) {
			unit_map units(units_with_moves);
			units.erase(analysis->target);
			vars.push_back(variant(new position_callable(&units, static_cast<int>(analysis->chance_to_kill*100))));

		}

		if(analysis->chance_to_kill < 1.0) {
			unit_map units(units_with_moves);
			vars.push_back(variant(new position_callable(&units, static_cast<int>(100 - analysis->chance_to_kill*100))));
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};


class evaluate_for_position_function : public function_expression {
public:
	evaluate_for_position_function(const args_list& args, formula_ai& ai)
	  : function_expression("evaluate_for_position", args, 2, 2), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		variant position = args()[0]->evaluate(variables);
                ai_.store_outcome_position(position);
		position_callable* pos = convert_variant<position_callable>(position);
		position_callable::swapper swapper(ai_, *pos);
		return args()[1]->evaluate(variables);
	}

	formula_ai& ai_;
};


class get_unit_type_function : public function_expression {
public:
	explicit get_unit_type_function(const args_list& args)
	  : function_expression("get_unit_type", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const std::string type = args()[0]->evaluate(variables).as_string();

		std::map<std::string,unit_type>::const_iterator unit_it = unit_type_data::types().find_unit_type( type );
		if (unit_it != unit_type_data::types().end() )
				return  variant( new unit_type_callable( unit_it->second ) );

		return variant();
	}
};


class recruit_function : public function_expression {
public:
	explicit recruit_function(const args_list& args)
	  : function_expression("recruit", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const std::string type = args()[0]->evaluate(variables).as_string();
		map_location loc;
		if(args().size() >= 2) {
			loc = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		}

		return variant(new recruit_callable(loc, type));
	}
};


class shortest_path_function : public function_expression {
public:
	explicit shortest_path_function(const args_list& args, const formula_ai& ai)
	  : function_expression("shortest_path", args, 2, 3), ai_(ai)
	{}

private:
	variant execute(const formula_callable& variables) const {

		std::vector<variant> locations;

		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
                map_location unit_loc;

                if( src == dst )
                    return variant(&locations);

                if(args().size() > 2)
                    unit_loc = convert_variant<location_callable>(args()[2]->evaluate(variables))->loc();
                else
                    unit_loc = src;

                unit_map::iterator unit_it = ai_.get_info().units.find(unit_loc);

		if( unit_it == ai_.get_info().units.end() ) {
			std::ostringstream str;
			str << "shortest_path function: expected unit at location (" << (unit_loc.x+1) << "," << (unit_loc.y+1) << ")";
			throw formula_error( str.str(), "", "", 0);
		}

                std::set<map_location> allowed_teleports = ai_.get_allowed_teleports(unit_it);

		plain_route route = ai_.shortest_path_calculator( src, dst, unit_it, allowed_teleports );

                if( route.steps.size() < 2 ) {
                    return variant(&locations);
                }

                for (std::vector<map_location>::const_iterator loc_iter = route.steps.begin() + 1 ; loc_iter !=route.steps.end(); ++loc_iter) {
                    locations.push_back( variant( new location_callable(*loc_iter) ));
                }

		return variant(&locations);
	}

	const formula_ai& ai_;
};


class simplest_path_function : public function_expression {
public:
	explicit simplest_path_function(const args_list& args, const formula_ai& ai)
	  : function_expression("simplest_path", args, 2, 3), ai_(ai)
	{}

private:
	variant execute(const formula_callable& variables) const {

		std::vector<variant> locations;

		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
                map_location unit_loc;

                if( src == dst )
                    return variant(&locations);

                if(args().size() > 2)
                    unit_loc = convert_variant<location_callable>(args()[2]->evaluate(variables))->loc();
                else
                    unit_loc = src;

                unit_map::iterator unit_it = ai_.get_info().units.find(unit_loc);

		if( unit_it == ai_.get_info().units.end() ) {
			std::ostringstream str;
			str << "simplest_path function: expected unit at location (" << (unit_loc.x+1) << "," << (unit_loc.y+1) << ")";
			throw formula_error( str.str(), "", "", 0);
		}

                std::set<map_location> allowed_teleports = ai_.get_allowed_teleports(unit_it);

                emergency_path_calculator em_calc(unit_it->second, ai_.get_info().map);

                plain_route route = a_star_search(src, dst, 1000.0, &em_calc, ai_.get_info().map.w(), ai_.get_info().map.h(), &allowed_teleports);

                if( route.steps.size() < 2 ) {
                    return variant(&locations);
                }

                for (std::vector<map_location>::const_iterator loc_iter = route.steps.begin() + 1 ; loc_iter !=route.steps.end(); ++loc_iter) {
                    if( unit_it->second.movement_cost(ai_.get_info().map[*loc_iter]) < 99 )
                        locations.push_back( variant( new location_callable(*loc_iter) ));
                    else
                        break;
                }

		return variant(&locations);
	}

	const formula_ai& ai_;
};


class move_function : public function_expression {
public:
	explicit move_function(const args_list& args)
	  : function_expression("move", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		LOG_AI << "move(): " << src << ", " << dst << ")\n";
		return variant(new move_callable(src, dst));
	}
};


class move_partial_function : public function_expression {
public:
	explicit move_partial_function(const args_list& args)
	  : function_expression("move_partial", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		LOG_AI << "move_partial(): " << src << ", " << dst << ")\n";
		return variant(new move_partial_callable(src, dst));
	}
};


class set_var_function : public function_expression {
public:
	explicit set_var_function(const args_list& args)
	  : function_expression("set_var", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(new set_var_callable(args()[0]->evaluate(variables).as_string(), args()[1]->evaluate(variables)));
	}
};


class set_unit_var_function : public function_expression {
public:
	explicit set_unit_var_function(const args_list& args)
	  : function_expression("set_unit_var", args, 3, 3)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(new set_unit_var_callable(args()[0]->evaluate(variables).as_string(), args()[1]->evaluate(variables), convert_variant<location_callable>(args()[2]->evaluate(variables))->loc()));
	}
};


class fallback_function : public function_expression {
public:
	explicit fallback_function(const args_list& args)
	  : function_expression("fallback", args, 0, 1)
	{}
private:
	variant execute(const formula_callable& variables) const {
		if( args().size() == 0 )
			return variant(new fallback_callable(""));
		return variant(new fallback_callable(args()[0]->evaluate(variables).as_string()));
	}
};


class attack_function : public function_expression {
public:
	explicit attack_function(const args_list& args, const formula_ai& ai)
	  : function_expression("attack", args, 3, 4),
		ai_(ai)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const map_location move_from = convert_variant<location_callable>(args()[0]->evaluate(variables))->loc();
		const map_location src = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		const map_location dst = convert_variant<location_callable>(args()[2]->evaluate(variables))->loc();
		const int weapon = args().size() == 4 ? args()[3]->evaluate(variables).as_int() : -1;
		if(ai_.get_info().units.count(move_from) == 0 || ai_.get_info().units.count(dst) == 0) {
			ERR_AI << "AI ERROR: Formula produced illegal attack: " << move_from << " -> " << src << " -> " << dst << "\n";
			return variant();
		}
		return variant(new attack_callable(ai_, move_from, src, dst, weapon));
	}

	const formula_ai& ai_;
};


class safe_call_function : public function_expression {
public:
	explicit safe_call_function(const args_list& args)
	  : function_expression("safe_call", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const variant main = args()[0]->evaluate(variables);
		const expression_ptr backup_formula = args()[1];

		return variant(new safe_call_callable(main, backup_formula));
	}
};

class debug_label_function : public function_expression {
public:
	explicit debug_label_function(const args_list& args, const formula_ai& ai)
	  : function_expression("debug_label", args, 2, 2),
		ai_(ai)
	{}
private:
        variant execute(const formula_callable& variables) const {
                const args_list& arguments = args();
                const variant var0 = arguments[0]->evaluate(variables);
                const variant var1 = arguments[1]->evaluate(variables);

                const map_location location = convert_variant<location_callable>(var0)->loc();
                std::string text;

                text = var1.to_debug_string();
                display_label(location,text);

		std::vector<variant> res;
		res.push_back(var0);
		res.push_back(var1);
                return variant(&res);
        }

        void display_label(const map_location& location, const std::string& text) const {
                game_display* gui = game_display::get_singleton();
		std::string team_name;

		SDL_Color colour = int_to_color(team::get_side_rgb(ai_.get_side()));

		const terrain_label *res;
		res = gui->labels().set_label(location, text, team_name, colour);
		if (res)
			recorder.add_label(res);
        }

	const formula_ai& ai_;
};


class is_village_function : public function_expression {
public:
	explicit is_village_function(const args_list& args)
	  : function_expression("is_village", args, 2, 3)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap& m = convert_variant<gamemap_callable>(args()[0]->evaluate(variables))->get_gamemap();

		map_location loc;
		if(args().size() == 2) {
			loc = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		} else {
			loc = map_location( args()[1]->evaluate(variables).as_int() - 1,
			                        args()[2]->evaluate(variables).as_int() - 1 );
		}
		return variant(m.is_village(loc));
	}
};


class is_unowned_village_function : public function_expression {
public:
	explicit is_unowned_village_function(const args_list& args, const formula_ai& ai)
		: function_expression("is_unowned_village", args, 2, 3),
		  ai_(ai)
	{}
private:
	variant execute(const formula_callable& variables) const {

		const gamemap& m = convert_variant<gamemap_callable>(args()[0]->evaluate(variables))->get_gamemap();
		const std::set<map_location>& my_villages = ai_.current_team().villages();

		map_location loc;
		if(args().size() == 2) {
			loc = convert_variant<location_callable>(args()[1]->evaluate(variables))->loc();
		} else {
			loc = map_location( args()[1]->evaluate(variables).as_int() - 1,
					args()[2]->evaluate(variables).as_int() - 1 );
		}

		if(m.is_village(loc) && (my_villages.count(loc)==0) ) {
			return variant(true);
		} else {
			return variant(false);
		}
	}

	const formula_ai& ai_;
};


class unit_at_function : public function_expression {
public:
	unit_at_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("unit_at", args, 1, 1), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const location_callable* loc = convert_variant<location_callable>(args()[0]->evaluate(variables));
		const unit_map::const_iterator i = ai_.get_info().units.find(loc->loc());
		if(i != ai_.get_info().units.end()) {
			return variant(new unit_callable(*i));
		} else {
			return variant();
		}
	}

	const formula_ai& ai_;
};


class unit_moves_function : public function_expression {
public:
	unit_moves_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("unit_moves", args, 1, 1), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant res = args()[0]->evaluate(variables);
		std::vector<variant> vars;
		if(res.is_null()) {
			return variant(&vars);
		}

		const map_location& loc = convert_variant<location_callable>(res)->loc();
		const ai::move_map& srcdst = ai_.srcdst();
		typedef ai::move_map::const_iterator Itor;
		std::pair<Itor,Itor> range = srcdst.equal_range(loc);

		for(Itor i = range.first; i != range.second; ++i) {
			vars.push_back(variant(new location_callable(i->second)));
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};


class units_can_reach_function : public function_expression {
public:
	units_can_reach_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("units_can_reach", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> vars;
		variant dstsrc_var = args()[0]->evaluate(variables);
		const ai::move_map& dstsrc = convert_variant<move_map_callable>(dstsrc_var)->dstsrc();
		std::pair<ai::move_map::const_iterator,ai::move_map::const_iterator> range =
		    dstsrc.equal_range(convert_variant<location_callable>(args()[1]->evaluate(variables))->loc());
		while(range.first != range.second) {
			unit_map::const_iterator un = ai_.get_info().units.find(range.first->second);
			assert(un != ai_.get_info().units.end());
			vars.push_back(variant(new unit_callable(*un)));
			++range.first;
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};


class defense_on_function : public function_expression {
public:
	defense_on_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("defense_on", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant u = args()[0]->evaluate(variables);
		variant loc_var = args()[1]->evaluate(variables);
		if(u.is_null() || loc_var.is_null()) {
			return variant();
		}

		const unit_callable* u_call = try_convert_variant<unit_callable>(u);
		const unit_type_callable* u_type = try_convert_variant<unit_type_callable>(u);
		const map_location& loc = convert_variant<location_callable>(loc_var)->loc();

		if (u_call)
		{
			const unit& un = u_call->get_unit();

                        if( un.total_movement() < un.movement_cost( ai_.get_info().map[loc]) )
                            return variant();

			if(!ai_.get_info().map.on_board(loc)) {
				return variant();
			}

			return variant(100 - un.defense_modifier(ai_.get_info().map[loc]));
		}

		if (u_type)
		{
			const unit_type& un = u_type->get_unit_type();

                        if( un.movement() < un.movement_type().movement_cost(ai_.get_info().map, ai_.get_info().map[loc]) )
                            return variant();

			if(!ai_.get_info().map.on_board(loc)) {
				return variant();
			}

			return variant(100 - un.movement_type().defense_modifier(ai_.get_info().map, ai_.get_info().map[loc]));
		}

		return variant();
	}

	const formula_ai& ai_;
};


class chance_to_hit_function : public function_expression {
public:
	chance_to_hit_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("chance_to_hit", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant u = args()[0]->evaluate(variables);
		variant loc_var = args()[1]->evaluate(variables);
		if(u.is_null() || loc_var.is_null()) {
			return variant();
		}

		const unit_callable* u_call = try_convert_variant<unit_callable>(u);
		const unit_type_callable* u_type = try_convert_variant<unit_type_callable>(u);
		const map_location& loc = convert_variant<location_callable>(loc_var)->loc();

		if (u_call)
		{
			const unit& un = u_call->get_unit();

			if(!ai_.get_info().map.on_board(loc)) {
				return variant();
			}

			return variant(un.defense_modifier(ai_.get_info().map[loc]));
		}

		if (u_type)
		{
			const unit_type& un = u_type->get_unit_type();

			if(!ai_.get_info().map.on_board(loc)) {
				return variant();
			}

			return variant(un.movement_type().defense_modifier(ai_.get_info().map, ai_.get_info().map[loc]));
		}

		return variant();
	}

	const formula_ai& ai_;
};


class movement_cost_function : public function_expression {
public:
	movement_cost_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("movement_cost", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant u = args()[0]->evaluate(variables);
		variant loc_var = args()[1]->evaluate(variables);
		if(u.is_null() || loc_var.is_null()) {
			return variant();
		}
		//we can pass to this function either unit_callable or unit_type callable
		const unit_callable* u_call = try_convert_variant<unit_callable>(u);
		const unit_type_callable* u_type = try_convert_variant<unit_type_callable>(u);
		const map_location& loc = convert_variant<location_callable>(loc_var)->loc();

		if (u_call)
		{
			const unit& un = u_call->get_unit();

			if(!ai_.get_info().map.on_board(loc)) {
				return variant();
			}

			return variant(un.movement_cost(ai_.get_info().map[loc]));
		}

		if (u_type)
		{
			const unit_type& un = u_type->get_unit_type();

			if(!ai_.get_info().map.on_board(loc)) {
				return variant();
			}

			return variant(un.movement_type().movement_cost(ai_.get_info().map, ai_.get_info().map[loc]));
		}

		return variant();
	}

	const formula_ai& ai_;
};


class max_possible_damage_function : public function_expression {
public:
	max_possible_damage_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("max_possible_damage", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant u1 = args()[0]->evaluate(variables);
		variant u2 = args()[1]->evaluate(variables);
		if(u1.is_null() || u2.is_null()) {
			return variant();
		}
		std::vector<attack_type> attacks_tmp;
		std::vector<attack_type>& attacks = attacks_tmp;

		//we have to make sure that this function works with any combination of unit_callable/unit_type_callable passed to it
		const unit_callable* u_attacker = try_convert_variant<unit_callable>(u1);
		if (u_attacker)
		{
			attacks = u_attacker->get_unit().attacks();
		} else
		{
			const unit_type_callable* u_t_attacker = convert_variant<unit_type_callable>(u1);
			attacks_tmp = u_t_attacker->get_unit_type().attacks();
		}

		const unit_callable* u_defender = try_convert_variant<unit_callable>(u2);
		if (u_defender)
		{
			const unit& defender = u_defender->get_unit();
			int best = 0;
			for(std::vector<attack_type>::const_iterator i = attacks.begin(); i != attacks.end(); ++i) {
				const int dmg = round_damage(i->damage(), defender.damage_from(*i, false, map_location()), 100) * i->num_attacks();
				if(dmg > best)
					best = dmg;
			}
			return variant(best);
		} else
		{
			const unit_type& defender = convert_variant<unit_type_callable>(u2)->get_unit_type();
			int best = 0;
			for(std::vector<attack_type>::const_iterator i = attacks.begin(); i != attacks.end(); ++i) {
				const int dmg = round_damage(i->damage(), defender.movement_type().resistance_against(*i), 100) * i->num_attacks();
				if(dmg > best)
					best = dmg;
			}
			return variant(best);
		}
	}

	const formula_ai& ai_;
};


class max_possible_damage_with_retaliation_function : public function_expression {
public:
	max_possible_damage_with_retaliation_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("max_possible_damage_with_retaliation", args, 2, 2), ai_(ai_object)
	{}
private:

	/*
	 * we have to make sure that this fuction works with any combination of unit_callable/unit_type_callable passed to it
	 * unit adapters let us treat unit and unit_type the same in the following code
	 */
	class unit_adapter {
		public:
			unit_adapter(const variant& arg) : unit_type_(), unit_() {
				const unit_callable* unit = try_convert_variant<unit_callable>(arg);

				if (unit) {
					unit_ = &unit->get_unit();
				} else {
					unit_type_ = &(convert_variant<unit_type_callable>(arg)->get_unit_type());
				}
			}

			int damage_from(const attack_type& attack) const {
				if(unit_type_ != NULL) {
					return unit_type_->movement_type().resistance_against(attack);
				} else {
					return unit_->damage_from(attack, false, map_location());
				}
			}

			// FIXME: we return a vector by value because unit_type and unit APIs
			// disagree as to what should be returned by their attacks() method
			std::vector<attack_type> attacks() const {
				if(unit_type_ != NULL) {
					return unit_type_->attacks();
				} else {
					return unit_->attacks();
				}
			}

		private:
			const unit_type *unit_type_;
			const unit* unit_;
	};

	std::pair<int, int> best_melee_and_ranged_attacks(unit_adapter attacker, unit_adapter defender) const {
		int highest_melee_damage = 0;
		int highest_ranged_damage = 0;

		std::vector<attack_type> attacks = attacker.attacks();

		foreach(const attack_type &attack, attacks) {
			const int dmg = round_damage(attack.damage(), defender.damage_from(attack), 100) * attack.num_attacks();
			if (attack.range() == "melee") {
				highest_melee_damage = std::max(highest_melee_damage, dmg);
			} else {
				highest_ranged_damage = std::max(highest_ranged_damage, dmg);
			}
		}

		return std::make_pair(highest_melee_damage, highest_ranged_damage);
	}

	variant execute(const formula_callable& variables) const {
		variant u1 = args()[0]->evaluate(variables);
		variant u2 = args()[1]->evaluate(variables);

		if(u1.is_null() || u2.is_null()) {
			return variant();
		}

		unit_adapter attacker(u1);
		unit_adapter defender(u2);

		// find max damage inflicted by attacker and by defender to the attacker
		std::pair<int, int> best_attacker_attacks = best_melee_and_ranged_attacks(attacker, defender);
		std::pair<int, int> best_defender_attacks = best_melee_and_ranged_attacks(defender, attacker);

		std::vector<variant> vars;
		vars.push_back(variant(best_attacker_attacks.first));
		vars.push_back(variant(best_attacker_attacks.second));
		vars.push_back(variant(best_defender_attacks.first));
		vars.push_back(variant(best_defender_attacks.second));

		return variant(&vars);
	}

	const formula_ai& ai_;
};

}


expression_ptr ai_function_symbol_table::create_function(const std::string &fn,
				const std::vector<expression_ptr>& args) const {
	if(fn == "outcomes") {
		return expression_ptr(new outcomes_function(args, ai_));
	} else if(fn == "evaluate_for_position") {
		return expression_ptr(new evaluate_for_position_function(args, ai_));
	} else if(fn == "move") {
		return expression_ptr(new move_function(args));
	} else if(fn == "move_partial") {
		return expression_ptr(new move_partial_function(args));
	} else if(fn == "attack") {
		return expression_ptr(new attack_function(args, ai_));
	} else if(fn == "recruit") {
		return expression_ptr(new recruit_function(args));
	} else if(fn == "safe_call") {
		return expression_ptr(new safe_call_function(args));
	} else if(fn == "get_unit_type") {
		return expression_ptr(new get_unit_type_function(args));
	} else if(fn == "is_village") {
		return expression_ptr(new is_village_function(args));
	} else if(fn == "is_unowned_village") {
		return expression_ptr(new is_unowned_village_function(args, ai_));
	} else if(fn == "unit_at") {
		return expression_ptr(new unit_at_function(args, ai_));
	} else if(fn == "unit_moves") {
		return expression_ptr(new unit_moves_function(args, ai_));
	} else if(fn == "set_var") {
		return expression_ptr(new set_var_function(args));
	} else if(fn == "set_unit_var") {
		return expression_ptr(new set_unit_var_function(args));
	} else if(fn == "fallback") {
		return expression_ptr(new fallback_function(args));
	} else if(fn == "units_can_reach") {
		return expression_ptr(new units_can_reach_function(args, ai_));
	} else if(fn == "debug_label") {
		return expression_ptr(new debug_label_function(args, ai_));
	} else if(fn == "defense_on") {
		return expression_ptr(new defense_on_function(args, ai_));
	} else if(fn == "chance_to_hit") {
		return expression_ptr(new chance_to_hit_function(args, ai_));
	} else if(fn == "movement_cost") {
		return expression_ptr(new movement_cost_function(args, ai_));
	} else if(fn == "max_possible_damage") {
		return expression_ptr(new max_possible_damage_function(args, ai_));
	} else if(fn == "max_possible_damage_with_retaliation") {
		return expression_ptr(new max_possible_damage_with_retaliation_function(args, ai_));
	} else if(fn == "adjacent_locs") {
		return expression_ptr(new adjacent_locs_function(args, ai_));
	} else if(fn == "locations_in_radius") {
		return expression_ptr(new locations_in_radius_function(args, ai_));
	} else if(fn == "castle_locs") {
		return expression_ptr(new castle_locs_function(args, ai_));
	} else if(fn == "timeofday_modifier") {
		return expression_ptr(new timeofday_modifier_function(args, ai_));
	} else if(fn == "distance_to_nearest_unowned_village") {
		return expression_ptr(new distance_to_nearest_unowned_village_function(args, ai_));
	} else if(fn == "shortest_path") {
		return expression_ptr(new shortest_path_function(args, ai_));
	} else if(fn == "simplest_path") {
		return expression_ptr(new simplest_path_function(args, ai_));
	} else if(fn == "nearest_keep") {
		return expression_ptr(new nearest_keep_function(args, ai_));
	} else if(fn == "suitable_keep") {
		return expression_ptr(new suitable_keep_function(args, ai_));
	} else if(fn == "nearest_loc") {
		return expression_ptr(new nearest_loc_function(args, ai_));
	} else if(fn == "find_shroud") {
		return expression_ptr(new find_shroud_function(args, ai_));
	} else if(fn == "close_enemies") {
		return expression_ptr(new close_enemies_function(args, ai_));
	} else if(fn == "calculate_outcome") {
		return expression_ptr(new calculate_outcome_function(args, ai_));
	} else if(fn == "distance_between") {
		return expression_ptr(new distance_between_function(args));
	} else if(fn == "run_file") {
		return expression_ptr(new run_file_function(args, ai_));
	} else {
		return function_symbol_table::create_function(fn, args);
	}
}

}
