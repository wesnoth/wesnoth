/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/formula_ai.cpp
 * Defines formula ai candidate actions - headers
 * */


#include <boost/lexical_cast.hpp>
#include <vector>
#include <queue>

#include "../foreach.hpp"
#include "../unit.hpp"

#include "ai_manager.hpp"
#include "../menu_events.hpp"
#include "../filesystem.hpp"
#include "../foreach.hpp"
#include "formula_ai.hpp"
#include "../log.hpp"
#include "../attack_prediction.hpp"
#include "formula_candidates.hpp"

static lg::log_domain log_formula_ai("ai/formula_ai");
#define LOG_AI LOG_STREAM(info, log_formula_ai)
#define WRN_AI LOG_STREAM(warn, log_formula_ai)
#define ERR_AI LOG_STREAM(err, log_formula_ai)

namespace {
using namespace game_logic;

class position_callable : public formula_callable {
	unit_map units_;
	int chance_;
	variant get_value(const std::string& key) const {
		if(key == "chance") {
			return variant(chance_);
		} else {
			return variant();
		}
	}

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("chance", game_logic::FORMULA_READ_ONLY));
	}
public:
	position_callable(unit_map* units, int chance) :
		units_(),
		chance_(chance)
	{
		units->swap(units_);
	}

	void swap_position(formula_ai& ai) {
		ai.get_info().units.swap(units_);
	}

	class swapper {
	public:
		formula_ai& ai;
		unit_map& a;
		unit_map& b;
		formula_ai::move_map_backup backup;
		void swap() {
			a.swap(b);
			ai.swap_move_map(backup);
		}
		swapper(formula_ai& ai, position_callable& pos) :
			ai(ai),
			a(ai.get_info().units),
			b(pos.units_),
			backup()
		{
		  swap();
		}

		~swapper() {
			swap();
		}
	};
	friend class swapper;
};

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

		return variant(combat_modifier(ai_.get_info().state, ai_.get_info().units, *loc, un.alignment(), un.is_fearless(), ai_.get_info().map));
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


class outcome_callable : public formula_callable {
	std::vector<variant> hitLeft_, prob_, status_;
	variant get_value(const std::string& key) const {
		if(key == "hitpoints_left") {
			return variant(new std::vector<variant>(hitLeft_));
		} else if(key == "probability") {
			return variant(new std::vector<variant>(prob_));
		} else if(key == "possible_status") {
			return variant(new std::vector<variant>(status_));
		} else {
			return variant();
		}
	}

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("hitpoints_left", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("probability", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("possible_status", game_logic::FORMULA_READ_ONLY));
	}
public:
	outcome_callable(		const std::vector<variant>& hitLeft,
					const std::vector<variant>& prob,
					const std::vector<variant>& status)
	  : hitLeft_(hitLeft), prob_(prob), status_(status)
	{
	}

	const std::vector<variant>& hitLeft() const { return hitLeft_; }
	const std::vector<variant>& prob() const { return prob_; }
	const std::vector<variant>& status() const { return status_; }
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
			ai_.get_info().state, convert_variant<location_callable>(args()[1]->evaluate(variables))->loc(),
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

class recruit_callable : public formula_callable {
	map_location loc_;
	std::string type_;
	variant get_value(const std::string& /*key*/) const { return variant(); }
public:
	recruit_callable(const map_location& loc, const std::string& type)
	  : loc_(loc), type_(type)
	{}

	const map_location& loc() const { return loc_; }
	const std::string& type() const { return type_; }
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

                paths::route route = ai_.shortest_path_calculator( src, dst, unit_it, allowed_teleports );

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

                paths::route route = a_star_search(src, dst, 1000.0, &em_calc, ai_.get_info().map.w(), ai_.get_info().map.h(), &allowed_teleports);

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

class set_var_callable : public formula_callable {
	std::string key_;
	variant value_;
	variant get_value(const std::string& /*key*/) const { return variant(); }
public:
	set_var_callable(const std::string& key, const variant& value)
	  : key_(key), value_(value)
	{}

	const std::string& key() const { return key_; }
	variant value() const { return value_; }
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

class set_unit_var_callable : public formula_callable {
	std::string key_;
	variant value_;
	map_location loc_;
	variant get_value(const std::string& /*key*/) const { return variant(); }
public:
	set_unit_var_callable(const std::string& key, const variant& value, const map_location loc)
	  : key_(key), value_(value), loc_(loc)
	{}

	const std::string& key() const { return key_; }
	variant value() const { return value_; }
	const map_location loc() const { return loc_; }
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

class fallback_callable : public formula_callable {
	std::string key_;
	variant get_value(const std::string& /*key*/) const { return variant(); }
public:
	explicit fallback_callable(const std::string& key) : key_(key) {
	}

	const std::string& key() const { return key_; }
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

class attack_callable : public formula_callable {
	map_location move_from_, src_, dst_;
	battle_context bc_;
	variant get_value(const std::string& key) const {
		if(key == "attacker") {
			return variant(new location_callable(src_));
		} else if(key == "defender") {
			return variant(new location_callable(dst_));
		} else if(key == "move_from") {
			return variant(new location_callable(move_from_));
		} else {
			return variant();
		}
	}

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("attacker", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("defender", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("move_from", game_logic::FORMULA_READ_ONLY));
	}
public:
	attack_callable(const formula_ai& ai,
					const map_location& move_from,
					const map_location& src, const map_location& dst,
	                int weapon)
	  : move_from_(move_from), src_(src), dst_(dst),
		bc_(ai.get_info().map, ai.get_info().teams, ai.get_info().units,
			ai.get_info().state, src, dst, weapon, -1, 1.0, NULL,
			&ai.get_info().units.find(move_from)->second)
	{
	}

	const map_location& move_from() const { return move_from_; }
	const map_location& src() const { return src_; }
	const map_location& dst() const { return dst_; }
	int weapon() const { return bc_.get_attacker_stats().attack_num; }
	int defender_weapon() const { return bc_.get_defender_stats().attack_num; }
};


class attack_map_callable : public formula_callable {
public:
	typedef std::multimap<map_location, map_location> move_map;
	attack_map_callable(const formula_ai& ai, const move_map& srcdst, const unit_map& units)
		: srcdst_(srcdst), units_(units), ai_(ai)
	{}
private:
	const move_map& srcdst_;
	const unit_map& units_;
	const formula_ai& ai_;

	variant get_value(const std::string& key) const {
		if(key == "attacks") {
			std::vector<variant> vars;
			for(move_map::const_iterator i = srcdst_.begin(); i != srcdst_.end(); ++i) {
				/* for each possible move check all adjacent tiles for enemies */
				if(units_.count(i->second) == 0) {
					collect_possible_attacks(vars, i->first, i->second);
				}
			}
			/* special case, when unit moved toward enemy and can only attack */
			for(unit_map::const_iterator i = ai_.get_info().units.begin(); i != ai_.get_info().units.end(); ++i) {
				if((i->second.side() == ai_.get_side()) && (i->second.attacks_left() > 0)) {
					collect_possible_attacks(vars, i->first, i->first);
				}
			}
			return variant(&vars);
		} else {
			return variant();
		}
	}

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("attacks", game_logic::FORMULA_READ_ONLY));
	}

	/* add to vars all attacks on enemy units around <attack_position> tile. attacker_location is tile where unit is currently standing. It's moved to attack_position first and then performs attack.*/
	void collect_possible_attacks(std::vector<variant>& vars, map_location attacker_location, map_location attack_position) const {
		map_location adj[6];
		get_adjacent_tiles(attack_position, adj);
		for(int n = 0; n != 6; ++n) {
			/* if adjacent tile is outside the board */
			if (! ai_.get_info().map.on_board(adj[n]))
				continue;
			unit_map::const_iterator unit = units_.find(adj[n]);
			/* if tile is empty */
			if (unit == units_.end())
				continue;
			/* if tile is occupied by friendly or petrified/invisible unit */
			if (! ai_.current_team().is_enemy(unit->second.side())  ||
					unit->second.incapacitated() ||
					unit->second.invisible(unit->first, units_, ai_.get_info().teams) )
				continue;
			/* add attacks with default weapon */
			attack_callable* item = new attack_callable(ai_, attacker_location, attack_position, adj[n], -1);
			vars.push_back(variant(item));
		}
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
		const formula_ai::move_map& srcdst = ai_.srcdst();
		typedef formula_ai::move_map::const_iterator Itor;
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
	variant execute(const formula_callable& variables) const {
		variant u1 = args()[0]->evaluate(variables);
		variant u2 = args()[1]->evaluate(variables);
		if(u1.is_null() || u2.is_null()) {
			return variant();
		}
		std::vector<variant> vars;

		//store best damage and best attack causing it
		int best_melee = 0;
		int best_ranged = 0;
		std::vector<attack_type>::const_iterator best_attack;

		//vectors with attacks for attacker and defender
		std::vector<attack_type> att_attacks_tmp;
		std::vector<attack_type>& att_attacks = att_attacks_tmp;

		std::vector<attack_type> def_attacks_tmp;
		std::vector<attack_type>& def_attacks = def_attacks_tmp;

		//we have to make sure that this fuction works with any combination of unit_callable/unit_type_callable passed to it
		const unit_callable* u_attacker = try_convert_variant<unit_callable>(u1);
		if (u_attacker)
		{
			const unit& attacker = u_attacker->get_unit();
			att_attacks = attacker.attacks();

			const unit_callable* u_defender = try_convert_variant<unit_callable>(u2);
			if (u_defender)
			{
				const unit& defender = u_defender->get_unit();

				for(std::vector<attack_type>::const_iterator i = att_attacks.begin(); i != att_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), defender.damage_from(*i, false, map_location()), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				//we have max damage inflicted by attacker, now we need to search for max possible damage of defender (search only for attack with the same range)
				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));

				best_melee = 0;
				best_ranged = 0;
				def_attacks = defender.attacks();

				for(std::vector<attack_type>::const_iterator i = def_attacks.begin(); i != def_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), attacker.damage_from(*i, false, map_location()), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));
				return variant(&vars);

			} else
			{
				const unit_type& defender = convert_variant<unit_type_callable>(u2)->get_unit_type();

				for(std::vector<attack_type>::const_iterator i = att_attacks.begin(); i != att_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), defender.movement_type().resistance_against(*i), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));

				best_melee = 0;
				best_ranged = 0;
				def_attacks_tmp = defender.attacks();

				for(std::vector<attack_type>::const_iterator i = def_attacks.begin(); i != def_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), attacker.damage_from(*i, false, map_location()), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));
				return variant(&vars);
			}

		} else
		{
			const unit_type& attacker = convert_variant<unit_type_callable>(u1)->get_unit_type();
			att_attacks = attacker.attacks();

			const unit_callable* u_defender = try_convert_variant<unit_callable>(u2);
			if (u_defender)
			{
				const unit& defender = u_defender->get_unit();

				for(std::vector<attack_type>::const_iterator i = att_attacks.begin(); i != att_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), defender.damage_from(*i, false, map_location()), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				//we have max damage inflicted by attacker, now we need to search for max possible damage of defender (search only for attack with the same range)
				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));

				best_melee = 0;
				best_ranged = 0;
				def_attacks = defender.attacks();

				for(std::vector<attack_type>::const_iterator i = def_attacks.begin(); i != def_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), attacker.movement_type().resistance_against(*i), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));
				return variant(&vars);

			} else
			{
				const unit_type& defender = convert_variant<unit_type_callable>(u2)->get_unit_type();

				for(std::vector<attack_type>::const_iterator i = att_attacks.begin(); i != att_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), defender.movement_type().resistance_against(*i), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));

				best_melee = 0;
				best_ranged = 0;
				def_attacks_tmp = defender.attacks();

				for(std::vector<attack_type>::const_iterator i = def_attacks.begin(); i != def_attacks.end(); ++i) {
					const int dmg = round_damage(i->damage(), attacker.movement_type().resistance_against(*i), 100) * i->num_attacks();
					if ( i->range() == "melee")
					{
						if(dmg > best_melee)
							best_melee = dmg;
					} else {
						if(dmg > best_ranged)
							best_ranged = dmg;
					}
				}

				vars.push_back(variant(best_melee));
				vars.push_back(variant(best_ranged));
				return variant(&vars);
			}
		}
	}

	const formula_ai& ai_;
};
}

std::string formula_ai::describe_self(){
	return "[formula_ai]";
}


namespace game_logic {
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

formula_ai::formula_ai(int side, bool master) :
	ai(side,master),
	recruit_formula_(),
	move_formula_(),
	outcome_positions_(),
	possible_moves_(),
	move_maps_valid_(false),
	srcdst_(),
	dstsrc_(),
	full_srcdst_(),
	full_dstsrc_(),
	enemy_srcdst_(),
	enemy_dstsrc_(),
	attacks_cache_(),
	keeps_cache_(),
	vars_(),
	function_table(*this),
	candidate_action_manager_()
{
	//make sure we don't run out of refcount
	vars_.add_ref();
	const config& ai_param = current_team().ai_parameters();

	// load candidate actions from config
	candidate_action_manager_.load_config(ai_param, this, &function_table);

	foreach (const config &func, ai_param.child_range("function"))
	{
		const t_string &name = func["name"];
		const t_string &inputs = func["inputs"];
		const t_string &formula_str = func["formula"];

		std::vector<std::string> args = utils::split(inputs);

		try {
			function_table.add_formula_function(name,
				game_logic::const_formula_ptr(new game_logic::formula(formula_str, &function_table)),
				game_logic::formula::create_optional_formula(func["precondition"], &function_table),
				args);
			}
			catch(formula_error& e) {
				handle_exception(e, "Error while registering function '" + name + "'");
			}
		}


        try{
                recruit_formula_ = game_logic::formula::create_optional_formula(current_team().ai_parameters()["recruitment"], &function_table);
        }
        catch(formula_error& e) {
                handle_exception(e);
                recruit_formula_ = game_logic::formula_ptr();
        }

        try{
                move_formula_ = game_logic::formula::create_optional_formula(current_team().ai_parameters()["move"], &function_table);
        }
        catch(formula_error& e) {
                handle_exception(e);
                move_formula_ = game_logic::formula_ptr();
        }

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
		display_message(e.type + " in " + e.filename + ":" + boost::lexical_cast<std::string>(e.line));
	} else {
		LOG_AI << e.type << std::endl;
		display_message(e.type);
	}
}

void formula_ai::display_message(const std::string& msg) const
{
	get_info().disp.add_chat_message(time(NULL), "fai", get_side(), msg,
				game_display::MESSAGE_PUBLIC, false);

}

formula_ptr formula_ai::create_optional_formula(const std::string& formula_string){
	try{
		return game_logic::formula::create_optional_formula(formula_string, &function_table);
	}
	catch(formula_error& e) {
		handle_exception(e);
		return game_logic::formula_ptr();
	}
}

void formula_ai::new_turn()
{
	move_maps_valid_ = false;
	ai::new_turn();
}

void formula_ai::play_turn()
{
	//execute units formulas first

        unit_formula_set units_with_formulas;

	for(unit_map::unit_iterator i = units_.begin() ; i != units_.end() ; ++i)
	{
            if ( (i->second.side() == get_side())  )
            {
                if ( i->second.has_formula() || i->second.has_loop_formula()) {
                    int priority = 0;
                    if( i->second.has_priority_formula() ) {
                        try {
                            game_logic::const_formula_ptr priority_formula(new game_logic::formula(i->second.get_priority_formula(), &function_table));
                            game_logic::map_formula_callable callable(this);
                            callable.add_ref();
                            callable.add("me", variant(new unit_callable(*i)));
                            priority = (formula::evaluate(priority_formula, callable)).as_int();
                        } catch(formula_error& e) {
                                if(e.filename == "formula")
                                        e.line = 0;
                                handle_exception( e, "Unit priority formula error for unit: '" + i->second.type_id() + "' standing at (" + boost::lexical_cast<std::string>(i->first.x+1) + "," + boost::lexical_cast<std::string>(i->first.y+1) + ")");

                                priority = 0;
                        } catch(type_error& e) {
                                priority = 0;
                                ERR_AI << "formula type error while evaluating unit priority formula  " << e.message << "\n";
                        }
                    }

                    units_with_formulas.insert( unit_formula_pair( i, priority ) );
                }
            }
        }

	for(unit_formula_set::iterator pair_it = units_with_formulas.begin() ; pair_it != units_with_formulas.end() ; ++pair_it)
	{
            unit_map::iterator i = pair_it->first;

            if( i.valid() ) {

                if ( i->second.has_formula() ) {
                    try {
                            game_logic::const_formula_ptr formula(new game_logic::formula(i->second.get_formula(), &function_table));
                            game_logic::map_formula_callable callable(this);
                            callable.add_ref();
                            callable.add("me", variant(new unit_callable(*i)));
                            make_action(formula, callable);
                    }
                    catch(formula_error& e) {
                            if(e.filename == "formula")
                                    e.line = 0;
                            handle_exception( e, "Unit formula error for unit: '" + i->second.type_id() + "' standing at (" + boost::lexical_cast<std::string>(i->first.x+1) + "," + boost::lexical_cast<std::string>(i->first.y+1) + ")");
                    }
                }
            }

            if( i.valid() ) {
                if( i->second.has_loop_formula() )
                {
                        try {
                                game_logic::const_formula_ptr loop_formula(new game_logic::formula(i->second.get_loop_formula(), &function_table));
                                game_logic::map_formula_callable callable(this);
                                callable.add_ref();
                                callable.add("me", variant(new unit_callable(*i)));
                                while ( make_action(loop_formula, callable) && i.valid() ) {}
                        }
                        catch(formula_error& e) {
                                if(e.filename == "formula")
                                        e.line = 0;
                                handle_exception( e, "Unit loop formula error for unit: '" + i->second.type_id() + "' standing at (" + boost::lexical_cast<std::string>(i->first.x+1) + "," + boost::lexical_cast<std::string>(i->first.y+1) + ")");
                        }
                }
            }
	}

	if( candidate_action_manager_.has_candidate_actions() ) {
		move_maps_valid_ = false;
		while( candidate_action_manager_.evaluate_candidate_actions(this, units_) )
		{
			game_logic::map_formula_callable callable(this);
			callable.add_ref();

			candidate_action_manager_.update_callable_map( callable );

			const_formula_ptr move_formula(candidate_action_manager_.get_best_action_formula());

			make_action(move_formula, callable);

			move_maps_valid_ = false;
		}
	}

	game_logic::map_formula_callable callable(this);
	callable.add_ref();
        while(make_action(move_formula_,callable)) { }

}

std::string formula_ai::evaluate(const std::string& formula_str)
{
	try{
		move_maps_valid_ = false;

		game_logic::formula f(formula_str, &function_table);

		game_logic::map_formula_callable callable(this);
		callable.add_ref();

		const variant v = f.execute(callable);

                //first read info about move, then clear outcome_positions
                std::string move_info = v.to_debug_string();
                outcome_positions_.clear();

		if ( execute_variant(v, true ) )
			return "Made move: " + move_info;

		return move_info;
	}
	catch(formula_error& e) {
		e.line = 0;
		handle_exception(e);
		throw;
	}
}

void formula_ai::store_outcome_position(const variant& var)
{
    outcome_positions_.push_back(var);
}

void formula_ai::swap_move_map(move_map_backup& backup)
{
	std::swap(move_maps_valid_, backup.move_maps_valid);
	std::swap(backup.attacks_cache, attacks_cache_);
	backup.move_maps_valid = move_maps_valid_;
	backup.srcdst.swap(srcdst_);
	backup.dstsrc.swap(dstsrc_);
	backup.full_srcdst.swap(full_srcdst_);
	backup.full_dstsrc.swap(full_dstsrc_);
	backup.enemy_srcdst.swap(enemy_srcdst_);
	backup.enemy_dstsrc.swap(enemy_dstsrc_);
}

void formula_ai::prepare_move() const
{
	if(move_maps_valid_) {
		return;
	}

	possible_moves_.clear();
	srcdst_.clear();
	dstsrc_.clear();

	calculate_possible_moves(possible_moves_, srcdst_, dstsrc_, false);

	full_srcdst_.clear();
	full_dstsrc_.clear();

	std::map<location,paths> possible_moves_dummy;
	calculate_possible_moves(possible_moves_dummy, full_srcdst_, full_dstsrc_, false, true);

	enemy_srcdst_.clear();
	enemy_dstsrc_.clear();
	possible_moves_dummy.clear();
	calculate_possible_moves(possible_moves_dummy, enemy_srcdst_, enemy_dstsrc_, true);

	attacks_cache_ = variant();
	move_maps_valid_ = true;
}

bool formula_ai::make_action(game_logic::const_formula_ptr formula_, const game_logic::formula_callable& variables)
{
	if(!formula_) {
		if(get_master()) {
			LOG_AI << "Falling back to default AI.\n";
			util::scoped_ptr< ai_interface > fallback( ai_manager::create_transient_ai(ai_manager::AI_TYPE_DEFAULT, get_side(),false));
			if (fallback != NULL){
				fallback->play_turn();
			}
		}
		return false;
	}

	move_maps_valid_ = false;

	LOG_AI << "do move...\n";
	const variant var = formula_->execute(variables);

        bool res = execute_variant(var);

        //remove outcome_positions
        outcome_positions_.clear();

	return res;
}

paths::route formula_ai::shortest_path_calculator(const map_location& src, const map_location& dst, unit_map::iterator& unit_it, std::set<map_location>& allowed_teleports) const {

    map_location destination = dst;

    ::shortest_path_calculator calc(unit_it->second, current_team(), units_, get_info().teams, get_info().map);

    unit_map::const_iterator dst_un = units_.find(destination);

    map_location res;

    if( dst_un != units_.end() ) {
        //there is unit standing at dst, let's try to find free hex to move to
        const map_location::DIRECTION preferred = destination.get_relative_dir(src);

        int best_rating = 100;//smaller is better
        map_location adj[6];
        get_adjacent_tiles(destination,adj);

        for(size_t n = 0; n != 6; ++n) {
                if(map_.on_board(adj[n]) == false) {
                        continue;
                }

                if(units_.find(adj[n]) != units_.end()) {
                        continue;
                }

                static const size_t NDIRECTIONS = map_location::NDIRECTIONS;
                unsigned int difference = abs(int(preferred - n));
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

    paths::route route = a_star_search(src, destination, 1000.0, &calc,
            get_info().map.w(), get_info().map.h(), &allowed_teleports);

    return route;
}

std::set<map_location> formula_ai::get_allowed_teleports(unit_map::iterator& unit_it) const {
    std::set<map_location> allowed_teleports;

    if(unit_it->second.get_ability_bool("teleport",unit_it->first)) {
            for(std::set<map_location>::const_iterator i = current_team().villages().begin();
                            i != current_team().villages().end(); ++i) {
                    //if (viewing_team().is_enemy(unit_it->second.side()) && viewing_team().fogged(*i))
                    //        continue;

                    unit_map::const_iterator occupant = units_.find(*i);
                    if (occupant != units_.end() && occupant != unit_it)
                            continue;

                    allowed_teleports.insert(*i);
            }
    }

    return allowed_teleports;
}

map_location formula_ai::path_calculator(const map_location& src, const map_location& dst, unit_map::iterator& unit_it) const{
    std::map<map_location,paths>::iterator path = possible_moves_.find(src);

    map_location destination = dst;

    //check if destination is within unit's reach, if not, calculate where to move
    if( path->second.routes.count(dst) == 0) {

            std::set<map_location> allowed_teleports = get_allowed_teleports(unit_it);
            //destination is too far, check where unit can go

             paths::route route = shortest_path_calculator( src, dst, unit_it, allowed_teleports );

            if( route.steps.size() == 0 ) {
                emergency_path_calculator em_calc(unit_it->second, get_info().map);

                route = a_star_search(src, dst, 1000.0, &em_calc, get_info().map.w(), get_info().map.h(), &allowed_teleports);

                if( route.steps.size() < 2 ) {
                    return map_location();
                }
            }

            destination = map_location();

            for (std::vector<map_location>::const_iterator loc_iter = route.steps.begin() + 1 ; loc_iter !=route.steps.end(); ++loc_iter) {
		typedef formula_ai::move_map::const_iterator Itor;
		std::pair<Itor,Itor> range = srcdst_.equal_range(src);

                bool found = false;
		for(Itor i = range.first; i != range.second; ++i) {
			if (i->second == *loc_iter ) {
                            found = true;
                            break;
                        }
		}
                if ( !found ) {
                    continue;
                }

                destination = *loc_iter;
            }
            return destination;
    }

    return destination;
}

//commandline=true when we evaluate formula from commandline, false otherwise (default)
bool formula_ai::execute_variant(const variant& var, bool commandline)
{
	std::vector<variant> vars;
	if(var.is_list()) {
		for(size_t n = 0; n != var.num_elements(); ++n) {
			vars.push_back(var[n]);
		}
	} else {
		vars.push_back(var);
	}

	bool made_move = false;

	for(std::vector<variant>::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		if(i->is_null()) {
			continue;
		}

		const move_callable* move = try_convert_variant<move_callable>(*i);
		const move_partial_callable* move_partial = try_convert_variant<move_partial_callable>(*i);
		const attack_callable* attack = try_convert_variant<attack_callable>(*i);
		const ai::attack_analysis* attack_analysis = try_convert_variant<ai::attack_analysis>(*i);
		const recruit_callable* recruit_command = try_convert_variant<recruit_callable>(*i);
		const set_var_callable* set_var_command = try_convert_variant<set_var_callable>(*i);
		const set_unit_var_callable* set_unit_var_command = try_convert_variant<set_unit_var_callable>(*i);
		const fallback_callable* fallback_command = try_convert_variant<fallback_callable>(*i);

		prepare_move();
		if(move) {
			unit_map::iterator unit_it = units_.find(move->src());
			if( (possible_moves_.count(move->src()) > 0) && (unit_it->second.movement_left() != 0) && get_info().map.on_board(move->dst() ) ) {

                                map_location destination = path_calculator(move->src(), move->dst(), unit_it);

                                if( destination != map_location()) {
                                    map_location new_location = move_unit(move->src(), destination, possible_moves_);
                                    if ( (new_location != move->src()) || (move->src()==move->dst()) ){
                                        unit_map::iterator unit = get_info().units.find(new_location);

                                        if(unit != get_info().units.end()) {
                                                unit->second.set_movement(0);
                                        } else {
                                                throw formula_error("Incorrect result of calling the move() formula", "", "", 0);
                                        }

                                        LOG_AI << "MOVE: " << move->src().x+1 << "," << move->src().y+1 << " -> " << move->dst().x+1 << "," << move->dst().y+1 << "\n";
                                        made_move = true;

                                    } else {
                                           ERR_AI << "IMPOSSIBLE MOVE ORDER - MOVE FAILED\n";
                                    }
                                } else
                                    ERR_AI << "IMPOSSIBLE MOVE ORDER\n";
			}
		} else if(move_partial) {
			unit_map::iterator unit_it = units_.find(move_partial->src());
			if( (possible_moves_.count(move_partial->src()) > 0) && (unit_it->second.movement_left() != 0) && get_info().map.on_board(move_partial->dst()) ) {
                                map_location destination = path_calculator(move_partial->src(), move_partial->dst(), unit_it);

                                if( destination != map_location()) {
                                    LOG_AI << "MOVE PARTIAL: " << move_partial->src().x+1 << "," << move_partial->src().y+1 << " -> " << move_partial->dst().x+1 << "," << move_partial->dst().y+1 << "\n";
                                    move_unit_partial(move_partial->src(), destination, possible_moves_);
                                    made_move = true;
                                } else
                                    ERR_AI << "IMPOSSIBLE MOVE PARTIAL ORDER\n";
			}
		} else if(attack) {
			if(get_info().units.count(attack->dst()) == 0) {
				//this is a legitimate situation; someone might send a series of units in
				//to attack, but if the defender dies in the middle, we'll save the unit
				//ordered to move so it can get a different command.
				continue;
			}

                        if( !get_info().map.on_board(attack->dst()) || !get_info().map.on_board(attack->src()) || !get_info().map.on_board(attack->move_from())) {
                            ERR_AI << "IMPOSSIBLE ATTACK ORDER\n";
                            continue;
                        }

                        if( attack->move_from() != attack->src() ) {

                            std::map<map_location,paths>::iterator path = possible_moves_.find(attack->move_from());

                            if( path->second.routes.count(attack->src()) == 0) {
                                ERR_AI << "IMPOSSIBLE ATTACK ORDER\n";
                                continue;
                            }

                            LOG_AI << "MOVE: " << attack->move_from().x+1 << "," << attack->move_from().y+1 << " -> " << attack->src().x+1 << "," << attack->src().y+1 << "\n";
                            move_unit(attack->move_from(), attack->src(), possible_moves_);
                        }

                        unit_map::iterator unit_it = units_.find(attack->src());

                        int x_diff = attack->src().x - attack->dst().x;
                        int y_diff = attack->src().y - attack->dst().y;

                        if( x_diff < 2 && x_diff > -2 && y_diff < 2 &&  y_diff > -2 && attack->src() != attack->dst() ) {
                            if( ( unit_it != units_.end() ) && (unit_it->second.attacks_left() != 0) ) {
                                LOG_AI << "ATTACK: " << attack->src() << " -> " << attack->dst() << " " << attack->weapon() << "\n";
                                attack_enemy(attack->src(), attack->dst(), attack->weapon(), attack->defender_weapon());
                            }
                        }

			made_move = true;
		} else if(attack_analysis) {
			//If we get an attack analysis back we will do the first attack.
			//Then the AI can get run again and re-choose.
			assert(attack_analysis->movements.empty() == false);

			//make sure that unit which has to attack is at given position and is able to attack
			unit_map::const_iterator unit = units_.find(attack_analysis->movements.front().first);
			if ( ( unit == units_.end() ) || (unit->second.attacks_left() == 0) )
				continue;

			const map_location& move_from = attack_analysis->movements.front().first;
			const map_location& att_src = attack_analysis->movements.front().second;
			const map_location& att_dst = attack_analysis->target;

			//check if target is still valid
			unit = units_.find(att_dst);
			if ( unit == units_.end() )
				continue;

                        //check if we need to move
                        if( move_from != att_src ) {
                            //now check if location to which we want to move is still unoccupied
                            unit = units_.find(att_src);
                            if ( unit != units_.end() )
                                    continue;

                            move_unit(move_from, att_src, possible_moves_);
                        }

			if(get_info().units.count(att_src)) {
				battle_context bc(get_info().map, get_info().teams,
				                  get_info().units, get_info().state,
				                  att_src, att_dst, -1, -1, 1.0, NULL,
								  &get_info().units.find(att_src)->second);
				attack_enemy(attack_analysis->movements.front().second,
				             attack_analysis->target,
							 bc.get_attacker_stats().attack_num,
							 bc.get_defender_stats().attack_num);
			}
			made_move = true;
		} else if(recruit_command) {
			LOG_AI << "RECRUIT: '" << recruit_command->type() << "'\n";
			if(recruit(recruit_command->type(), recruit_command->loc())) {
				made_move = true;
			}
		} else if(set_var_command) {
			LOG_AI << "setting var: " << set_var_command->key() << " -> " << set_var_command->value().to_debug_string() << "\n";
			vars_.add(set_var_command->key(), set_var_command->value());
			made_move = true;
		} else if(set_unit_var_command) {
			LOG_AI << "setting unit var: " << set_unit_var_command->key() << " -> " << set_unit_var_command->value().to_debug_string() << "\n";
			unit_map::iterator unit = units_.find(set_unit_var_command->loc());
			if(unit != units_.end()) {
				if( unit->second.side() == get_side() ) {
					unit->second.add_formula_var(set_unit_var_command->key(), set_unit_var_command->value());
				}
			} else {
				std::ostringstream str;
				str << "set_var function: expected unit at location (" << (set_unit_var_command->loc().x+1) << "," << (set_unit_var_command->loc().y+1) << ")";
				throw formula_error( str.str(), "", "", 0);
			}
			made_move = true;
		} else if(i->is_string() && i->as_string() == "recruit") {
			do_recruitment();
			made_move = true;
		} else if(i->is_string() && (i->as_string() == "end_turn" || i->as_string() == "end" )  ) {
			return false;
		} else if(fallback_command) {
			if (get_master())
			{
				if(fallback_command->key() == "human")
				{
					//we want give control of the side to human for the rest of this turn
					throw fallback_ai_to_human_exception();
				} else
				{
					LOG_AI << "Explicit fallback to: " << fallback_command->key() << std::endl;
					util::scoped_ptr< ai_interface > fallback ( ai_manager::create_transient_ai(fallback_command->key(), get_side(),false));
					if(fallback != NULL) {
						fallback->play_turn();
					}
				}
			}
			return false;
		} else {
			//this information is unneded when evaluating formulas form commandline
			if (!commandline) {
				ERR_AI << "UNRECOGNIZED MOVE: " << i->to_debug_string() << "\n";
			}
		}
	}

	return made_move;
}


bool formula_ai::do_recruitment()
{
	if(!recruit_formula_) {
		return false;
	}

	variant var = recruit_formula_->execute(*this);
	std::vector<variant> vars;
	if(var.is_list()) {
		for(size_t n = 0; n != var.num_elements(); ++n) {
			vars.push_back(var[n]);
		}
	} else {
		vars.push_back(var);
	}

	bool ret = false;
	for(std::vector<variant>::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		if(!i->is_string()) {
			return false;
		}

		if(!recruit(i->as_string())) {
			return ret;
		}
		ret = true;
	}

	return do_recruitment();
}

namespace {
template<typename Container>
variant villages_from_set(const Container& villages,
				          const std::set<map_location>* exclude=NULL) {
	std::vector<variant> vars;
	foreach(const map_location& loc, villages) {
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
	if(key == "attacks")
	{
		prepare_move();
		if(attacks_cache_.is_null() == false) {
			return attacks_cache_;
		}

		std::vector<attack_analysis> attacks = const_cast<formula_ai*>(this)->analyze_targets(srcdst_, dstsrc_, enemy_srcdst_, enemy_dstsrc_);
		std::vector<variant> vars;
		for(std::vector<attack_analysis>::const_iterator i = attacks.begin(); i != attacks.end(); ++i) {
			vars.push_back(variant(new attack_analysis(*i)));
		}

		attacks_cache_ = variant(&vars);
		return attacks_cache_;

	} else if(key == "turn")
	{
		return variant(get_info().state.turn());

	} else if(key == "time_of_day")
	{
		return variant(get_info().state.get_time_of_day().id);

	} else if(key == "my_side")
	{
		return variant(new team_callable((*get_info().state.teams)[get_side()-1]));

	} else if(key == "my_side_number")
	{
		return variant(get_side()-1);

	} else if(key == "teams")
	{
		std::vector<variant> vars;
		for(std::vector<team>::const_iterator i = get_info().state.teams->begin(); i != get_info().state.teams->end(); ++i) {
			vars.push_back(variant(new team_callable(*i)));
		}
		return variant(&vars);

	} else if(key == "allies")
	{
		std::vector<variant> vars;
		for( size_t i = 0; i < get_info().teams.size(); ++i) {
			if ( !current_team().is_enemy( i+1 ) )
				vars.push_back(variant( i ));
		}
		return variant(&vars);

	} else if(key == "enemies")
	{
		std::vector<variant> vars;
		for( size_t i = 0; i < get_info().teams.size(); ++i) {
			if ( current_team().is_enemy( i+1 ) )
				vars.push_back(variant( i ));
		}
		return variant(&vars);

	} else if(key == "my_recruits")
	{
		std::vector<variant> vars;

		unit_type_data::types().build_all(unit_type::FULL);

		const std::set<std::string>& recruits = current_team().recruits();
		if(recruits.size()==0)
			return variant( &vars );
		for(std::set<std::string>::const_iterator i = recruits.begin(); i != recruits.end(); ++i)
		{
			std::map<std::string,unit_type>::const_iterator unit_it = unit_type_data::types().find_unit_type(*i);
			if (unit_it != unit_type_data::types().end() )
			{
				vars.push_back(variant(new unit_type_callable(unit_it->second) ));
			}
		}
		return variant( &vars );

	} else if(key == "recruits_of_side")
	{
		std::vector<variant> vars;
		std::vector< std::vector< variant> > tmp;

		unit_type_data::types().build_all(unit_type::FULL);

		for( size_t i = 0; i<get_info().teams.size(); ++i)
		{
			std::vector<variant> v;
			tmp.push_back( v );

			const std::set<std::string>& recruits = get_info().teams[i].recruits();
			if(recruits.size()==0)
				continue;
			for(std::set<std::string>::const_iterator str_it = recruits.begin(); str_it != recruits.end(); ++str_it)
			{
				std::map<std::string,unit_type>::const_iterator unit_it = unit_type_data::types().find_unit_type(*str_it);
				if (unit_it != unit_type_data::types().end() )
				{
					tmp[i].push_back(variant(new unit_type_callable(unit_it->second) ));
				}
			}
		}

		for( size_t i = 0; i<tmp.size(); ++i)
			vars.push_back( variant( &tmp[i] ));
		return variant(&vars);

	} else if(key == "units")
	{
		std::vector<variant> vars;
		for(unit_map::const_iterator i = get_info().units.begin(); i != get_info().units.end(); ++i) {
			vars.push_back(variant(new unit_callable(*i)));
		}
		return variant(&vars);

	} else if(key == "units_of_side")
	{
		std::vector<variant> vars;
		std::vector< std::vector< variant> > tmp;
		for( size_t i = 0; i<get_info().teams.size(); ++i)
		{
			std::vector<variant> v;
			tmp.push_back( v );
		}
		for(unit_map::const_iterator i = get_info().units.begin(); i != get_info().units.end(); ++i) {
			tmp[ i->second.side()-1 ].push_back( variant(new unit_callable(*i)) );
		}
		for( size_t i = 0; i<tmp.size(); ++i)
			vars.push_back( variant( &tmp[i] ));
		return variant(&vars);

	} else if(key == "my_units")
	{
		std::vector<variant> vars;
		for(unit_map::const_iterator i = get_info().units.begin(); i != get_info().units.end(); ++i) {
			if(i->second.side() == get_side()) {
				vars.push_back(variant(new unit_callable(*i)));
			}
		}
		return variant(&vars);

	} else if(key == "enemy_units")
	{
		std::vector<variant> vars;
		for(unit_map::const_iterator i = get_info().units.begin(); i != get_info().units.end(); ++i) {
			if(current_team().is_enemy(i->second.side())) {
				if (!i->second.incapacitated()) {
					vars.push_back(variant(new unit_callable(*i)));
				}
			}
		}
		return variant(&vars);

	} else if(key == "my_moves")
	{
		prepare_move();
		return variant(new move_map_callable(srcdst_, dstsrc_, get_info().units));

	} else if(key == "my_attacks")
	{
		prepare_move();
		return variant(new attack_map_callable(*this, srcdst_, get_info().units));
	} else if(key == "enemy_moves")
	{
		prepare_move();
		return variant(new move_map_callable(enemy_srcdst_, enemy_dstsrc_, get_info().units));

	} else if(key == "my_leader")
	{
		unit_map::const_iterator i = team_leader(get_side(), get_info().units);
		if(i == get_info().units.end()) {
			return variant();
		}
		return variant(new unit_callable(*i));

	} else if(key == "vars")
	{
		return variant(&vars_);
	} else if(key == "keeps")
	{
		return get_keeps();

	} else if(key == "villages")
	{
		return villages_from_set(get_info().map.villages());

	} else if(key == "villages_of_side")
	{
		std::vector<variant> vars;
		for(size_t i = 0; i<get_info().teams.size(); ++i)
		{
			vars.push_back( variant() );
		}
		for(size_t i = 0; i<vars.size(); ++i)
		{
			vars[i] = villages_from_set(get_info().teams[i].villages());
		}
		return variant(&vars);

	} else if(key == "my_villages")
	{
		return villages_from_set(current_team().villages());

	} else if(key == "enemy_and_unowned_villages")
	{
		return villages_from_set(get_info().map.villages(), &current_team().villages());
	}

	return ai_interface::get_value(key);
}

void formula_ai::get_inputs(std::vector<formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("teams", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("turn", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("time_of_day", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("keeps", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("vars", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("allies", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemies", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_moves", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_moves", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_leader", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_recruits", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("recruits_of_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("units_of_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("villages", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_villages", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("villages_of_side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_and_unowned_villages", FORMULA_READ_ONLY));

	ai_interface::get_inputs(inputs);
}

variant formula_ai::get_keeps() const
{
	if(keeps_cache_.is_null()) {
		std::vector<variant> vars;
		for(size_t x = 0; x != size_t(get_info().map.w()); ++x) {
			for(size_t y = 0; y != size_t(get_info().map.h()); ++y) {
				const map_location loc(x,y);
				if(get_info().map.is_keep(loc)) {
					map_location adj[6];
					get_adjacent_tiles(loc,adj);
					for(size_t n = 0; n != 6; ++n) {
						if(get_info().map.is_castle(adj[n])) {
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

bool formula_ai::can_reach_unit(unit_map::const_unit_iterator unit_A,
		unit_map::const_unit_iterator unit_B) const {
        prepare_move();
	move_map::iterator i;
	std::pair<move_map::iterator,
			  move_map::iterator> unit_moves;

	unit_moves = srcdst_.equal_range(unit_A->first);
	for(i = unit_moves.first; i != unit_moves.second; ++i) {
		map_location diff(((*i).second).vector_difference(unit_B->first));
		if((abs(diff.x) <= 1) && (abs(diff.y) <= 1)) {
			return true;
		}
	}
	return false;
}
