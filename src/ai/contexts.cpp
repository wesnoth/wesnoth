/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Helper functions for the object which operates in the context of AI for specific side
 * This is part of AI interface
 * @file
 */

#include "ai/contexts.hpp"

#include "actions/attack.hpp"

#include "ai/actions.hpp"                  // for actions
#include "ai/composite/aspect.hpp"         // for typesafe_aspect, aspect, etc
#include "ai/composite/engine.hpp"         // for engine, engine_factory, etc
#include "ai/composite/goal.hpp"           // for goal
#include "ai/composite/stage.hpp"       // for ministage
#include "ai/game_info.hpp"             // for aspect_type<>::typesafe_ptr, etc
#include "ai/lua/aspect_advancements.hpp"
#include "ai/manager.hpp"                  // for manager

#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"             // for config, etc
#include "display_chat_manager.hpp"
#include "game_board.hpp"            // for game_board
#include "game_config.hpp"              // for debug
#include "game_display.hpp"          // for game_display
#include "log.hpp"                   // for LOG_STREAM, logger, etc
#include "map/map.hpp"                   // for gamemap
#include "pathfind/pathfind.hpp"        // for paths::dest_vect, paths, etc
#include "recall_list_manager.hpp"   // for recall_list_manager
#include "resources.hpp"             // for units, gameboard, etc
#include "serialization/string_utils.hpp"  // for split, etc
#include "team.hpp"                     // for team
#include "terrain/filter.hpp"  // for terrain_filter
#include "terrain/translation.hpp"      // for terrain_code
#include "time_of_day.hpp"              // for time_of_day
#include "tod_manager.hpp"           // for tod_manager
#include "units/unit.hpp"                  // for unit
#include "units/map.hpp"  // for unit_map::iterator_base, etc
#include "units/ptr.hpp"                 // for unit_ptr
#include "units/types.hpp"  // for attack_type, unit_type, etc
#include "formula/variant.hpp"                  // for variant

#include <algorithm>                    // for find, count, max, fill_n
#include <cmath>                       // for sqrt
#include <cstdlib>                     // for abs
#include <ctime>                       // for time
#include <iterator>                     // for back_inserter
#include <ostream>                      // for operator<<, basic_ostream, etc

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

// =======================================================================
//
// =======================================================================
namespace ai {

int side_context_impl::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


int readonly_context_impl::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


int readwrite_context_impl::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


void readonly_context_impl::raise_user_interact() const
{
	manager::raise_user_interact();
}


void readwrite_context_impl::raise_gamestate_changed() const
{
	manager::raise_gamestate_changed();
}


team& readwrite_context_impl::current_team_w()
{
	return resources::gameboard->get_team(get_side());
}

attack_result_ptr readwrite_context_impl::execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon){
	unit_map::iterator i = resources::gameboard->units().find(attacker_loc);
	double m_aggression = i.valid() && i->can_recruit() ? get_leader_aggression() : get_aggression();
	const unit_advancements_aspect& m_advancements = get_advancements();
	return actions::execute_attack_action(get_side(),true,attacker_loc,defender_loc,attacker_weapon, m_aggression, m_advancements);
}


attack_result_ptr readonly_context_impl::check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon){
	unit_map::iterator i = resources::gameboard->units().find(attacker_loc);
	double m_aggression = i.valid() && i->can_recruit() ? get_leader_aggression() : get_aggression();
	const unit_advancements_aspect& m_advancements = get_advancements();
	return actions::execute_attack_action(get_side(),false,attacker_loc,defender_loc,attacker_weapon, m_aggression, m_advancements);
}


move_result_ptr readwrite_context_impl::execute_move_action(const map_location& from, const map_location& to, bool remove_movement, bool unreach_is_ok){
	return actions::execute_move_action(get_side(),true,from,to,remove_movement,unreach_is_ok);
}


move_result_ptr readonly_context_impl::check_move_action(const map_location& from, const map_location& to, bool remove_movement, bool unreach_is_ok){
	return actions::execute_move_action(get_side(),false,from,to,remove_movement,unreach_is_ok);
}


recall_result_ptr readwrite_context_impl::execute_recall_action(const std::string& id, const map_location &where, const map_location &from){
	return actions::execute_recall_action(get_side(),true,id,where,from);
}


recruit_result_ptr readwrite_context_impl::execute_recruit_action(const std::string& unit_name, const map_location &where, const map_location &from){
	return actions::execute_recruit_action(get_side(),true,unit_name,where,from);
}


recall_result_ptr readonly_context_impl::check_recall_action(const std::string& id, const map_location &where, const map_location &from){
	return actions::execute_recall_action(get_side(),false,id,where,from);
}


recruit_result_ptr readonly_context_impl::check_recruit_action(const std::string& unit_name, const map_location &where, const map_location &from){
	return actions::execute_recruit_action(get_side(),false,unit_name,where,from);
}


stopunit_result_ptr readwrite_context_impl::execute_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	return actions::execute_stopunit_action(get_side(),true,unit_location,remove_movement,remove_attacks);
}


stopunit_result_ptr readonly_context_impl::check_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	return actions::execute_stopunit_action(get_side(),false,unit_location,remove_movement,remove_attacks);
}


synced_command_result_ptr readwrite_context_impl::execute_synced_command_action(const std::string& lua_code, const map_location& location){
	return actions::execute_synced_command_action(get_side(),true,lua_code,location);
}


synced_command_result_ptr readonly_context_impl::check_synced_command_action(const std::string& lua_code, const map_location& location){
	return actions::execute_synced_command_action(get_side(),false,lua_code,location);
}


template<typename T>
void readonly_context_impl::add_known_aspect(const std::string &name, std::shared_ptr< typesafe_aspect <T> > &where)
{
	std::shared_ptr< typesafe_known_aspect <T> > ka_ptr(new typesafe_known_aspect<T>(name,where,aspects_));
	known_aspects_.emplace(name,ka_ptr);
}

readonly_context_impl::readonly_context_impl(side_context &context, const config &cfg)
		: cfg_(cfg),
		engines_(),
		known_aspects_(),
		advancements_(),
		aggression_(),
		attack_depth_(),
		aspects_(),
		attacks_(),
		avoid_(),
		caution_(),
		defensive_position_cache_(),
		dstsrc_(),enemy_dstsrc_(),
		enemy_possible_moves_(),
		enemy_srcdst_(),
		grouping_(),
		goals_(),
		keeps_(),
		leader_aggression_(),
		leader_goal_(),
		leader_ignores_keep_(),
		leader_value_(),
		move_maps_enemy_valid_(false),
		move_maps_valid_(false),
		dst_src_valid_lua_(false),
		dst_src_enemy_valid_lua_(false),
		src_dst_valid_lua_(false),
		src_dst_enemy_valid_lua_(false),
		passive_leader_(),
		passive_leader_shares_keep_(),
		possible_moves_(),
		recruitment_diversity_(),
		recruitment_instructions_(),
		recruitment_more_(),
		recruitment_pattern_(),
		recruitment_randomness_(),
		recruitment_save_gold_(),
		recursion_counter_(context.get_recursion_count()),
		scout_village_targeting_(),
		simple_targeting_(),
		srcdst_(),
		support_villages_(),
		unit_stats_cache_(),
		village_value_(),
		villages_per_scout_()
	{
		init_side_context_proxy(context);
		manager::add_gamestate_observer(this);

		add_known_aspect("advancements", advancements_);
		add_known_aspect("aggression",aggression_);
		add_known_aspect("attack_depth",attack_depth_);
		add_known_aspect("attacks",attacks_);
		add_known_aspect("avoid",avoid_);
		add_known_aspect("caution",caution_);
		add_known_aspect("grouping",grouping_);
		add_known_aspect("leader_aggression",leader_aggression_);
		add_known_aspect("leader_goal",leader_goal_);
		add_known_aspect("leader_ignores_keep",leader_ignores_keep_);
		add_known_aspect("leader_value",leader_value_);
		add_known_aspect("passive_leader",passive_leader_);
		add_known_aspect("passive_leader_shares_keep",passive_leader_shares_keep_);
		add_known_aspect("recruitment_diversity",recruitment_diversity_);
		add_known_aspect("recruitment_instructions",recruitment_instructions_);
		add_known_aspect("recruitment_more",recruitment_more_);
		add_known_aspect("recruitment_pattern",recruitment_pattern_);
		add_known_aspect("recruitment_randomness",recruitment_randomness_);
		add_known_aspect("recruitment_save_gold",recruitment_save_gold_);
		add_known_aspect("scout_village_targeting",scout_village_targeting_);
		add_known_aspect("simple_targeting",simple_targeting_);
		add_known_aspect("support_villages",support_villages_);
		add_known_aspect("village_value",village_value_);
		add_known_aspect("villages_per_scout",villages_per_scout_);
		keeps_.init(resources::gameboard->map());

	}

void readonly_context_impl::on_readonly_context_create() {
	//init the composite ai engines
	for(const config &cfg_element : cfg_.child_range("engine")) {
		engine::parse_engine_from_config(*this,cfg_element,std::back_inserter(engines_));
	}

	// init the composite ai aspects
	for(const config &cfg_element : cfg_.child_range("aspect")) {
		std::vector<aspect_ptr> aspects;
		engine::parse_aspect_from_config(*this,cfg_element,cfg_element["id"],std::back_inserter(aspects));
		add_aspects(aspects);
	}

	// init the composite ai goals
	for(const config &cfg_element : cfg_.child_range("goal")) {
		engine::parse_goal_from_config(*this,cfg_element,std::back_inserter(get_goals()));
	}
}


config side_context_impl::to_side_context_config() const
{
	return config();
}

config readwrite_context_impl::to_readwrite_context_config() const
{
	return config();
}


config readonly_context_impl::to_readonly_context_config() const
{
	config cfg;
	for(const engine_ptr e : engines_) {
		cfg.add_child("engine",e->to_config());
	}
	for(const aspect_map::value_type a : aspects_) {
		cfg.add_child("aspect",a.second->to_config());
	}
	for(const goal_ptr g : goals_) {
		cfg.add_child("goal",g->to_config());
	}
	return cfg;
}

readonly_context_impl::~readonly_context_impl()
{
	manager::remove_gamestate_observer(this);
}

void readonly_context_impl::handle_generic_event(const std::string& /*event_name*/)
{
	invalidate_move_maps();
}


const game_info& readonly_context_impl::get_info() const{
	return manager::get_active_ai_info_for_side(get_side());
}


game_info& readwrite_context_impl::get_info_w(){
	return manager::get_active_ai_info_for_side(get_side());
}

void readonly_context_impl::diagnostic(const std::string& msg)
{
	if(game_config::debug) {
		resources::screen->set_diagnostic(msg);
	}
}


const team& readonly_context_impl::current_team() const
{
	return resources::gameboard->get_team(get_side());
}


void readonly_context_impl::log_message(const std::string& msg)
{
	if(game_config::debug) {
		resources::screen->get_chat_manager().add_chat_message(time(nullptr), "ai", get_side(), msg,
				events::chat_handler::MESSAGE_PUBLIC, false);
	}
}


void readonly_context_impl::calculate_possible_moves(std::map<map_location,pathfind::paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
		const terrain_filter* remove_destinations) const
{
  calculate_moves(resources::gameboard->units(),res,srcdst,dstsrc,enemy,assume_full_movement,remove_destinations);
}

void readonly_context_impl::calculate_moves(const unit_map& units, std::map<map_location,pathfind::paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
		const terrain_filter* remove_destinations,
		bool see_all
          ) const
{

	for(unit_map::const_iterator un_it = units.begin(); un_it != units.end(); ++un_it) {
		// If we are looking for the movement of enemies, then this unit must be an enemy unit.
		// If we are looking for movement of our own units, it must be on our side.
		// If we are assuming full movement, then it may be a unit on our side, or allied.
		if ((enemy && current_team().is_enemy(un_it->side()) == false) ||
		    (!enemy && !assume_full_movement && un_it->side() != get_side()) ||
		    (!enemy && assume_full_movement && current_team().is_enemy(un_it->side()))) {
			continue;
		}
		// Discount incapacitated units
		if (un_it->incapacitated() ||
		    (!assume_full_movement && un_it->movement_left() == 0)) {
			continue;
		}

		// We can't see where invisible enemy units might move.
		if (enemy && un_it->invisible(un_it->get_location(), *resources::gameboard) && !see_all) {
			continue;
		}
		// If it's an enemy unit, reset its moves while we do the calculations.
		const unit_movement_resetter move_resetter(*un_it,enemy || assume_full_movement);

		// Insert the trivial moves of staying on the same map location.
		if (un_it->movement_left() > 0) {
			std::pair<map_location,map_location> trivial_mv(un_it->get_location(), un_it->get_location());
			srcdst.insert(trivial_mv);
			dstsrc.insert(trivial_mv);
		}
		/**
		 * @todo This is where support for a speculative unit map is incomplete.
		 *       There are several places (deep) within the paths constructor
		 *       where resources::gameboard->units() is assumed to be the unit map. Rather
		 *       than introduce a new parameter to numerous functions, a better
		 *       solution may be for the creator of the speculative map (if one
		 *       is used in the future) to cause resources::gameboard->units() to point to
		 *       that map (and restore the "real" pointer when the speculating
		 *       is completed). If that approach is adopted, calculate_moves()
		 *       and calculate_possible_moves() become redundant, and one of
		 *       them should probably be eliminated.
		 */
		res.emplace(un_it->get_location(), pathfind::paths(*un_it, false, true, current_team(), 0, see_all));
	}

	// deactivate terrain filtering if it's just the dummy 'matches nothing'
	static const config only_not_tag("not");
	if(remove_destinations && remove_destinations->to_config() == only_not_tag) {
		remove_destinations = nullptr;
	}

	for(std::map<map_location,pathfind::paths>::iterator m = res.begin(); m != res.end(); ++m) {
		for(const pathfind::paths::step &dest : m->second.destinations)
		{
			const map_location& src = m->first;
			const map_location& dst = dest.curr;

			if(remove_destinations != nullptr && remove_destinations->match(dst)) {
				continue;
			}

			bool friend_owns = false;

			// Don't take friendly villages
			if(!enemy && resources::gameboard->map().is_village(dst)) {
				for(size_t n = 0; n != resources::gameboard->teams().size(); ++n) {
					if(resources::gameboard->teams()[n].owns_village(dst)) {
						int side = n + 1;
						if (get_side() != side && !current_team().is_enemy(side)) {
							friend_owns = true;
						}

						break;
					}
				}
			}

			if(friend_owns) {
				continue;
			}

			if(src != dst && (resources::gameboard->find_visible_unit(dst, current_team()) == resources::gameboard->units().end()) ) {
				srcdst.emplace(src, dst);
				dstsrc.emplace(dst, src);
			}
		}
	}
}


void readonly_context_impl::add_aspects(std::vector< aspect_ptr > &aspects )
{
	for(aspect_ptr a : aspects) {
		const std::string id = a->get_id();
		known_aspect_map::iterator i = known_aspects_.find(id);
		if (i != known_aspects_.end()) {
			i->second->set(a);
		} else {
			ERR_AI << "when adding aspects, unknown aspect id["<<id<<"]"<<std::endl;
		}
	}
}

void readonly_context_impl::add_facet(const std::string &id, const config &cfg) const
{
	known_aspect_map::const_iterator i = known_aspects_.find(id);
	if (i != known_aspects_.end()) {
		i->second->add_facet(cfg);
	} else {
		ERR_AI << "when adding aspects, unknown aspect id["<<id<<"]"<<std::endl;
	}
}

const defensive_position& readonly_context_impl::best_defensive_position(const map_location& loc,
		const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc) const
{
	const unit_map::const_iterator itor = resources::gameboard->units().find(loc);
	if(itor == resources::gameboard->units().end()) {
		static defensive_position pos;
		pos.chance_to_hit = 0;
		pos.vulnerability = pos.support = 0;
		return pos;
	}

	const std::map<map_location,defensive_position>::const_iterator position =
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
		const int defense = itor->defense_modifier(resources::gameboard->map().get_terrain(i->second));
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

	defensive_position_cache_.emplace(loc, pos);
	return defensive_position_cache_[loc];
}


std::map<map_location,defensive_position>& readonly_context_impl::defensive_position_cache() const
{
	return defensive_position_cache_;
}


const unit_advancements_aspect& readonly_context_impl::get_advancements() const
{
	if (advancements_) {
		return advancements_->get();
	}

	static unit_advancements_aspect uaa = unit_advancements_aspect();
	return uaa;
}


double readonly_context_impl::get_aggression() const
{
	if (aggression_) {
		return aggression_->get();
	}
	return 0;
}


int readonly_context_impl::get_attack_depth() const
{
	if (attack_depth_) {
		return std::max<int>(1,attack_depth_->get()); ///@todo 1.9: add validators, such as minmax filters to aspects
	}
	return 1;
}


const aspect_map& readonly_context_impl::get_aspects() const
{
	return aspects_;
}


aspect_map& readonly_context_impl::get_aspects()
{
	return aspects_;
}


const attacks_vector& readonly_context_impl::get_attacks() const
{
	if (attacks_) {
		return attacks_->get();
	}
	static attacks_vector av;
	return av;
}


const wfl::variant& readonly_context_impl::get_attacks_as_variant() const
{
	if (attacks_) {
		return attacks_->get_variant();
	}
	static wfl::variant v;///@todo 1.9: replace with variant::null_variant;
	return v;
}

const terrain_filter& readonly_context_impl::get_avoid() const
{
	if (avoid_) {
		return avoid_->get();
	}
	config cfg;
	cfg.add_child("not");
	static terrain_filter tf(vconfig(cfg, true), resources::filter_con);
	return tf;
}


double readonly_context_impl::get_caution() const
{
	if (caution_) {
		return caution_->get();
	}
	return 0;
}

const move_map& readonly_context_impl::get_dstsrc() const
{
	if (!move_maps_valid_) {
		recalculate_move_maps();
	}
	return dstsrc_;
}


const move_map& readonly_context_impl::get_enemy_dstsrc() const
{
	if (!move_maps_enemy_valid_) {
		recalculate_move_maps_enemy();
	}
	return enemy_dstsrc_;
}


const moves_map& readonly_context_impl::get_enemy_possible_moves() const
{
	if (!move_maps_enemy_valid_) {
		recalculate_move_maps_enemy();
	}
	return enemy_possible_moves_;
}


const move_map& readonly_context_impl::get_enemy_srcdst() const
{
	if (!move_maps_enemy_valid_) {
		recalculate_move_maps_enemy();
	}
	return enemy_srcdst_;
}


engine_ptr readonly_context_impl::get_engine_by_cfg(const config& cfg)
{
	std::string engine_name = cfg["engine"];
	if (engine_name.empty()) {
		engine_name="cpp";//default engine
	}

	std::vector<engine_ptr>::iterator en = engines_.begin();
	while (en!=engines_.end() && ((*en)->get_name()!=engine_name) && ((*en)->get_id()!=engine_name)) {
		++en;
	}

	if (en != engines_.end()){
		return *en;
	}

	//TODO: fix, removing some code duplication
	engine_factory::factory_map::iterator eng = engine_factory::get_list().find(engine_name);
	if (eng == engine_factory::get_list().end()){
		ERR_AI << "side "<<get_side()<<" : UNABLE TO FIND engine["<<
			engine_name <<"]" << std::endl;
		DBG_AI << "config snippet contains: " << std::endl << cfg << std::endl;
		return engine_ptr();
	}

	engine_ptr new_engine = eng->second->get_new_instance(*this,engine_name);
	if (!new_engine) {
		ERR_AI << "side "<<get_side()<<" : UNABLE TO CREATE engine["<<
			engine_name <<"] " << std::endl;
		DBG_AI << "config snippet contains: " << std::endl << cfg << std::endl;
		return engine_ptr();
	}
	engines_.push_back(new_engine);
	return engines_.back();
}


const std::vector<engine_ptr>& readonly_context_impl::get_engines() const
{
	return engines_;
}


std::vector<engine_ptr>& readonly_context_impl::get_engines()
{
	return engines_;
}


std::string readonly_context_impl::get_grouping() const
{
	if (grouping_) {
		return grouping_->get();
	}
	return std::string();
}


const std::vector<goal_ptr>& readonly_context_impl::get_goals() const
{
	return goals_;
}


std::vector<goal_ptr>& readonly_context_impl::get_goals()
{
	return goals_;
}



double readonly_context_impl::get_leader_aggression() const
{
	if (leader_aggression_) {
		return leader_aggression_->get();
	}
	return 0;
}


config readonly_context_impl::get_leader_goal() const
{
	if (leader_goal_) {
		return leader_goal_->get();
	}
	return config();
}


bool readonly_context_impl::get_leader_ignores_keep() const
{
	if (leader_ignores_keep_) {
		return leader_ignores_keep_->get();
	}
	return false;
}


double readonly_context_impl::get_leader_value() const
{
	if (leader_value_) {
		return leader_value_->get();
	}
	return 0;
}


bool readonly_context_impl::get_passive_leader() const
{
	if (passive_leader_) {
		return passive_leader_->get();
	}
	return false;
}


bool readonly_context_impl::get_passive_leader_shares_keep() const
{
	if (passive_leader_shares_keep_) {
		return passive_leader_shares_keep_->get();
	}
	return false;
}


const moves_map& readonly_context_impl::get_possible_moves() const
{
	if (!move_maps_valid_) {
		recalculate_move_maps();
	}
	return possible_moves_;
}


const std::vector<unit_ptr>& readonly_context_impl::get_recall_list() const
{
	///@todo 1.9: check for (level_["disallow_recall"]))
	return current_team().recall_list().recall_list_; //TODO: Refactor ai so that friend of ai context is not required of recall_list_manager at this line
}


double readonly_context_impl::get_recruitment_diversity() const
{
	if (recruitment_diversity_) {
		return recruitment_diversity_->get();
	}
	return 0.;
}


const config readonly_context_impl::get_recruitment_instructions() const
{
	if (recruitment_instructions_) {
		return recruitment_instructions_->get();
	}
	return config();
}


const std::vector<std::string> readonly_context_impl::get_recruitment_more() const
{
	if (recruitment_more_) {
		return recruitment_more_->get();
	}
	return std::vector<std::string>();
}


const std::vector<std::string> readonly_context_impl::get_recruitment_pattern() const
{
	if (recruitment_pattern_) {
		return recruitment_pattern_->get();
	}
	return std::vector<std::string>();
}


int readonly_context_impl::get_recruitment_randomness() const
{
	if (recruitment_randomness_) {
		return recruitment_randomness_->get();
	}
	return 0;
}


const config readonly_context_impl::get_recruitment_save_gold() const
{
	if (recruitment_save_gold_) {
		return recruitment_save_gold_->get();
	}
	return config();
}


double readonly_context_impl::get_scout_village_targeting() const
{
	if (scout_village_targeting_) {
		return scout_village_targeting_->get();
	}
	return 1;
}


bool readonly_context_impl::get_simple_targeting() const
{
	if (simple_targeting_) {
		return simple_targeting_->get();
	}
	return false;
}


const move_map& readonly_context_impl::get_srcdst() const
{
	if (!move_maps_valid_) {
		recalculate_move_maps();
	}
	return srcdst_;
}


bool readonly_context_impl::get_support_villages() const
{
	if (support_villages_) {
		return support_villages_->get();
	}
	return false;
}


double readonly_context_impl::get_village_value() const
{
	if (village_value_) {
		return village_value_->get();
	}
	return 0;
}


int readonly_context_impl::get_villages_per_scout() const
{
	if (villages_per_scout_) {
		return villages_per_scout_->get();
	}
	return 0;
}


bool readonly_context_impl::is_dst_src_valid_lua() const
{
	return dst_src_valid_lua_;
}

bool readonly_context_impl::is_dst_src_enemy_valid_lua() const
{
	return dst_src_enemy_valid_lua_;
}

bool readonly_context_impl::is_src_dst_valid_lua() const
{
	return src_dst_valid_lua_;
}

bool readonly_context_impl::is_src_dst_enemy_valid_lua() const
{
	return src_dst_enemy_valid_lua_;
}

void readonly_context_impl::invalidate_defensive_position_cache() const
{
	defensive_position_cache_.clear();
}


void readonly_context_impl::invalidate_keeps_cache() const
{
	keeps_.clear();
}


void keeps_cache::handle_generic_event(const std::string &/*event_name*/)
{
	clear();
}


void readonly_context_impl::invalidate_move_maps() const
{
	move_maps_valid_ = false;
	move_maps_enemy_valid_ = false;

	dst_src_valid_lua_ = false;
	dst_src_enemy_valid_lua_ = false;

	src_dst_valid_lua_ = false;
	src_dst_enemy_valid_lua_ = false;
}


const std::set<map_location>& readonly_context_impl::keeps() const
{
	return keeps_.get();
}


keeps_cache::keeps_cache()
	: map_(nullptr)
	, keeps_()
{
	ai::manager::add_turn_started_observer(this);
	ai::manager::add_map_changed_observer(this);
}


keeps_cache::~keeps_cache()
{
	ai::manager::remove_turn_started_observer(this);
	ai::manager::remove_map_changed_observer(this);
}

void keeps_cache::clear()
{
	keeps_.clear();
}


void keeps_cache::init(const gamemap &map)
{
	map_ = &map;
}

const std::set<map_location>& keeps_cache::get()
{
	if(keeps_.empty()) {
		// Generate the list of keeps:
		// iterate over the entire map and find all keeps.
		for(int x = 0; x != map_->w(); ++x) {
			for(int y = 0; y != map_->h(); ++y) {
				const map_location loc(x,y);
				if(map_->is_keep(loc)) {
					map_location adj[6];
					get_adjacent_tiles(loc,adj);
					for(size_t n = 0; n != 6; ++n) {
						if(map_->is_castle(adj[n])) {
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


bool readonly_context_impl::leader_can_reach_keep() const
{
	const unit_map::iterator leader = resources::gameboard->units().find_leader(get_side());
	if(leader == resources::gameboard->units().end() || leader->incapacitated()) {
		return false;
	}

	const map_location &start_pos = nearest_keep(leader->get_location());
	if(start_pos.valid() == false) {
		return false;
	}

	if (leader->get_location() == start_pos) {
		return true;
	}

	// Find where the leader can move
	const pathfind::paths leader_paths(*leader, false, true, current_team());

	return leader_paths.destinations.contains(start_pos);
}


const map_location& readonly_context_impl::nearest_keep(const map_location& loc) const
{
	std::set<map_location> avoided_locations;
	get_avoid().get_locations(avoided_locations);
	const std::set<map_location>& keeps = this->keeps();
	if(keeps.empty()) {
		static const map_location dummy;
		return dummy;
	}

	const map_location* res = nullptr;
	int closest = -1;
	for(std::set<map_location>::const_iterator i = keeps.begin(); i != keeps.end(); ++i) {
		if (avoided_locations.find(*i)!=avoided_locations.end()) {
			continue;
		}
		const int distance = distance_between(*i,loc);
		if(res == nullptr || distance < closest) {
			closest = distance;
			res = &*i;
		}
	}
	if (res) {
		return *res;
	} else {
		return map_location::null_location();
	}
}


double readonly_context_impl::power_projection(const map_location& loc, const move_map& dstsrc) const
{
	map_location used_locs[6];
	int ratings[6];
	std::fill_n(ratings, 0, 6);
	int num_used_locs = 0;

	map_location locs[6];
	get_adjacent_tiles(loc,locs);

	const gamemap& map_ = resources::gameboard->map();
	unit_map& units_ = resources::gameboard->units();

	int res = 0;

	bool changed = false;
	for (int i = 0;; ++i) {
		if (i == 6) {
			if (!changed) break;
			// Loop once again, in case a unit found a better spot
			// and freed the place for another unit.
			changed = false;
			i = 0;
		}

		if (map_.on_board(locs[i]) == false) {
			continue;
		}

		const t_translation::terrain_code terrain = map_[locs[i]];

		typedef move_map::const_iterator Itor;
		typedef std::pair<Itor,Itor> Range;
		Range its = dstsrc.equal_range(locs[i]);

		map_location* const beg_used = used_locs;
		map_location* end_used = used_locs + num_used_locs;

		int best_rating = 0;
		map_location best_unit;

		for(Itor it = its.first; it != its.second; ++it) {
			const unit_map::const_iterator u = units_.find(it->second);

			// Unit might have been killed, and no longer exist
			if(u == units_.end()) {
				continue;
			}

			const unit& un = *u;

			// The unit might play on the next turn
			int attack_turn = resources::tod_manager->turn();
			if(un.side() < get_side()) {
				++attack_turn;
			}
			// Considering the unit location would be too slow, we only apply the bonus granted by the global ToD
			const int lawful_bonus = resources::tod_manager->get_time_of_day(attack_turn).lawful_bonus;
			int tod_modifier = 0;
			if(un.alignment() == unit_type::ALIGNMENT::LAWFUL) {
				tod_modifier = lawful_bonus;
			} else if(un.alignment() == unit_type::ALIGNMENT::CHAOTIC) {
				tod_modifier = -lawful_bonus;
			} else if(un.alignment() == unit_type::ALIGNMENT::LIMINAL) {
				tod_modifier = -(std::abs(lawful_bonus));
			}

			// The 0.5 power avoids underestimating too much the damage of a wounded unit.
			int64_t hp = int(sqrt(double(un.hitpoints()) / un.max_hitpoints()) * 1000);
			int64_t most_damage = 0;
			for(const attack_type &att : un.attacks())
			{
				int damage = att.damage() * att.num_attacks() * (100 + tod_modifier);
				if (damage > most_damage) {
					most_damage = damage;
				}
			}

			int64_t village_bonus = map_.is_village(terrain) ? 3 : 2;
			int64_t defense = 100 - un.defense_modifier(terrain);
			int64_t rating_64 = hp * defense * most_damage * village_bonus / 200;
			int rating = rating_64;
			if(static_cast<int64_t>(rating) != rating_64) {
				WRN_AI << "overflow in ai attack calculation\n";
			}
			if(rating > best_rating) {
				map_location *pos = std::find(beg_used, end_used, it->second);
				// Check if the spot is the same or better than an older one.
				if (pos == end_used || rating >= ratings[pos - beg_used]) {
					best_rating = rating;
					best_unit = it->second;
				}
			}
		}

		if (!best_unit.valid()) continue;
		map_location *pos = std::find(beg_used, end_used, best_unit);
		int index = pos - beg_used;
		if (index == num_used_locs)
			++num_used_locs;
		else if (best_rating == ratings[index])
			continue;
		else {
			// The unit was in another spot already, so remove its older rating
			// from the final result, and require a new run to fill its old spot.
			res -= ratings[index];
			changed = true;
		}
		used_locs[index] = best_unit;
		ratings[index] = best_rating;
		res += best_rating;
	}

	return res / 100000.;
}

void readonly_context_impl::recalculate_move_maps() const
{
	dstsrc_ = move_map();
	possible_moves_ = moves_map();
	srcdst_ = move_map();
	calculate_possible_moves(possible_moves_,srcdst_,dstsrc_,false,false,&get_avoid());
	if (get_passive_leader()||get_passive_leader_shares_keep()) {
		unit_map::iterator i = resources::gameboard->units().find_leader(get_side());
		if (i.valid()) {
			map_location loc = i->get_location();
			srcdst_.erase(loc);
			for(move_map::iterator it = dstsrc_.begin(); it != dstsrc_.end(); ) {
				if(it->second == loc) {
					it = dstsrc_.erase(it);
				} else {
					++it;
				}
			}
		///@todo: shall possible moves be modified as well ?
		}
	}
	move_maps_valid_ = true;

	// invalidate lua cache
	dst_src_valid_lua_ = false;
	src_dst_valid_lua_ = false;
}


void readonly_context_impl::recalculate_move_maps_enemy() const
{
	enemy_dstsrc_ = move_map();
	enemy_srcdst_ = move_map();
	enemy_possible_moves_ = moves_map();
	calculate_possible_moves(enemy_possible_moves_,enemy_srcdst_,enemy_dstsrc_,true);
	move_maps_enemy_valid_ = true;

	// invalidate lua cache
	dst_src_enemy_valid_lua_ = false;
	src_dst_enemy_valid_lua_ = false;
}

void readonly_context_impl::set_dst_src_valid_lua()
{
	dst_src_valid_lua_ = true;
}

void readonly_context_impl::set_dst_src_enemy_valid_lua()
{
	dst_src_enemy_valid_lua_ = true;
}

void readonly_context_impl::set_src_dst_valid_lua()
{
	src_dst_valid_lua_ = true;
}

void readonly_context_impl::set_src_dst_enemy_valid_lua()
{
	src_dst_enemy_valid_lua_ = true;
}

const map_location& readonly_context_impl::suitable_keep(const map_location& leader_location, const pathfind::paths& leader_paths) const {
	if (resources::gameboard->map().is_keep(leader_location)) {
		return leader_location; //if leader already on keep, then return leader_location
	}

	map_location const* best_free_keep = &map_location::null_location();
	double move_left_at_best_free_keep = 0.0;

	map_location const* best_occupied_keep = &map_location::null_location();
	double move_left_at_best_occupied_keep = 0.0;

	for(const pathfind::paths::step &dest : leader_paths.destinations)
	{
		const map_location &loc = dest.curr;
		if (keeps().find(loc)!=keeps().end()){

			const int move_left_at_loc = dest.move_left;
			if (resources::gameboard->units().count(loc) == 0) {
				if ((*best_free_keep==map_location::null_location())||(move_left_at_loc>move_left_at_best_free_keep)){
					best_free_keep = &loc;
					move_left_at_best_free_keep = move_left_at_loc;
				}
			} else {
				if ((*best_occupied_keep==map_location::null_location())||(move_left_at_loc>move_left_at_best_occupied_keep)){
					best_occupied_keep = &loc;
				        move_left_at_best_occupied_keep = move_left_at_loc;
				}
			}
		}
	}

	if (*best_free_keep != map_location::null_location()){
		return *best_free_keep; // if there is a free keep reachable during current turn, return it
	}

	if (*best_occupied_keep != map_location::null_location()){
		return *best_occupied_keep; // if there is an occupied keep reachable during current turn, return it
	}

	return nearest_keep(leader_location); // return nearest keep
}


	/** Weapon choice cache, to speed simulations. */
readonly_context::unit_stats_cache_t & readonly_context_impl::unit_stats_cache() const
{
	return unit_stats_cache_;
}


bool readonly_context_impl::is_active(const std::string &time_of_day, const std::string &turns) const
{
		if(time_of_day.empty() == false) {
			const std::vector<std::string>& times = utils::split(time_of_day);
			if(std::count(times.begin(),times.end(),resources::tod_manager->get_time_of_day().id) == 0) {
				return false;
			}
		}

		if(turns.empty() == false) {
			int turn = resources::tod_manager->turn();
			const std::vector<std::string>& turns_list = utils::split(turns);
			for(std::vector<std::string>::const_iterator j = turns_list.begin(); j != turns_list.end() ; ++j ) {
				const std::pair<int,int> range = utils::parse_range(*j);
				if(turn >= range.first && turn <= range.second) {
				      return true;
				}
			}
			return false;
		}
		return true;
}

} //of namespace ai
