/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "actions.hpp"
#include "ai.hpp"
#include "ai_attack.hpp"
#include "ai_move.hpp"
#include "dialogs.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"

#include <iostream>

namespace {

bool recruit(const gamemap& map, const gamemap::location& leader,
             const std::string& usage, const game_data& gameinfo,
			 int team_num, team& tm, int min_gold,
			 std::map<gamemap::location,unit>& units, display& disp)
{
	log_scope("recruiting troops");
	std::cerr << "recruiting " << usage << "\n";

	std::vector<std::map<std::string,unit_type>::const_iterator> options;

	//record the number of the recruit for replay recording
	std::vector<int> option_numbers;

	//find an available unit that can be recruited, matches the desired
	//usage type, and comes in under budget
	const std::set<std::string>& recruits = tm.recruits();
	for(std::map<std::string,unit_type>::const_iterator i =
	    gameinfo.unit_types.begin(); i != gameinfo.unit_types.end(); ++i) {

		if(i->second.usage() == usage && recruits.count(i->second.name())
		   && tm.gold() - i->second.cost() > min_gold) {

			options.push_back(i);
			option_numbers.push_back(std::distance(recruits.begin(),
			                                       recruits.find(i->first)));
		}
	}

	//from the available options, choose one at random
	if(options.empty() == false) {

		const gamemap::location loc = gamemap::location::null_location;
		const int option = rand()%options.size();

		//add the recruit conditionally. If recruiting fails,
		//we will undo it.
		recorder.add_recruit(option_numbers[option],loc);
		replay_undo replay_guard(recorder);

		const unit_type& u = options[option]->second;
		unit new_unit(&u,team_num,true);

		//see if we can actually recruit (i.e. have enough room etc)
		if(recruit_unit(map,team_num,units,new_unit,loc,&disp).empty()) {
			std::cerr << "recruiting a " << u.name() << " for " << u.cost() << " have " << tm.gold() << " left\n";

			tm.spend_gold(u.cost());

			//confirm the transaction - i.e. don't undo recruitment
			replay_guard.confirm_transaction();
			return true;
		} else {
			return false;
		}
	}

	return false;
}

}

namespace ai {

void move_unit(const game_data& gameinfo, display& disp,
               const gamemap& map,
               std::map<gamemap::location,unit>& units,
			   const location& from, const location& to,
			   std::map<location,paths>& possible_moves,
			   std::vector<team>& teams, int team_num)
{
	assert(units.find(to) == units.end() || from == to);

	disp.select_hex(from);
	disp.update_display();

	log_scope("move_unit");
	std::map<location,unit>::iterator u_it = units.find(from);
	if(u_it == units.end()) {
		std::cout << "Could not find unit at " << from.x << ", "
		          << from.y << "\n";
		assert(false);
		return;
	}

	recorder.add_movement(from,to);

	const bool ignore_zocs = u_it->second.type().is_skirmisher();
	const bool teleport = u_it->second.type().teleports();
	paths current_paths = paths(map,gameinfo,units,from,teams,
	                            ignore_zocs,teleport);
	paths_wiper wiper(disp);

	if(!disp.fogged(from.x,from.y))
		disp.set_paths(&current_paths);

	disp.scroll_to_tiles(from.x,from.y,to.x,to.y);

	unit current_unit = u_it->second;
	units.erase(u_it);

	const std::map<location,paths>::iterator p_it = possible_moves.find(from);

	if(p_it != possible_moves.end()) {
		paths& p = p_it->second;
		std::map<location,paths::route>::iterator rt = p.routes.begin();
		for(; rt != p.routes.end(); ++rt) {
			if(rt->first == to) {
				break;
			}
		}

		if(rt != p.routes.end()) {
			std::vector<location> steps = rt->second.steps;
			steps.push_back(to); //add the destination to the steps
			disp.move_unit(steps,current_unit);
		}
	}

	current_unit.set_movement(0);
	units.insert(std::pair<location,unit>(to,current_unit));
	if(map.underlying_terrain(map[to.x][to.y]) == gamemap::TOWER)
		get_tower(to,teams,team_num-1,units);

	disp.draw_tile(to.x,to.y);
	disp.draw();

	game_events::fire("moveto",to);

	if((teams.front().uses_fog() || teams.front().uses_shroud()) && !teams.front().fogged(to.x,to.y)) {
		game_events::fire("sighted",to);
	}
}

void do_move(display& disp, const gamemap& map, const game_data& gameinfo,
             unit_map& units,
             std::vector<team>& teams, int team_num, const gamestatus& state,
             bool consider_combat, std::vector<target>* additional_targets)
{
	std::vector<target> tgts;
	if(additional_targets == NULL)
		additional_targets = &tgts;

	log_scope("doing ai move");

	team& current_team = teams[team_num-1];

	typedef paths::route route;

	std::multimap<location,location> enemy_srcdst;
	std::multimap<location,location> enemy_dstsrc;

	std::multimap<location,location> srcdst;
	std::multimap<location,location> dstsrc;

	std::vector<gamemap::location> leader_locations;

	typedef std::map<location,paths> moves_map;
	moves_map possible_moves;

	for(std::map<gamemap::location,unit>::iterator un_it = units.begin();
	    un_it != units.end(); ++un_it) {

		if(current_team.is_enemy(un_it->second.side())) {
			std::pair<location,location> trivial_mv(un_it->first,un_it->first);
			enemy_srcdst.insert(trivial_mv);
			enemy_dstsrc.insert(trivial_mv);

			const unit_movement_resetter resetter(un_it->second);

			const bool ignore_zocs = un_it->second.type().is_skirmisher();
			const bool teleports = un_it->second.type().teleports();
			const paths new_paths(map,gameinfo,units,
			                      un_it->first,teams,ignore_zocs,teleports);
			for(paths::routes_map::const_iterator rt = new_paths.routes.begin();
			    rt != new_paths.routes.end(); ++rt) {
				const std::pair<location,location> item(un_it->first,rt->first);
				const std::pair<location,location> item_reverse(rt->first,un_it->first);
				enemy_srcdst.insert(item);
				enemy_dstsrc.insert(item_reverse);
			}
		}
		
		if(un_it->second.side() != team_num) {
			continue;
		}

		//insert the trivial moves of staying on the same location
		if(un_it->second.movement_left() == un_it->second.total_movement()) {
			std::pair<location,location> trivial_mv(un_it->first,un_it->first);
			srcdst.insert(trivial_mv);
			dstsrc.insert(trivial_mv);
		}

		if(un_it->second.can_recruit()) {
			//save so we can remove from possible moves later
			leader_locations.push_back(un_it->first);
			continue;
		}

		const bool ignore_zocs = un_it->second.type().is_skirmisher();
		const bool teleports = un_it->second.type().teleports();
		possible_moves.insert(std::pair<gamemap::location,paths>(
		                un_it->first,paths(map,gameinfo,units,
		                un_it->first,teams,ignore_zocs,teleports)));
	}


	for(moves_map::iterator m = possible_moves.begin();
	    m != possible_moves.end(); ++m) {
		for(paths::routes_map::iterator rtit =
		    m->second.routes.begin(); rtit != m->second.routes.end();
			++rtit) {
			const location& src = m->first;
			const location& dst = rtit->first;

			if(src != dst && units.find(dst) == units.end()) {
				srcdst.insert(std::pair<location,location>(src,dst));
				dstsrc.insert(std::pair<location,location>(dst,src));
			}
		}
	}

	unit_map::iterator leader = find_leader(units,team_num);

	//no moves left, recruitment phase and leader movement phase
	//take stock of our current set of units
	if(srcdst.empty() || leader != units.end() && srcdst.count(leader->first) == srcdst.size()) {
		if(leader == units.end()) {
			recorder.end_turn();
			return;
		}

		//find where the leader can move
		const paths leader_paths(map,gameinfo,units,leader->first,teams,false,false);
		const gamemap::location& start_pos = map.starting_position(leader->second.side());

		possible_moves.insert(std::pair<gamemap::location,paths>(leader->first,leader_paths));

		bool leader_moved = false;
		//if the leader is not on his starting location, move him there.
		if(leader->first != start_pos) {
			leader_moved = true;
			const paths::routes_map::const_iterator itor = leader_paths.routes.find(start_pos);
			if(itor != leader_paths.routes.end() && units.count(start_pos) == 0) {
				move_unit(gameinfo,disp,map,units,leader->first,start_pos,
		                  possible_moves,teams,team_num);

				leader = find_leader(units,team_num);
				assert(leader != units.end());
			}
		}

		std::cout << "recruitment......\n";

		//currently just spend all the gold we can!
		const int min_gold = 0;

		const int towers = map.towers().size();
		int taken_towers = 0;
		for(size_t j = 0; j != teams.size(); ++j) {
			taken_towers += teams[j].towers().size();
		}

		const int neutral_towers = towers - taken_towers;

		//get scouts depending on how many neutral villages there are
		int scouts_wanted = current_team.villages_per_scout() > 0 ?
		                neutral_towers/current_team.villages_per_scout() : 0;

		std::map<std::string,int> unit_types;
		while(unit_types["scout"] < scouts_wanted) {
			if(recruit(map,leader->first,"scout",gameinfo,team_num,current_team,
			           min_gold,units,disp) == false)
				break;

			++unit_types["scout"];
		}

		const std::vector<std::string>& options =
		                        current_team.recruitment_pattern();

		if(options.empty()) {
			assert(false);
			return;
		}

		//buy fighters as long as we have room and can afford it
		while(recruit(map,leader->first,options[rand()%options.size()].c_str(),
		              gameinfo,team_num,current_team,min_gold,units,disp)) {

		}

		//see if the leader can capture a village safely
		if(!leader_moved) {
			//if the keep is accessible by an enemy unit, we don't want to leave it
			gamemap::location adj[6];
			get_adjacent_tiles(leader->first,adj);
			for(size_t n = 0; n != 6; ++n) {
				if(enemy_dstsrc.count(adj[n])) {
					leader_moved = true;
					break;
				}
			}
		}

		//search through villages finding one to capture
		if(!leader_moved) {
			const std::vector<gamemap::location>& villages = map.towers();
			for(std::vector<gamemap::location>::const_iterator v = villages.begin();
			    v != villages.end(); ++v) {
				const paths::routes_map::const_iterator itor = leader_paths.routes.find(*v);
				if(itor == leader_paths.routes.end() || units.count(*v) != 0) {
					continue;
				}

				const int owner = tower_owner(*v,teams);
				if(owner == -1 || current_team.is_enemy(owner+1) || leader->second.hitpoints() < leader->second.max_hitpoints()) {

					//check that no enemies can reach the village
					gamemap::location adj[6];
					get_adjacent_tiles(*v,adj);
					size_t n;
					for(n = 0; n != 6; ++n) {
						if(enemy_dstsrc.count(adj[n]))
							break;
					}

					if(n != 6)
						continue;

					move_unit(gameinfo,disp,map,units,leader->first,*v,
			                  possible_moves,teams,team_num);

					break;
				}
			}
		}

		recorder.end_turn();
		return;
	}

	int ticks = SDL_GetTicks();

	std::vector<attack_analysis> analysis;

	if(consider_combat) {
		analysis = analyze_targets(map,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc,
		                           units,current_team,team_num,state,gameinfo);
	}

	int time_taken = SDL_GetTicks() - ticks;
	std::cout << "took " << time_taken << " ticks for " << analysis.size() << " positions. Analyzing...\n";

	ticks = SDL_GetTicks();

	const int max_sims = 30000;
	int num_sims = analysis.empty() ? 0 : max_sims/analysis.size();
	if(num_sims < 12)
		num_sims = 12;
	if(num_sims > 20)
		num_sims = 20;

	std::cout << "simulations: " << num_sims << "\n";

	const int max_positions = 20000;
	const int skip_num = analysis.size()/max_positions;

	std::vector<attack_analysis>::iterator choice_it = analysis.end();
	double choice_rating = -1000.0;
	for(std::vector<attack_analysis>::iterator it = analysis.begin();
	    it != analysis.end(); ++it) {
		if(skip_num > 0 && ((it - analysis.begin())%skip_num) &&
		   it->movements.size() > 1)
			continue;

		const double rating = it->rating(current_team.aggression());
		std::cout << "attack option rated at " << rating << " (" << current_team.aggression() << ")\n";
		if(rating > choice_rating) {
			choice_it = it;
			choice_rating = rating;
		}
	}

	time_taken = SDL_GetTicks() - ticks;
	std::cout << "analysis took " << time_taken << " ticks\n";

	if(choice_rating > 0.0) {
		const location& from   = choice_it->movements[0].first;
		const location& to     = choice_it->movements[0].second;
		const location& target_loc = choice_it->target;
		const int weapon = choice_it->weapons[0];

		const std::map<gamemap::location,unit>::const_iterator tgt =
		                                              units.find(target_loc);

		const bool defender_human = (tgt != units.end()) ?
		             teams[tgt->second.side()-1].is_human() : false;

		move_unit(gameinfo,disp,map,units,from,to,
		          possible_moves,teams,team_num);

		std::cerr << "attacking...\n";
		recorder.add_attack(to,target_loc,weapon);

		game_events::fire("attack",to,target_loc);
		if(units.count(to) && units.count(target_loc)) {
			attack(disp,map,to,target_loc,weapon,units,state,gameinfo,false);
			check_victory(units,teams);
		}
		std::cerr << "done attacking...\n";

		//if this is the only unit in the planned attack, and the target
		//is still alive, then also summon reinforcements
		if(choice_it->movements.size() == 1 && units.count(target_loc)) {
			additional_targets->push_back(target(target_loc,3.0));
		}

		dialogs::advance_unit(gameinfo,units,to,disp,true);
		dialogs::advance_unit(gameinfo,units,target_loc,disp,!defender_human);

		do_move(disp,map,gameinfo,units,teams,team_num,state,consider_combat,
		        additional_targets);
		return;

	} else {
		log_scope("summoning reinforcements...\n");
		consider_combat = false;

		std::set<gamemap::location> already_done;

		for(std::vector<attack_analysis>::iterator it = analysis.begin();
		    it != analysis.end(); ++it) {
			assert(it->movements.empty() == false);
			const gamemap::location& loc = it->movements.front().first;
			if(already_done.count(loc) > 0)
				continue;

			additional_targets->push_back(target(loc,3.0));
			already_done.insert(loc);
		}
	}

	//mark the leader as having moved and attacked by now
	for(std::vector<location>::const_iterator lead = leader_locations.begin();
	    lead != leader_locations.end(); ++lead) {
		//remove leader from further consideration
		srcdst.erase(*lead);
		dstsrc.erase(*lead);
	}

	//try to acquire towers
	for(std::multimap<location,location>::const_iterator i = dstsrc.begin();
	    i != dstsrc.end(); ++i) {
		if(map.underlying_terrain(map[i->first.x][i->first.y]) != gamemap::TOWER)
			continue;

		bool want_tower = true, owned = false;
		for(size_t j = 0; j != teams.size(); ++j) {
			owned = teams[j].owns_tower(i->first);
			if(owned && !current_team.is_enemy(j+1)) {
				want_tower = false;
			}
			
			if(owned) {
				break;
			}
		}

		//if it's a neutral tower, and we have no leader, then the village is no use to us,
		//and we don't want it.
		if(!owned && leader == units.end())
			want_tower = false;

		if(want_tower) {
			std::cerr << "trying to acquire village: " << i->first.x
			          << ", " << i->first.y << "\n";

			const std::map<location,unit>::iterator un = units.find(i->second);
			if(un == units.end()) {
				assert(false);
				return;
			}

			if(un->second.is_guardian())
				continue;

			move_unit(gameinfo,disp,map,units,i->second,i->first,
			          possible_moves,teams,team_num);

			do_move(disp,map,gameinfo,units,teams,team_num,
			        state,consider_combat,additional_targets);
			return;
		}
	}

	//find units in need of healing
	std::map<location,unit>::iterator u_it = units.begin();
	for(; u_it != units.end(); ++u_it) {
		unit& u = u_it->second;

		//if the unit is on our side, has lost as many or more than 1/2 round
		//worth of healing, and doesn't regenerate itself, then try to
		//find a vacant village for it to rest in
		if(u.side() == team_num &&
		   u.type().hitpoints() - u.hitpoints() >= game_config::cure_amount/2 &&
		   !u.type().regenerates()) {

			//look for the village which is the least vulnerable to enemy attack
			typedef std::multimap<location,location>::iterator Itor;
			std::pair<Itor,Itor> it = srcdst.equal_range(u_it->first);
			double best_vulnerability = 100000.0;
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const location& dst = it.first->second;
				if(map.underlying_terrain(map[dst.x][dst.y]) == gamemap::TOWER &&
				   units.find(dst) == units.end()) {
					const double vuln = power_projection(it.first->first,
					                    enemy_srcdst,enemy_dstsrc,units,map);
					if(vuln < best_vulnerability) {
						best_vulnerability = vuln;
						best_loc = it.first;
					}
				}
				
				++it.first;
			}

			//if we have found an eligible village
			if(best_loc != it.second) {
				const location& src = best_loc->first;
				const location& dst = best_loc->second;

				std::cerr << "moving unit to village for healing...\n";

				move_unit(gameinfo,disp,map,units,src,dst,
				          possible_moves,teams,team_num);
				do_move(disp,map,gameinfo,units,teams,team_num,state,
				        consider_combat,additional_targets);
				return;
			}
		}
	}

	if(dstsrc.empty()) {
		do_move(disp,map,gameinfo,units,teams,team_num,state,
		        consider_combat,additional_targets);
		return;
	}

	std::cout << "finding targets...\n";
	std::vector<target> targets = find_targets(map,units,teams,team_num,leader != units.end());
	targets.insert(targets.end(),additional_targets->begin(),
	                             additional_targets->end());
	for(;;) {
		if(targets.empty()) {
			recorder.end_turn();
			return;
		}

		std::cout << "choosing move...\n";
		std::pair<location,location> move = choose_move(targets,dstsrc,units,
		                                                map,teams,team_num,
														gameinfo);
		for(std::vector<target>::const_iterator ittg = targets.begin();
	    	ittg != targets.end(); ++ittg) {
			assert(map.on_board(ittg->loc));
		}


		if(move.first.valid() == false)
			break;

		std::cout << "move: " << move.first.x << ", " << move.first.y << " - " << move.second.x << ", " << move.second.y << "\n";

		//search to see if there are any enemy units next
		//to the tile which really should be attacked once the move is done
		gamemap::location adj[6];
		get_adjacent_tiles(move.second,adj);
		battle_stats bat_stats;
		gamemap::location target;
		int weapon = -1;
		for(int n = 0; n != 6; ++n) {
			const unit_map::iterator enemy = units.find(adj[n]);
			if(enemy != units.end() &&
			   current_team.is_enemy(enemy->second.side()) &&
			   !enemy->second.invisible(map[enemy->first.x][enemy->first.y])) {
				target = adj[n];
				weapon = choose_weapon(map,units,state,gameinfo,move.first,
				                       target,bat_stats,
				                       map[move.second.x][move.second.y]);
				break;
			}
		}

		move_unit(gameinfo,disp,map,units,move.first,move.second,
		          possible_moves,teams,team_num);

		//if we're going to attack someone
		if(weapon != -1) {

			const location& attacker = move.second;
				
			recorder.add_attack(attacker,target,weapon);

			game_events::fire("attack",attacker,target);
			if(units.count(attacker) && units.count(target)) {
				attack(disp,map,attacker,target,weapon,units,state,
				       gameinfo,false);

				const std::map<gamemap::location,unit>::const_iterator tgt =
		                                              units.find(target);

				const bool defender_human = (tgt != units.end()) ?
		             teams[tgt->second.side()-1].is_human() : false;


				dialogs::advance_unit(gameinfo,units,attacker,disp,true);
				dialogs::advance_unit(gameinfo,units,target,disp,!defender_human);

				check_victory(units,teams);
			}
			
		}

		//don't allow any other units to move onto the tile our unit
		//just moved onto
		typedef std::multimap<location,location>::iterator Itor;
		std::pair<Itor,Itor> del = dstsrc.equal_range(move.second);
		dstsrc.erase(del.first,del.second);
	}

	do_move(disp,map,gameinfo,units,teams,team_num,state,
	        consider_combat,additional_targets);
	return;
}

}
