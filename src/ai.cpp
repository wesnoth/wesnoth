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

ai::ai(display& disp, const gamemap& map, const game_data& gameinfo,
	   std::map<gamemap::location,unit>& units,
	   std::vector<team>& teams, int team_num, const gamestatus& state)
	   : disp_(disp), map_(map), gameinfo_(gameinfo), units_(units),
	     teams_(teams), team_num_(team_num), state_(state),
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

team& ai::current_team()
{
	return teams_[team_num_-1];
}

void ai::move_unit(const location& from, const location& to, std::map<location,paths>& possible_moves)
{
	assert(units_.find(to) == units_.end() || from == to);

	disp_.select_hex(from);
	disp_.update_display();

	log_scope("move_unit");
	unit_map::iterator u_it = units_.find(from);
	if(u_it == units_.end()) {
		std::cout << "Could not find unit at " << from.x << ", "
		          << from.y << "\n";
		assert(false);
		return;
	}

	recorder.add_movement(from,to);

	const bool ignore_zocs = u_it->second.type().is_skirmisher();
	const bool teleport = u_it->second.type().teleports();
	paths current_paths = paths(map_,state_,gameinfo_,units_,from,teams_,ignore_zocs,teleport);
	paths_wiper wiper(disp_);

	if(!disp_.fogged(from.x,from.y))
		disp_.set_paths(&current_paths);

	disp_.scroll_to_tiles(from.x,from.y,to.x,to.y);

	unit current_unit = u_it->second;
	units_.erase(u_it);

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
			disp_.move_unit(steps,current_unit);
		}
	}

	current_unit.set_movement(0);
	units_.insert(std::pair<location,unit>(to,current_unit));
	if(map_.underlying_terrain(map_[to.x][to.y]) == gamemap::TOWER)
		get_tower(to,teams_,team_num_-1,units_);

	disp_.draw_tile(to.x,to.y);
	disp_.draw();

	game_events::fire("moveto",to);

	if((teams_.front().uses_fog() || teams_.front().uses_shroud()) && !teams_.front().fogged(to.x,to.y)) {
		game_events::fire("sighted",to);
	}
}

void ai::calculate_possible_moves(std::map<location,paths>& res, move_map& srcdst, move_map& dstsrc, bool enemy)
{
	for(std::map<gamemap::location,unit>::iterator un_it = units_.begin(); un_it != units_.end(); ++un_it) {
		if(enemy && current_team().is_enemy(un_it->second.side()) == false ||
		   !enemy && un_it->second.side() != team_num_) {
			continue;
		}

		if(!enemy && (un_it->second.can_recruit() || un_it->second.stone())) {
			continue;
		}

		//if it's an enemy unit, reset its moves while we do the calculations
		const unit_movement_resetter move_resetter(un_it->second,enemy);

		//insert the trivial moves of staying on the same location
		if(un_it->second.movement_left() == un_it->second.total_movement()) {
			std::pair<location,location> trivial_mv(un_it->first,un_it->first);
			srcdst.insert(trivial_mv);
			dstsrc.insert(trivial_mv);
		}

		const bool ignore_zocs = un_it->second.type().is_skirmisher();
		const bool teleports = un_it->second.type().teleports();
		res.insert(std::pair<gamemap::location,paths>(
		                un_it->first,paths(map_,state_,gameinfo_,units_,
		                un_it->first,teams_,ignore_zocs,teleports)));
	}

	for(std::map<location,paths>::iterator m = res.begin(); m != res.end(); ++m) {
		for(paths::routes_map::iterator rtit =
		    m->second.routes.begin(); rtit != m->second.routes.end(); ++rtit) {
			const location& src = m->first;
			const location& dst = rtit->first;

			if(src != dst && units_.find(dst) == units_.end()) {
				srcdst.insert(std::pair<location,location>(src,dst));
				dstsrc.insert(std::pair<location,location>(dst,src));
			}
		}
	}
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

	//no moves left, recruitment phase and leader movement phase
	//take stock of our current set of units
	if(srcdst.empty() || leader != units_.end() && srcdst.count(leader->first) == srcdst.size()) {
		if(leader == units_.end()) {
			recorder.end_turn();
			return;
		}

		move_leader_to_keep(enemy_dstsrc);
		do_recruitment();
		move_leader_after_recruit(enemy_dstsrc);

		recorder.end_turn();
		return;
	}

	int ticks = SDL_GetTicks();

	std::vector<attack_analysis> analysis;

	if(consider_combat_) {
		analysis = analyze_targets(srcdst,dstsrc,enemy_srcdst,enemy_dstsrc);
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
		const location& from   = choice_it->movements[0].first;
		const location& to     = choice_it->movements[0].second;
		const location& target_loc = choice_it->target;
		const int weapon = choice_it->weapons[0];

		const unit_map::const_iterator tgt = units_.find(target_loc);

		const bool defender_human = (tgt != units_.end()) ?
		             teams_[tgt->second.side()-1].is_human() : false;

		move_unit(from,to,possible_moves);

		do_attack(to,target_loc,weapon);

		//if this is the only unit in the planned attack, and the target
		//is still alive, then also summon reinforcements
		if(choice_it->movements.size() == 1 && units_.count(target_loc)) {
			additional_targets_.push_back(target(target_loc,3.0));
		}

		dialogs::advance_unit(gameinfo_,units_,to,disp_,true);
		dialogs::advance_unit(gameinfo_,units_,target_loc,disp_,!defender_human);

		do_move();
		return;

	} else {
		log_scope("summoning reinforcements...\n");
		consider_combat_ = false;

		std::set<gamemap::location> already_done;

		for(std::vector<attack_analysis>::iterator it = analysis.begin(); it != analysis.end(); ++it) {
			assert(it->movements.empty() == false);
			const gamemap::location& loc = it->movements.front().first;
			if(already_done.count(loc) > 0)
				continue;

			additional_targets_.push_back(target(loc,3.0));
			already_done.insert(loc);
		}
	}
	
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
				return;
			}

			if(un->second.is_guardian())
				continue;

			move_unit(i->second,i->first,possible_moves);
			do_move();
			return;
		}
	}

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
			typedef std::multimap<location,location>::iterator Itor;
			std::pair<Itor,Itor> it = srcdst.equal_range(u_it->first);
			double best_vulnerability = 100000.0;
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const location& dst = it.first->second;
				if(map_.underlying_terrain(map_[dst.x][dst.y]) == gamemap::TOWER &&
				   units_.find(dst) == units_.end()) {
					const double vuln = power_projection(it.first->first,
					                    enemy_srcdst,enemy_dstsrc);
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

				move_unit(src,dst,possible_moves);
				do_move();
				return;
			}
		}
	}

	if(dstsrc.empty()) {
		do_move();
		return;
	}

	std::cout << "finding targets...\n";
	std::vector<target> targets = find_targets(leader != units_.end());
	targets.insert(targets.end(),additional_targets_.begin(),
	                             additional_targets_.end());
	for(;;) {
		if(targets.empty()) {
			recorder.end_turn();
			return;
		}

		std::cout << "choosing move...\n";
		std::pair<location,location> move = choose_move(targets,dstsrc);
		for(std::vector<target>::const_iterator ittg = targets.begin(); ittg != targets.end(); ++ittg) {
			assert(map_.on_board(ittg->loc));
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

		move_unit(move.first,move.second,possible_moves);

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
		std::pair<Itor,Itor> del = dstsrc.equal_range(move.second);
		dstsrc.erase(del.first,del.second);
	}

	do_move();
	return;
}

void ai::do_attack(const location& u, const location& target, int weapon)
{
	recorder.add_attack(u,target,weapon);

	game_events::fire("attack",u,target);
	if(units_.count(u) && units_.count(target)) {
		attack(disp_,map_,teams_,u,target,weapon,units_,state_,gameinfo_,false);
		check_victory(units_,teams_);
	}
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
		do_attack(leader->first,choice,weapon);
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