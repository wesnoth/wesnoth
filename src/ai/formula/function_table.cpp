/*
   Copyright (C) 2009 - 2013 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
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
#include "../../game_display.hpp"
#include "../../log.hpp"
#include "../../map_label.hpp"
#include "../../menu_events.hpp"
#include "../../pathfind/teleport.hpp"
#include "../../replay.hpp"
#include "../../resources.hpp"
#include "../../terrain_filter.hpp"
#include "../../unit.hpp"
#include "../../pathfind/pathfind.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_formula_ai("ai/engine/fai");
#define LOG_AI LOG_STREAM(info, log_formula_ai)
#define WRN_AI LOG_STREAM(warn, log_formula_ai)
#define ERR_AI LOG_STREAM(err, log_formula_ai)

namespace game_logic {
using ai::formula_ai;

namespace {

/*
 * unit adapters let us treat unit and unit_type the same if we want to get access to attacks or movement cost
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

		int movement_cost(const gamemap& map, const t_translation::t_terrain terrain) const {
			if(unit_type_ != NULL) {
				return unit_type_->movement_type().movement_cost(map, terrain);
			} else {
				return unit_->movement_cost(terrain);
			}
		}


	private:
		const unit_type *unit_type_;
		const unit* unit_;
};

class distance_between_function : public function_expression {
public:
	explicit distance_between_function(const args_list& args)
	  : function_expression("distance_between", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc1 = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"distance_between:location_A")))->loc();
		const map_location loc2 = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"distance_between:location_B")))->loc();
		return variant(distance_between(loc1, loc2));
	}
};


class distance_to_nearest_unowned_village_function : public function_expression {
public:
	distance_to_nearest_unowned_village_function(const args_list& args, const formula_ai& ai)
	  : function_expression("distance_to_nearest_unowned_village", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"distance_to_nearest_unowned_village:location")))->loc();
		int best = 1000000;
		const std::vector<map_location>& villages = resources::game_map->villages();
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

static unsigned search_counter;

///@todo 1.8: document
class calculate_map_ownership_function : public function_expression {
public:
	calculate_map_ownership_function(const args_list& args, const formula_ai& ai)
	  : function_expression("calculate_map_ownership", args, 2, 5), ai_(ai) {
	}

private:

	struct indexer {
		int w, h;
		indexer(int a, int b) : w(a), h(b) { }
		int operator()(const map_location& loc) const {
			return loc.y * w + loc.x;
		}
	};

	struct node {
		int movement_cost_;
		map_location loc_;

		/**
		 * If equal to search_counter, the node is off the list.
		 * If equal to search_counter + 1, the node is on the list.
		 * Otherwise it is outdated.
		 */
		unsigned in;

		node(int moves, const map_location &loc)
			: movement_cost_(moves)
			, loc_(loc)
			, in(0)
		{
		}

		node()
			: movement_cost_(0)
			, loc_()
			, in(0)
		{
		}

		bool operator<(const node& o) const {
			return movement_cost_ < o.movement_cost_;
		}
	};

	struct comp {
		const std::vector<node>& nodes;
		comp(const std::vector<node>& n) : nodes(n) { }
		bool operator()(int l, int r) const {
			return nodes[r] < nodes[l];
		}
	};

	void find_movemap(const unit_adapter& u, const map_location& loc,
				std::vector<int>& scores, bool allow_teleport) const
	{
		const std::set<map_location>& teleports = allow_teleport ? ai_.current_team().villages() : std::set<map_location>();

		const gamemap& map = *resources::game_map;

		std::vector<map_location> locs(6 + teleports.size());
		std::copy(teleports.begin(), teleports.end(), locs.begin() + 6);

		search_counter += 2;
		if (search_counter == 0) search_counter = 2;

		static std::vector<node> nodes;
		nodes.resize(map.w() * map.h());

		indexer index(map.w(), map.h());
		comp node_comp(nodes);

		nodes[index(loc)] = node(0, loc);
		std::vector<int> pq;
		pq.push_back(index(loc));
		while (!pq.empty()) {
			node& n = nodes[pq.front()];
			std::pop_heap(pq.begin(), pq.end(), node_comp);
			pq.pop_back();
			n.in = search_counter;

			get_adjacent_tiles(n.loc_, &locs[0]);
			for (int i = teleports.count(n.loc_) ? locs.size() : 6; i-- > 0; ) {
				if (!locs[i].valid(map.w(), map.h())) continue;

				node& next = nodes[index(locs[i])];
				bool next_visited = next.in - search_counter <= 1u;

				// test if the current path to locs[i] is better than this one could possibly be.
				// we do this a couple more times below
				if (next_visited &&  !(n < next) ) continue;
				const int move_cost = u.movement_cost(map, map[locs[i]]);

				node t = node(n.movement_cost_ + move_cost, locs[i]);

				if (next_visited &&  !(t < next) ) continue;

				bool in_list = next.in == search_counter + 1;
				t.in = search_counter + 1;
				next = t;

				// if already in the priority queue then we just update it, else push it.
				if (in_list) {
					std::push_heap(pq.begin(), std::find(pq.begin(), pq.end(), index(locs[i])) + 1, node_comp);
				} else {
					pq.push_back(index(locs[i]));
					std::push_heap(pq.begin(), pq.end(), node_comp);
				}
			}
		}

		for (int x = 0; x < map.w(); ++x) {
			for (int y = 0; y < map.h(); ++y)
			{
				int i = y * map.w() + x;
				const node &n = nodes[i];
				scores[i] = scores[i] + n.movement_cost_;
				//std::cout << x << "," << y << ":" << n.movement_cost << std::endl;
			}
		}
	}

	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		int w = resources::game_map->w();
		int h = resources::game_map->h();

		const variant units_input = args()[0]->evaluate(variables,fdb);
		const variant leaders_input = args()[1]->evaluate(variables,fdb);

		int enemy_tollerancy = 3;
		if( args().size() > 2 )
			enemy_tollerancy = args()[2]->evaluate(variables,fdb).as_int();

		int enemy_border_tollerancy = 5;
		if( args().size() > 3 )
			enemy_border_tollerancy = args()[3]->evaluate(variables,fdb).as_int();

		int ally_tollerancy = 3;
		if( args().size() > 4 )
			ally_tollerancy = args()[4]->evaluate(variables,fdb).as_int();

		if( !units_input.is_list() )
			return variant();

		size_t number_of_teams = units_input.num_elements();

		std::vector< std::vector<int> > scores( number_of_teams );

		for( size_t i = 0; i< number_of_teams; ++i)
			scores[i].resize(w*h);

//		for(unit_map::const_iterator i = resources::units->begin(); i != resources::units->end(); ++i) {
//			unit_counter[i->second.side()-1]++;
//			unit_adapter unit(i->second);
//			find_movemap( *resources::game_map, *resources::units, unit, i->first, scores[i->second.side()-1], ai_.*resources::teams , true );
//		}

		for(size_t side = 0 ; side < units_input.num_elements() ; ++side) {
			if( leaders_input[side].is_empty() )
				continue;

			const map_location loc = convert_variant<location_callable>(leaders_input[side][0])->loc();
			const variant units_of_side = units_input[side];

			for(size_t unit_it = 0 ; unit_it < units_of_side.num_elements() ; ++unit_it) {
				unit_adapter unit(units_of_side[unit_it]);
				find_movemap( unit, loc, scores[side], true );
			}
		}

		size_t index = 0;
		for( std::vector< std::vector<int> >::iterator i = scores.begin() ; i != scores.end() ; ++i) {
			for( std::vector<int>::iterator j = i->begin() ; j != i->end() ; ++j ) {
				if(units_input[index].num_elements() != 0) {
					*j /= units_input[index].num_elements();
				} else {
					*j = 0;
				}
			}

			++index;
		}
		//std::vector<variant> res;
		std::map<variant, variant> res;

		size_t current_side = ai_.get_side() - 1 ;

		std::vector<int> enemies;
		std::vector<int> allies;

		for(size_t side = 0 ; side < units_input.num_elements() ; ++side) {
			if( side == current_side)
				continue;

			if( ai_.current_team().is_enemy(side+1) ) {
				if( !leaders_input[side].is_empty() )
					enemies.push_back(side);
			} else {
				if( !leaders_input[side].is_empty() )
					allies.push_back(side);
			}
		}

		//calculate_map_ownership( recruits_of_side, map(units_of_side, 'units', map( filter(units, leader), loc) ) )
		//map(, debug_label(key,value))
		for (int x = 0; x < w; ++x) {
			for (int y = 0; y < h; ++y)
			{
				int i = y * w + x;
				bool valid = true;
				bool enemy_border = false;

				if( scores[current_side][i] > 98 )
					continue;

				BOOST_FOREACH( int side , enemies) {
					int diff = scores[current_side][i] - scores[side][i];
					if ( diff > enemy_tollerancy) {
						valid = false;
						break;
					} else if( abs(diff) < enemy_border_tollerancy )
						enemy_border = true;
				}

				if( valid ) {
					BOOST_FOREACH( int side , allies) {
						if ( scores[current_side][i] - scores[side][i] > ally_tollerancy ) {
							valid = false;
							break;
						}
					}
				}

				if( valid ) {
					if( enemy_border )
						res.insert( std::pair<variant, variant>(variant(new location_callable(map_location(x, y))), variant(scores[0][i] + 10000) ));
					else
						res.insert( std::pair<variant, variant>(variant(new location_callable(map_location(x, y))), variant(scores[0][i] ) ));
				}
			}
		}
		return variant(&res);
	}

	const formula_ai& ai_;
};


class nearest_loc_function : public function_expression {
public:
	nearest_loc_function(const args_list& args, const formula_ai& ai)
	  : function_expression("nearest_loc", args, 2, 2), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"nearest_loc:location")))->loc();
		variant items = args()[1]->evaluate(variables,add_debug_info(fdb,1,"nearest_loc:locations"));
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"adjacent_locs:location")))->loc();
		map_location adj[6];
		get_adjacent_tiles(loc, adj);

		std::vector<variant> v;
		for(int n = 0; n != 6; ++n) {
                        if (resources::game_map->on_board(adj[n]) )
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables,fdb))->loc();

		int range = args()[1]->evaluate(variables,fdb).as_int();

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
                        if (resources::game_map->on_board(res[n]) )
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const args_list& arguments = args();
		const variant var0 = arguments[0]->evaluate(variables,add_debug_info(fdb,0,"run_file:file"));
		const std::string filename = var0.string_cast();

		//NOTE: get_wml_location also filters file path to ensure it doesn't contain things like "../../top/secret"
		std::string path = get_wml_location(filename);
		if(path.empty()) {
			ERR_AI << "run_file : not found [" << filename <<"]"<< std::endl;
			return variant(); //no suitable file
		}

		std::string formula_string = read_file(path);
		//need to get function_table from somewhere or delegate to someone who has access to it
		formula_ptr parsed_formula = ai_.create_optional_formula(formula_string);
		if(parsed_formula == game_logic::formula_ptr()) {
			ERR_AI << "run_file : unable to create formula"<< std::endl;
			return variant(); //was unable to create a formula from file
		}
		return parsed_formula->evaluate(variables,add_debug_info(fdb,-1,"run_file:formula_from_file"));
	}

	formula_ai& ai_;
};


class castle_locs_function : public function_expression {
public:
	castle_locs_function(const args_list& args, const formula_ai& ai)
	  : function_expression("castle_locs", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location starting_loc = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"castle_locs:location")))->loc();

		//looks like reimplementing a generic graph search algorithm to me
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
                        if (resources::game_map->on_board(adj[n]) && visited_locs.find( adj[n] ) == visited_locs.end() ) {
                            if (resources::game_map->get_terrain_info(adj[n]).is_keep() ||
                                    resources::game_map->get_terrain_info(adj[n]).is_castle() ) {
                                queued_locs.push(adj[n]);
                            }
                        }
                   }
                }

                if ( !resources::game_map->get_terrain_info(starting_loc).is_keep() &&
                     !resources::game_map->get_terrain_info(starting_loc).is_castle() )
                    visited_locs.erase(starting_loc);

                std::vector<variant> res;
                BOOST_FOREACH( const map_location& ml, visited_locs) {
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant u = args()[0]->evaluate(variables,add_debug_info(fdb,0,"timeofday_modifier:unit"));

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
			loc = &convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"timeofday_modifier:location")))->loc();
		}

		if (loc == NULL) {
			loc = &u_call->get_location();
		}

		return variant(combat_modifier(*loc, un.alignment(), un.is_fearless()));
	}

	const formula_ai& ai_;
};


class nearest_keep_function : public function_expression {
public:
	nearest_keep_function(const args_list& args, const formula_ai& ai)
	  : function_expression("nearest_keep", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"nearest_keep:location")))->loc();
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"suitable_keep:location")))->loc();
		const unit_map& units = *resources::units;
		if (units.find(loc) == units.end()){
			return variant();
		}
		const pathfind::paths unit_paths(*resources::game_map, units, loc ,*resources::teams, false, true, ai_.current_team());
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::vector<variant> vars;
		int w,h;

		if(args().size()==1) {
			const gamemap& m = convert_variant<gamemap_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"find_shroud:gamemap")))->get_gamemap();
			w = m.w();
			h = m.h();
		} else {
			w = resources::game_map->w();
			h = resources::game_map->h();
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::vector<variant> vars;
		const map_location loc = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"close_enemies:location")))->loc();
		int range_s = args()[1]->evaluate(variables,add_debug_info(fdb,1,"close_enemies:distance")).as_int();
		if (range_s < 0) {
			WRN_AI << "close_enemies_function: range is negative (" << range_s << ")\n";
			range_s = 0;
		}
		size_t range = static_cast<size_t>(range_s);
		unit_map::const_iterator un = resources::units->begin();
		unit_map::const_iterator end = resources::units->end();
		while (un != end) {
			if (distance_between(loc, un->get_location()) <= range) {
				if (un->side() != ai_.get_side()) {//fixme: ignores allied units
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::vector<variant> vars;
		int weapon;
		if (args().size() > 3) weapon = args()[3]->evaluate(variables,add_debug_info(fdb,3,"calculate_outcome:weapon")).as_int();
		else weapon = -1;

		const unit_map& units = *resources::units;
		map_location attacker_location =
			convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"calculate_outcome:attacker_current_location")))->loc();
		if(units.count(attacker_location) == 0) {
			ERR_AI << "Performing calculate_outcome() with non-existent attacker at (" <<
				attacker_location.x+1 << "," << attacker_location.y+1 << ")\n";
			return variant();
		}

		map_location defender_location =
			convert_variant<location_callable>(args()[2]->evaluate(variables,add_debug_info(fdb,2,"calculate_outcome:defender_location")))->loc();
		if(units.count(defender_location) == 0) {
			ERR_AI << "Performing calculate_outcome() with non-existent defender at (" <<
				defender_location.x+1 << "," << defender_location.y+1 << ")\n";
			return variant();
		}

		battle_context bc(units, convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"calculate_outcome:attacker_attack_location")))->loc(),
			defender_location, weapon, -1, 1.0, NULL, &*units.find(attacker_location));
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant attack = args()[0]->evaluate(variables,add_debug_info(fdb,0,"outcomes:attack"));
		ai::attack_analysis* analysis = convert_variant<ai::attack_analysis>(attack);
		//unit_map units_with_moves(*resources::units);
		//typedef std::pair<map_location, map_location> mv;
		//BOOST_FOREACH(const mv &m, analysis->movements) {
		//	units_with_moves.move(m.first, m.second);
		//}

		std::vector<variant> vars;
		if(analysis->chance_to_kill > 0.0) {
			//unit_map units(units_with_moves);
			//units.erase(analysis->target);
			vars.push_back(variant(new position_callable(/*&units,*/ static_cast<int>(analysis->chance_to_kill*100))));

		}

		if(analysis->chance_to_kill < 1.0) {
			//unit_map units(units_with_moves);
			vars.push_back(variant(new position_callable(/*&units,*/ static_cast<int>(100 - analysis->chance_to_kill*100))));
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};

//class evaluate_for_position_function : public function_expression {
//public:
//	evaluate_for_position_function(const args_list& args, formula_ai& ai)
//	  : function_expression("evaluate_for_position", args, 2, 2), ai_(ai) {
//	}
//
//private:
//	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
//		variant position = args()[0]->evaluate(variables,add_debug_info(fdb,0,"evaluate_for_position:position"));
//              ai_.store_outcome_position(position);
//		position_callable* pos = convert_variant<position_callable>(position);
//		position_callable::swapper swapper(ai_, *pos);
//		return args()[1]->evaluate(variables,add_debug_info(fdb,1,"evaluate_for_position:formula"));
//	}
//
//	formula_ai& ai_;
//};

class get_unit_type_function : public function_expression {
public:
	explicit get_unit_type_function(const args_list& args)
	  : function_expression("get_unit_type", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const std::string type = args()[0]->evaluate(variables,add_debug_info(fdb,0,"get_unit_type:name")).as_string();

		const unit_type *ut = unit_types.find(type);
		if (ut)
			return variant(new unit_type_callable(*ut));

		return variant();
	}
};



class rate_action_function : public function_expression {
public:
	explicit rate_action_function(const args_list& args, const formula_ai &ai)
		: function_expression("rate_action", args, 1, 1), ai_(ai)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant act = args()[0]->evaluate(variables,add_debug_info(fdb,0,"rate_action:action"));
		ai::attack_analysis* analysis = convert_variant<ai::attack_analysis>(act);

		return variant(analysis->rating(ai_.get_aggression(),ai_)*1000,variant::DECIMAL_VARIANT);
	}
	const formula_ai &ai_;
};



class recall_function : public function_expression {
public:
	explicit recall_function(const args_list& args)
	  : function_expression("recall", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const std::string id = args()[0]->evaluate(variables,add_debug_info(fdb,0,"recall:id")).as_string();
		map_location loc;
		if(args().size() >= 2) {
			loc = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"recall:location")))->loc();
		}

		return variant(new recall_callable(loc, id));
	}
};


class recruit_function : public function_expression {
public:
	explicit recruit_function(const args_list& args)
	  : function_expression("recruit", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const std::string type = args()[0]->evaluate(variables,add_debug_info(fdb,0,"recruit:type")).as_string();
		map_location loc;
		if(args().size() >= 2) {
			loc = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"recruit:location")))->loc();
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {

		std::vector<variant> locations;

		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"shortest_path:src")))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"shortest_path:dst")))->loc();
                map_location unit_loc;

                if( src == dst )
                    return variant(&locations);

                if(args().size() > 2)
			unit_loc = convert_variant<location_callable>(args()[2]->evaluate(variables,add_debug_info(fdb,2,"shortest_path:unit_location")))->loc();
                else
                    unit_loc = src;

                unit_map::iterator unit_it = resources::units->find(unit_loc);

		if( unit_it == resources::units->end() ) {
			std::ostringstream str;
			str << "shortest_path function: expected unit at location (" << (unit_loc.x+1) << "," << (unit_loc.y+1) << ")";
			throw formula_error( str.str(), "", "", 0);
		}

		pathfind::teleport_map allowed_teleports = ai_.get_allowed_teleports(unit_it);

		pathfind::plain_route route = ai_.shortest_path_calculator( src, dst, unit_it, allowed_teleports );

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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {

		std::vector<variant> locations;

		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"simplest_path:src")))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"simplest_path:dst")))->loc();
                map_location unit_loc;

                if( src == dst )
                    return variant(&locations);

                if(args().size() > 2)
			unit_loc = convert_variant<location_callable>(args()[2]->evaluate(variables,add_debug_info(fdb,2,"simplest_path:unit_location")))->loc();
                else
                    unit_loc = src;

                unit_map::iterator unit_it = resources::units->find(unit_loc);

		if( unit_it == resources::units->end() ) {
			std::ostringstream str;
			str << "simplest_path function: expected unit at location (" << (unit_loc.x+1) << "," << (unit_loc.y+1) << ")";
			throw formula_error( str.str(), "", "", 0);
		}

		pathfind::teleport_map allowed_teleports = ai_.get_allowed_teleports(unit_it);

		pathfind::emergency_path_calculator em_calc(*unit_it, *resources::game_map);

                pathfind::plain_route route = pathfind::a_star_search(src, dst, 1000.0, &em_calc, resources::game_map->w(), resources::game_map->h(), &allowed_teleports);

                if( route.steps.size() < 2 ) {
                    return variant(&locations);
                }

                for (std::vector<map_location>::const_iterator loc_iter = route.steps.begin() + 1 ; loc_iter !=route.steps.end(); ++loc_iter) {
                    if (unit_it->movement_cost((*resources::game_map)[*loc_iter]) < 99 )
                        locations.push_back( variant( new location_callable(*loc_iter) ));
                    else
                        break;
                }

		return variant(&locations);
	}

	const formula_ai& ai_;
};



class next_hop_function : public function_expression {
public:
	explicit next_hop_function(const args_list& args, const formula_ai& ai)
	  : function_expression("next_hop", args, 2, 3), ai_(ai)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {

		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"next_hop:src")))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"next_hop:dst")))->loc();
                map_location unit_loc;

                if( src == dst )
			return variant();

                if(args().size() > 2)
			unit_loc = convert_variant<location_callable>(args()[2]->evaluate(variables,add_debug_info(fdb,2,"next_hop:unit_location")))->loc();
                else
                    unit_loc = src;

                unit_map::iterator unit_it = resources::units->find(unit_loc);

		if( unit_it == resources::units->end() ) {
			std::ostringstream str;
			str << "next_hop function: expected unit at location (" << (unit_loc.x+1) << "," << (unit_loc.y+1) << ")";
			throw formula_error( str.str(), "", "", 0);
		}

		pathfind::teleport_map allowed_teleports = ai_.get_allowed_teleports(unit_it);

		pathfind::plain_route route = ai_.shortest_path_calculator( src, dst, unit_it, allowed_teleports );

                if( route.steps.size() < 2 ) {
			return variant();
                }

		map_location loc = map_location::null_location;
		const ai::moves_map &possible_moves = ai_.get_possible_moves();
		const ai::moves_map::const_iterator& p_it = possible_moves.find(unit_loc);
		if (p_it==possible_moves.end() ) {
			return variant();
		}

                for (std::vector<map_location>::const_iterator loc_iter = route.steps.begin() + 1 ; loc_iter !=route.steps.end(); ++loc_iter) {

			if (p_it->second.destinations.find(*loc_iter) != p_it->second.destinations.end() ) {
				loc = *loc_iter;
			} else {
				break;
			}
                }
		if (loc==map_location::null_location) {
			return variant();
		}
		return variant(new location_callable(loc));
	}

	const formula_ai& ai_;
};



class move_function : public function_expression {
public:
	explicit move_function(const args_list& args)
	  : function_expression("move", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"move:src")))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"move:dst")))->loc();
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location src = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"move_partial:src")))->loc();
		const map_location dst = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"move_partial:dst")))->loc();
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		return variant(new set_var_callable(args()[0]->evaluate(variables,add_debug_info(fdb,0,"set_var:key")).as_string(), args()[1]->evaluate(variables,add_debug_info(fdb,1,"set_var:value"))));
	}
};


class set_unit_var_function : public function_expression {
public:
	explicit set_unit_var_function(const args_list& args)
	  : function_expression("set_unit_var", args, 3, 3)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		return variant(new set_unit_var_callable(args()[0]->evaluate(variables,add_debug_info(fdb,0,"set_unit_var:key")).as_string(), args()[1]->evaluate(variables,add_debug_info(fdb,1,"set_unit_var:value")), convert_variant<location_callable>(args()[2]->evaluate(variables,add_debug_info(fdb,2,"set_unit_var:unit_location")))->loc()));
	}
};


class fallback_function : public function_expression {
public:
	explicit fallback_function(const args_list& args)
	  : function_expression("fallback", args, 0, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		if( args().size() == 0 )
			return variant(new fallback_callable(""));
		return variant(new fallback_callable(args()[0]->evaluate(variables,add_debug_info(fdb,0,"fallback:name")).as_string()));
	}
};


class attack_function : public function_expression {
public:
	explicit attack_function(const args_list& args, const formula_ai& ai)
	  : function_expression("attack", args, 3, 4),
		ai_(ai)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location move_from = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"attack:move_from")))->loc();
		const map_location src = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"attack:src")))->loc();
		const map_location dst = convert_variant<location_callable>(args()[2]->evaluate(variables,add_debug_info(fdb,2,"attack:dst")))->loc();
		const int weapon = args().size() == 4 ? args()[3]->evaluate(variables,add_debug_info(fdb,3,"attack:weapon")).as_int() : -1;
		if(resources::units->count(move_from) == 0 || resources::units->count(dst) == 0) {
			ERR_AI << "AI ERROR: Formula produced illegal attack: " << move_from << " -> " << src << " -> " << dst << "\n";
			return variant();
		}
		return variant(new attack_callable(move_from, src, dst, weapon));
	}

	const formula_ai& ai_;
};


class safe_call_function : public function_expression {
public:
	explicit safe_call_function(const args_list& args)
	  : function_expression("safe_call", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant main = args()[0]->evaluate(variables,fdb);
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
        variant execute(const formula_callable& variables, formula_debugger *fdb) const {
                const args_list& arguments = args();
                const variant var0 = arguments[0]->evaluate(variables,fdb);
                const variant var1 = arguments[1]->evaluate(variables,fdb);

                const map_location location = convert_variant<location_callable>(var0)->loc();
                std::string text;
		if( var1.is_string() )
			text = var1.as_string();
		else
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

		SDL_Color color = int_to_color(team::get_side_rgb(ai_.get_side()));

		const terrain_label *res;
		res = gui->labels().set_label(location, text, team_name, color);
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const gamemap& m = convert_variant<gamemap_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"is_village:map")))->get_gamemap();

		map_location loc;
		if(args().size() == 2) {
			loc = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"is_village:location")))->loc();
		} else {
			loc = map_location( args()[1]->evaluate(variables,add_debug_info(fdb,1,"is_village:x")).as_int() - 1,
					    args()[2]->evaluate(variables,add_debug_info(fdb,2,"is_village:y")).as_int() - 1 );
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {

		const gamemap& m = convert_variant<gamemap_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"is_unowned_village:map")))->get_gamemap();
		const std::set<map_location>& my_villages = ai_.current_team().villages();

		map_location loc;
		if(args().size() == 2) {
			loc = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"is_unowned_village:location")))->loc();
		} else {
			loc = map_location( args()[1]->evaluate(variables,add_debug_info(fdb,1,"is_unowned_village:x")).as_int() - 1,
					    args()[2]->evaluate(variables,add_debug_info(fdb,2,"is_unowned_village:y")).as_int() - 1 );
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant loc_var = args()[0]->evaluate(variables,add_debug_info(fdb,0,"unit_at:location"));
		if (loc_var.is_null()) {
			return variant();
		}
		const location_callable* loc = convert_variant<location_callable>(loc_var);
		const unit_map::const_iterator i = resources::units->find(loc->loc());
		if(i != resources::units->end()) {
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant res = args()[0]->evaluate(variables,add_debug_info(fdb,0,"unit_moves:unit_location"));
		std::vector<variant> vars;
		if(res.is_null()) {
			return variant(&vars);
		}

		const map_location& loc = convert_variant<location_callable>(res)->loc();
		const ai::move_map& srcdst = ai_.get_srcdst();
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::vector<variant> vars;
		variant dstsrc_var = args()[0]->evaluate(variables,add_debug_info(fdb,0,"units_can_reach:possible_move_list"));
		const ai::move_map& dstsrc = convert_variant<move_map_callable>(dstsrc_var)->dstsrc();
		std::pair<ai::move_map::const_iterator,ai::move_map::const_iterator> range =
			dstsrc.equal_range(convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"units_can_reach:possible_move_list")))->loc());
		while(range.first != range.second) {
			unit_map::const_iterator un = resources::units->find(range.first->second);
			assert(un != resources::units->end());
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant u = args()[0]->evaluate(variables,add_debug_info(fdb,0,"defense_on:unit"));
		variant loc_var = args()[1]->evaluate(variables,add_debug_info(fdb,1,"defense_on:location"));
		if(u.is_null() || loc_var.is_null()) {
			return variant();
		}

		const unit_callable* u_call = try_convert_variant<unit_callable>(u);
		const unit_type_callable* u_type = try_convert_variant<unit_type_callable>(u);
		const map_location& loc = convert_variant<location_callable>(loc_var)->loc();

		if (u_call)
		{
			const unit& un = u_call->get_unit();

                        if( un.total_movement() < un.movement_cost( (*resources::game_map)[loc]) )
                            return variant();

			if(!resources::game_map->on_board(loc)) {
				return variant();
			}

			return variant(100 - un.defense_modifier((*resources::game_map)[loc]));
		}

		if (u_type)
		{
			const unit_type& un = u_type->get_unit_type();

                        if( un.movement() < un.movement_type().movement_cost(*resources::game_map, (*resources::game_map)[loc]) )
                            return variant();

			if(!resources::game_map->on_board(loc)) {
				return variant();
			}

			return variant(100 - un.movement_type().defense_modifier(*resources::game_map, (*resources::game_map)[loc]));
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant u = args()[0]->evaluate(variables,add_debug_info(fdb,0,"chance_to_hit:unit"));
		variant loc_var = args()[1]->evaluate(variables,add_debug_info(fdb,1,"chance_to_hit:location"));
		if(u.is_null() || loc_var.is_null()) {
			return variant();
		}

		const unit_callable* u_call = try_convert_variant<unit_callable>(u);
		const unit_type_callable* u_type = try_convert_variant<unit_type_callable>(u);
		const map_location& loc = convert_variant<location_callable>(loc_var)->loc();

		if (u_call)
		{
			const unit& un = u_call->get_unit();

			if(!resources::game_map->on_board(loc)) {
				return variant();
			}

			return variant(un.defense_modifier((*resources::game_map)[loc]));
		}

		if (u_type)
		{
			const unit_type& un = u_type->get_unit_type();

			if(!resources::game_map->on_board(loc)) {
				return variant();
			}

			return variant(un.movement_type().defense_modifier(*resources::game_map, (*resources::game_map)[loc]));
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant u = args()[0]->evaluate(variables,add_debug_info(fdb,0,"movement_cost:unit"));
		variant loc_var = args()[1]->evaluate(variables,add_debug_info(fdb,0,"movement_cost:location"));
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

			if(!resources::game_map->on_board(loc)) {
				return variant();
			}

			return variant(un.movement_cost((*resources::game_map)[loc]));
		}

		if (u_type)
		{
			const unit_type& un = u_type->get_unit_type();

			if(!resources::game_map->on_board(loc)) {
				return variant();
			}

			return variant(un.movement_type().movement_cost(*resources::game_map, (*resources::game_map)[loc]));
		}

		return variant();
	}

	const formula_ai& ai_;
};

class is_avoided_location_function : public function_expression {
public:
	is_avoided_location_function(const args_list& args, const formula_ai& ai_object)
		: function_expression("is_avoided_location",args, 1, 1), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant res = args()[0]->evaluate(variables,add_debug_info(fdb,0,"is_avoided_location:location"));
		if(res.is_null()) {
			return variant();
		}
		const map_location& loc = convert_variant<location_callable>(res)->loc();
		return variant(ai_.get_avoid().match(loc));
	}

	const formula_ai &ai_;
};

class max_possible_damage_function : public function_expression {
public:
	max_possible_damage_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("max_possible_damage", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant u1 = args()[0]->evaluate(variables,add_debug_info(fdb,0,"max_possible_damage:unit1"));
		variant u2 = args()[1]->evaluate(variables,add_debug_info(fdb,1,"max_possible_damage:unit2"));
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

	std::pair<int, int> best_melee_and_ranged_attacks(unit_adapter attacker, unit_adapter defender) const {
		int highest_melee_damage = 0;
		int highest_ranged_damage = 0;

		std::vector<attack_type> attacks = attacker.attacks();

		BOOST_FOREACH(const attack_type &attack, attacks) {
			const int dmg = round_damage(attack.damage(), defender.damage_from(attack), 100) * attack.num_attacks();
			if (attack.range() == "melee") {
				highest_melee_damage = std::max(highest_melee_damage, dmg);
			} else {
				highest_ranged_damage = std::max(highest_ranged_damage, dmg);
			}
		}

		return std::make_pair(highest_melee_damage, highest_ranged_damage);
	}

	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant u1 = args()[0]->evaluate(variables,add_debug_info(fdb,0,"max_possible_damage_with_retaliation:unit1"));
		variant u2 = args()[1]->evaluate(variables,add_debug_info(fdb,1,"max_possible_damage_with_retaliation:unit2"));

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
	//} else if(fn == "evaluate_for_position") {
	//	return expression_ptr(new evaluate_for_position_function(args, ai_));
	} else if(fn == "move") {
		return expression_ptr(new move_function(args));
	} else if(fn == "move_partial") {
		return expression_ptr(new move_partial_function(args));
	} else if(fn == "attack") {
		return expression_ptr(new attack_function(args, ai_));
	} else if(fn == "rate_action") {
		return expression_ptr(new rate_action_function(args, ai_));
	} else if(fn == "recall") {
		return expression_ptr(new recall_function(args));
	} else if(fn == "recruit") {
		return expression_ptr(new recruit_function(args));
	} else if(fn == "safe_call") {
		return expression_ptr(new safe_call_function(args));
	} else if(fn == "get_unit_type") {
		return expression_ptr(new get_unit_type_function(args));
	} else if(fn == "is_avoided_location") {
		return expression_ptr(new is_avoided_location_function(args,ai_));
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
	} else if(fn == "next_hop") {
		return expression_ptr(new next_hop_function(args, ai_));
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
	} else if(fn == "calculate_map_ownership") {
		return expression_ptr(new calculate_map_ownership_function(args, ai_));
	} else {
		return function_symbol_table::create_function(fn, args);
	}
}

}
