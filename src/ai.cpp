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

ai_interface* create_ai(const std::string& name, ai_interface::info& info)
{
	//to add an AI of your own, put
	//if(name == "my_ai")
	//	return new my_ai(info);
	//at the top of this function

	return new ai(info);
}

ai::ai(ai_interface::info& info)
	   : ai_interface(info), disp_(info.disp), map_(info.map), gameinfo_(info.gameinfo), units_(info.units),
	     teams_(info.teams), team_num_(info.team_num), state_(info.state),
		 consider_combat_(true)
{}

bool ai::recruit(const std::string& usage)
{
	const int min_gold = 0;
	
	log_scope("recruiting troops");
	std::cerr << "recruiting " << usage << "\n";

	std::vector<std::map<std::string,unit_type>::const_iterator> options;

	//record the number of the recruit for replay recording
	std::vector<int> option_numbers;

	//find an available unit that can be recruited, matches the desired
	//usage type, and comes in under budget
	const std::set<std::string>& recruits = current_team().recruits();
	for(std::map<std::string,unit_type>::const_iterator i =
	    gameinfo_.unit_types.begin(); i != gameinfo_.unit_types.end(); ++i) {

		if(i->second.usage() == usage && recruits.count(i->second.name())
		   && current_team().gold() - i->second.cost() > min_gold) {

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
		unit new_unit(&u,team_num_,true);

		//see if we can actually recruit (i.e. have enough room etc)
		if(recruit_unit(map_,team_num_,units_,new_unit,loc,&disp_).empty()) {
			current_team().spend_gold(u.cost());

			//confirm the transaction - i.e. don't undo recruitment
			replay_guard.confirm_transaction();
			return true;
		} else {
			return false;
		}
	}

	return false;
}

team& ai_interface::current_team()
{
	return info_.teams[info_.team_num-1];
}

const team& ai_interface::current_team() const
{
	return info_.teams[info_.team_num-1];
}


gamemap::location ai_interface::move_unit(location from, location to, std::map<location,paths>& possible_moves)
{
	assert(info_.units.find(to) == info_.units.end() || from == to);

	info_.disp.select_hex(from);
	info_.disp.update_display();

	log_scope("move_unit");
	unit_map::iterator u_it = info_.units.find(from);
	if(u_it == info_.units.end()) {
		std::cout << "Could not find unit at " << from.x << ", "
		          << from.y << "\n";
		assert(false);
		return location();
	}

	recorder.add_movement(from,to);

	const bool ignore_zocs = u_it->second.type().is_skirmisher();
	const bool teleport = u_it->second.type().teleports();
	paths current_paths = paths(info_.map,info_.state,info_.gameinfo,info_.units,from,info_.teams,ignore_zocs,teleport);
	paths_wiper wiper(info_.disp);

	if(!info_.disp.fogged(from.x,from.y))
		info_.disp.set_paths(&current_paths);

	info_.disp.scroll_to_tiles(from.x,from.y,to.x,to.y);

	unit current_unit = u_it->second;
	info_.units.erase(u_it);

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

			if(steps.empty() == false) {
				//check if there are any invisible units that we uncover
				for(std::vector<location>::iterator i = steps.begin()+1; i != steps.end(); ++i) {
					location adj[6];
					get_adjacent_tiles(*i,adj);
					size_t n;
					for(n = 0; n != 6; ++n) {

						//see if there is an enemy unit next to this tile. Note that we don't
						//actually have to check if it's invisible, since it being invisible is
						//the only possibility
						const unit_map::const_iterator invisible_unit = info_.units.find(adj[n]);
						if(invisible_unit != info_.units.end() && current_team().is_enemy(invisible_unit->second.side())) {
							to = *i;
							steps.erase(i,steps.end());
							break;
						}
					}

					if(n != 6) {
						break;
					}
				}
			}

			steps.push_back(to); //add the destination to the steps
			info_.disp.move_unit(steps,current_unit);
		}
	}

	current_unit.set_movement(0);
	info_.units.insert(std::pair<location,unit>(to,current_unit));
	if(info_.map.underlying_terrain(info_.map[to.x][to.y]) == gamemap::TOWER)
		get_tower(to,info_.teams,info_.team_num-1,info_.units);

	info_.disp.draw_tile(to.x,to.y);
	info_.disp.draw();

	game_events::fire("moveto",to);

	if((info_.teams.front().uses_fog() || info_.teams.front().uses_shroud()) && !info_.teams.front().fogged(to.x,to.y)) {
		game_events::fire("sighted",to);
	}

	return to;
}

void ai_interface::calculate_possible_moves(std::map<location,paths>& res, move_map& srcdst, move_map& dstsrc, bool enemy, bool assume_full_movement)
{
	for(std::map<gamemap::location,unit>::iterator un_it = info_.units.begin(); un_it != info_.units.end(); ++un_it) {
		//if we are looking for the movement of enemies, then this unit must be an enemy unit
		//if we are looking for movement of our own units, it must be on our side.
		//if we are assuming full movement, then it may be a unit on our side, or allied
		if(enemy && current_team().is_enemy(un_it->second.side()) == false ||
		   !enemy && !assume_full_movement && un_it->second.side() != info_.team_num ||
		   !enemy && assume_full_movement && current_team().is_enemy(un_it->second.side())) {
			continue;
		}

		//discount our own leader, and our units that have been turned to stone
		if(!enemy && (un_it->second.can_recruit() || un_it->second.stone())) {
			continue;
		}

		//we can't see where invisible enemy units might move
		if(enemy &&
		   un_it->second.invisible(info_.map.underlying_terrain(info_.map.get_terrain(un_it->first)), 
		   info_.state.get_time_of_day().lawful_bonus,un_it->first,info_.units,info_.teams)) {
			continue;
		}

		//if it's an enemy unit, reset its moves while we do the calculations
		const unit_movement_resetter move_resetter(un_it->second,enemy || assume_full_movement);

		//insert the trivial moves of staying on the same location
		if(un_it->second.movement_left() == un_it->second.total_movement()) {
			std::pair<location,location> trivial_mv(un_it->first,un_it->first);
			srcdst.insert(trivial_mv);
			dstsrc.insert(trivial_mv);
		}

		const bool ignore_zocs = un_it->second.type().is_skirmisher();
		const bool teleports = un_it->second.type().teleports();
		res.insert(std::pair<gamemap::location,paths>(
		                un_it->first,paths(info_.map,info_.state,info_.gameinfo,info_.units,
		                un_it->first,info_.teams,ignore_zocs,teleports)));
	}

	for(std::map<location,paths>::iterator m = res.begin(); m != res.end(); ++m) {
		for(paths::routes_map::iterator rtit =
		    m->second.routes.begin(); rtit != m->second.routes.end(); ++rtit) {
			const location& src = m->first;
			const location& dst = rtit->first;

			if(src != dst && info_.units.find(dst) == info_.units.end()) {
				srcdst.insert(std::pair<location,location>(src,dst));
				dstsrc.insert(std::pair<location,location>(dst,src));
			}
		}
	}
}

void ai::play_turn()
{
	consider_combat_ = true;
	do_move();
}

void ai::do_move()
{
	log_scope("doing ai move");

	typedef paths::route route;

	typedef std::map<location,paths> moves_map;
	moves_map possible_moves, enemy_possible_moves;

	move_map srcdst, dstsrc, enemy_srcdst, enemy_dstsrc;

	calculate_possible_moves(possible_moves,srcdst,dstsrc,false);
	calculate_possible_moves(enemy_possible_moves,enemy_srcdst,enemy_dstsrc,true);

	unit_map::iterator leader = find_leader(units_,team_num_);

	int ticks = SDL_GetTicks();

	std::vector<attack_analysis> analysis;

	if(consider_combat_) {
		std::cerr << "combat...\n";
		consider_combat_ = do_combat(possible_moves,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc);
		if(consider_combat_) {
			do_move();
			return;
		}
	}

	std::cerr << "villages...\n";
	const bool got_village = get_villages(possible_moves,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc,leader);
	if(got_village) {
		do_move();
		return;
	}

	std::cerr << "healing...\n";
	const bool healed_unit = get_healing(possible_moves,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc);
	if(healed_unit) {
		do_move();
		return;
	}

	std::cerr << "retreating...\n";
	const bool retreated_unit = retreat_units(possible_moves,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc,leader);
	if(retreated_unit) {
		do_move();
		return;
	}

	const bool met_invisible_unit = move_to_targets(possible_moves,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc,leader);
	if(met_invisible_unit) {
		do_move();
		return;
	}

	//recruitment phase and leader movement phase
	if(leader != units_.end()) {
		move_leader_to_keep(enemy_dstsrc);
		do_recruitment();
		move_leader_after_recruit(enemy_dstsrc);
	}

	recorder.end_turn();
}

bool ai::do_combat(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc)
{
	int ticks = SDL_GetTicks();

	std::vector<attack_analysis> analysis = analyze_targets(srcdst,dstsrc,enemy_srcdst,enemy_dstsrc);

	int time_taken = SDL_GetTicks() - ticks;
	std::cout << "took " << time_taken << " ticks for " << analysis.size() << " positions. Analyzing...\n";

	ticks = SDL_GetTicks();

	const int max_sims = 50000;
	int num_sims = analysis.empty() ? 0 : max_sims/analysis.size();
	if(num_sims < 20)
		num_sims = 20;
	if(num_sims > 40)
		num_sims = 40;

	std::cout << "simulations: " << num_sims << "\n";

	const int max_positions = 30000;
	const int skip_num = analysis.size()/max_positions;

	std::vector<attack_analysis>::iterator choice_it = analysis.end();
	double choice_rating = -1000.0;
	for(std::vector<attack_analysis>::iterator it = analysis.begin(); it != analysis.end(); ++it) {
		if(skip_num > 0 && ((it - analysis.begin())%skip_num) && it->movements.size() > 1)
			continue;

		const double rating = it->rating(current_team().aggression());
		std::cout << "attack option rated at " << rating << " (" << current_team().aggression() << ")\n";
		if(rating > choice_rating) {
			choice_it = it;
			choice_rating = rating;
		}
	}

	time_taken = SDL_GetTicks() - ticks;
	std::cout << "analysis took " << time_taken << " ticks\n";

	if(choice_rating > 0.0) {
		location from   = choice_it->movements[0].first;
		location to     = choice_it->movements[0].second;
		location target_loc = choice_it->target;
		const int weapon = choice_it->weapons[0];

		const unit_map::const_iterator tgt = units_.find(target_loc);

		const bool defender_human = (tgt != units_.end()) ?
		             teams_[tgt->second.side()-1].is_human() : false;

		const location arrived_at = move_unit(from,to,possible_moves);
		if(arrived_at != to) {
			std::cerr << "unit moving to attack has ended up unexpectedly at " << (arrived_at.x+1) << "," << (arrived_at.y+1) << " when moving to "
			          << (to.x+1) << "," << (to.y+1) << " moved from " << (from.x+1) << "," << (from.y+1) << "\n";
			return true;
		}

		attack_enemy(to,target_loc,weapon);

		//if this is the only unit in the planned attack, and the target
		//is still alive, then also summon reinforcements
		if(choice_it->movements.size() == 1 && units_.count(target_loc)) {
			additional_targets_.push_back(target(target_loc,3.0));
		}

		dialogs::advance_unit(gameinfo_,units_,to,disp_,true);
		dialogs::advance_unit(gameinfo_,units_,target_loc,disp_,!defender_human);

		return true;

	} else {
		log_scope("summoning reinforcements...\n");

		std::set<gamemap::location> already_done;

		for(std::vector<attack_analysis>::iterator it = analysis.begin(); it != analysis.end(); ++it) {
			assert(it->movements.empty() == false);
			const gamemap::location& loc = it->movements.front().first;
			if(already_done.count(loc) > 0)
				continue;

			additional_targets_.push_back(target(loc,3.0));
			already_done.insert(loc);
		}

		return false;
	}
}

void ai_interface::attack_enemy(const location& u, const location& target, int weapon)
{
	recorder.add_attack(u,target,weapon);

	game_events::fire("attack",u,target);
	if(info_.units.count(u) && info_.units.count(target)) {
		attack(info_.disp,info_.map,info_.teams,u,target,weapon,info_.units,info_.state,info_.gameinfo,false);
		check_victory(info_.units,info_.teams);
	}
}

bool ai::get_villages(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc, unit_map::const_iterator leader)
{
	//try to acquire towers
	for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
		if(map_.underlying_terrain(map_[i->first.x][i->first.y]) != gamemap::TOWER)
			continue;

		bool want_tower = true, owned = false;
		for(size_t j = 0; j != teams_.size(); ++j) {
			owned = teams_[j].owns_tower(i->first);
			if(owned && !current_team().is_enemy(j+1)) {
				want_tower = false;
			}
			
			if(owned) {
				break;
			}
		}

		//if it's a neutral tower, and we have no leader, then the village is no use to us,
		//and we don't want it.
		if(!owned && leader == units_.end())
			want_tower = false;

		if(want_tower) {
			std::cerr << "trying to acquire village: " << i->first.x
			          << ", " << i->first.y << "\n";

			const std::map<location,unit>::iterator un = units_.find(i->second);
			if(un == units_.end()) {
				assert(false);
				return false;
			}

			if(un->second.is_guardian())
				continue;

			move_unit(i->second,i->first,possible_moves);
			return true;
		}
	}

	return false;
}

bool ai::get_healing(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc)
{
	//find units in need of healing
	unit_map::iterator u_it = units_.begin();
	for(; u_it != units_.end(); ++u_it) {
		unit& u = u_it->second;

		//if the unit is on our side, has lost as many or more than 1/2 round
		//worth of healing, and doesn't regenerate itself, then try to
		//find a vacant village for it to rest in
		if(u.side() == team_num_ &&
		   u.type().hitpoints() - u.hitpoints() >= game_config::cure_amount/2 &&
		   !u.type().regenerates()) {

			//look for the village which is the least vulnerable to enemy attack
			typedef std::multimap<location,location>::const_iterator Itor;
			std::pair<Itor,Itor> it = srcdst.equal_range(u_it->first);
			double best_vulnerability = 100000.0;
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const location& dst = it.first->second;
				if(map_.underlying_terrain(map_[dst.x][dst.y]) == gamemap::TOWER &&
				   units_.find(dst) == units_.end()) {
					const double vuln = power_projection(it.first->first,
					                    enemy_srcdst,enemy_dstsrc);
					std::cerr << "found village with vulnerability: " << vuln << "\n";
					if(vuln < best_vulnerability || best_loc == it.second) {
						best_vulnerability = vuln;
						best_loc = it.first;
						std::cerr << "chose village " << (dst.x+1) << "," << (dst.y+1) << "\n";
					}
				}
				
				++it.first;
			}

			//if we have found an eligible village
			if(best_loc != it.second) {
				const location& src = best_loc->first;
				const location& dst = best_loc->second;

				std::cerr << "moving unit to village for healing...\n";

				move_unit(src,dst,possible_moves);
				return true;
			}
		}
	}

	return false;
}

bool ai::should_retreat(const gamemap::location& loc, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc) const
{
	const double caution = current_team().caution();
	if(caution <= 0.0) {
		return false;
	}

	const double our_power = power_projection(loc,srcdst,dstsrc);
	const double their_power = power_projection(loc,enemy_srcdst,enemy_dstsrc);
	return caution*their_power > 2.0*our_power + our_power*current_team().aggression();
}

bool ai::retreat_units(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc, unit_map::const_iterator leader)
{
	//get versions of the move map that assume that all units are at full movement
	std::map<gamemap::location,paths> dummy_possible_moves;
	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves,fullmove_srcdst,fullmove_dstsrc,false,true);

	gamemap::location leader_adj[6];
	if(leader != units_.end()) {
		get_adjacent_tiles(leader->first,leader_adj);
	}

	for(unit_map::iterator i = units_.begin(); i != units_.end(); ++i) {
		if(i->second.side() == team_num_ && i->second.movement_left() == i->second.total_movement() && unit_map::const_iterator(i) != leader) {

			//this unit still has movement left, and is a candidate to retreat. We see the amount
			//of power of each side on the situation, and decide whether it should retreat.
			if(should_retreat(i->first,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc)) {

				bool can_reach_leader = false;

				//time to retreat. Look for the place where the power balance is most in our favor.
				//If we can't find anywhere where we like the power balance, just try to
				//get to the best defensive hex
				typedef move_map::const_iterator Itor;
				std::pair<Itor,Itor> itors = srcdst.equal_range(i->first);
				gamemap::location best_pos, best_defensive;
				double best_rating = 0.0;
				int best_defensive_rating = 100;
				while(itors.first != itors.second) {

					if(leader != units_.end() && std::count(leader_adj,leader_adj+6,itors.first->second)) {
						can_reach_leader = true;
						break;
					}

					//we rate the power balance of a hex based on our power projection compared
					//to theirs, multiplying their power projection by their chance to hit us
					//on the hex we're planning to flee to.
					const gamemap::location& hex = itors.first->second;
					const int defense = i->second.type().movement_type().defense_modifier(map_,map_.get_terrain(hex));
					const double our_power = power_projection(hex,srcdst,dstsrc);
					const double their_power = power_projection(hex,enemy_srcdst,enemy_dstsrc) * double(defense)/100.0;
					const double rating = our_power - their_power;
					if(rating > best_rating) {
						best_pos = hex;
						best_rating = rating;
					}

					//give a bonus for getting to a village.
					const int modified_defense = defense - (map_.underlying_terrain(map_.get_terrain(hex)) == gamemap::TOWER ? 10 : 0);

					if(modified_defense < best_defensive_rating) {
						best_defensive_rating = modified_defense;
						best_defensive = hex;
					}

					++itors.first;
				}

				//if the unit is in range of its leader, it should never retreat --
				//it has to defend the leader instead
				if(can_reach_leader) {
					continue;
				}

				if(!best_pos.valid()) {
					best_pos = best_defensive;
				}

				if(best_pos.valid()) {
					std::cerr << "retreating '" << i->second.type().name() << "' " << i->first.x << "," << i->first.y << " -> " << best_pos.x << "," << best_pos.y << "\n";
					move_unit(i->first,best_pos,possible_moves);
					return true;
				}
			}
		}
	}

	return false;
}

bool ai::move_to_targets(std::map<gamemap::location,paths>& possible_moves, move_map& srcdst, move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc, unit_map::const_iterator leader)
{
	std::cerr << "finding targets...\n";
	std::vector<target> targets;
	for(;;) {
		if(targets.empty()) {
			targets = find_targets(leader,enemy_srcdst,enemy_dstsrc);
			targets.insert(targets.end(),additional_targets_.begin(),
			                             additional_targets_.end());
			if(targets.empty()) {
				return false;
			}
		}

		std::cerr << "choosing move...\n";
		std::pair<location,location> move = choose_move(targets,dstsrc,enemy_srcdst,enemy_dstsrc);
		for(std::vector<target>::const_iterator ittg = targets.begin(); ittg != targets.end(); ++ittg) {
			assert(map_.on_board(ittg->loc));
		}

		if(move.first.valid() == false)
			break;

		std::cerr << "move: " << move.first.x << ", " << move.first.y << " - " << move.second.x << ", " << move.second.y << "\n";

		//search to see if there are any enemy units next
		//to the tile which really should be attacked once the move is done
		gamemap::location adj[6];
		get_adjacent_tiles(move.second,adj);
		battle_stats bat_stats;
		gamemap::location target;
		int weapon = -1;
		for(int n = 0; n != 6; ++n) {
			const unit_map::iterator enemy = find_visible_unit(units_,adj[n],
					map_,
					state_.get_time_of_day().lawful_bonus,
					teams_,current_team());

			if(enemy != units_.end() &&
			   current_team().is_enemy(enemy->second.side())) {
				target = adj[n];
				weapon = choose_weapon(move.first,target,bat_stats,
				                       map_[move.second.x][move.second.y]);
				break;
			}
		}

		const location arrived_at = move_unit(move.first,move.second,possible_moves);

		//we didn't arrive at our intended destination. We return true, meaning that
		//the AI algorithm should be recalculated from the start.
		if(arrived_at != move.second) {
			return true;
		}

		//if we're going to attack someone
		if(weapon != -1) {

			const location& attacker = move.second;
				
			recorder.add_attack(attacker,target,weapon);

			game_events::fire("attack",attacker,target);
			if(units_.count(attacker) && units_.count(target)) {
				attack(disp_,map_,teams_,attacker,target,weapon,units_,state_,gameinfo_,false);

				const std::map<gamemap::location,unit>::const_iterator tgt = units_.find(target);

				const bool defender_human = (tgt != units_.end()) ?
		             teams_[tgt->second.side()-1].is_human() : false;

				dialogs::advance_unit(gameinfo_,units_,attacker,disp_,true);
				dialogs::advance_unit(gameinfo_,units_,target,disp_,!defender_human);

				check_victory(units_,teams_);
			}
			
		}

		//don't allow any other units to move onto the tile our unit
		//just moved onto
		typedef std::multimap<location,location>::iterator Itor;
		std::pair<Itor,Itor> del = dstsrc.equal_range(arrived_at);
		dstsrc.erase(del.first,del.second);
	}

	return false;
}

void ai::do_recruitment()
{
	//currently just spend all the gold we can!
	const int min_gold = 0;

	const int towers = map_.towers().size();
	int taken_towers = 0;
	for(size_t j = 0; j != teams_.size(); ++j) {
		taken_towers += teams_[j].towers().size();
	}

	const int neutral_towers = towers - taken_towers;

	//get scouts depending on how many neutral villages there are
	int scouts_wanted = current_team().villages_per_scout() > 0 ?
	                neutral_towers/current_team().villages_per_scout() : 0;

	std::map<std::string,int> unit_types;
	while(unit_types["scout"] < scouts_wanted) {
		if(recruit("scout") == false)
			break;

		++unit_types["scout"];
	}

	const std::vector<std::string>& options = current_team().recruitment_pattern();

	if(options.empty()) {
		assert(false);
		return;
	}

	//buy units as long as we have room and can afford it
	while(recruit(options[rand()%options.size()])) {
	}
}

void ai::move_leader_to_keep(const move_map& enemy_dstsrc)
{
	const unit_map::iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end())
		return;

	//find where the leader can move
	const paths leader_paths(map_,state_,gameinfo_,units_,leader->first,teams_,false,false);
	const gamemap::location& start_pos = map_.starting_position(leader->second.side());

	std::map<gamemap::location,paths> possible_moves;
	possible_moves.insert(std::pair<gamemap::location,paths>(leader->first,leader_paths));

	bool leader_moved = false;
	//if the leader is not on his starting location, move him there.
	if(leader->first != start_pos) {
		leader_moved = true;
		const paths::routes_map::const_iterator itor = leader_paths.routes.find(start_pos);
		if(itor != leader_paths.routes.end() && units_.count(start_pos) == 0) {
			move_unit(leader->first,start_pos,possible_moves);
		}
	}
}

void ai::leader_attack()
{
	std::cerr << "leader attack analysis...\n";
	const unit_map::iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end())
		return;

	gamemap::location choice;
	double rating = 0.0;
	int weapon = -1;

	gamemap::location adj[6];
	get_adjacent_tiles(leader->first,adj);
	for(size_t n = 0; n != 6; ++n) {
		const unit_map::const_iterator u = units_.find(adj[n]);
		if(u != units_.end() && current_team().is_enemy(u->second.side())) {
			attack_analysis analysis;
			analysis.target = adj[n];
			analysis.movements.push_back(std::pair<location,location>(leader->first,leader->first));
			analysis.analyze(map_,units_,state_,gameinfo_,20,*this);
			const double value = analysis.chance_to_kill*analysis.target_value - analysis.avg_losses*10.0 - analysis.avg_damage_taken;
			if(value >= rating) {
				rating = value;
				choice = adj[n];

				assert(analysis.weapons.size() == 1);
				weapon = analysis.weapons.front();
			}
		}
	}

	if(choice.valid()) {
		attack_enemy(leader->first,choice,weapon);
	}

	std::cerr << "end leader attack analysis...\n";
}

void ai::move_leader_after_recruit(const move_map& enemy_dstsrc)
{
	std::cerr << "moving leader after recruit...\n";
	leader_attack();

	const unit_map::iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end())
		return;

	const paths leader_paths(map_,state_,gameinfo_,units_,leader->first,teams_,false,false);

	std::map<gamemap::location,paths> possible_moves;
	possible_moves.insert(std::pair<gamemap::location,paths>(leader->first,leader_paths));

	//see if the leader can capture a village safely
	//if the keep is accessible by an enemy unit, we don't want to leave it
	gamemap::location adj[6];
	get_adjacent_tiles(leader->first,adj);
	for(size_t n = 0; n != 6; ++n) {
		if(enemy_dstsrc.count(adj[n])) {
			return;
		}
	}

	//search through villages finding one to capture
	const std::vector<gamemap::location>& villages = map_.towers();
	for(std::vector<gamemap::location>::const_iterator v = villages.begin();
	    v != villages.end(); ++v) {
		const paths::routes_map::const_iterator itor = leader_paths.routes.find(*v);
		if(itor == leader_paths.routes.end() || units_.count(*v) != 0) {
			continue;
		}

		const int owner = tower_owner(*v,teams_);
		if(owner == -1 || current_team().is_enemy(owner+1) || leader->second.hitpoints() < leader->second.max_hitpoints()) {

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

			move_unit(leader->first,*v,possible_moves);

			break;
		}
	}
}