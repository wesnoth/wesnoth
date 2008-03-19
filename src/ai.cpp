/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version
   or at your option any later version2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


//! @file ai.cpp
//! Artificial intelligence - The computer commands the enemy.

#include "ai.hpp"
#include "ai2.hpp"
#include "ai_dfool.hpp"
#ifdef HAVE_PYTHON
#include "ai_python.hpp"
#endif
#include "actions.hpp"
#include "callable_objects.hpp"
#include "dialogs.hpp"
#include "foreach.hpp"
#include "formula_ai.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "menu_events.hpp"
#include "replay.hpp"
#include "statistics.hpp"
#include "unit_display.hpp"
#include "unit.hpp"
#include "playturn.hpp"
#include "wml_exception.hpp"

#include <cassert>

#define LOG_AI LOG_STREAM(info, ai)
#define WRN_AI LOG_STREAM(warn, ai)
#define ERR_AI LOG_STREAM(err, ai)

//! A trivial ai that sits around doing absolutely nothing.
class idle_ai : public ai_interface {
public:
	idle_ai(info& i) : ai_interface(i) {}
	void play_turn() {
		game_events::fire("ai turn");
	}
};

//! Sample ai, with simple strategy.
class sample_ai : public ai_interface {
public:
	sample_ai(info& i) : ai_interface(i) {}

	void play_turn() {
		game_events::fire("ai turn");
		do_attacks();
		get_villages();
		do_moves();
		do_recruitment();
	}

protected:
	void do_attacks() {
		std::map<location,paths> possible_moves;
		move_map srcdst, dstsrc;
		calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

		for(unit_map::const_iterator i = get_info().units.begin(); i != get_info().units.end(); ++i) {
			if(current_team().is_enemy(i->second.side())) {
				location adjacent_tiles[6];
				get_adjacent_tiles(i->first,adjacent_tiles);

				int best_defense = -1;
				std::pair<location,location> best_movement;

				for(size_t n = 0; n != 6; ++n) {
					typedef move_map::const_iterator Itor;
					std::pair<Itor,Itor> range = dstsrc.equal_range(adjacent_tiles[n]);
					while(range.first != range.second) {
						const location& dst = range.first->first;
						const location& src = range.first->second;
						const unit_map::const_iterator un = get_info().units.find(src);

						const t_translation::t_terrain terrain = get_info().map.get_terrain(dst);

						const int chance_to_hit = un->second.defense_modifier(terrain);

						if(best_defense == -1 || chance_to_hit < best_defense) {
							best_defense = chance_to_hit;
							best_movement = *range.first;
                        }

						++range.first;
					}
				}

				if(best_defense != -1) {
					move_unit(best_movement.second,best_movement.first,possible_moves);
					battle_context bc(get_info().map, get_info().teams,
									  get_info().units, get_info().state,
									  get_info().gameinfo, best_movement.first,
									  i->first, -1, -1, current_team().aggression());
					attack_enemy(best_movement.first,i->first,
								 bc.get_attacker_stats().attack_num,
								 bc.get_defender_stats().attack_num);
					do_attacks();
					return;
				}
			}
		}
	}

	void get_villages() {
        std::map<location,paths> possible_moves;
        move_map srcdst, dstsrc;
        calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

        for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
            if(get_info().map.is_village(i->first) && current_team().owns_village(i->first) == false) {
                move_unit(i->second,i->first,possible_moves);
                get_villages();
                return;
            }
        }
	}

	void do_moves() {
		unit_map::const_iterator leader;
		for(leader = get_info().units.begin(); leader != get_info().units.end(); ++leader) {
			if(leader->second.can_recruit() && current_team().is_enemy(leader->second.side())) {
				break;
			}
		}

		if(leader == get_info().units.end())
			return;

		std::map<location,paths> possible_moves;
		move_map srcdst, dstsrc;
		calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

		int closest_distance = -1;
		std::pair<location,location> closest_move;

		for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
			const int distance = distance_between(i->first,leader->first);
			if(closest_distance == -1 || distance < closest_distance) {
				closest_distance = distance;
				closest_move = *i;
			}
		}

		if(closest_distance != -1) {
			move_unit(closest_move.second,closest_move.first,possible_moves);
			do_moves();
		}
	}

	void do_recruitment() {
		const std::set<std::string>& options = current_team().recruits();
	if (!options.empty()) {
			const int choice = (rand()%options.size());
		        std::set<std::string>::const_iterator i = options.begin();
		        std::advance(i,choice);

			const bool res = recruit(*i);
			if(res) {
				do_recruitment();
			}
		}
	}
};

std::vector<std::string> get_available_ais()
{
    std::vector<std::string> ais;
    ais.push_back("default");
    ais.push_back("sample_ai");
    //ais.push_back("idle_ai");
    ais.push_back("dfool_ai");
#ifdef HAVE_PYTHON
    std::vector<std::string> scripts = python_ai::get_available_scripts();
    ais.insert(ais.end(), scripts.begin(), scripts.end());
#endif
    return ais;
}

ai_interface* create_ai(const std::string& name, ai_interface::info& info)
{
	// To add an AI of your own, put
	//		if(name == "my_ai")
	//			return new my_ai(info);
	// at the top of this function

	if(name == "sample_ai")
		return new sample_ai(info);
	else if(name == "idle_ai")
		return new idle_ai(info);
	else if(name == "formula_ai")
		return new formula_ai(info);
	else if(name == "dfool_ai")
		return new dfool::dfool_ai(info);
	//else if(name == "advanced_ai")
	//	return new advanced_ai(info);
	else if(name == "ai2")
		return new ai2(info);
	else if(name == "python_ai")
#ifdef HAVE_PYTHON
		return new python_ai(info);
#else
    {
		LOG_STREAM(err, ai) << "No Python AI support available in this Wesnoth build!\n";
		return new ai2(info);
    }
#endif
	else if(name != "") {
		LOG_STREAM(err, ai) << "AI not found: '" << name << "'\n";
	}

	return new ai(info);
}

ai::ai(ai_interface::info& info) :
	ai_interface(info), 
	defensive_position_cache_(),
	threats_found_(false), 
	attacks_(),
	disp_(info.disp),
	map_(info.map), 
	gameinfo_(info.gameinfo), 
	units_(info.units),
	teams_(info.teams), 
	team_num_(info.team_num),
	state_(info.state), 
	consider_combat_(true),
	additional_targets_(),
	unit_movement_scores_(),
	not_recommended_units_(),
	unit_combat_scores_(),
	keeps_(),
	avoid_(),
	unit_stats_cache_(),
	attack_depth_(0)
{}

bool ai::recruit_usage(const std::string& usage)
{
	raise_user_interact();

	const int min_gold = 0;

	log_scope2(ai, "recruiting troops");
	LOG_AI << "recruiting '" << usage << "'\n";

	std::vector<std::string> options;
	bool found = false;
	// Find an available unit that can be recruited,
	// matches the desired usage type, and comes in under budget.
	const std::set<std::string>& recruits = current_team().recruits();
	for(std::map<std::string,unit_type>::const_iterator i =
	    gameinfo_.unit_types.begin(); i != gameinfo_.unit_types.end(); ++i)
	{
		const std::string& name = i->second.id();
		// If usage is empty consider any unit.
		if (i->second.usage() == usage || usage == "") {
			found = true;
			if(recruits.count(name)
				&& current_team().gold() - i->second.cost() > min_gold
				&& not_recommended_units_.count(name) == 0)
			{
				LOG_AI << "recommending '" << name << "'\n";
				options.push_back(name);
			}
		}
	}

	// From the available options, choose one at random
	if(options.empty() == false) {
		const int option = rand()%options.size();
		return recruit(options[option]);
	}
	if (found) {
		LOG_AI << "No available units to recruit that come under the price.\n";
	} else {
		WRN_AI << "Trying to recruit a: " << usage
			<< " but no unit of that type (usage=) is available.\n";
	}
	return false;
}

bool ai_interface::recruit(const std::string& unit_name, location loc)
{
	const std::set<std::string>& recruits = current_team().recruits();

	const std::set<std::string>::const_iterator i = recruits.find(unit_name);
	if(i == recruits.end()) {
		return false;
	}

	const int num = std::distance(recruits.begin(),i);

	// We have to add the recruit command now, because when the unit
	// is created it has to have the recruit command in the recorder
	// to be able to put random numbers into to generate unit traits.
	// However, we're not sure if the transaction will be successful,
	// so use a replay_undo object to cancel it if we don't get
	// a confirmation for the transaction.
	recorder.add_recruit(num,loc);
	replay_undo replay_guard(recorder);

	game_data::unit_type_map::const_iterator u = info_.gameinfo.unit_types.find(unit_name);
	if(u == info_.gameinfo.unit_types.end()) {
		return false;
	}

	// Check if we have enough money
	if(current_team().gold() < u->second.cost()) {
		return false;
	}
	LOG_AI << "trying recruit: team=" << (info_.team_num) <<
	    " type=" << unit_name <<
	    " cost=" << u->second.cost() <<
	    " loc=(" << loc << ')' <<
	    " gold=" << (current_team().gold()) <<
	    " (-> " << (current_team().gold()-u->second.cost()) << ")\n";

	unit new_unit(&info_.gameinfo,&info_.units,&info_.map,&info_.state,&info_.teams,&u->second,info_.team_num,true);

	// See if we can actually recruit (i.e. have enough room etc.)
	std::string recruit_err = recruit_unit(info_.map,info_.team_num,info_.units,new_unit,loc,preferences::show_ai_moves());
	if(recruit_err.empty()) {

		statistics::recruit_unit(new_unit);
		current_team().spend_gold(u->second.cost());

		// Confirm the transaction - i.e. don't undo recruitment
		replay_guard.confirm_transaction();

		raise_unit_recruited();
		const team_data data = calculate_team_data(current_team(),info_.team_num,info_.units);
		LOG_AI <<
		"recruit confirmed: team=" << (info_.team_num) <<
		" units=" << data.units <<
		" gold=" << data.gold <<
		((data.net_income < 0) ? "" : "+") <<
		data.net_income << "\n";
		recorder.add_checksum_check(loc);
		return true;
	} else {
		const team_data data = calculate_team_data(current_team(),info_.team_num,info_.units);
		LOG_AI << recruit_err << "\n";
		LOG_AI <<
		"recruit UNconfirmed: team=" << (info_.team_num) <<
		" units=" << data.units <<
		" gold=" << data.gold <<
		((data.net_income < 0) ? "" : "+") <<
		data.net_income << "\n";
		return false;
	}
}

void ai_interface::raise_user_interact()
{
	const int interact_time = 30;
	const int time_since_interact = SDL_GetTicks() - last_interact_;
	if(time_since_interact < interact_time) {
		return;
	}

	user_interact_.notify_observers();

	last_interact_ = SDL_GetTicks();
}

void ai_interface::diagnostic(const std::string& msg)
{
	if(game_config::debug) {
		info_.disp.set_diagnostic(msg);
	}
}

void ai_interface::log_message(const std::string& msg)
{
	if(game_config::debug) {
		info_.disp.add_chat_message(time(NULL), "ai", info_. team_num, msg,
				game_display::MESSAGE_PUBLIC, false);
	}
}


gamemap::location ai_interface::move_unit(location from, location to,
		std::map<location,paths>& possible_moves)
{
	const location loc = move_unit_partial(from,to,possible_moves);
	const unit_map::iterator u = info_.units.find(loc);
	if(u != info_.units.end()) {
		if(u->second.movement_left()==u->second.total_movement()) {
			u->second.set_movement(0);
			u->second.set_state("not_moved","yes");
		} else {
			u->second.set_movement(0);
		}
	}

	return loc;
}

gamemap::location ai_interface::move_unit_partial(location from, location to,
		std::map<location,paths>& possible_moves)
{
	LOG_AI << "ai_interface::move_unit " << from << " -> " << to << '\n';

	// Stop the user from issuing any commands while the unit is moving.
	const events::command_disabler disable_commands;

	info_.disp.select_hex(from);
	info_.disp.update_display();

	log_scope2(ai, "move_unit");
	unit_map::iterator u_it = info_.units.find(from);
	if(u_it == info_.units.end()) {
		LOG_STREAM(err, ai) << "Could not find unit at " << from << '\n';
		assert(false);
		return location();
	}

	if(from == to) {
		LOG_AI << "moving unit at " << from << " on spot. resetting moves\n";
		return to;
	}

	const bool show_move = preferences::show_ai_moves();

	const bool teleport = u_it->second.get_ability_bool("teleport",u_it->first);
	paths current_paths(info_.map,info_.units,from,
			info_.teams,false,teleport,current_team());

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
			u_it->second.set_movement(rt->second.move_left);

			std::vector<location> steps = rt->second.steps;

			while(steps.empty() == false && (!(info_.units.find(to) == info_.units.end() || from == to))){
				    LOG_AI << "AI attempting illegal move. Attempting to move onto existing unit\n";
				    LOG_AI << "\t" << info_.units.find(to)->second.underlying_id() <<" already on " << to << "\n";
				    LOG_AI <<"\tremoving "<<*(steps.end()-1)<<"\n";
				    to = *(steps.end()-1);
				    steps.pop_back();
				    LOG_AI << "\tresetting to " << from << " -> " << to << '\n';
			}

			if(steps.size()>1) { // First step is starting hex
			  unit_map::const_iterator utest=info_.units.find(*(steps.begin()+1));
			  if(utest != info_.units.end() && current_team().is_enemy(utest->second.side())){
			    LOG_STREAM(err, ai) << "AI tried to move onto existing enemy unit at"<<*(steps.begin())<<"\n";
			    //			    return(from);
			  }

				// Check if there are any invisible units that we uncover
				for(std::vector<location>::iterator i = steps.begin()+1; i != steps.end(); ++i) {
					location adj[6];
					get_adjacent_tiles(*i,adj);

					size_t n;
					for(n = 0; n != 6; ++n) {

						// See if there is an enemy unit next to this tile.
						// If it's invisible, we need to stop: we're ambushed.
						// If it's not, we must be a skirmisher, otherwise AI wouldn't try.

						// Or would it?  If it doesn't cheat, it might...
						const unit_map::const_iterator u = info_.units.find(adj[n]);
						if (u != info_.units.end() && u->second.emits_zoc()
							&& current_team().is_enemy(u->second.side())) {
							if (u->second.invisible(adj[n], info_.units, info_.teams)) {
								to = *i;
								steps.erase(i,steps.end());
								break;
							} else {
							  if (!u_it->second.get_ability_bool("skirmisher",*i)){
							    LOG_STREAM(err, ai) << "AI tried to skirmish with non-skirmisher\n";
							    LOG_AI << "\tresetting destination from " <<to;
							    to = *i;
							    LOG_AI << " to " << to;
							    steps.erase(i,steps.end());
							    while(steps.empty() == false && (!(info_.units.find(to) == info_.units.end() || from == to))){
								 to = *(steps.end()-1);
								 steps.pop_back();
								 LOG_AI << "\tresetting to " << from << " -> " << to << '\n';
							    }

							    break;
							  }
							}
						}
					}

					if(n != 6) {
						u_it->second.set_movement(0); // Enter enemy ZoC, no movement left
						break;
					}
				}
			}

			steps.push_back(to); // Add the destination to the steps

			if(show_move && unit_display::unit_visible_on_path(steps,
						u_it->second, info_.units,info_.teams)) {

				info_.disp.scroll_to_tiles(from,to);

				unit_map::iterator up = info_.units.find(u_it->first);
				unit_display::move_unit(steps,up->second,info_.teams);
			}
		}
	}

	std::pair<gamemap::location,unit> *p = info_.units.extract(u_it->first);

	p->first = to;
	info_.units.add(p);
	p->second.set_standing(p->first);
	if(info_.map.is_village(to)) {
		// If a new village is captured, disallow any future movement.
		if (!info_.teams[info_.team_num-1].owns_village(to))
			info_.units.find(to)->second.set_movement(-1);
		get_village(to,info_.disp,info_.teams,info_.team_num-1,info_.units);
	}

	if(show_move) {
		info_.disp.invalidate(to);
		info_.disp.draw();
	}

	recorder.add_movement(from,to);

	game_events::fire("moveto",to);

	if((info_.teams.front().uses_fog() || info_.teams.front().uses_shroud()) &&
			!info_.teams.front().fogged(to)) {
		game_events::fire("sighted",to);
	}

	// would have to go via mousehandler to make this work:
	//info_.disp.unhighlight_reach();
	raise_unit_moved();

	return to;
}

bool ai::multistep_move_possible(const location& from, 
	const location& to, const location& via,
	const std::map<location,paths>& possible_moves) const
{
	const unit_map::const_iterator i = units_.find(from);
	if(i != units_.end()) {
		if(from != via && to != via && units_.count(via) == 0) {
			LOG_AI << "when seeing if leader can move from "
				<< from << " -> " << to
				<< " seeing if can detour to keep at " << via << '\n';
			const std::map<location,paths>::const_iterator moves = possible_moves.find(from);
			if(moves != possible_moves.end()) {

				LOG_AI << "found leader moves..\n";

				int move_left = 0;

				// See if the unit can make it to 'via', and if it can,
				// how much movement it will have left when it gets there.
				const paths::routes_map::const_iterator itor = moves->second.routes.find(via);
				if(itor != moves->second.routes.end()) {
					move_left = itor->second.move_left;
					LOG_AI << "can make it to keep with " << move_left << " movement left\n";
					unit temp_unit(i->second);
					temp_unit.set_movement(move_left);
					const temporary_unit_placer unit_placer(units_,via,temp_unit);
					const paths unit_paths(map_,units_,via,teams_,false,false,current_team());

					LOG_AI << "found " << unit_paths.routes.size() << " moves for temp leader\n";

					// See if this leader could make it back to the keep.
					if(unit_paths.routes.count(to) != 0) {
						LOG_AI << "can make it back to the keep\n";
						return true;
					}
				}
			}
		}
	}

	return false;
}

gamemap::location ai::move_unit(location from, location to, std::map<location,paths>& possible_moves)
{
	std::map<location,paths> temp_possible_moves;
	std::map<location,paths>* possible_moves_ptr = &possible_moves;

	const unit_map::const_iterator i = units_.find(from);
	if(i != units_.end() && i->second.can_recruit()) {

		// If the leader isn't on its keep, and we can move to the keep
		// and still make our planned movement, then try doing that.
		const gamemap::location& start_pos = nearest_keep(i->first);

		// If we can make it back to the keep and then to our original destination, do so.
		if(multistep_move_possible(from,to,start_pos,possible_moves)) {
			from = ai_interface::move_unit(from,start_pos,possible_moves);
			if(from != start_pos) {
				return from;
			}

			const unit_map::iterator itor = units_.find(from);
			if(itor != units_.end()) {
				// Just set the movement to one less than the maximum possible, since we know
				// we can reach the destination, and we're going to move there immediately.
				itor->second.set_movement(itor->second.total_movement()-1);
			}

			move_map srcdst, dstsrc;
			calculate_possible_moves(temp_possible_moves,srcdst,dstsrc,false);
			possible_moves_ptr = &temp_possible_moves;
		}

		do_recruitment();
	}

	if(units_.count(to) == 0 || from == to) {
		const location res = ai_interface::move_unit(from,to,*possible_moves_ptr);
		if(res != to) {
			// We've been ambushed; find the ambushing unit and attack them.
			adjacent_tiles_array locs;
			get_adjacent_tiles(res,locs.data());
			for(adjacent_tiles_array::const_iterator adj_i = locs.begin(); adj_i != locs.end(); ++adj_i) {
				const unit_map::const_iterator itor = units_.find(*adj_i);
				if(itor != units_.end() && current_team().is_enemy(itor->second.side()) &&
				   !itor->second.incapacitated()) {
					battle_context bc(map_, teams_, units_, state_,
									  gameinfo_, res, *adj_i, -1, -1, current_team().aggression());
					attack_enemy(res,itor->first,bc.get_attacker_stats().attack_num,bc.get_defender_stats().attack_num);
					break;
				}
			}
		}

		return res;
	} else {
		return from;
	}
}

bool ai::attack_close(const gamemap::location& loc) const
{
	for(std::set<location>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		if(distance_between(*i,loc) < 4) {
			return true;
		}
	}

	return false;
}

void ai::attack_enemy(const location& attacking_unit, const location& target,
		int att_weapon, int def_weapon)
{
	attacks_.insert(attacking_unit);
	ai_interface::attack_enemy(attacking_unit,target,att_weapon,def_weapon);
}

void ai_interface::calculate_possible_moves(std::map<location,paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
		const std::set<gamemap::location>* remove_destinations) const
{
  calculate_moves(info_.units,res,srcdst,dstsrc,enemy,assume_full_movement,remove_destinations);
}

void ai_interface::calculate_moves(const unit_map& units, std::map<location,paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
	     const std::set<gamemap::location>* remove_destinations,
		bool see_all
          ) const
{

	for(unit_map::const_iterator un_it = units.begin(); un_it != units.end(); ++un_it) {
		// If we are looking for the movement of enemies, then this unit must be an enemy unit.
		// If we are looking for movement of our own units, it must be on our side.
		// If we are assuming full movement, then it may be a unit on our side, or allied.
		if(enemy && current_team().is_enemy(un_it->second.side()) == false ||
		   !enemy && !assume_full_movement && un_it->second.side() != info_.team_num ||
		   !enemy && assume_full_movement && current_team().is_enemy(un_it->second.side())) {
			continue;
		}
		// Discount incapacitated units
		if(un_it->second.incapacitated()) {
			continue;
		}

		// We can't see where invisible enemy units might move.
		if(enemy && un_it->second.invisible(un_it->first,units,info_.teams) && !see_all) {
			continue;
		}
		// If it's an enemy unit, reset its moves while we do the calculations.
		unit* held_unit = const_cast<unit*>(&(un_it->second));
		const unit_movement_resetter move_resetter(*held_unit,enemy || assume_full_movement);

		// Insert the trivial moves of staying on the same location.
		if(un_it->second.movement_left() == un_it->second.total_movement()) {
			std::pair<location,location> trivial_mv(un_it->first,un_it->first);
			srcdst.insert(trivial_mv);
			dstsrc.insert(trivial_mv);
		}
		const bool teleports = un_it->second.get_ability_bool("teleport",un_it->first);
		res.insert(std::pair<gamemap::location,paths>(
		                un_it->first,paths(info_.map,units,
					 un_it->first,info_.teams,false,teleports,
									current_team(),0,see_all)));
	}

	for(std::map<location,paths>::iterator m = res.begin(); m != res.end(); ++m) {
		for(paths::routes_map::iterator rtit =
		    m->second.routes.begin(); rtit != m->second.routes.end(); ++rtit) {
			const location& src = m->first;
			const location& dst = rtit->first;

			if(remove_destinations != NULL && remove_destinations->count(dst) != 0) {
				continue;
			}

			bool friend_owns = false;

			// Don't take friendly villages
			if(!enemy && info_.map.is_village(dst)) {
				for(size_t n = 0; n != info_.teams.size(); ++n) {
					if(info_.teams[n].owns_village(dst)) {
						if(n+1 != info_.team_num && current_team().is_enemy(n+1) == false) {
							friend_owns = true;
						}

						break;
					}
				}
			}

			if(friend_owns) {
				continue;
			}

			if(src != dst && units.find(dst) == units.end()) {
				srcdst.insert(std::pair<location,location>(src,dst));
				dstsrc.insert(std::pair<location,location>(dst,src));
			}
		}
	}
}

void ai::remove_unit_from_moves(const gamemap::location& loc, move_map& srcdst, move_map& dstsrc)
{
	srcdst.erase(loc);
	for(move_map::iterator i = dstsrc.begin(); i != dstsrc.end(); ) {
		if(i->second == loc) {
			dstsrc.erase(i++);
		} else {
			++i;
		}
	}
}

namespace {

//! A structure for storing an item we're trying to protect.
struct protected_item {
	protected_item(double value, int radius, const gamemap::location& loc) :
		value(value), radius(radius), loc(loc) {}

	double value;
	int radius;
	gamemap::location loc;
};

}


void ai::find_threats()
{
	if(threats_found_) {
		return;
	}

	threats_found_ = true;

	const config& parms = current_team().ai_parameters();

	std::vector<protected_item> items;

	// We want to protect our leader.
	const unit_map::const_iterator leader = find_leader(units_,team_num_);
	if(leader != units_.end()) {
		items.push_back(protected_item(
					lexical_cast_default<double>(parms["protect_leader"], 1.0),
					lexical_cast_default<int>(parms["protect_leader_radius"], 20),
					leader->first));
	}

	// Look for directions to protect a specific location.
	const config::child_list& locations = parms.get_children("protect_location");
	for(config::child_list::const_iterator i = locations.begin(); i != locations.end(); ++i) {
		items.push_back(protected_item(
					lexical_cast_default<double>((**i)["value"], 1.0),
					lexical_cast_default<int>((**i)["radius"], 20),
					gamemap::location(**i, &get_info().game_state_)));
	}

	// Look for directions to protect a unit.
	const config::child_list& protected_units = parms.get_children("protect_unit");
	for(config::child_list::const_iterator j = protected_units.begin(); j != protected_units.end(); ++j) {

		for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
			if(game_events::unit_matches_filter(u, *j)) {
				items.push_back(protected_item(
							lexical_cast_default<double>((**j)["value"], 1.0),
							lexical_cast_default<int>((**j)["radius"], 20),
							u->first));
			}
		}
	}

	// Iterate over all protected locations, and if enemy units
	// are within the protection radius, set them as hostile targets.
	for(std::vector<protected_item>::const_iterator k = items.begin(); k != items.end(); ++k) {
		const protected_item& item = *k;

		for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
			const int distance = distance_between(u->first,item.loc);
			if(current_team().is_enemy(u->second.side()) && distance < item.radius) {
				add_target(target(u->first, item.value * double(item.radius-distance) /
							double(item.radius),target::THREAT));
			}
		}
	}
}

void ai::play_turn()
{
	// Protect against a memory over commitment:
	//! @todo Not in the mood to figure out the exact cause:
	// For some reason -1 hitpoints cause a segmentation fault.
	// If -1 hitpoints are sent, we crash :/
	try {
		consider_combat_ = true;
		game_events::fire("ai turn");
		do_move();
	} catch(std::bad_alloc) {
		lg::wml_error << "Memory exhausted - a unit has either a lot of hitpoints or a negative amount.\n";
	}
}

void ai::do_move()
{
	log_scope2(ai, "doing ai move");

	invalidate_defensive_position_cache();

	raise_user_interact();

	typedef paths::route route;

	typedef std::map<location,paths> moves_map;
	moves_map possible_moves, enemy_possible_moves;

	move_map srcdst, dstsrc, enemy_srcdst, enemy_dstsrc;

	calculate_possible_moves(possible_moves,srcdst,dstsrc,false,false,&avoided_locations());
	calculate_possible_moves(enemy_possible_moves,enemy_srcdst,enemy_dstsrc,true);

	const bool passive_leader = utils::string_bool(current_team().ai_parameters()["passive_leader"]);

	if (passive_leader) {
		unit_map::iterator leader = find_leader(units_,team_num_);
		if(leader != units_.end()) {
			remove_unit_from_moves(leader->first,srcdst,dstsrc);
		}
	}

	// Execute goto-movements - first collect gotos in a list
	std::vector<gamemap::location> gotos;

	for(unit_map::iterator ui = units_.begin(); ui != units_.end(); ++ui) {
		if(ui->second.get_goto() == ui->first) {
			ui->second.set_goto(gamemap::location());
		} else if(ui->second.side() == team_num_ && map_.on_board(ui->second.get_goto())) {
			gotos.push_back(ui->first);
		}
	}

	for(std::vector<gamemap::location>::const_iterator g = gotos.begin(); g != gotos.end(); ++g) {
		unit_map::const_iterator ui = units_.find(*g);
		int closest_distance = -1;
		std::pair<location,location> closest_move;
		for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
			if(i->second != ui->first) {
				continue;
			}
			const int distance = distance_between(i->first,ui->second.get_goto());
			if(closest_distance == -1 || distance < closest_distance) {
				closest_distance = distance;
				closest_move = *i;
			}
		}

		if(closest_distance != -1) {
			move_unit(ui->first,closest_move.first,possible_moves);
		}
	}


	std::vector<attack_analysis> analysis;

	LOG_AI << "combat phase\n";

	if(consider_combat_) {
		LOG_AI << "combat...\n";
		consider_combat_ = do_combat(possible_moves,srcdst,dstsrc,enemy_srcdst,enemy_dstsrc);
		if(consider_combat_) {
			do_move();
			return;
		}
	}

	move_leader_to_goals(enemy_dstsrc);

	LOG_AI << "get villages phase\n";

	// Iterator could be invalidated by combat analysis or move_leader_to_goals.
	unit_map::iterator leader = find_leader(units_,team_num_);

	LOG_AI << "villages...\n";
	if(get_villages(possible_moves, dstsrc, enemy_dstsrc, leader)) {
		do_move();
		return;
	}

	LOG_AI << "healing...\n";
	const bool healed_unit = get_healing(possible_moves,srcdst,enemy_dstsrc);
	if(healed_unit) {
		do_move();
		return;
	}

	LOG_AI << "retreat phase\n";

	LOG_AI << "retreating...\n";

	leader = find_leader(units_,team_num_);
	const bool retreated_unit = retreat_units(possible_moves,srcdst,dstsrc,enemy_dstsrc,leader);
	if(retreated_unit) {
		do_move();
		return;
	}

	if(leader != units_.end()) {
		remove_unit_from_moves(leader->first,srcdst,dstsrc);
	}

	find_threats();

	LOG_AI << "move/targetting phase\n";

	const bool met_invisible_unit = move_to_targets(possible_moves,srcdst,dstsrc,enemy_dstsrc,leader);
	if(met_invisible_unit) {
		LOG_AI << "met_invisible_unit\n";
		do_move();
		return;
	}

	LOG_AI << "done move to targets\n";

	LOG_AI << "leader/recruitment phase\n";

	// Recruitment phase and leader movement phase.
	if(leader != units_.end()) {

		if(!passive_leader) {
			move_leader_to_keep(enemy_dstsrc);
		}

		do_recruitment();

		if(!passive_leader) {
			move_leader_after_recruit(srcdst,dstsrc,enemy_dstsrc);
		}
	}
}

bool ai::do_combat(std::map<gamemap::location,paths>& possible_moves, const move_map& srcdst,
		const move_map& dstsrc, const move_map& enemy_srcdst, const move_map& enemy_dstsrc)
{
	int ticks = SDL_GetTicks();

	std::vector<attack_analysis> analysis = analyze_targets(srcdst, dstsrc,
			enemy_srcdst, enemy_dstsrc);

	int time_taken = SDL_GetTicks() - ticks;
	LOG_AI << "took " << time_taken << " ticks for " << analysis.size()
		<< " positions. Analyzing...\n";

	ticks = SDL_GetTicks();

	const int max_sims = 50000;
	int num_sims = analysis.empty() ? 0 : max_sims/analysis.size();
	if(num_sims < 20)
		num_sims = 20;
	if(num_sims > 40)
		num_sims = 40;

	LOG_AI << "simulations: " << num_sims << "\n";

	const int max_positions = 30000;
	const int skip_num = analysis.size()/max_positions;

	std::vector<attack_analysis>::iterator choice_it = analysis.end();
	double choice_rating = -1000.0;
	for(std::vector<attack_analysis>::iterator it = analysis.begin();
			it != analysis.end(); ++it) {

		if(skip_num > 0 && ((it - analysis.begin())%skip_num) && it->movements.size() > 1)
			continue;

		const double rating = it->rating(current_team().aggression(),*this);
		LOG_AI << "attack option rated at " << rating << " ("
			<< current_team().aggression() << ")\n";

		if(rating > choice_rating) {
			choice_it = it;
			choice_rating = rating;
		}
	}

	time_taken = SDL_GetTicks() - ticks;
	LOG_AI << "analysis took " << time_taken << " ticks\n";

	if(choice_rating > 0.0) {
		location from   = choice_it->movements[0].first;
		location to     = choice_it->movements[0].second;
		location target_loc = choice_it->target;

		// Never used:
		//		const unit_map::const_iterator tgt = units_.find(target_loc);

		const location arrived_at = move_unit(from,to,possible_moves);
		if(arrived_at != to || units_.find(to) == units_.end()) {
			LOG_STREAM(warn, ai) << "unit moving to attack has ended up unexpectedly at "
			                     << arrived_at << " when moving to " << to << " moved from "
			                     << from << '\n';
			return true;
		}

		// Recalc appropriate weapons here: AI uses approximations.
		battle_context bc(map_, teams_, units_, state_,
						  gameinfo_, to, target_loc, -1, -1,
						  current_team().aggression());
		attack_enemy(to, target_loc, bc.get_attacker_stats().attack_num,
				bc.get_defender_stats().attack_num);

		// If this is the only unit in the attack, and the target
		// is still alive, then also summon reinforcements
		if(choice_it->movements.size() == 1 && units_.count(target_loc)) {
			add_target(target(target_loc,3.0,target::BATTLE_AID));
		}

		return true;

	} else {
		return false;
	}
}

//! This function should be called to attack an enemy.
//!
//! @param u            The location of the attacking unit. (Note this shouldn't
//!                     be a reference since attack::attack() can invalidate the
//!                     unit_map and references to the map are also invalid then.)
//! @param target       The location of the target unit. This unit must be in 
//!                     range of the attacking unit's weapon. (See note at param u.)
//! @param weapon       The number of the weapon (0-based) which should be used 
//!                     by the attacker. (It must be a valid weapon of the attacker.)
//! @param def_weapon   The number of the weapon (0-based) which should be used 
//!                     by the defender. (It must be a valid weapon of the defender.)
void ai_interface::attack_enemy(const location u, 
		const location target, int weapon, int def_weapon)
{
	// Stop the user from issuing any commands while the unit is attacking
	const events::command_disabler disable_commands;

	if(info_.units.count(u) && info_.units.count(target)) {
		if(info_.units.find(target)->second.incapacitated()) {
			LOG_STREAM(err, ai) << "attempt to attack unit that is turned to stone\n";
			return;
		}
		if(!info_.units.find(u)->second.attacks_left()) {
			LOG_STREAM(err, ai) << "attempt to attack twice with the same unit\n";
			return;
		}

		recorder.add_attack(u,target,weapon,def_weapon);
		try {
			attack(info_.disp, info_.map, info_.teams, u, target, weapon, def_weapon,
					info_.units, info_.state, info_.gameinfo);
		}
		catch (end_level_exception&)
		{
			dialogs::advance_unit(info_.gameinfo,info_.map,info_.units,u,info_.disp,true);
			
			const unit_map::const_iterator defender = info_.units.find(target);
			if(defender != info_.units.end()) {
				const size_t defender_team = size_t(defender->second.side()) - 1;
				if(defender_team < info_.teams.size()) {
					dialogs::advance_unit(info_.gameinfo, info_.map, info_.units,
							target, info_.disp, !info_.teams[defender_team].is_human());
				}
			}

			throw;
		}
		dialogs::advance_unit(info_.gameinfo,info_.map,info_.units,u,info_.disp,true);

		const unit_map::const_iterator defender = info_.units.find(target);
		if(defender != info_.units.end()) {
			const size_t defender_team = size_t(defender->second.side()) - 1;
			if(defender_team < info_.teams.size()) {
				dialogs::advance_unit(info_.gameinfo, info_.map, info_.units,
						target, info_.disp, !info_.teams[defender_team].is_human());
			}
		}

		check_victory(info_.units,info_.teams, info_.disp);
		raise_enemy_attacked();
	}
}

bool ai::get_healing(std::map<gamemap::location,paths>& possible_moves,
		const move_map& srcdst, const move_map& enemy_dstsrc)
{
	// Find units in need of healing.
	unit_map::iterator u_it = units_.begin();
	for(; u_it != units_.end(); ++u_it) {
		unit& u = u_it->second;

		// If the unit is on our side, has lost as many or more than
		// 1/2 round worth of healing, and doesn't regenerate itself,
		// then try to find a vacant village for it to rest in.
		if(u.side() == team_num_ &&
		   u.max_hitpoints() - u.hitpoints() >= game_config::poison_amount/2 &&
		   !u.get_ability_bool("regenerate",u_it->first)) {

			// Look for the village which is the least vulnerable to enemy attack.
			typedef std::multimap<location,location>::const_iterator Itor;
			std::pair<Itor,Itor> it = srcdst.equal_range(u_it->first);
			double best_vulnerability = 100000.0;
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const location& dst = it.first->second;
				if(map_.gives_healing(dst) && (units_.find(dst) == units_.end() || dst == u_it->first)) {
					const double vuln = power_projection(it.first->first, enemy_dstsrc);
					LOG_AI << "found village with vulnerability: " << vuln << "\n";
					if(vuln < best_vulnerability || best_loc == it.second) {
						best_vulnerability = vuln;
						best_loc = it.first;
						LOG_AI << "chose village " << dst << '\n';
					}
				}

				++it.first;
			}

			// If we have found an eligible village:
			if(best_loc != it.second) {
				const location& src = best_loc->first;
				const location& dst = best_loc->second;

				LOG_AI << "moving unit to village for healing...\n";

				move_unit(src,dst,possible_moves);
				return true;
			}
		}
	}

	return false;
}

bool ai::should_retreat(const gamemap::location& loc, const unit_map::const_iterator un,
		const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_dstsrc,
		double caution)
{
	if(caution <= 0.0) {
		return false;
	}

	const double optimal_terrain = best_defensive_position(un->first, dstsrc,
			srcdst, enemy_dstsrc).chance_to_hit/100.0;
	const double proposed_terrain =
		un->second.defense_modifier(map_.get_terrain(loc))/100.0;

	// The 'exposure' is the additional % chance to hit
	// this unit receives from being on a sub-optimal defensive terrain.
	const double exposure = proposed_terrain - optimal_terrain;

	const double our_power = power_projection(loc,dstsrc);
	const double their_power = power_projection(loc,enemy_dstsrc);
	return caution*their_power*(1.0+exposure) > our_power;
}

bool ai::retreat_units(std::map<gamemap::location,paths>& possible_moves,
		const move_map& srcdst, const move_map& dstsrc,
		const move_map& enemy_dstsrc, unit_map::const_iterator leader)
{
	// Get versions of the move map that assume that all units are at full movement
	std::map<gamemap::location,paths> dummy_possible_moves;
	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves, fullmove_srcdst, fullmove_dstsrc,
			false, true, &avoided_locations());

	gamemap::location leader_adj[6];
	if(leader != units_.end()) {
		get_adjacent_tiles(leader->first,leader_adj);
	}

	for(unit_map::iterator i = units_.begin(); i != units_.end(); ++i) {
		if(i->second.side() == team_num_ &&
				i->second.movement_left() == i->second.total_movement() &&
				unit_map::const_iterator(i) != leader &&
				!i->second.incapacitated()) {

			// This unit still has movement left, and is a candidate to retreat.
			// We see the amount of power of each side on the situation,
			// and decide whether it should retreat.
			if(should_retreat(i->first, i, fullmove_srcdst, fullmove_dstsrc,
						enemy_dstsrc, current_team().caution())) {

				bool can_reach_leader = false;

				// Time to retreat. Look for the place where the power balance
				// is most in our favor.
				// If we can't find anywhere where we like the power balance,
				// just try to get to the best defensive hex.
				typedef move_map::const_iterator Itor;
				std::pair<Itor,Itor> itors = srcdst.equal_range(i->first);
				gamemap::location best_pos, best_defensive(i->first);
				double best_rating = 0.0;
				int best_defensive_rating = i->second.defense_modifier(map_.get_terrain(i->first))
					- (map_.is_village(i->first) ? 10 : 0);
				while(itors.first != itors.second) {

					if(leader != units_.end() && std::count(leader_adj,
								leader_adj + 6, itors.first->second)) {

						can_reach_leader = true;
						break;
					}

					// We rate the power balance of a hex based on our power projection
					// compared to theirs, multiplying their power projection by their
					// chance to hit us on the hex we're planning to flee to.
					const gamemap::location& hex = itors.first->second;
					const int defense = i->second.defense_modifier(map_.get_terrain(hex));
					const double our_power = power_projection(hex,dstsrc);
					const double their_power = power_projection(hex,enemy_dstsrc) * double(defense)/100.0;
					const double rating = our_power - their_power;
					if(rating > best_rating) {
						best_pos = hex;
						best_rating = rating;
					}

					// Give a bonus for getting to a village.
					const int modified_defense = defense - (map_.is_village(hex) ? 10 : 0);

					if(modified_defense < best_defensive_rating) {
						best_defensive_rating = modified_defense;
						best_defensive = hex;
					}

					++itors.first;
				}

				// If the unit is in range of its leader, it should
				// never retreat -- it has to defend the leader instead.
				if(can_reach_leader) {
					continue;
				}

				if(!best_pos.valid()) {
					best_pos = best_defensive;
				}

				// If we can't move, we should be more aggressive in lashing out.
				if (best_pos == i->first) {
					if (i->second.attacks_left()) {
						return desperate_attack(i->first);
					}
					return false;
				}

				if(best_pos.valid()) {
					LOG_AI << "retreating '" << i->second.type_id() << "' " << i->first
					       << " -> " << best_pos << '\n';
					move_unit(i->first,best_pos,possible_moves);
					return true;
				}
			}
		}
	}

	return false;
}

bool ai::move_to_targets(std::map<gamemap::location, paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, const move_map& enemy_dstsrc,
		unit_map::const_iterator leader)
{
	LOG_AI << "finding targets...\n";
	std::vector<target> targets;
	for(;;) {
		if(targets.empty()) {
			targets = find_targets(leader,enemy_dstsrc);
			targets.insert(targets.end(),additional_targets_.begin(),
			                             additional_targets_.end());
			if(targets.empty()) {
				return false;
			}
		}

		LOG_AI << "choosing move...\n";
		std::pair<location,location> move = choose_move(targets, srcdst,
				dstsrc, enemy_dstsrc);

		for(std::vector<target>::const_iterator ittg = targets.begin();
				ittg != targets.end(); ++ittg) {
			assert(map_.on_board(ittg->loc));
		}

		if(move.first.valid() == false) {
			break;
		}

		if(move.second.valid() == false) {
			return true;
		}

		LOG_AI << "move: " << move.first << " -> " << move.second << '\n';

		const location arrived_at = move_unit(move.first,move.second,possible_moves);

		// We didn't arrive at our intended destination.
		// We return true, meaning that the AI algorithm
		// should be recalculated from the start.
		if(arrived_at != move.second) {
			LOG_STREAM(warn, ai) << "didn't arrive at destination\n";
			return true;
		}

		const unit_map::const_iterator u_it = units_.find(arrived_at);
		// Event could have done anything: check
		if (u_it == units_.end() || u_it->second.incapacitated()) {
			LOG_STREAM(warn, ai) << "stolen or incapacitated\n";
		} else {
			// Search to see if there are any enemy units next to the tile
			// which really should be attacked now the move is done.
			gamemap::location adj[6];
			get_adjacent_tiles(arrived_at,adj);
			gamemap::location target;

			for(int n = 0; n != 6; ++n) {
				const unit_map::iterator enemy = find_visible_unit(units_,adj[n],
																   map_,
																   teams_,current_team());

				if(enemy != units_.end() &&
				   current_team().is_enemy(enemy->second.side()) && !enemy->second.incapacitated()) {
					// Current behavior is to only make risk-free attacks.
					battle_context bc(map_, teams_, units_, state_, gameinfo_, arrived_at, adj[n], -1, -1, 100.0);
					if (bc.get_defender_stats().damage == 0) {
						attack_enemy(arrived_at, adj[n], bc.get_attacker_stats().attack_num,
								bc.get_defender_stats().attack_num);
						break;
					}
				}
			}
		}

		// Don't allow any other units to move onto the tile
		// our unit just moved onto
		typedef move_map::iterator Itor;
		std::pair<Itor,Itor> del = dstsrc.equal_range(arrived_at);
		dstsrc.erase(del.first,del.second);
	}

	return false;
}

int ai::average_resistance_against(const unit_type& a, const unit_type& b) const
{
	int weighting_sum = 0, defense = 0;
	const std::map<t_translation::t_terrain, size_t>& terrain =
		map_.get_weighted_terrain_frequencies();

	for (std::map<t_translation::t_terrain, size_t>::const_iterator j = terrain.begin(),
	     j_end = terrain.end(); j != j_end; ++j)
	{
		// Use only reachable tiles when computing the average defense.
		if (a.movement_type().movement_cost(map_, j->first) < 99) {
			defense += a.movement_type().defense_modifier(map_, j->first) * j->second;
			weighting_sum += j->second;
		}
	}

	if (weighting_sum == 0) {
		// This unit can't move on this map, so just get the average weighted
		// of all available terrains. This still is a kind of silly
		// since the opponent probably can't recruit this unit and it's a static unit.
		for (std::map<t_translation::t_terrain, size_t>::const_iterator jj = terrain.begin(),
				jj_end = terrain.end(); jj != jj_end; ++jj)
		{
			defense += a.movement_type().defense_modifier(map_, jj->first) * jj->second;
			weighting_sum += jj->second;
		}
	}

	if(weighting_sum != 0) {
		defense /= weighting_sum;
	} else {
		ERR_AI << "The weighting sum is 0 and is ignored.\n";
	}

	LOG_AI << "average defense of '" << a.id() << "': " << defense << "\n";

	int sum = 0, weight_sum = 0;

	// calculation of the average damage taken
	bool steadfast = a.has_ability_by_id("steadfast");
	bool living = !a.not_living();
	const std::vector<attack_type>& attacks = b.attacks();
	for (std::vector<attack_type>::const_iterator i = attacks.begin(),
	     i_end = attacks.end(); i != i_end; ++i)
	{
		int resistance = a.movement_type().resistance_against(*i);
		// Apply steadfast resistance modifier.
		if (steadfast && resistance < 100)
			resistance = maximum<int>(resistance * 2 - 100, 50);
		// Do not look for filters or values, simply assume 70% if CTH is customized.
		int cth = i->get_special_bool("chance_to_hit", true) ? 70 : defense;
		int weight = i->damage() * i->num_attacks();
		// if cth == 0 the division will do 0/0 so don't execute this part
		if (living && cth != 0 && i->get_special_bool("poison", true)) {
			// Compute the probability of not poisoning the unit.
			int prob = 100;
			for (int j = 0; j < i->num_attacks(); ++j)
				prob = prob * (100 - cth);
			// Assume poison works one turn.
			weight += game_config::poison_amount * (100 - prob) / 100;
		}
		sum += cth * resistance * weight * weight; // average damage * weight
		weight_sum += weight;
	}

	// normalize by HP
	sum /= maximum<int>(1,minimum<int>(a.hitpoints(),1000)); // avoid values really out of range

	// Catch division by zero here if the attacking unit
	// has zero attacks and/or zero damage.
	// If it has no attack at all, the ai shouldn't prefer
	// that unit anyway.
	if (weight_sum == 0) {
		return sum;
	}
	return sum/weight_sum;
}

int ai::compare_unit_types(const unit_type& a, const unit_type& b) const
{
	const int a_effectiveness_vs_b = average_resistance_against(b,a);
	const int b_effectiveness_vs_a = average_resistance_against(a,b);

	LOG_AI << "comparison of '" << a.id() << " vs " << b.id() << ": "
	          << a_effectiveness_vs_b << " - " << b_effectiveness_vs_a << " = "
			  << (a_effectiveness_vs_b - b_effectiveness_vs_a) << "\n";
	return a_effectiveness_vs_b - b_effectiveness_vs_a;
}

void ai::analyze_potential_recruit_combat()
{
	if(unit_combat_scores_.empty() == false ||
			utils::string_bool(current_team().ai_parameters()["recruitment_ignore_bad_combat"])) {
		return;
	}

	log_scope2(ai, "analyze_potential_recruit_combat()");

	// Records the best combat analysis for each usage type.
	std::map<std::string,int> best_usage;

	const std::set<std::string>& recruits = current_team().recruits();
	std::set<std::string>::const_iterator i;
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const game_data::unit_type_map::const_iterator info = gameinfo_.unit_types.find(*i);
		if(info == gameinfo_.unit_types.end() || not_recommended_units_.count(*i)) {
			continue;
		}

		int score = 0, weighting = 0;

		for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {
			if(j->second.can_recruit() || current_team().is_enemy(j->second.side()) == false) {
				continue;
			}

			unit const &un = j->second;
			const game_data::unit_type_map::const_iterator enemy_info = gameinfo_.unit_types.find(un.type_id());
			VALIDATE((enemy_info != gameinfo_.unit_types.end()), _("Unknown unit type : ") + un.type_id());

			int weight = un.cost() * un.hitpoints() / un.max_hitpoints();
			weighting += weight;
			score += compare_unit_types(info->second, enemy_info->second) * weight;
		}

		if(weighting != 0) {
			score /= weighting;
		}

		LOG_AI << "combat score of '" << *i << "': " << score << "\n";
		unit_combat_scores_[*i] = score;

		if(best_usage.count(info->second.usage()) == 0 ||
				score > best_usage[info->second.usage()]) {
			best_usage[info->second.usage()] = score;
		}
	}

	// Recommend not to use units of a certain usage type
	// if they have a score more than 600 below
	// the best unit of that usage type.
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const game_data::unit_type_map::const_iterator info = gameinfo_.unit_types.find(*i);
		if(info == gameinfo_.unit_types.end() || not_recommended_units_.count(*i)) {
			continue;
		}

		if(unit_combat_scores_[*i] + 600 < best_usage[info->second.usage()]) {
			LOG_AI << "recommending not to use '" << *i << "' because of poor combat performance "
				      << unit_combat_scores_[*i] << "/" << best_usage[info->second.usage()] << "\n";
			not_recommended_units_.insert(*i);
		}
	}
}

namespace {

struct target_comparer_distance {
	target_comparer_distance(const gamemap::location& loc) : loc_(loc) {}

	bool operator()(const ai::target& a, const ai::target& b) const {
		return distance_between(a.loc,loc_) < distance_between(b.loc,loc_);
	}

private:
	gamemap::location loc_;
};

}

void ai::analyze_potential_recruit_movements()
{
	if(unit_movement_scores_.empty() == false ||
			utils::string_bool(current_team().ai_parameters()["recruitment_ignore_bad_movement"])) {
		return;
	}

	const unit_map::const_iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end()) {
		return;
	}

	const location& start = nearest_keep(leader->first);
	if(map_.on_board(start) == false) {
		return;
	}

	log_scope2(ai, "analyze_potential_recruit_movements()");

	const unsigned int max_targets = 5;

	const move_map srcdst, dstsrc;
	std::vector<target> targets = find_targets(leader,dstsrc);
	if(targets.size() > max_targets) {
		std::sort(targets.begin(),targets.end(),target_comparer_distance(start));
		targets.erase(targets.begin()+max_targets,targets.end());
	}

	const std::set<std::string>& recruits = current_team().recruits();

	LOG_AI << "targets: " << targets.size() << "\n";

	std::map<std::string,int> best_scores;

	for(std::set<std::string>::const_iterator i = recruits.begin(); i != recruits.end(); ++i) {
		const game_data::unit_type_map::const_iterator info = gameinfo_.unit_types.find(*i);
		if(info == gameinfo_.unit_types.end()) {
			continue;
		}

		const unit temp_unit(&get_info().gameinfo, &get_info().units,&get_info().map,
				&get_info().state, &get_info().teams, &info->second, team_num_);
		unit_map units;
		const temporary_unit_placer placer(units,start,temp_unit);

		int cost = 0;
		int targets_reached = 0;
		int targets_missed = 0;

		const shortest_path_calculator calc(temp_unit,current_team(),units,teams_,map_);
		for(std::vector<target>::const_iterator t = targets.begin(); t != targets.end(); ++t) {
			LOG_AI << "analyzing '" << *i << "' getting to target...\n";
			const paths::route& route = a_star_search(start, t->loc, 100.0, &calc,
					get_info().map.w(), get_info().map.h());

			if(route.steps.empty() == false) {
				LOG_AI << "made it: " << route.move_left << "\n";
				cost += route.move_left;
				++targets_reached;
			} else {
				LOG_AI << "failed\n";
				++targets_missed;
			}
		}

		if(targets_reached == 0 || targets_missed >= targets_reached*2) {
			unit_movement_scores_[*i] = 100000;
			not_recommended_units_.insert(*i);
		} else {
			const int average_cost = cost/targets_reached;
			const int score = (average_cost * (targets_reached+targets_missed))/targets_reached;
			unit_movement_scores_[*i] = score;

			const std::map<std::string,int>::const_iterator current_best = best_scores.find(temp_unit.usage());
			if(current_best == best_scores.end() || score < current_best->second) {
				best_scores[temp_unit.usage()] = score;
			}
		}
	}

	for(std::map<std::string,int>::iterator j = unit_movement_scores_.begin();
			j != unit_movement_scores_.end(); ++j) {

		const game_data::unit_type_map::const_iterator info =
			gameinfo_.unit_types.find(j->first);

		if(info == gameinfo_.unit_types.end()) {
			continue;
		}

		const int best_score = best_scores[info->second.usage()];
		if(best_score > 0) {
			j->second = (j->second*10)/best_score;
			if(j->second > 15) {
				LOG_AI << "recommending against recruiting '" << j->first << "' (score: " << j->second << ")\n";
				not_recommended_units_.insert(j->first);
			} else {
				LOG_AI << "recommending recruit of '" << j->first << "' (score: " << j->second << ")\n";
			}
		}
	}

	if(not_recommended_units_.size() == unit_movement_scores_.size()) {
		not_recommended_units_.clear();
	}
}

void ai::do_recruitment()
{
	const unit_map::const_iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end()) {
		return;
	}

	const location& start_pos = nearest_keep(leader->first);

	analyze_potential_recruit_movements();
	analyze_potential_recruit_combat();

	size_t neutral_villages = 0;

	// We recruit the initial allocation of scouts
	// based on how many neutral villages there are
	// that are closer to us than to other keeps.
	const std::vector<location>& villages = map_.villages();
	for(std::vector<location>::const_iterator v = villages.begin(); v != villages.end(); ++v) {
		const int owner = village_owner(*v,teams_);
		if(owner == -1) {
			const size_t distance = distance_between(start_pos,*v);

			bool closest = true;
			for(std::vector<team>::const_iterator i = teams_.begin(); i != teams_.end(); ++i) {
				const int index = i - teams_.begin() + 1;
				const gamemap::location& loc = map_.starting_position(index);
				if(loc != start_pos && distance_between(loc,*v) < distance) {
					closest = false;
					break;
				}
			}

			if(closest) {
				++neutral_villages;
			}
		}
	}

	// The villages per scout is for a two-side battle,
	// accounting for all neutral villages on the map.
	// We only look at villages closer to us, so we halve it,
	// making us get twice as many scouts.
	const int villages_per_scout = current_team().villages_per_scout()/2;

	// Get scouts depending on how many neutral villages there are.
	int scouts_wanted = villages_per_scout > 0 ? neutral_villages/villages_per_scout : 0;

	LOG_AI << "scouts_wanted: " << neutral_villages << "/"
		<< villages_per_scout << " = " << scouts_wanted << "\n";

	std::map<std::string,int> unit_types;

	for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
		if(u->second.side() == team_num_) {
			++unit_types[u->second.usage()];
		}
	}

	LOG_AI << "we have " << unit_types["scout"] << " scouts already and we want "
		<< scouts_wanted << " in total\n";

	while(unit_types["scout"] < scouts_wanted) {
		if(recruit_usage("scout") == false)
			break;

		++unit_types["scout"];
	}

	std::vector<std::string> options = current_team().recruitment_pattern();

	// If there is no recruitment_pattern use "" which makes us consider
	// any unit available.
	if (options.empty()) {
		options.push_back("");
	}
	// Buy units as long as we have room and can afford it.
	while(recruit_usage(options[rand()%options.size()])) {
	}
}

void ai::move_leader_to_goals( const move_map& enemy_dstsrc)
{
	const config* const goal = current_team().ai_parameters().child("leader_goal");

	if(goal == NULL) {
		LOG_AI << "No goal found\n";
		return;
	}

	const gamemap::location dst(*goal, &get_info().game_state_);
	if (!dst.valid()) {
		ERR_AI << "Invalid goal\n";
		return;
	}

	const unit_map::iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end() || leader->second.incapacitated()) {
		WRN_AI << "Leader not found\n";
		return;
	}

	LOG_AI << "Doing recruitment before goals\n";

	do_recruitment();

	shortest_path_calculator calc(leader->second, current_team(), units_, teams_, map_);
	const paths::route route = a_star_search(leader->first, dst, 1000.0, &calc,
			get_info().map.w(), get_info().map.h());
	if(route.steps.empty()) {
		LOG_AI << "route empty";
		return;
	}

	const paths leader_paths(map_, units_, leader->first,
			teams_, false, false, current_team());

	std::map<gamemap::location,paths> possible_moves;
	possible_moves.insert(std::pair<gamemap::location,paths>(leader->first,leader_paths));

	gamemap::location loc;
	for(std::vector<gamemap::location>::const_iterator itor = route.steps.begin();
			itor != route.steps.end(); ++itor) {

		if(leader_paths.routes.count(*itor) == 1 &&
				power_projection(*itor,enemy_dstsrc) < double(leader->second.hitpoints()/2)) {
			loc = *itor;
		}
	}

	if(loc.valid()) {
		LOG_AI << "Moving leader to goal\n";
		move_unit(leader->first,loc,possible_moves);
	}
}

void ai::move_leader_to_keep(const move_map& enemy_dstsrc)
{
	const unit_map::iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end() || leader->second.incapacitated()) {
		return;
	}

	// Find where the leader can move
	const paths leader_paths(map_, units_, leader->first,
			teams_, false, false, current_team());
	const gamemap::location& start_pos = nearest_keep(leader->first);

	std::map<gamemap::location,paths> possible_moves;
	possible_moves.insert(std::pair<gamemap::location,paths>(leader->first,leader_paths));

	// If the leader is not on his starting location, move him there.
	if(leader->first != start_pos) {
		const paths::routes_map::const_iterator itor = leader_paths.routes.find(start_pos);
		if(itor != leader_paths.routes.end() && units_.count(start_pos) == 0) {
			move_unit(leader->first,start_pos,possible_moves);
		} else {
			// Make a map of the possible locations the leader can move to,
			// ordered by the distance from the keep.
			std::multimap<int,gamemap::location> moves_toward_keep;

			// The leader can't move to his keep, try to move to the closest location
			// to the keep where there are no enemies in range.
			const int current_distance = distance_between(leader->first,start_pos);
			for(paths::routes_map::const_iterator i = leader_paths.routes.begin();
					i != leader_paths.routes.end(); ++i) {

				const int new_distance = distance_between(i->first,start_pos);
				if(new_distance < current_distance) {
					moves_toward_keep.insert(std::pair<int,gamemap::location>(new_distance,i->first));
				}
			}

			// Find the first location which we can move to,
			// without the threat of enemies.
			for(std::multimap<int,gamemap::location>::const_iterator j = moves_toward_keep.begin();
					j != moves_toward_keep.end(); ++j) {

				if(enemy_dstsrc.count(j->second) == 0) {
					move_unit(leader->first,j->second,possible_moves);
					break;
				}
			}
		}
	}
}

void ai::move_leader_after_recruit(const move_map& /*srcdst*/,
		const move_map& /*dstsrc*/, const move_map& enemy_dstsrc)
{
	LOG_AI << "moving leader after recruit...\n";

	unit_map::iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end() || leader->second.incapacitated()) {
		return;
	}

	const paths leader_paths(map_, units_, leader->first,
			teams_, false, false, current_team());

	std::map<gamemap::location,paths> possible_moves;
	possible_moves.insert(std::pair<gamemap::location,paths>(leader->first,leader_paths));

	if(current_team().gold() < 20 && is_accessible(leader->first,enemy_dstsrc) == false) {
		// See if we want to ward any enemy units off from getting our villages.
		for(move_map::const_iterator i = enemy_dstsrc.begin(); i != enemy_dstsrc.end(); ++i) {

			// If this is a village of ours, that an enemy can capture
			// on their turn, and which we might be able to reach in two turns.
			if(map_.is_village(i->first) && current_team().owns_village(i->first) &&
				int(distance_between(i->first,leader->first)) <= leader->second.total_movement()*2) {

				int current_distance = distance_between(i->first,leader->first);
				location current_loc;

				for(paths::routes_map::const_iterator j = leader_paths.routes.begin();
						j != leader_paths.routes.end(); ++j) {

					const int distance = distance_between(i->first,j->first);
					if(distance < current_distance && is_accessible(j->first,enemy_dstsrc) == false) {
						current_distance = distance;
						current_loc = j->first;
					}
				}

				// If this location is in range of the village,
				// then we consider moving to it
				if(current_loc.valid()) {
					LOG_AI << "considering movement to " << str_cast(current_loc.x + 1)
						<< "," << str_cast(current_loc.y+1);
					unit_map temp_units(current_loc,leader->second);
					const paths p(map_,temp_units,current_loc,teams_,false,false,current_team());

					if(p.routes.count(i->first)) {
						move_unit(leader->first,current_loc,possible_moves);
						return;
					}
				}
			}
		}
	}

	// See if any friendly leaders can make it to our keep.
	// If they can, then move off it, so that they can recruit if they want.
	if(nearest_keep(leader->first) == leader->first) {
		const location keep = leader->first;
		std::pair<gamemap::location,unit> *temp_leader;

		temp_leader = units_.extract(keep);

		bool friend_can_reach_keep = false;

		std::map<location,paths> friends_possible_moves;
		move_map friends_srcdst, friends_dstsrc;
		calculate_possible_moves(friends_possible_moves,friends_srcdst,friends_dstsrc,false,true);
		for(move_map::const_iterator i = friends_dstsrc.begin(); i != friends_dstsrc.end(); ++i) {
			if(i->first == keep) {
				const unit_map::const_iterator itor = units_.find(i->second);
				if(itor != units_.end() && itor->second.can_recruit()) {
					friend_can_reach_keep = true;
					break;
				}
			}
		}

		units_.add(temp_leader);

		if(friend_can_reach_keep) {
			// Find a location for our leader to vacate the keep to
			location adj[6];
			get_adjacent_tiles(keep,adj);
			for(size_t n = 0; n != 6; ++n) {
				// Vacate to the first location found that is on the board,
				// our leader can move to, and no enemies can reach.
				if(map_.on_board(adj[n]) &&
						leader_paths.routes.count(adj[n]) != 0 &&
						is_accessible(adj[n],enemy_dstsrc) == false) {

					move_unit(keep,adj[n],possible_moves);
					return;
				}
			}
		}
	}

	// We didn't move: are we in trouble?
	leader = find_leader(units_,team_num_);
	if (!leader->second.has_moved() && leader->second.attacks_left()) {
		std::map<gamemap::location,paths> dummy_possible_moves;
		move_map fullmove_srcdst;
		move_map fullmove_dstsrc;
		calculate_possible_moves(dummy_possible_moves,fullmove_srcdst,fullmove_dstsrc,false,true,&avoided_locations());

		if (should_retreat(leader->first, leader, fullmove_srcdst, fullmove_dstsrc, enemy_dstsrc, 0.5)) {
			desperate_attack(leader->first);
		}
	}
}

bool ai::leader_can_reach_keep()
{
	const unit_map::iterator leader = find_leader(units_,team_num_);
	if(leader == units_.end() || leader->second.incapacitated()) {
		return false;
	}

	const gamemap::location& start_pos = nearest_keep(leader->first);
	if(start_pos.valid() == false) {
		return false;
	}

	if(leader->first == start_pos) {
		return true;
	}

	// Find where the leader can move
	const paths leader_paths(map_,units_,leader->first,teams_,false,false,current_team());


	return leader_paths.routes.count(start_pos) > 0;
}

int ai::rate_terrain(const unit& u, const gamemap::location& loc)
{
	const t_translation::t_terrain terrain = map_.get_terrain(loc);
	const int defense = u.defense_modifier(terrain);
	int rating = 100 - defense;

	const int healing_value = 10;
	const int friendly_village_value = 5;
	const int neutral_village_value = 10;
	const int enemy_village_value = 15;

	if(map_.gives_healing(terrain) && u.get_ability_bool("regenerates",loc) == false) {
		rating += healing_value;
	}

	if(map_.is_village(terrain)) {
		const int owner = village_owner(loc,teams_);

		if(owner + 1 == static_cast<int>(team_num_)) {
			rating += friendly_village_value;
		} else if(owner == -1) {
			rating += neutral_village_value;
		} else {
			rating += enemy_village_value;
		}
	}

	return rating;
}

const ai::defensive_position& ai::best_defensive_position(const gamemap::location& loc,
		const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc)
{
	const unit_map::const_iterator itor = units_.find(loc);
	if(itor == units_.end()) {
		static defensive_position pos;
		pos.chance_to_hit = 0;
		pos.vulnerability = pos.support = 0;
		return pos;
	}

	const std::map<location,defensive_position>::const_iterator position =
		defensive_position_cache_.find(loc);

	if(position != defensive_position_cache_.end()) {
		return position->second;
	}

	defensive_position pos;
	pos.chance_to_hit = 100;
	pos.vulnerability = 10000.0;
	pos.support = 0.0;

	typedef move_map::const_iterator Itor;
	const std::pair<Itor,Itor> itors = srcdst.equal_range(loc);
	for(Itor i = itors.first; i != itors.second; ++i) {
		const int defense = itor->second.defense_modifier(map_.get_terrain(i->second));
		if(defense > pos.chance_to_hit) {
			continue;
		}

		const double vulnerability = power_projection(i->second,enemy_dstsrc);
		const double support = power_projection(i->second,dstsrc);

		if(defense < pos.chance_to_hit || support - vulnerability > pos.support - pos.vulnerability) {
			pos.loc = i->second;
			pos.chance_to_hit = defense;
			pos.vulnerability = vulnerability;
			pos.support = support;
		}
	}

	defensive_position_cache_.insert(std::pair<location,defensive_position>(loc,pos));
	return defensive_position_cache_[loc];
}

bool ai::is_accessible(const location& loc, const move_map& dstsrc) const
{
	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if(dstsrc.count(adj[n]) > 0) {
			return true;
		}
	}

	return dstsrc.count(loc) > 0;
}


const std::set<gamemap::location>& ai::keeps()
{
	if(keeps_.empty()) {
		// Generate the list of keeps:
		// iterate over the entire map and find all keeps.
		for(size_t x = 0; x != size_t(map_.w()); ++x) {
			for(size_t y = 0; y != size_t(map_.h()); ++y) {
				const gamemap::location loc(x,y);
				if(map_.is_keep(loc)) {
					gamemap::location adj[6];
					get_adjacent_tiles(loc,adj);
					for(size_t n = 0; n != 6; ++n) {
						if(map_.is_castle(adj[n])) {
							keeps_.insert(loc);
							break;
						}
					}
				}
			}
		}
	}

	return keeps_;
}

const gamemap::location& ai::nearest_keep(const gamemap::location& loc)
{
	const std::set<gamemap::location>& keeps = this->keeps();
	if(keeps.empty()) {
		static const gamemap::location dummy;
		return dummy;
	}

	const gamemap::location* res = NULL;
	int closest = -1;
	for(std::set<gamemap::location>::const_iterator i = keeps.begin(); i != keeps.end(); ++i) {
		const int distance = distance_between(*i,loc);
		if(res == NULL || distance < closest) {
			closest = distance;
			res = &*i;
		}
	}

	return *res;
}

const std::set<gamemap::location>& ai::avoided_locations()
{
	if(avoid_.empty()) {
		const config::child_list& avoids = current_team().ai_parameters().get_children("avoid");
		for(config::child_list::const_iterator a = avoids.begin(); a != avoids.end(); ++a) {

			const std::vector<location>& locs = parse_location_range((**a)["x"],(**a)["y"]);
			for(std::vector<location>::const_iterator i = locs.begin(); i != locs.end(); ++i) {
				avoid_.insert(*i);
			}
		}

		if(avoid_.empty()) {
			avoid_.insert(location());
		}
	}

	return avoid_;
}

int ai::attack_depth()
{
	if(attack_depth_ > 0) {
		return attack_depth_;
	}

	const config& parms = current_team().ai_parameters();
	attack_depth_ = maximum<int>(1,lexical_cast_default<int>(parms["attack_depth"],5));
	return attack_depth_;
}

namespace {
template<typename Container>
variant villages_from_set(const Container& villages,
				          const std::set<gamemap::location>* exclude=NULL) {
	std::vector<variant> vars;
	foreach(const gamemap::location& loc, villages) {
		if(exclude && exclude->count(loc)) {
			continue;
		}
		vars.push_back(variant(new location_callable(loc)));
	}

	return variant(&vars);
}
}

variant ai_interface::get_value(const std::string& key) const
{
	if(key == "turn") {
		return variant(get_info().state.turn());
	} else if(key == "units") {
		std::vector<variant> vars;
		for(unit_map::const_iterator i = info_.units.begin(); i != info_.units.end(); ++i) {
			vars.push_back(variant(new unit_callable(*i, info_.teams[info_.team_num-1], info_.team_num)));
		}
		return variant(&vars);
	} else if(key == "my_units") {
		std::vector<variant> vars;
		for(unit_map::const_iterator i = info_.units.begin(); i != info_.units.end(); ++i) {
			if(i->second.side() == info_.team_num) {
				vars.push_back(variant(new unit_callable(*i, info_.teams[info_.team_num-1], info_.team_num)));
			}
		}
		return variant(&vars);
	} else if(key == "enemy_units") {
		std::vector<variant> vars;
		for(unit_map::const_iterator i = info_.units.begin(); i != info_.units.end(); ++i) {
			if(info_.teams[info_.team_num-1].is_enemy(i->second.side())) {
				vars.push_back(variant(new unit_callable(*i, info_.teams[info_.team_num-1], info_.team_num)));
			}
		}
		return variant(&vars);
	} else if(key == "villages") {
		return villages_from_set(info_.map.villages());
	} else if(key == "my_villages") {
		return villages_from_set(current_team().villages());
	} else if(key == "enemy_and_unowned_villages") {
		return villages_from_set(info_.map.villages(), &current_team().villages());
	} else if(key == "map") {
		return variant(new gamemap_callable(info_.map));
	}
	return variant();
}

void ai_interface::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("turn", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_units", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("villages", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_villages", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_and_unowned_villages", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("map", FORMULA_READ_ONLY));
}

variant ai::attack_analysis::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "target") {
		return variant(new location_callable(target));
	} else if(key == "movements") {
		std::vector<variant> res;
		for(int n = 0; n != movements.size(); ++n) {
			map_formula_callable* item = new map_formula_callable(NULL);
			item->add("src", variant(new location_callable(movements[n].first)));
			item->add("dst", variant(new location_callable(movements[n].second)));
			res.push_back(variant(item));
		}

		return variant(&res);
	} else if(key == "units") {
		std::vector<variant> res;
		for(int n = 0; n != movements.size(); ++n) {
			res.push_back(variant(new location_callable(movements[n].first)));
		}

		return variant(&res);
	} else if(key == "target_value") {
		return variant(static_cast<int>(target_value*1000));
	} else if(key == "avg_losses") {
		return variant(static_cast<int>(avg_losses*1000));
	} else if(key == "chance_to_kill") {
		return variant(static_cast<int>(chance_to_kill*100));
	} else if(key == "avg_damage_inflicted") {
		return variant(static_cast<int>(avg_damage_inflicted));
	} else if(key == "target_starting_damage") {
		return variant(target_starting_damage);
	} else if(key == "avg_damage_taken") {
		return variant(static_cast<int>(avg_damage_taken));
	} else if(key == "resources_used") {
		return variant(static_cast<int>(resources_used));
	} else if(key == "terrain_quality") {
		return variant(static_cast<int>(terrain_quality));
	} else if(key == "alternative_terrain_quality") {
		return variant(static_cast<int>(alternative_terrain_quality));
	} else if(key == "vulnerability") {
		return variant(static_cast<int>(vulnerability));
	} else if(key == "support") {
		return variant(static_cast<int>(support));
	} else if(key == "leader_threat") {
		return variant(leader_threat);
	} else if(key == "uses_leader") {
		return variant(uses_leader);
	} else if(key == "is_surrounded") {
		return variant(is_surrounded);
	} else {
		return variant();
	}
}

void ai::attack_analysis::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using namespace game_logic;
	inputs->push_back(formula_input("target", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("movements", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("units", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("target_value", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_losses", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("chance_to_kill", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_damage_inflicted", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("target_starting_damage", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_damage_taken", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("resources_used", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("terrain_quality", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("alternative_terrain_quality", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("vulnerability", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("support", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("leader_threat", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("uses_leader", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("is_surrounded", FORMULA_READ_ONLY));
}
