/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/default/ai.cpp
 * Artificial intelligence - The computer commands the enemy.
 */

#include "ai.hpp"

#include "../dfool/ai.hpp"
#include "../manager.hpp"
#include "../formula/ai.hpp"

#include "../../array.hpp"
#include "../../dialogs.hpp"
#include "../../foreach.hpp"
#include "../../game_end_exceptions.hpp"
#include "../../game_events.hpp"
#include "../../game_preferences.hpp"
#include "../../log.hpp"
#include "../../mouse_handler_base.hpp"
#include "../../replay.hpp"
#include "../../statistics.hpp"
#include "../../unit_display.hpp"
#include "../../wml_exception.hpp"

#include <fstream>

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

typedef util::array<map_location,6> adjacent_tiles_array;


idle_ai::idle_ai(ai::readwrite_context &context) : recursion_counter_(context.get_recursion_count())
{
	init_readwrite_context_proxy(context);
}

std::string idle_ai::describe_self()
{
	return "[idle_ai]";
}

void idle_ai::switch_side(ai::side_number side)
{
	set_side(side);
}

int idle_ai::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


void idle_ai::play_turn()
{
	game_events::fire("ai turn");
}


#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

/** Sample ai, with simple strategy. */
class sample_ai : public ai::readwrite_context_proxy, public ai::interface {
public:
	sample_ai(ai::readwrite_context &context)
		: recursion_counter_(context.get_recursion_count()) {
		init_readwrite_context_proxy(context);
	}

	virtual void play_turn() {
		game_events::fire("ai turn");
		do_attacks();
		get_villages();
		do_moves();
		do_recruitment();
	}

	virtual std::string describe_self(){
		return "[sample_ai]";
	}

	virtual int get_recursion_count() const
	{
		return recursion_counter_.get_count();
	}


protected:
	void do_attacks() {
		std::map<map_location,paths> possible_moves;
		ai::move_map srcdst, dstsrc;
		calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

		for(unit_map::const_iterator i = get_info().units.begin(); i != get_info().units.end(); ++i) {
			if(current_team().is_enemy(i->second.side())) {
				map_location adjacent_tiles[6];
				get_adjacent_tiles(i->first,adjacent_tiles);

				int best_defense = -1;
				std::pair<map_location,map_location> best_movement;

				for(size_t n = 0; n != 6; ++n) {
					typedef ai::move_map::const_iterator Itor;
					std::pair<Itor,Itor> range = dstsrc.equal_range(adjacent_tiles[n]);
					while(range.first != range.second) {
						const map_location& dst = range.first->first;
						const map_location& src = range.first->second;
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
									  get_info().units, get_info().state, get_info().tod_manager_,
									  best_movement.first,
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
        std::map<map_location,paths> possible_moves;
        ai::move_map srcdst, dstsrc;
        calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

        for(ai::move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
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

		std::map<map_location,paths> possible_moves;
		ai::move_map srcdst, dstsrc;
		calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

		int closest_distance = -1;
		std::pair<map_location,map_location> closest_move;

		for(ai::move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
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

	void switch_side(ai::side_number side){
		set_side(side);
	}


	bool do_recruitment() {
		const std::set<std::string>& options = current_team().recruits();
		if (!options.empty()) {
			const int choice = (rand()%options.size());
		        std::set<std::string>::const_iterator i = options.begin();
		        std::advance(i,choice);

			const bool res = recruit(*i);
			if(res) {
				return do_recruitment();
			}
			return true;
		}
		return false;
	}
private:
	ai::recursion_counter recursion_counter_;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif


ai_default::ai_default(ai::default_ai_context &context) :
	game_logic::formula_callable(),
	recursion_counter_(context.get_recursion_count()),
	defensive_position_cache_(),
	threats_found_(false),
	attacks_(),
	disp_(context.get_info().disp),
	map_(context.get_info().map),
	units_(context.get_info().units),
	teams_(context.get_info().teams),
	state_(context.get_info().state),
	tod_manager_(context.get_info().tod_manager_),
	consider_combat_(true),
	additional_targets_(),
	unit_movement_scores_(),
	not_recommended_units_(),
	unit_combat_scores_(),
	keeps_(),
	avoid_(),
	unit_stats_cache_(),
	attack_depth_(0),
	recruiting_preferred_(0),
	formula_ai_()
{
	add_ref();
	init_default_ai_context_proxy(context);
}

ai_default::~ai_default(){
}

void ai_default::switch_side(ai::side_number side){
	set_side(side);
}


void ai_default::new_turn()
{
	defensive_position_cache_.clear();
	threats_found_ = false;
	attacks_.clear();
	consider_combat_ = true;
	additional_targets_.clear();
	unit_movement_scores_.clear();
	not_recommended_units_.clear();
	unit_combat_scores_.clear();
	keeps_.clear();
	avoid_.clear();
	unit_stats_cache_.clear();
	attack_depth_ = 0;
	if (formula_ai_ != NULL){
		formula_ai_->new_turn();
	}
	ai::interface::new_turn();
}

std::string ai_default::describe_self(){
	return "[default_ai]";
}

int ai_default::get_recursion_count() const
{
	return recursion_counter_.get_count();
}

bool ai_default::recruit_usage(const std::string& usage)
{
	raise_user_interact();

	const int min_gold = 0;

	log_scope2(log_ai, "recruiting troops");
	LOG_AI << "recruiting '" << usage << "'\n";

	//make sure id, usage and cost are known for the coming evaluation of unit types
	unit_type_data::types().build_all(unit_type::HELP_INDEX);

	std::vector<std::string> options;
	bool found = false;
	// Find an available unit that can be recruited,
	// matches the desired usage type, and comes in under budget.
	const std::set<std::string>& recruits = current_team().recruits();
	for(std::map<std::string,unit_type>::const_iterator i =
	    unit_type_data::types().begin(); i != unit_type_data::types().end(); ++i)
	{
		const std::string& name = i->second.id();
		// If usage is empty consider any unit.
//		DBG_AI << name << " considered\n";
		if (i->second.usage() == usage || usage == "") {
			if (!recruits.count(name)) {
//				DBG_AI << name << " rejected, not in recruitment list\n";
				continue;
			}
			LOG_AI << name << " considered for " << usage << " recruitment\n";
			found = true;

			if (current_team().gold() - i->second.cost() < min_gold) {
				LOG_AI << name << " rejected, cost too high (cost: " << i->second.cost() << ", current gold: " << current_team().gold() <<", min_gold: " << min_gold << ")\n";
				continue;
			}

			if (not_recommended_units_.count(name))
			{
				LOG_AI << name << " rejected, bad terrain or combat\n";
				continue;
			}

			LOG_AI << "recommending '" << name << "'\n";
			options.push_back(name);
		}
	}

	// From the available options, choose one at random
	if(options.empty() == false) {
		const int option = rand()%options.size();
		return recruit(options[option]);
	}
	if (found) {
		LOG_AI << "No available units to recruit that come under the price.\n";
	} else if (usage != "")	{
		//FIXME: This message should be suppressed when WML author
		//chooses the default recruitment pattern.
		const std::string warning = "At difficulty level " +
			get_info().game_state_.classification().difficulty + ", trying to recruit a:" +
			usage + " but no unit of that type (usage=) is"
			" available. Check the recruit and [ai]"
			" recruitment_pattern keys for team '" +
			current_team().name() + "' (" +
			lexical_cast<std::string>(get_side()) + ")"
			" against the usage key of the"
			" units in question! Removing invalid"
			" recruitment_pattern entry and continuing...\n";
		WRN_AI << warning;
		// Uncommented until the recruitment limiting macro can be fixed to not trigger this warning.
		//lg::wml_error << warning;
		return current_team_w().remove_recruitment_pattern_entry(usage);
	}
	return false;
}

bool ai_default::multistep_move_possible(const map_location& from,
	const map_location& to, const map_location& via,
	const std::map<map_location,paths>& possible_moves) const
{
	const unit_map::const_iterator i = units_.find(from);
	if(i != units_.end()) {
		if(from != via && to != via && units_.count(via) == 0) {
			LOG_AI << "when seeing if leader can move from "
				<< from << " -> " << to
				<< " seeing if can detour to keep at " << via << '\n';
			const std::map<map_location,paths>::const_iterator moves = possible_moves.find(from);
			if(moves != possible_moves.end()) {

				LOG_AI << "found leader moves..\n";

				// See if the unit can make it to 'via', and if it can,
				// how much movement it will have left when it gets there.
				paths::dest_vect::const_iterator itor =
					moves->second.destinations.find(via);
				if (itor != moves->second.destinations.end())
				{
					LOG_AI << "Can make it to keep with " << itor->move_left << " movement left.\n";
					unit temp_unit(i->second);
					temp_unit.set_movement(itor->move_left);
					const temporary_unit_placer unit_placer(units_,via,temp_unit);
					const paths unit_paths(map_,units_,via,teams_,false,false,current_team());

					LOG_AI << "Found " << unit_paths.destinations.size() << " moves for temp leader.\n";

					// See if this leader could make it back to the keep.
					if (unit_paths.destinations.contains(to)) {
						LOG_AI << "can make it back to the keep\n";
						return true;
					}
				}
			}
		}
	}

	return false;
}

map_location ai_default::move_unit(map_location from, map_location to, ai::moves_map& possible_moves)
{
	ai::moves_map temp_possible_moves;
	ai::moves_map* possible_moves_ptr = &possible_moves;

	const unit_map::const_iterator i = units_.find(from);
	if(i != units_.end() && i->second.can_recruit()) {

		// If the leader isn't on its keep, and we can move to the keep
		// and still make our planned movement, then try doing that.
		const map_location& start_pos = nearest_keep(i->first);

		// If we can make it back to the keep and then to our original destination, do so.
		if(multistep_move_possible(from,to,start_pos,possible_moves)) {
			from = readwrite_context_proxy::move_unit(from,start_pos,possible_moves);
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

		if (map_.is_keep(from))
			do_recruitment();
	}

	if(units_.count(to) == 0 || from == to) {
		const map_location res = readwrite_context_proxy::move_unit(from,to,*possible_moves_ptr);
		if(res != to) {
			// We've been ambushed; find the ambushing unit and attack them.
			adjacent_tiles_array locs;
			get_adjacent_tiles(res,locs.data());
			for(adjacent_tiles_array::const_iterator adj_i = locs.begin(); adj_i != locs.end(); ++adj_i) {
				const unit_map::const_iterator itor = units_.find(*adj_i);
				if(itor != units_.end() && current_team().is_enemy(itor->second.side()) &&
				   !itor->second.incapacitated()) {
					battle_context bc(map_, teams_, units_, state_, tod_manager_,
									  res, *adj_i, -1, -1, current_team().aggression());
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

bool ai_default::attack_close(const map_location& loc) const
{
	for(std::set<map_location>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		if(distance_between(*i,loc) < 4) {
			return true;
		}
	}

	return false;
}

void ai_default::attack_enemy(const map_location& attacking_unit, const map_location& target,
		int att_weapon, int def_weapon)
{
	attacks_.insert(attacking_unit);
	readwrite_context_proxy::attack_enemy(attacking_unit,target,att_weapon,def_weapon);
}


void ai_default::remove_unit_from_moves(const map_location& loc, move_map& srcdst, move_map& dstsrc)
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

/** A structure for storing an item we're trying to protect. */
struct protected_item {
	protected_item(double value, int radius, const map_location& loc) :
		value(value), radius(radius), loc(loc) {}

	double value;
	int radius;
	map_location loc;
};

}


void ai_default::find_threats()
{
	if(threats_found_) {
		return;
	}

	threats_found_ = true;

	const config& parms = current_team().ai_parameters();

	std::vector<protected_item> items;

	// We want to protect our leader.
	// FIXME: suokko tweaked these from (1.0, 20)->(2.0,15).  Should this have been kept?
	const unit_map::const_iterator leader = units_.find_leader(get_side());
	if(leader != units_.end()) {
		items.push_back(protected_item(
					lexical_cast_default<double>(parms["protect_leader"], 1.0),
					lexical_cast_default<int>(parms["protect_leader_radius"], 20),
					leader->first));
	}

	// Look for directions to protect a specific location.
	foreach (const config &p, parms.child_range("protect_location"))
	{
		items.push_back(protected_item(
					lexical_cast_default<double>(p["value"], 1.0),
					lexical_cast_default<int>(p["radius"], 20),
					map_location(p, &get_info().game_state_)));
	}

	// Look for directions to protect a unit.
	foreach (const config &p, parms.child_range("protect_unit"))
	{
		for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
			if (game_events::unit_matches_filter(u, vconfig(p))) {
				items.push_back(protected_item(
							lexical_cast_default<double>(p["value"], 1.0),
							lexical_cast_default<int>(p["radius"], 20),
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
			if(current_team().is_enemy(u->second.side()) && distance < item.radius
			&& !u->second.invisible(u->first, units_, teams_)) {
				LOG_AI << "found threat target... " << u->first << "\n";
				add_target(target(u->first, item.value * double(item.radius-distance) /
							double(item.radius),target::THREAT));
			}
		}
	}
}

void ai_default::play_turn()
{
	// Protect against a memory over commitment:
	/**
	 * @todo 2.0 Not in the mood to figure out the exact cause:
	 * For some reason -1 hitpoints cause a segmentation fault.
	 * If -1 hitpoints are sent, we crash :/
	 */
	try {
		consider_combat_ = true;
		game_events::fire("ai turn");
		do_move();
	} catch(std::bad_alloc) {
		lg::wml_error << "Memory exhausted - a unit has either a lot of hitpoints or a negative amount.\n";
	}
}

void ai_default::evaluate_recruiting_value(const map_location &leader_loc)
{
	if (recruiting_preferred_ == 2)
	{
		recruiting_preferred_ = 0;
		return;
	}
	if (current_team().num_pos_recruits_to_force()< 0.01f)
	{
		return;
	}

	float free_slots = 0.0f;
	const float gold = current_team().gold();
	const float unit_price = current_team_w().average_recruit_price();
	if (map_.is_keep(leader_loc))
	{
		std::set<map_location> checked_hexes;
		checked_hexes.insert(leader_loc);
		free_slots = count_free_hexes_in_castle(leader_loc, checked_hexes);
	} else {
		map_location loc = nearest_keep(leader_loc);
               if (units_.find(loc) == units_.end() && gold/unit_price > 1.0f)
               {
                       free_slots -= current_team().num_pos_recruits_to_force();
               }
	}
	recruiting_preferred_ = (gold/unit_price) - free_slots > current_team().num_pos_recruits_to_force();
	DBG_AI << "recruiting preferred: " << (recruiting_preferred_?"yes":"no") <<
		" units to recruit: " << (gold/unit_price) <<
		" unit_price: " << unit_price <<
		" free slots: " << free_slots <<
		" limit: " << current_team().num_pos_recruits_to_force() << "\n";
}

void ai_default::do_move()
{
	log_scope2(log_ai, "doing ai move");

	invalidate_defensive_position_cache();

	raise_user_interact();

	typedef std::map<location,paths> moves_map;
	moves_map possible_moves, enemy_possible_moves;

	move_map srcdst, dstsrc, enemy_srcdst, enemy_dstsrc;

	calculate_possible_moves(possible_moves,srcdst,dstsrc,false,false,&avoided_locations());
	calculate_possible_moves(enemy_possible_moves,enemy_srcdst,enemy_dstsrc,true);

	const bool passive_leader_shares_keep = utils::string_bool(current_team().ai_parameters()["passive_leader_shares_keep"],false);
	const bool passive_leader = utils::string_bool(current_team().ai_parameters()["passive_leader"])||passive_leader_shares_keep;


	unit_map::iterator leader = units_.find_leader(get_side());
	if (leader != units_.end())
	{
		evaluate_recruiting_value(leader->first);
		if (passive_leader)
		{
			remove_unit_from_moves(leader->first,srcdst,dstsrc);
		}
	}

	// Execute goto-movements - first collect gotos in a list
	std::vector<map_location> gotos;

	for(unit_map::iterator ui = units_.begin(); ui != units_.end(); ++ui) {
		if(ui->second.get_goto() == ui->first) {
			ui->second.set_goto(map_location());
		} else if(ui->second.side() == get_side() && map_.on_board(ui->second.get_goto())) {
			gotos.push_back(ui->first);
		}
	}

	for(std::vector<map_location>::const_iterator g = gotos.begin(); g != gotos.end(); ++g) {
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
	leader = units_.find_leader(get_side());

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

	leader = units_.find_leader(get_side());
	const bool retreated_unit = retreat_units(possible_moves,srcdst,dstsrc,enemy_dstsrc,leader);
	if(retreated_unit) {
		do_move();
		return;
	}

	if(leader != units_.end()) {
		remove_unit_from_moves(leader->first,srcdst,dstsrc);
	}

	find_threats();

	LOG_AI << "move/targeting phase\n";

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
		if(!passive_leader||passive_leader_shares_keep) {
			map_location before = leader->first;
			move_leader_to_keep(enemy_dstsrc);
			leader = units_.find_leader(get_side());
			if(leader == units_.end()) {
				return;
			}
			// if leaders move is interrupted while we were trying to get to the keep, then do another move , but this next move should not be a recruiting, even if we originally wanted to recruit"
			if (leader->first != before
				&& leader->second.movement_left() > 0
				&& recruiting_preferred_)
			{
				recruiting_preferred_ = 2;
				do_move();
				return;
			}
		}

		if (map_.is_keep(leader->first))
		{
			if (do_recruitment())
			{
				do_move();
				return;
			} else if (recruiting_preferred_){
				recruiting_preferred_ = 2;
				do_move();
				return;
			}
		}

		if(!passive_leader||passive_leader_shares_keep) {
			move_leader_after_recruit(srcdst,dstsrc,enemy_dstsrc);
		}
	}
}

bool ai_default::do_combat(std::map<map_location,paths>& possible_moves, const move_map& srcdst,
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

		if (recruiting_preferred_)
		{
			unit_map::unit_iterator u = units_.find(it->movements[0].first);
			if (u != units_.end()
				&& u->second.can_recruit())
			{
				LOG_AI << "Not fighting with leader because recruiting is more preferable\n";
				continue;
			}
		}
		if(rating > choice_rating) {
			choice_it = it;
			choice_rating = rating;
		}
	}

	time_taken = SDL_GetTicks() - ticks;
	LOG_AI << "analysis took " << time_taken << " ticks\n";

	// suokko tested the rating against current_team().caution()
	// Bad mistake -- the AI became extremely reluctant to attack anything.
	// Documenting this in case someone has this bright idea again...*don't*...
	if(choice_rating > 0.0) {
		map_location from   = choice_it->movements[0].first;
		map_location to     = choice_it->movements[0].second;
		map_location target_loc = choice_it->target;

		// Never used:
		//		const unit_map::const_iterator tgt = units_.find(target_loc);

		const map_location arrived_at = move_unit(from,to,possible_moves);
		if(arrived_at != to || units_.find(to) == units_.end()) {
			WRN_AI << "unit moving to attack has ended up unexpectedly at "
				<< arrived_at << " when moving to " << to << " from " << from << '\n';
			return true;
		}

		// Recalc appropriate weapons here: AI uses approximations.
		battle_context bc(map_, teams_, units_, state_, tod_manager_,
						  to, target_loc, -1, -1,
						  current_team().aggression());
		attack_enemy(to, target_loc, bc.get_attacker_stats().attack_num,
				bc.get_defender_stats().attack_num);

		// If this is the only unit in the attack, and the target
		// is still alive, then also summon reinforcements
		if(choice_it->movements.size() == 1 && units_.count(target_loc)) {
			LOG_AI << "found reinforcement target... " << target_loc << "\n";
			//FIXME: sukko raised this value to 5.0.  Is that correct?
			add_target(target(target_loc,3.0,target::BATTLE_AID));
		}

		return true;

	} else {
		return false;
	}
}


bool ai_default::get_healing(std::map<map_location,paths>& possible_moves,
		const move_map& srcdst, const move_map& enemy_dstsrc)
{
	// Find units in need of healing.
	unit_map::iterator u_it = units_.begin();
	for(; u_it != units_.end(); ++u_it) {
		unit& u = u_it->second;

		// If the unit is on our side, has lost as many or more than
		// 1/2 round worth of healing, and doesn't regenerate itself,
		// then try to find a vacant village for it to rest in.
		if(u.side() == get_side() &&
		   (u.max_hitpoints() - u.hitpoints() >= game_config::poison_amount/2
		   || u.get_state(unit::STATE_POISONED)) &&
		    !u.get_ability_bool("regenerate"))
		{
			// Look for the village which is the least vulnerable to enemy attack.
			typedef std::multimap<location,location>::const_iterator Itor;
			std::pair<Itor,Itor> it = srcdst.equal_range(u_it->first);
			double best_vulnerability = 100000.0;
			// Make leader units more unlikely to move to vulnerable villages
			const double leader_penalty = (u.can_recruit()?2.0:1.0);
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const location& dst = it.first->second;
				if(map_.gives_healing(dst) && (units_.find(dst) == units_.end() || dst == u_it->first)) {
					const double vuln = power_projection(it.first->first, enemy_dstsrc);
					LOG_AI << "found village with vulnerability: " << vuln << "\n";
					if(vuln < best_vulnerability) {
						best_vulnerability = vuln;
						best_loc = it.first;
						LOG_AI << "chose village " << dst << '\n';
					}
				}

				++it.first;
			}

			// If we have found an eligible village,
			// and we can move there without expecting to get whacked next turn:
			if(best_loc != it.second && best_vulnerability*leader_penalty < u.hitpoints()) {
				const location& src = best_loc->first;
				const location& dst = best_loc->second;

				LOG_AI << "moving unit to village for healing...\n";

				unit_map::iterator u = units_.find(move_unit(src,dst,possible_moves));
				if (u != units_.end())
					u->second.set_movement(0);
				return true;
			}
		}
	}

	return false;
}

bool ai_default::should_retreat(const map_location& loc, const unit_map::const_iterator un,
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

bool ai_default::retreat_units(std::map<map_location,paths>& possible_moves,
		const move_map& srcdst, const move_map& dstsrc,
		const move_map& enemy_dstsrc, unit_map::const_iterator leader)
{
	// Get versions of the move map that assume that all units are at full movement
	std::map<map_location,paths> dummy_possible_moves;
	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves, fullmove_srcdst, fullmove_dstsrc,
			false, true, &avoided_locations());

	map_location leader_adj[6];
	if(leader != units_.end()) {
		get_adjacent_tiles(leader->first,leader_adj);
	}

	for(unit_map::iterator i = units_.begin(); i != units_.end(); ++i) {
		if(i->second.side() == get_side() &&
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
				map_location best_pos, best_defensive(i->first);
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
					const map_location& hex = itors.first->second;
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
#ifdef SUOKKO
					// FIXME: This was in sukko's r29531 but backed out.
					// Is it correct?
					i->second.set_movement(0);
					if (best_rating < 0.0)
						add_target(target(best_pos, -3.0*best_rating, target::SUPPORT));
#endif
					return true;
				}
			}
		}
	}

	return false;
}

bool ai_default::move_to_targets(std::map<map_location, paths>& possible_moves,
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
			LOG_AI << "Found " << targets.size() << " targets\n";
			if(targets.empty()) {
				return false;
			}
		}

		LOG_AI << "choosing move with " << targets.size() << " targets\n";
		std::pair<location,location> move = choose_move(targets, srcdst,
				dstsrc, enemy_dstsrc);
		LOG_AI << "choose_move ends with " << targets.size() << " targets\n";

		for(std::vector<target>::const_iterator ittg = targets.begin();
				ittg != targets.end(); ++ittg) {
			assert(map_.on_board(ittg->loc));
		}

		if(move.first.valid() == false || move.second.valid() == false) {
			break;
		}

		assert (map_.on_board(move.first)
			&& map_.on_board(move.second));

		LOG_AI << "move: " << move.first << " -> " << move.second << '\n';

		const location arrived_at = move_unit(move.first,move.second,possible_moves);

		// We didn't arrive at our intended destination.
		// We return true, meaning that the AI algorithm
		// should be recalculated from the start.
		if(arrived_at != move.second) {
			WRN_AI << "didn't arrive at destination\n";
			return true;
		}

		const unit_map::const_iterator u_it = units_.find(arrived_at);
		// Event could have done anything: check
		if (u_it == units_.end() || u_it->second.incapacitated()) {
			WRN_AI << "stolen or incapacitated\n";
		} else {
			// FIXME: sukko's r29531 inserted the following line, is it correct?
 			// u_it->second.set_movement(0);
			// Search to see if there are any enemy units next to the tile
			// which really should be attacked now the move is done.
			map_location adj[6];
			get_adjacent_tiles(arrived_at,adj);
			map_location target;
#ifdef SUOKKO
			// FIXME: This code was in sukko's r29531 and was backed out. Correct?
			int selected = -1;
			boost::scoped_ptr<battle_context> bc_sel;

			double harm_weight = 2.0 + current_team().caution();
			if (chosen->type == target::THREAT
			    || chosen->type == target::BATTLE_AID)
			  harm_weight -= 1.0;
			harm_weight =  current_team().aggression() - harm_weight;
#endif

			for(int n = 0; n != 6; ++n) {
				const unit *enemy = get_visible_unit(units_,adj[n], map_, teams_,current_team());

				if (!enemy || !current_team().is_enemy(enemy->side()) || enemy->incapacitated())
					continue;
				// Current behavior is to only make risk-free attacks.
				battle_context bc(map_, teams_, units_, state_, tod_manager_, arrived_at, adj[n], -1, -1, 100.0);
#ifndef SUOKKO
					if (bc.get_defender_stats().damage == 0) {
						attack_enemy(arrived_at, adj[n], bc.get_attacker_stats().attack_num,
								bc.get_defender_stats().attack_num);
						break;
					}
#else
					// FIXME: This code was in sukko's r29531. Correct?
					const double value = (bc.get_defender_combatant().hp_dist[0] - bc.get_attacker_combatant().hp_dist[0]*harm_weight)*u_it->second.max_hitpoints()/2
						+ (bc.get_defender_combatant().average_hp() - bc.get_attacker_combatant().average_hp() * harm_weight);

					if (value > 0.0
						&& (selected == -1
							|| bc_sel->better_attack(bc,harm_weight)))
					{
						// Select attack target
						bc_sel.reset(new battle_context(bc));
						selected = n;
					}
#endif
			}
#ifdef SUOKKO
			// FIXME: This code was in sukko's r29531 and was backed out. Correct?
			if (selected >= 0) {
				attack_enemy(arrived_at, adj[selected], bc_sel->get_attacker_stats().attack_num,
						bc_sel->get_defender_stats().attack_num);
			}
#endif
		}

		// Don't allow any other units to move onto the tile
		// our unit just moved onto
		typedef move_map::iterator Itor;
		std::pair<Itor,Itor> del = dstsrc.equal_range(arrived_at);
		dstsrc.erase(del.first,del.second);
	}

	return false;
}

int ai_default::average_resistance_against(const unit_type& a, const unit_type& b) const
{
	int weighting_sum = 0, defense = 0;
	const std::map<t_translation::t_terrain, size_t>& terrain =
		map_.get_weighted_terrain_frequencies();

	for (std::map<t_translation::t_terrain, size_t>::const_iterator j = terrain.begin(),
	     j_end = terrain.end(); j != j_end; ++j)
	{
		// Use only reachable tiles when computing the average defense.
	  if (a.movement_type().movement_cost(map_, j->first) < unit_movement_type::UNREACHABLE) {
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
			resistance = std::max<int>(resistance * 2 - 100, 50);
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
	sum /= std::max<int>(1,std::min<int>(a.hitpoints(),1000)); // avoid values really out of range

	// Catch division by zero here if the attacking unit
	// has zero attacks and/or zero damage.
	// If it has no attack at all, the ai shouldn't prefer
	// that unit anyway.
	if (weight_sum == 0) {
		return sum;
	}
	return sum/weight_sum;
}

int ai_default::compare_unit_types(const unit_type& a, const unit_type& b) const
{
	const int a_effectiveness_vs_b = average_resistance_against(b,a);
	const int b_effectiveness_vs_a = average_resistance_against(a,b);

	LOG_AI << "comparison of '" << a.id() << " vs " << b.id() << ": "
		<< a_effectiveness_vs_b << " - " << b_effectiveness_vs_a << " = "
		<< (a_effectiveness_vs_b - b_effectiveness_vs_a) << '\n';
	return a_effectiveness_vs_b - b_effectiveness_vs_a;
}

void ai_default::analyze_potential_recruit_combat()
{
	if(unit_combat_scores_.empty() == false ||
			utils::string_bool(current_team().ai_parameters()["recruitment_ignore_bad_combat"])) {
		return;
	}

	log_scope2(log_ai, "analyze_potential_recruit_combat()");

	// Records the best combat analysis for each usage type.
	std::map<std::string,int> best_usage;

	const std::set<std::string>& recruits = current_team().recruits();
	std::set<std::string>::const_iterator i;
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const unit_type_data::unit_type_map::const_iterator info = unit_type_data::types().find_unit_type(*i);
		if(info == unit_type_data::types().end() || info->first == "dummy_unit" || not_recommended_units_.count(*i)) {
			continue;
		}

		int score = 0, weighting = 0;

		for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {
			if(j->second.can_recruit() || current_team().is_enemy(j->second.side()) == false) {
				continue;
			}

			unit const &un = j->second;
			const unit_type_data::unit_type_map::const_iterator enemy_info = unit_type_data::types().find_unit_type(un.type_id());
			VALIDATE((enemy_info != unit_type_data::types().end()), "Unknown unit type : " + un.type_id() + " while scoring units.");

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
		const unit_type_data::unit_type_map::const_iterator info = unit_type_data::types().find_unit_type(*i);
		if(info == unit_type_data::types().end() || info->first == "dummy_unit" || not_recommended_units_.count(*i)) {
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
	target_comparer_distance(const map_location& loc) : loc_(loc) {}

	bool operator()(const ai_default::target& a, const ai_default::target& b) const {
		return distance_between(a.loc,loc_) < distance_between(b.loc,loc_);
	}

private:
	map_location loc_;
};

}

void ai_default::analyze_potential_recruit_movements()
{
	if(unit_movement_scores_.empty() == false ||
			utils::string_bool(current_team().ai_parameters()["recruitment_ignore_bad_movement"])) {
		return;
	}

	const unit_map::const_iterator leader = units_.find_leader(get_side());
	if(leader == units_.end()) {
		return;
	}

	const location& start = nearest_keep(leader->first);
	if(map_.on_board(start) == false) {
		return;
	}

	log_scope2(log_ai, "analyze_potential_recruit_movements()");

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
		const unit_type_data::unit_type_map::const_iterator info = unit_type_data::types().find_unit_type(*i);
		if(info == unit_type_data::types().end() || info->first == "dummy_unit") {
			continue;
		}

		const unit temp_unit(&get_info().units,&get_info().map,
				&get_info().state, &get_info().tod_manager_, &get_info().teams, &info->second, get_side());
		// since we now use the ignore_units switch, no need to use a empty unit_map
		// unit_map units;
		// const temporary_unit_placer placer(units,start,temp_unit);

		// pathfinding ignoring other units and terrain defense
		const shortest_path_calculator calc(temp_unit,current_team(),get_info().units,teams_,map_,true,true);

		int cost = 0;
		int targets_reached = 0;
		int targets_missed = 0;

		for(std::vector<target>::const_iterator t = targets.begin(); t != targets.end(); ++t) {
			LOG_AI << "analyzing '" << *i << "' getting to target...\n";
			plain_route route = a_star_search(start, t->loc, 100.0, &calc,
					get_info().map.w(), get_info().map.h());

			if (!route.steps.empty()) {
				LOG_AI << "made it: " << route.move_cost << "\n";
				cost += route.move_cost;
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

		const unit_type_data::unit_type_map::const_iterator info =
			unit_type_data::types().find_unit_type(j->first);

		if(info == unit_type_data::types().end() || info->first == "dummy_unit") {
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

bool ai_default::do_recruitment()
{
	const unit_map::const_iterator leader = units_.find_leader(get_side());
	if(leader == units_.end()) {
		return false;
	}

	raise_user_interact();
	// Let formula ai to do recruiting first
	if (get_recursion_count()<ai::recursion_counter::MAX_COUNTER_VALUE)
	{
		if (!current_team().ai_parameters()["recruitment"].empty()){
			if (!formula_ai_){
				formula_ai_ptr_ = (ai::manager::create_transient_ai(ai::manager::AI_TYPE_FORMULA_AI, this));
				formula_ai_ = static_cast<formula_ai*> (formula_ai_ptr_.get());
			}

			assert(formula_ai_!=NULL);

			if (formula_ai_->do_recruitment()) {
				LOG_AI << "Recruitment done by formula_ai\n";
				return true;
			}
		}
	}

	const location& start_pos = nearest_keep(leader->first);

	analyze_potential_recruit_movements();
	analyze_potential_recruit_combat();

	std::vector<std::string> options = current_team().recruitment_pattern();
	if (std::count(options.begin(), options.end(), "scout") > 0) {
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
					const map_location& loc = map_.starting_position(index);
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
			if(u->second.side() == get_side()) {
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
	}

	// If there is no recruitment_pattern use "" which makes us consider
	// any unit available.
	if (options.empty()) {
		options.push_back("");
	}
	// Buy units as long as we have room and can afford it.
        bool ret = false;
	while (recruit_usage(options[rand()%options.size()])) {
		ret = true;
		options = current_team().recruitment_pattern();
		if (options.empty()) {
			options.push_back("");
		}
	}

	return ret;
}

void ai_default::move_leader_to_goals( const move_map& enemy_dstsrc)
{
	const config &goal = current_team().ai_parameters().child("leader_goal");

	if (!goal) {
		LOG_AI << "No goal found\n";
		return;
	}

	const map_location dst(goal, &get_info().game_state_);
	if (!dst.valid()) {
		ERR_AI << "Invalid goal\n";
		return;
	}

	const unit_map::iterator leader = units_.find_leader(get_side());
	if(leader == units_.end() || leader->second.incapacitated()) {
		WRN_AI << "Leader not found\n";
		return;
	}

	LOG_AI << "Doing recruitment before goals\n";

	do_recruitment();

	shortest_path_calculator calc(leader->second, current_team(), units_, teams_, map_);
	plain_route route = a_star_search(leader->first, dst, 1000.0, &calc,
			get_info().map.w(), get_info().map.h());
	if(route.steps.empty()) {
		LOG_AI << "route empty";
		return;
	}

	const paths leader_paths(map_, units_, leader->first,
			teams_, false, false, current_team());

	std::map<map_location,paths> possible_moves;
	possible_moves.insert(std::pair<map_location,paths>(leader->first,leader_paths));

	map_location loc;
	foreach (const map_location &l, route.steps)
	{
		if (leader_paths.destinations.contains(l) &&
		    power_projection(l, enemy_dstsrc) < double(leader->second.hitpoints() / 2))
		{
			loc = l;
		}
	}

	if(loc.valid()) {
		LOG_AI << "Moving leader to goal\n";
		move_unit(leader->first,loc,possible_moves);
	}
}

void ai_default::move_leader_after_recruit(const move_map& /*srcdst*/,
		const move_map& /*dstsrc*/, const move_map& enemy_dstsrc)
{

	unit_map::iterator leader = units_.find_leader(get_side());
	if(leader == units_.end() || leader->second.incapacitated() || leader->second.movement_left() == 0) {
		return;
	}

	const bool passive_leader_shares_keep = utils::string_bool(current_team().ai_parameters()["passive_leader_shares_keep"],false);
	const bool passive_leader = utils::string_bool(current_team().ai_parameters()["passive_leader"])||passive_leader_shares_keep;

	const paths leader_paths(map_, units_, leader->first,
			teams_, false, false, current_team());

	std::map<map_location,paths> possible_moves;
	possible_moves.insert(std::pair<map_location,paths>(leader->first,leader_paths));

	if(!passive_leader && current_team().gold() < 20 && is_accessible(leader->first,enemy_dstsrc) == false) {
		// See if we want to ward any enemy units off from getting our villages.
		for(move_map::const_iterator i = enemy_dstsrc.begin(); i != enemy_dstsrc.end(); ++i) {

			// If this is a village of ours, that an enemy can capture
			// on their turn, and which we might be able to reach in two turns.
			if(map_.is_village(i->first) && current_team().owns_village(i->first) &&
				int(distance_between(i->first,leader->first)) <= leader->second.total_movement()*2) {

				int current_distance = distance_between(i->first,leader->first);
				location current_loc;

				foreach (const paths::step &dest, leader_paths.destinations)
				{
					const int distance = distance_between(i->first, dest.curr);
					if (distance < current_distance &&
					    !is_accessible(dest.curr, enemy_dstsrc))
					{
						current_distance = distance;
						current_loc = dest.curr;
					}
				}

				// If this location is in range of the village,
				// then we consider moving to it
				if(current_loc.valid()) {
					LOG_AI << "considering movement to " << str_cast(current_loc.x + 1)
						<< "," << str_cast(current_loc.y+1);
					unit_map temp_units;
					temp_units.add(current_loc, leader->second);
					const paths p(map_, temp_units, current_loc, teams_, false,
					              false, current_team());

					if (p.destinations.contains(i->first))
					{
						move_unit(leader->first,current_loc,possible_moves);
						// FIXME: suokko's r29531 included this line
						// leader->second.set_movement(0);
						return;
					}
				}
			}
		}
	}

	// See if any friendly leaders can make it to our keep.
	// If they can, then move off it, so that they can recruit if they want.
	if((!passive_leader || passive_leader_shares_keep) && nearest_keep(leader->first) == leader->first) {
		const location keep = leader->first;
		std::pair<map_location,unit> *temp_leader;

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

		units_.insert(temp_leader);

		if(friend_can_reach_keep) {
			// Find a location for our leader to vacate the keep to
			location adj[6];
			get_adjacent_tiles(keep,adj);
			for(size_t n = 0; n != 6; ++n) {
				// Vacate to the first location found that is on the board,
				// our leader can move to, and no enemies can reach.
				if (map_.on_board(adj[n]) &&
				    leader_paths.destinations.contains(adj[n]) &&
				    !is_accessible(adj[n], enemy_dstsrc))
				{
					if (move_unit(keep,adj[n],possible_moves)!=keep) {
						return;
					}
				}
			}
		}
	}

	// We didn't move: are we in trouble?
	leader = units_.find_leader(get_side());
	if (!passive_leader && !leader->second.has_moved() && leader->second.attacks_left()) {
		std::map<map_location,paths> dummy_possible_moves;
		move_map fullmove_srcdst;
		move_map fullmove_dstsrc;
		calculate_possible_moves(dummy_possible_moves,fullmove_srcdst,fullmove_dstsrc,false,true,&avoided_locations());

		if (should_retreat(leader->first, leader, fullmove_srcdst, fullmove_dstsrc, enemy_dstsrc, 0.5)) {
			desperate_attack(leader->first);
		}
	}
}

bool ai_default::leader_can_reach_keep()
{
	const unit_map::iterator leader = units_.find_leader(get_side());
	if(leader == units_.end() || leader->second.incapacitated()) {
		return false;
	}

	const map_location& start_pos = nearest_keep(leader->first);
	if(start_pos.valid() == false) {
		return false;
	}

	if(leader->first == start_pos) {
		return true;
	}

	// Find where the leader can move
	const paths leader_paths(map_,units_,leader->first,teams_,false,false,current_team());


	return leader_paths.destinations.contains(start_pos);
}

int ai_default::rate_terrain(const unit& u, const map_location& loc)
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
		int owner = village_owner(loc, teams_) + 1;

		if(owner == get_side()) {
			rating += friendly_village_value;
		} else if(owner == 0) {
			rating += neutral_village_value;
		} else {
			rating += enemy_village_value;
		}
	}

	return rating;
}

const ai_default::defensive_position& ai_default::best_defensive_position(const map_location& loc,
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

bool ai_default::is_accessible(const location& loc, const move_map& dstsrc) const
{
	map_location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if(dstsrc.count(adj[n]) > 0) {
			return true;
		}
	}

	return dstsrc.count(loc) > 0;
}


const std::set<map_location>& ai_default::keeps()
{
	if(keeps_.empty()) {
		// Generate the list of keeps:
		// iterate over the entire map and find all keeps.
		for(size_t x = 0; x != size_t(map_.w()); ++x) {
			for(size_t y = 0; y != size_t(map_.h()); ++y) {
				const map_location loc(x,y);
				if(map_.is_keep(loc)) {
					map_location adj[6];
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

const map_location& ai_default::nearest_keep(const map_location& loc)
{
	const std::set<map_location>& keeps = this->keeps();
	if(keeps.empty()) {
		static const map_location dummy;
		return dummy;
	}

	const map_location* res = NULL;
	int closest = -1;
	for(std::set<map_location>::const_iterator i = keeps.begin(); i != keeps.end(); ++i) {
		const int distance = distance_between(*i,loc);
		if(res == NULL || distance < closest) {
			closest = distance;
			res = &*i;
		}
	}

	return *res;
}

const std::set<map_location>& ai_default::avoided_locations()
{
	if(avoid_.empty()) {
		foreach (const config &av, current_team().ai_parameters().child_range("avoid"))
		{
			foreach (const location &loc, parse_location_range(av["x"], av["y"])) {
				avoid_.insert(loc);
			}
		}

		if(avoid_.empty()) {
			avoid_.insert(location());
		}
	}

	return avoid_;
}

int ai_default::attack_depth()
{
	if(attack_depth_ > 0) {
		return attack_depth_;
	}

	const config& parms = current_team().ai_parameters();
	attack_depth_ = std::max<int>(1,lexical_cast_default<int>(parms["attack_depth"],5));
	return attack_depth_;
}

variant ai_default::get_value(const std::string& key) const
{
       if(key == "map") {
               return variant(new gamemap_callable(get_info().map));
       }
       return variant();
}

void ai_default::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
       using game_logic::FORMULA_READ_ONLY;
       inputs->push_back(game_logic::formula_input("map", FORMULA_READ_ONLY));
}


variant ai_default::attack_analysis::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "target") {
		return variant(new location_callable(target));
	} else if(key == "movements") {
		std::vector<variant> res;
		for(size_t n = 0; n != movements.size(); ++n) {
			map_formula_callable* item = new map_formula_callable(NULL);
			item->add("src", variant(new location_callable(movements[n].first)));
			item->add("dst", variant(new location_callable(movements[n].second)));
			res.push_back(variant(item));
		}

		return variant(&res);
	} else if(key == "units") {
		std::vector<variant> res;
		for(size_t n = 0; n != movements.size(); ++n) {
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

void ai_default::attack_analysis::get_inputs(std::vector<game_logic::formula_input>* inputs) const
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


