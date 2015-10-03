/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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

#include "../actions.hpp"
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
#include "../../terrain_filter.hpp"
#include "../../unit_display.hpp"
#include "../../wml_exception.hpp"

#include <iterator>
#include <algorithm>
#include <fstream>

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

typedef util::array<map_location,6> adjacent_tiles_array;

idle_ai::idle_ai(readwrite_context &context, const config &cfg)
	: cfg_(cfg), recursion_counter_(context.get_recursion_count())
{
	init_readwrite_context_proxy(context);
}

std::string idle_ai::describe_self() const
{
	return "[idle_ai]";
}


void idle_ai::new_turn()
{
}


void idle_ai::switch_side(side_number side)
{
	set_side(side);
}


config idle_ai::to_config() const
{
	config cfg;
	cfg["ai_algorithm"]= "idle_ai";
	return cfg;
}


int idle_ai::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


void idle_ai::play_turn()
{
	game_events::fire("ai turn");
}



/** Sample ai, with simple strategy. */
class sample_ai : public readwrite_context_proxy, public interface {
public:
	sample_ai(readwrite_context &context, const config &cfg)
		: cfg_(cfg), recursion_counter_(context.get_recursion_count()) {
		init_readwrite_context_proxy(context);
	}

	virtual void play_turn() {
		game_events::fire("ai turn");
		do_attacks();
		get_villages();
		do_moves();
		do_recruitment();
	}

	virtual std::string describe_self() const
	{
		return "[sample_ai]";
	}

	virtual int get_recursion_count() const
	{
		return recursion_counter_.get_count();
	}


protected:
	void do_attacks() {
		std::map<map_location,pathfind::paths> possible_moves;
		move_map srcdst, dstsrc;
		calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

		for(unit_map::const_iterator i = get_info().units.begin(); i != get_info().units.end(); ++i) {
			if(current_team().is_enemy(i->second.side())) {
				map_location adjacent_tiles[6];
				get_adjacent_tiles(i->first,adjacent_tiles);

				int best_defense = -1;
				std::pair<map_location,map_location> best_movement;

				for(size_t n = 0; n != 6; ++n) {
					typedef move_map::const_iterator Itor;
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
					bool gamestate_changed = false;
					bool do_attack = false;
					if (best_movement.first!=best_movement.second) {
						move_result_ptr move_res = execute_move_action(best_movement.second,best_movement.first,true);
						gamestate_changed |= move_res->is_gamestate_changed();
						if (move_res->is_ok()) {
							do_attack = true;
						} else {
							LOG_AI << "move_failed" << std::endl;
						}
					} else {
						do_attack = true;
					}
					if (do_attack) {
						attack_result_ptr attack_res = execute_attack_action(best_movement.first,i->first,-1);
						gamestate_changed |= attack_res->is_gamestate_changed();
						if (!attack_res->is_ok()){
							LOG_AI << "attack failed" << std::endl;
						}
					}

					if (gamestate_changed) {
						do_attacks();
					}
					return;
				}
			}
		}
	}

	void get_villages() {
        std::map<map_location,pathfind::paths> possible_moves;
        move_map srcdst, dstsrc;
        calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

        for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
            if(get_info().map.is_village(i->first) && current_team().owns_village(i->first) == false) {
                move_result_ptr move_res = execute_move_action(i->second,i->first,true);
		if (!move_res->is_ok()) {
			LOG_AI << "move failed!" << std::endl;
		}
		if (move_res->is_gamestate_changed()) {
			get_villages();
		}
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

		std::map<map_location,pathfind::paths> possible_moves;
		move_map srcdst, dstsrc;
		calculate_possible_moves(possible_moves,srcdst,dstsrc,false);

		int closest_distance = -1;
		std::pair<map_location,map_location> closest_move;

		for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
			const int distance = distance_between(i->first,leader->first);
			if(closest_distance == -1 || distance < closest_distance) {
				closest_distance = distance;
				closest_move = *i;
			}
		}

		if(closest_distance != -1) {
			move_result_ptr move_ptr = execute_move_action(closest_move.second,closest_move.first,true);
			if (!move_ptr->is_ok()) {
				LOG_AI << "move failed!" << std::endl;
			}
			if (move_ptr->is_gamestate_changed()) {
				do_moves();
			}
		}
	}

	void switch_side(side_number side){
		set_side(side);
	}


	config to_config() const
	{
		return config();
	}


	bool do_recruitment() {
		const std::set<std::string>& options = current_team().recruits();
		if (!options.empty()) {
			const int choice = (rand()%options.size());
		        std::set<std::string>::const_iterator i = options.begin();
		        std::advance(i,choice);

			recruit_result_ptr recruit_res = execute_recruit_action(*i);
			if (!recruit_res->is_ok()) {
				LOG_AI << "recruitment failed!" << std::endl;
			}
			if (recruit_res->is_gamestate_changed()) {
				return do_recruitment();
			}
			return true;
		}
		return false;
	}
private:
	const config &cfg_;
	recursion_counter recursion_counter_;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif


ai_default::ai_default(ai_context &context, const config &cfg) :
	game_logic::formula_callable(),
	cfg_(cfg),
	recursion_counter_(context.get_recursion_count()),
	leader(),
	threats_found_(false),
	disp_(context.get_info().disp),
	map_(context.get_info().map),
	units_(context.get_info().units),
	teams_(context.get_info().teams),
	tod_manager_(context.get_info().tod_manager_),
	consider_combat_(true),
	recruiting_preferred_(0),
	formula_ai_(),
	formula_ai_ptr_()
{
	add_ref();
	init_ai_context_proxy(context);
}

ai_default::~ai_default(){
}

void ai_default::switch_side(side_number side){
	set_side(side);
}


void ai_default::new_turn()
{
	threats_found_ = false;
	consider_combat_ = true;
}

std::string ai_default::describe_self() const
{
	return "[default_ai]";
}

int ai_default::get_recursion_count() const
{
	return recursion_counter_.get_count();
}

config ai_default::to_config() const
{
	return config();
}


ai_default_recruitment_stage::recruit_situation_change_observer::recruit_situation_change_observer()
	: valid_(false)
{
	manager::add_recruit_list_changed_observer(this);
	manager::add_turn_started_observer(this);
}


void ai_default_recruitment_stage::recruit_situation_change_observer::handle_generic_event(const std::string &/*event_name*/)
{
	valid_ = false;
}


ai_default_recruitment_stage::recruit_situation_change_observer::~recruit_situation_change_observer()
{
	manager::remove_recruit_list_changed_observer(this);
	manager::remove_turn_started_observer(this);
}


bool ai_default_recruitment_stage::recruit_situation_change_observer::get_valid()
{
	return valid_;
}


void ai_default_recruitment_stage::recruit_situation_change_observer::set_valid(bool valid)
{
	valid_ = valid;
}


void ai_default_recruitment_stage::on_create() {
	stage::on_create();
	BOOST_FOREACH (const config &c, cfg_.child_range("limit")) {
		if (c.has_attribute("type") && c.has_attribute("max") ) {
			maximum_counts_.insert(std::make_pair(c["type"],lexical_cast_default<int>(c["max"],0)));
		}
	}
}

config ai_default_recruitment_stage::to_config() const
{
	config cfg = stage::to_config();
	for (std::map<std::string,int>::const_iterator i = maximum_counts_.begin(); i!= maximum_counts_.end(); ++i) {
		config lim;
		lim["type"] = i->first;
		lim["max"] = str_cast(i->second);
		cfg.add_child("limit",lim);
	}
	return cfg;
}


void ai_default_recruitment_stage::analyze_all()
{
	if (!recruit_situation_change_observer_.get_valid()) {
		not_recommended_units_.clear();
		unit_movement_scores_.clear();
		unit_combat_scores_.clear();
		analyze_potential_recruit_movements();
		analyze_potential_recruit_combat();
		recruit_situation_change_observer_.set_valid(true);
	}
}

bool ai_default_recruitment_stage::recruit_usage(const std::string& usage)
{
	raise_user_interact();
	analyze_all();

	const int min_gold = 0;

	log_scope2(log_ai, "recruiting troops");
	LOG_AI << "recruiting '" << usage << "'\n";

	//make sure id, usage and cost are known for the coming evaluation of unit types
	unit_types.build_all(unit_type::HELP_INDEX);

	std::vector<std::string> options;
	bool found = false;
	// Find an available unit that can be recruited,
	// matches the desired usage type, and comes in under budget.
	BOOST_FOREACH (const std::string &name, current_team().recruits())
	{
		const unit_type *ut = unit_types.find(name);
		if (!ut) continue;
		// If usage is empty consider any unit.
		if (usage.empty() || ut->usage() == usage) {
			LOG_AI << name << " considered for " << usage << " recruitment\n";
			found = true;

			if (current_team().gold() - ut->cost() < min_gold) {
				LOG_AI << name << " rejected, cost too high (cost: " << ut->cost() << ", current gold: " << current_team().gold() <<", min_gold: " << min_gold << ")\n";
				continue;
			}

			if (not_recommended_units_.count(name))
			{
				LOG_AI << name << " rejected, bad terrain or combat\n";
				continue;
			}


			std::map<std::string,int>::iterator imc = maximum_counts_.find(name);

			if (imc != maximum_counts_.end()) {
				int count_active = 0;
				for (unit_map::iterator u = get_info().units.begin(); u != get_info().units.end(); ++u) {
					if ((u->second.side()==get_side()) && (!u->second.incapacitated()) && (u->second.type_id() == name)) {
						++count_active;
					}
				}

				if (count_active >= imc->second) {
					LOG_AI << name << " rejected, too many in the field\n";
					continue;
				}
			}

			LOG_AI << "recommending '" << name << "'\n";
			options.push_back(name);
		}
	}

	// From the available options, choose one at random
	if(options.empty() == false) {
		const int option = rand()%options.size();
		recruit_result_ptr recruit_res = check_recruit_action(options[option]);
		if (recruit_res->is_ok()) {
			recruit_res->execute();
			if (!recruit_res->is_ok()) {
				ERR_AI << "recruitment failed "<< std::endl;
			}
		}
		return recruit_res->is_gamestate_changed();
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
		//@fixme
		//return current_team_w().remove_recruitment_pattern_entry(usage);
		return false;
	}
	return false;
}

bool ai_default::multistep_move_possible(const map_location& from,
	const map_location& to, const map_location& via,
	const std::map<map_location,pathfind::paths>& possible_moves) const
{
	const unit_map::const_iterator i = units_.find(from);
	if(i != units_.end()) {
		if(from != via && to != via && units_.count(via) == 0) {
			LOG_AI << "when seeing if leader can move from "
				<< from << " -> " << to
				<< " seeing if can detour to keep at " << via << '\n';
			const std::map<map_location,pathfind::paths>::const_iterator moves = possible_moves.find(from);
			if(moves != possible_moves.end()) {

				LOG_AI << "found leader moves..\n";

				// See if the unit can make it to 'via', and if it can,
				// how much movement it will have left when it gets there.
				pathfind::paths::dest_vect::const_iterator itor =
					moves->second.destinations.find(via);
				if (itor != moves->second.destinations.end())
				{
					LOG_AI << "Can make it to keep with " << itor->move_left << " movement left.\n";
					unit temp_unit(i->second);
					temp_unit.set_movement(itor->move_left);
					const temporary_unit_placer unit_placer(units_,via,temp_unit);
					const pathfind::paths unit_paths(map_,units_,via,teams_,false,false,current_team());

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

map_location ai_default::move_unit(map_location from, map_location to, bool &gamestate_changed)
{
	const unit_map::const_iterator i = units_.find(from);
	if(i != units_.end() && i->second.can_recruit()) {

		// If the leader isn't on its keep, and we can move to the keep
		// and still make our planned movement, then try doing that.
		const map_location& start_pos = nearest_keep(i->first);

		// If we can make it back to the keep and then to our original destination, do so.
		if(multistep_move_possible(from,to,start_pos,get_possible_moves())) {
			move_result_ptr move_to_keep_res = execute_move_action(from,start_pos,false);
			gamestate_changed |= move_to_keep_res->is_gamestate_changed();
			from = move_to_keep_res->get_unit_location();
			if (!move_to_keep_res->is_ok()) {
				LOG_AI << "first part of multistep move (getting to keep) failed" << std::endl;
				return from;
			}

		}

		if (map_.is_keep(from)) {
			gamestate_changed |= do_recruitment();
		}
	}

	move_result_ptr move_to_dst_ptr = check_move_action(from,to,true);
	if (move_to_dst_ptr->is_ok()) {
		move_to_dst_ptr->execute();
		gamestate_changed |= move_to_dst_ptr->is_gamestate_changed();
		if (!move_to_dst_ptr->is_ok()) {
			LOG_AI << "full move failed" << std::endl;
		}

		//ambush ?
		if (move_to_dst_ptr->get_move_spectator().get_ambusher().valid()) {
			attack_result_ptr attack_res = check_attack_action(move_to_dst_ptr->get_unit_location(),move_to_dst_ptr->get_move_spectator().get_ambusher()->first,-1);
			if (attack_res->is_ok()) {
				attack_res->execute();
				gamestate_changed |= attack_res->is_gamestate_changed();
				if (!attack_res->is_ok()) {
					LOG_AI << "attack on the ambusher (after move) failed" << std::endl;
				}
			}
		}
		return move_to_dst_ptr->get_unit_location();
	}
	return from;
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

	const config& parms = cfg_;

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
	BOOST_FOREACH (const config &p, parms.child_range("protect_location"))
	{
		items.push_back(protected_item(
					lexical_cast_default<double>(p["value"], 1.0),
					lexical_cast_default<int>(p["radius"], 20),
					map_location(p, &get_info().game_state_)));
	}

	// Look for directions to protect a unit.
	BOOST_FOREACH (const config &p, parms.child_range("protect_unit"))
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
		//give a learder a chance to fight
		consider_combat_ = true;
		return;
	}
	if (get_number_of_possible_recruits_to_force_recruit()< 0.01f)
	{
		return;
	}

	float free_slots = 0.0f;
	const float gold = static_cast<float>(current_team().gold());
	const float unit_price = static_cast<float>(current_team().average_recruit_price());
	if (map_.is_keep(leader_loc))
	{
		std::set<map_location> checked_hexes;
		checked_hexes.insert(leader_loc);
		free_slots = static_cast<float>(count_free_hexes_in_castle(leader_loc, checked_hexes));
	} else {
		map_location loc = nearest_keep(leader_loc);
               if (units_.find(loc) == units_.end() && gold/unit_price > 1.0f)
               {
                       free_slots -= static_cast<float>(get_number_of_possible_recruits_to_force_recruit());
               }
	}
	recruiting_preferred_ = (gold/unit_price) - free_slots > get_number_of_possible_recruits_to_force_recruit();
	if (recruiting_preferred_==0) {
		//give a learder a chance to fight
		consider_combat_ = true;
	}
	DBG_AI << "recruiting preferred: " << (recruiting_preferred_?"yes":"no") <<
		" units to recruit: " << (gold/unit_price) <<
		" unit_price: " << unit_price <<
		" free slots: " << free_slots <<
		" limit: " << get_number_of_possible_recruits_to_force_recruit() << "\n";
}

void ai_default::do_move()
{
	log_scope2(log_ai, "doing ai move");

	invalidate_defensive_position_cache();

	raise_user_interact();

	typedef std::map<location,pathfind::paths> moves_map;

	const bool passive_leader_shares_keep = get_passive_leader_shares_keep();
	const bool passive_leader = get_passive_leader()||passive_leader_shares_keep;


	unit_map::iterator leader = units_.find_leader(get_side());
	if (leader != units_.end())
	{
		evaluate_recruiting_value(leader->first);
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
		for(move_map::const_iterator i = get_dstsrc().begin(); i != get_dstsrc().end(); ++i) {
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
			move_result_ptr move_ptr = check_move_action(ui->first,closest_move.first);
			if (move_ptr->is_ok()) {
				move_ptr->execute();
				if (!move_ptr->is_ok()) {
					WRN_AI << "'goto' move failed" << std::endl;
				}
			}
		}
	}


	LOG_AI << "combat phase\n";

	if(consider_combat_) {
		LOG_AI << "combat...\n";
		consider_combat_ = do_combat();
		if(consider_combat_) {
			do_move();
			return;
		}
	}

	move_leader_to_goals();

	LOG_AI << "get villages phase\n";

	// Iterator could be invalidated by combat analysis or move_leader_to_goals.
	leader = units_.find_leader(get_side());

	LOG_AI << "villages...\n";
	if(get_villages(get_possible_moves(),get_dstsrc(),get_enemy_dstsrc(),leader)) {
		do_move();
		return;
	}

	LOG_AI << "healing...\n";
	const bool healed_unit = get_healing();
	if(healed_unit) {
		do_move();
		return;
	}

	LOG_AI << "retreat phase\n";

	LOG_AI << "retreating...\n";

	leader = units_.find_leader(get_side());
	const bool retreated_unit = retreat_units(leader);
	if(retreated_unit) {
		do_move();
		return;
	}

	find_threats();

	LOG_AI << "move/targeting phase\n";

	const bool met_invisible_unit = move_to_targets(leader);
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
			move_leader_to_keep();
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
			move_leader_after_recruit();
		}
	}
}

bool ai_default::do_combat()
{

	const std::vector<attack_analysis> &analysis = get_attacks();
	int ticks = SDL_GetTicks();

	const int max_sims = 50000;
	int num_sims = analysis.empty() ? 0 : max_sims/analysis.size();
	if(num_sims < 20)
		num_sims = 20;
	if(num_sims > 40)
		num_sims = 40;

	LOG_AI << "simulations: " << num_sims << "\n";

	const int max_positions = 30000;
	const int skip_num = analysis.size()/max_positions;

	std::vector<attack_analysis>::const_iterator choice_it = analysis.end();
	double choice_rating = -1000.0;
	for(std::vector<attack_analysis>::const_iterator it = analysis.begin();
			it != analysis.end(); ++it) {

		if(skip_num > 0 && ((it - analysis.begin())%skip_num) && it->movements.size() > 1)
			continue;

		const double rating = it->rating(get_aggression(),*this);
		LOG_AI << "attack option rated at " << rating << " ("
			<< get_aggression() << ")\n";

		unit_map::unit_iterator u = units_.find(it->movements[0].first);
		if (!u.valid()) {
			continue;
		}

		if (u->second.attacks().size()==0) {
			continue;
		}

		if (recruiting_preferred_) {
			if (u->second.can_recruit()) {
				LOG_AI << "Not fighting with leader because recruiting is more preferable\n";
				continue;
			}
		}
		if(rating > choice_rating) {
			choice_it = it;
			choice_rating = rating;
		}
	}

	int time_taken = SDL_GetTicks() - ticks;
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

		bool gamestate_changed = false;
		const map_location arrived_at = move_unit(from,to,gamestate_changed);
		if(arrived_at != to || units_.find(to) == units_.end()) {
			WRN_AI << "unit moving to attack has ended up unexpectedly at "
				<< arrived_at << " when moving to " << to << " from " << from << '\n';
			return gamestate_changed;
		}

		attack_result_ptr attack_res = execute_attack_action(to, target_loc, -1);
		gamestate_changed |= attack_res->is_gamestate_changed();
		if (!attack_res->is_ok()) {
			WRN_AI << "attack failed" << std::endl;
		}


		// If this is the only unit in the attack, and the target
		// is still alive, then also summon reinforcements
		if(choice_it->movements.size() == 1 && units_.count(target_loc)) {
			LOG_AI << "found reinforcement target... " << target_loc << "\n";
			//FIXME: sukko raised this value to 5.0.  Is that correct?
			add_target(target(target_loc,3.0,target::BATTLE_AID));
		}

		return gamestate_changed;

	} else {
		return false;
	}
}


bool ai_default::get_healing()
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
			std::pair<Itor,Itor> it = get_srcdst().equal_range(u_it->first);
			double best_vulnerability = 100000.0;
			// Make leader units more unlikely to move to vulnerable villages
			const double leader_penalty = (u.can_recruit()?2.0:1.0);
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const location& dst = it.first->second;
				if(map_.gives_healing(dst) && (units_.find(dst) == units_.end() || dst == u_it->first)) {
					const double vuln = power_projection(it.first->first, get_enemy_dstsrc());
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
				bool gamestate_changed = false;
				unit_map::iterator u = units_.find(move_unit(src,dst,gamestate_changed));
				return gamestate_changed;
			}
		}
	}

	return false;
}

bool ai_default::should_retreat(const map_location& loc, const unit_map::const_iterator& un,
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

bool ai_default::retreat_units(const unit_map::const_iterator& leader)
{
	if (get_caution()<=0) {
		return false;//note: this speeds up the evaluation - per-unit caution is not implemented anyway
	}
	// Get versions of the move map that assume that all units are at full movement
	std::map<map_location,pathfind::paths> dummy_possible_moves;
	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves, fullmove_srcdst, fullmove_dstsrc,
			false, true, &get_avoid());

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
					  get_enemy_dstsrc(), get_caution())) {

				bool can_reach_leader = false;

				// Time to retreat. Look for the place where the power balance
				// is most in our favor.
				// If we can't find anywhere where we like the power balance,
				// just try to get to the best defensive hex.
				typedef move_map::const_iterator Itor;
				std::pair<Itor,Itor> itors = get_srcdst().equal_range(i->first);
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
					const double our_power = power_projection(hex,get_dstsrc());
					const double their_power = power_projection(hex,get_enemy_dstsrc()) * double(defense)/100.0;
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
					bool gamestate_changed = false;
					move_unit(i->first,best_pos,gamestate_changed);
#ifdef SUOKKO
					// FIXME: This was in sukko's r29531 but backed out.
					// Is it correct?
					i->second.remove_movement_ai();
					if (best_rating < 0.0)
						add_target(target(best_pos, -3.0*best_rating, target::SUPPORT));
#endif
					return gamestate_changed;
				}
			}
		}
	}

	return false;
}

class remove_wrong_targets {
public:
	remove_wrong_targets(const readonly_context &context)
		:avoid_(context.get_avoid()), map_(context.get_info().map)
	{
	}

bool operator()(const target &t){
	if (!map_.on_board(t.loc)) {
		DBG_AI << "removing target "<< t.loc << " due to it not on_board" << std::endl;
		return true;
	}

	if (t.value<=0) {
		DBG_AI << "removing target "<< t.loc << " due to value<=0" << std::endl;
		return true;
	}

	if (avoid_.match(t.loc)) {
		DBG_AI << "removing target "<< t.loc << " due to 'avoid' match" << std::endl;
		return true;
	}

	return false;
}
private:
	const terrain_filter &avoid_;
	const gamemap &map_;

};

bool ai_default::move_to_targets(const unit_map::const_iterator& leader)
{
	LOG_AI << "finding targets...\n";
	std::vector<target> targets;
	for(;;) {
		if(targets.empty()) {
			targets = find_targets(leader,get_enemy_dstsrc());
			targets.insert(targets.end(),additional_targets().begin(),
				       additional_targets().end());
			LOG_AI << "Found " << targets.size() << " targets\n";
			if(targets.empty()) {
				return false;
			}
		}

		targets.erase( std::remove_if(targets.begin(),targets.end(),remove_wrong_targets(*this)), targets.end() );

		if(targets.empty()) {
			return false;
		}

		LOG_AI << "choosing move with " << targets.size() << " targets\n";
		std::pair<location,location> move = choose_move(targets, get_srcdst(),
								get_dstsrc(), get_enemy_dstsrc());
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
		bool gamestate_changed = false;
		const map_location arrived_at = move_unit(move.first,move.second,gamestate_changed);

		// We didn't arrive at our intended destination.
		// We return true, meaning that the AI algorithm
		// should be recalculated from the start.
		if(arrived_at != move.second) {
			WRN_AI << "didn't arrive at destination\n";
			return gamestate_changed;
		}
	}

	return false;
}

int ai_default_recruitment_stage::average_resistance_against(const unit_type& a, const unit_type& b) const
{
	int weighting_sum = 0, defense = 0;
	gamemap &map_ = get_info().map;
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

int ai_default_recruitment_stage::compare_unit_types(const unit_type& a, const unit_type& b) const
{
	const int a_effectiveness_vs_b = average_resistance_against(b,a);
	const int b_effectiveness_vs_a = average_resistance_against(a,b);

	LOG_AI << "comparison of '" << a.id() << " vs " << b.id() << ": "
		<< a_effectiveness_vs_b << " - " << b_effectiveness_vs_a << " = "
		<< (a_effectiveness_vs_b - b_effectiveness_vs_a) << '\n';
	return a_effectiveness_vs_b - b_effectiveness_vs_a;
}

void ai_default_recruitment_stage::get_combat_score_vs(const unit_type& ut, const std::string &enemy_type_id, int &score, int &weighting, int hitpoints, int max_hitpoints) const
{
	const unit_type *enemy_info = unit_types.find(enemy_type_id);
	VALIDATE(enemy_info, "Unknown unit type : " + enemy_type_id + " while scoring units.");
	int weight = ut.cost();
	if ((hitpoints>0) && (max_hitpoints>0)) {
		weight = weight * hitpoints / max_hitpoints;
	}

	weighting += weight;
	score += compare_unit_types(ut, *enemy_info) * weight;
}

int ai_default_recruitment_stage::get_combat_score(const unit_type& ut) const
{
	int score = 0, weighting = 0;
	const unit_map & units_ = get_info().units;
	for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {
		if(current_team().is_enemy(j->second.side()) == false) {
			continue;
		}

		if (j->second.can_recruit()) {

			team &enemy_team = get_info().teams[j->second.side()-1];
			const std::set<std::string> &recruits = enemy_team.recruits();
			BOOST_FOREACH (const std::string &rec, recruits) {
				get_combat_score_vs(ut,rec,score,weighting,0,0);
			}
			continue;
		}

		unit const &un = j->second;
		get_combat_score_vs(ut,un.type_id(),score,weighting,un.hitpoints(),un.max_hitpoints());
	}

	if(weighting != 0) {
		score /= weighting;
	}
	return score;
}

void ai_default_recruitment_stage::analyze_potential_recruit_combat()
{
	if(unit_combat_scores_.empty() == false ||
			get_recruitment_ignore_bad_combat()) {
		return;
	}

	log_scope2(log_ai, "analyze_potential_recruit_combat()");

	// Records the best combat analysis for each usage type.
	best_usage_.clear();

	const std::set<std::string>& recruits = current_team().recruits();
	std::set<std::string>::const_iterator i;
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const unit_type *info = unit_types.find(*i);
		if (!info || not_recommended_units_.count(*i)) {
			continue;
		}

		int score = get_combat_score(*info);
		LOG_AI << "combat score of '" << *i << "': " << score << "\n";
		unit_combat_scores_[*i] = score;

		if(best_usage_.count(info->usage()) == 0 ||
				score > best_usage_[info->usage()]) {
			best_usage_[info->usage()] = score;
		}
	}

	// Recommend not to use units of a certain usage type
	// if they have a score more than 600 below
	// the best unit of that usage type.
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const unit_type *info = unit_types.find(*i);
		if (!info || not_recommended_units_.count(*i)) {
			continue;
		}

		if(unit_combat_scores_[*i] + 600 < best_usage_[info->usage()]) {
			LOG_AI << "recommending not to use '" << *i << "' because of poor combat performance "
				<< unit_combat_scores_[*i] << "/" << best_usage_[info->usage()] << "\n";
			not_recommended_units_.insert(*i);
		}
	}
}

namespace {

struct target_comparer_distance {
	target_comparer_distance(const map_location& loc) : loc_(loc) {}

	bool operator()(const ai::target& a, const ai::target& b) const {
		return distance_between(a.loc,loc_) < distance_between(b.loc,loc_);
	}

private:
	map_location loc_;
};

}


ai_default_recruitment_stage::ai_default_recruitment_stage(ai_context &context, const config &cfg)
	: stage(context,cfg),
	  best_usage_(),
	  cfg_(cfg),
	  maximum_counts_(),
	  not_recommended_units_(),
	  recall_list_scores_(),
	  recruit_situation_change_observer_(),
	  unit_combat_scores_(),
	  unit_movement_scores_()

{
}


ai_default_recruitment_stage::~ai_default_recruitment_stage()
{
}

void ai_default_recruitment_stage::analyze_potential_recruit_movements()
{
	unit_map &units_ = get_info().units;
	gamemap &map_ = get_info().map;

	if(unit_movement_scores_.empty() == false ||
			get_recruitment_ignore_bad_movement()) {
		return;
	}

	const unit_map::const_iterator leader = units_.find_leader(get_side());
	if(leader == units_.end()) {
		return;
	}

	const map_location& start = nearest_keep(leader->first);
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
		const unit_type *info = unit_types.find(*i);
		if (!info) {
			continue;
		}

		const unit_type &ut = *info;
		//@todo 1.9: we give max movement, but recruited will get 0? Seems inaccurate
		//but keep it like that for now
		// pathfinding ignoring other units and terrain defense
		const pathfind::move_type_path_calculator calc(ut.movement_type(), ut.movement(), ut.movement(), current_team(),map_);

		int cost = 0;
		int targets_reached = 0;
		int targets_missed = 0;

		for(std::vector<target>::const_iterator t = targets.begin(); t != targets.end(); ++t) {
			LOG_AI << "analyzing '" << *i << "' getting to target...\n";
			pathfind::plain_route route = a_star_search(start, t->loc, 100.0, &calc,
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

			const std::map<std::string,int>::const_iterator current_best = best_scores.find(ut.usage());
			if(current_best == best_scores.end() || score < current_best->second) {
				best_scores[ut.usage()] = score;
			}
		}
	}

	for(std::map<std::string,int>::iterator j = unit_movement_scores_.begin();
			j != unit_movement_scores_.end(); ++j) {

		const unit_type *info = unit_types.find(j->first);

		if (!info) {
			continue;
		}

		const int best_score = best_scores[info->usage()];
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
	raise_user_interact();
	stage_ptr r = get_recruitment(*this);
	if (r) {
		return r->play_stage();
	}
	ERR_AI << "no recruitment aspect - skipping recruitment and recall" << std::endl;
	return false;
}


std::string ai_default_recruitment_stage::find_suitable_recall_id()
{
	if (recall_list_scores_.empty()) {
		return "";
	}
	std::string best_id = recall_list_scores_.back().first;
	recall_list_scores_.pop_back();
	return best_id;
}

class unit_combat_score_getter {
public:
	unit_combat_score_getter(const ai_default_recruitment_stage &s)
		: stage_(s)
	{
	}
	std::pair<std::string, double> operator()(const unit &u) {
		std::pair<std::string,int> p;
		p.first = u.id();
		const unit_type* u_type = u.type();
		assert(u_type!=NULL);

		double xp_ratio = 0;
		if (u.can_advance() && (u.max_experience()>0)) {
			xp_ratio = u.experience()/u.max_experience();
		}

		p.second = (1-xp_ratio) * stage_.get_combat_score(*u_type);
		double recall_cost = game_config::recall_cost != 0 ? game_config::recall_cost : 1;

		p.second *= static_cast<double>(u_type->cost())/recall_cost;
		if (u.can_advance() && (xp_ratio>0) ) {
		        double best_combat_score_of_advancement = 0;
			bool best_combat_score_of_advancement_found = false;
			int best_cost = recall_cost;
			BOOST_FOREACH (const std::string &i, u.advances_to()) {
				const unit_type *ut = unit_types.find(i);
				if (!ut) {
					continue;
				}

				int combat_score_of_advancement = stage_.get_combat_score(*ut);
				if (!best_combat_score_of_advancement_found || (best_combat_score_of_advancement<combat_score_of_advancement)) {
					best_combat_score_of_advancement = combat_score_of_advancement;
					best_combat_score_of_advancement_found = true;
					best_cost = ut->cost();
				}

			}
			p.second += xp_ratio*best_combat_score_of_advancement*best_cost/recall_cost;
		}

		return p;

	}
private:
	const ai_default_recruitment_stage &stage_;
};



class bad_recalls_remover {
public:
	bad_recalls_remover(const std::map<std::string, int>& unit_combat_scores)
		: allow_any_(false), best_combat_score_()
	{
		std::map<std::string, int>::const_iterator cs = std::min_element(unit_combat_scores.begin(),unit_combat_scores.end(),unit_combat_scores.value_comp());

		if (cs == unit_combat_scores.end()) {
			allow_any_ = true;
		} else {
			best_combat_score_ = cs->second;
		}
	}
	bool operator()(const std::pair<std::string,double>& p) {
		if (allow_any_) {
			return false;
		}
		if (p.second>=best_combat_score_) {
			return false;
		}
		return true;
	}

private:
	bool allow_any_;
	double best_combat_score_;
};


class combat_score_less {
public:
	bool operator()(const std::pair<std::string,double> &s1, const std::pair<std::string,double> &s2)
	{
		return s1.second<s2.second;
	}
};

bool ai_default_recruitment_stage::analyze_recall_list()
{
	if (current_team().gold() < game_config::recall_cost ) {
		return false;
	}

	const std::vector<unit> &recalls = current_team().recall_list();

	if (recalls.empty()) {
		return false;
	}

	std::transform(recalls.begin(), recalls.end(), std::back_inserter< std::vector <std::pair<std::string,double> > > (recall_list_scores_), unit_combat_score_getter(*this) );

	if (!lg::debug.dont_log(log_ai)) {
		std::stringstream s;
		s << "Recall list (after scoring):"<< std::endl;
		for (std::vector< std::pair<std::string,double> >::const_iterator p = recall_list_scores_.begin(); p!=recall_list_scores_.end();++p) {
			s << p->first << " ["<<p->second<<"]"<<std::endl;
		}
		DBG_AI << s.str();
	}

	recall_list_scores_.erase( std::remove_if(recall_list_scores_.begin(), recall_list_scores_.end(), bad_recalls_remover(unit_combat_scores_)), recall_list_scores_.end() );

	if (!lg::debug.dont_log(log_ai)) {
		std::stringstream s;
		s << "Recall list, after erase:"<< std::endl;
		for (std::vector< std::pair<std::string,double> >::const_iterator p = recall_list_scores_.begin(); p!=recall_list_scores_.end();++p) {
			s << p->first << " ["<<p->second<<"]"<<std::endl;
		}
		DBG_AI << s.str();
	}

	sort(recall_list_scores_.begin(),recall_list_scores_.end(),combat_score_less());

	if (!lg::debug.dont_log(log_ai)) {
		std::stringstream s;
		s << "Recall list, after sort (worst to best):"<< std::endl;
		for (std::vector< std::pair<std::string,double> >::const_iterator p = recall_list_scores_.begin(); p!=recall_list_scores_.end();++p) {
			s << p->first << " ["<<p->second<<"]"<<std::endl;
		}
		DBG_AI << s.str();
	}
	return !(recall_list_scores_.empty());
}


bool ai_default_recruitment_stage::do_play_stage()
{
	const unit_map &units_ = get_info().units;
	const unit_map::const_iterator leader = units_.find_leader(get_side());
	if(leader == units_.end()) {
		return false;
	}

	const map_location& start_pos = nearest_keep(leader->first);

	analyze_all();

	//handle recalls
	//if there any recalls left which have a better combat score/cost ratio, get them
	bool gamestate_changed = false;
	std::string id;
	if (analyze_recall_list()) {
		while ( !(id = find_suitable_recall_id()).empty() ) {

			recall_result_ptr recall_res = check_recall_action(id);
			if (recall_res->is_ok()) {
				recall_res->execute();
				if (!recall_res->is_ok()) {
					ERR_AI << "recall failed "<< std::endl;
					break;
				}
			}
			gamestate_changed |= recall_res->is_gamestate_changed();

		}
	}

	std::vector<std::string> options = get_recruitment_pattern();
	if (std::count(options.begin(), options.end(), "scout") > 0) {
		size_t neutral_villages = 0;

		// We recruit the initial allocation of scouts
		// based on how many neutral villages there are
		// that are closer to us than to other keeps.
		const std::vector<map_location>& villages = get_info().map.villages();
		for(std::vector<map_location>::const_iterator v = villages.begin(); v != villages.end(); ++v) {
			const int owner = village_owner(*v,get_info().teams);
			if(owner == -1) {
				const size_t distance = distance_between(start_pos,*v);

				bool closest = true;
				for(std::vector<team>::const_iterator i = get_info().teams.begin(); i != get_info().teams.end(); ++i) {
					const int index = i - get_info().teams.begin() + 1;
					const map_location& loc = get_info().map.starting_position(index);
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
		const int villages_per_scout = get_villages_per_scout()/2;

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

	while (recruit_usage(options[rand()%options.size()])) {
		gamestate_changed = true;
		options = get_recruitment_pattern();
		if (options.empty()) {
			options.push_back("");
		}
	}

	return gamestate_changed;
}


void ai_default::move_leader_to_goals()
{
	const config &goal = get_leader_goal();

	if (!goal) {
		LOG_AI << "No goal found\n";
		return;
	}

	if (goal.empty()) {
		return;
	}

	const map_location dst(goal, &get_info().game_state_);
	if (!dst.valid()) {
		ERR_AI << "Invalid goal: "<<std::endl<<goal;
		return;
	}

	const unit_map::iterator leader = units_.find_leader(get_side());
	if(leader == units_.end() || leader->second.incapacitated()) {
		WRN_AI << "Leader not found\n";
		return;
	}

	LOG_AI << "Doing recruitment before goals\n";

	do_recruitment();

	pathfind::shortest_path_calculator calc(leader->second, current_team(), units_, teams_, map_);
	pathfind::plain_route route = a_star_search(leader->first, dst, 1000.0, &calc,
			get_info().map.w(), get_info().map.h());
	if(route.steps.empty()) {
		LOG_AI << "route empty";
		return;
	}

	const pathfind::paths leader_paths(map_, units_, leader->first,
			teams_, false, false, current_team());

	std::map<map_location,pathfind::paths> possible_moves;
	possible_moves.insert(std::pair<map_location,pathfind::paths>(leader->first,leader_paths));

	map_location loc;
	BOOST_FOREACH (const map_location &l, route.steps)
	{
		if (leader_paths.destinations.contains(l) &&
		    power_projection(l, get_enemy_dstsrc()) < double(leader->second.hitpoints() / 2))
		{
			loc = l;
		}
	}

	if(loc.valid()) {
		LOG_AI << "Moving leader to goal\n";
		bool gamestate_changed = false;
		move_unit(leader->first,loc,gamestate_changed);
		if (!gamestate_changed) {
			ERR_AI << "side: "<< get_side() << " trying to move leader to goal has failed" << std::endl;
		}
	}
}

void ai_default::move_leader_after_recruit()
{

	unit_map::iterator leader = units_.find_leader(get_side());
	if(leader == units_.end() || leader->second.incapacitated() || leader->second.movement_left() == 0) {
		return;
	}

	const bool passive_leader_shares_keep = get_passive_leader_shares_keep();
	const bool passive_leader = get_passive_leader()||passive_leader_shares_keep;

	const pathfind::paths leader_paths(map_, units_, leader->first,
			teams_, false, false, current_team());

	std::map<map_location,pathfind::paths> possible_moves;
	possible_moves.insert(std::pair<map_location,pathfind::paths>(leader->first,leader_paths));

	if(!passive_leader && current_team().gold() < 20 && is_accessible(leader->first,get_enemy_dstsrc()) == false) {
		// See if we want to ward any enemy units off from getting our villages.
		for(move_map::const_iterator i = get_enemy_dstsrc().begin(); i != get_enemy_dstsrc().end(); ++i) {

			// If this is a village of ours, that an enemy can capture
			// on their turn, and which we might be able to reach in two turns.
			if(map_.is_village(i->first) && current_team().owns_village(i->first) &&
				int(distance_between(i->first,leader->first)) <= leader->second.total_movement()*2) {

				int current_distance = distance_between(i->first,leader->first);
				location current_loc;

				BOOST_FOREACH (const pathfind::paths::step &dest, leader_paths.destinations)
				{
					const int distance = distance_between(i->first, dest.curr);
					if (distance < current_distance &&
					    !is_accessible(dest.curr, get_enemy_dstsrc()))
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
					const pathfind::paths p(map_, temp_units, current_loc, teams_, false,
					              false, current_team());

					if (p.destinations.contains(i->first))
					{
						bool gamestate_changed = false;
						move_unit(leader->first,current_loc,gamestate_changed);
						if (!gamestate_changed) {
							ERR_AI << "moving leader after recruit failed"<< std::endl;
						}
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

		std::map<location,pathfind::paths> friends_possible_moves;
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
				    !is_accessible(adj[n], get_enemy_dstsrc()))
				{
					move_result_ptr move_res = check_move_action(keep,adj[n],true);
					if (!move_res->is_ok())
					{
						continue;
					}
					bool gamestate_changed = false;
					move_res->execute();
					gamestate_changed |= move_res->is_gamestate_changed();
					if (!gamestate_changed) {
						ERR_AI << "moving leader after recruit failed" << std::endl;
					}
					if (!move_res->is_ok()) {
						return;
					}
				}
			}
		}
	}

	// We didn't move: are we in trouble?
	leader = units_.find_leader(get_side());
	if (!passive_leader && !leader->second.has_moved() && leader->second.attacks_left()) {
		std::map<map_location,pathfind::paths> dummy_possible_moves;
		move_map fullmove_srcdst;
		move_map fullmove_dstsrc;
		calculate_possible_moves(dummy_possible_moves,fullmove_srcdst,fullmove_dstsrc,false,true,&get_avoid());

		if (should_retreat(leader->first, leader, fullmove_srcdst, fullmove_dstsrc, get_enemy_dstsrc(), 0.5)) {
			desperate_attack(leader->first);
		}
	}
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


variant ai_default::get_value(const std::string &/*key*/) const
{
       return variant();
}

void ai_default::get_inputs(std::vector<game_logic::formula_input>* /*inputs*/) const
{
}


variant attack_analysis::get_value(const std::string& key) const
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

void attack_analysis::get_inputs(std::vector<game_logic::formula_input>* inputs) const
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

} //end of namespace ai

