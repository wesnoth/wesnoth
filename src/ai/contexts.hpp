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
 * @file
 * Helper functions for the object which operates in the context of AI for specific side
 * this is part of AI interface
 */

#ifndef AI_CONTEXTS_HPP_INCLUDED
#define AI_CONTEXTS_HPP_INCLUDED

#include "ai/game_info.hpp"                // for move_map, aspect_type, etc

#include "config.hpp"                // for config
#include "game_errors.hpp"
#include "generic_event.hpp"         // for observer
#include "units/ptr.hpp"              // for unit_ptr
#include "map/location.hpp"       // for map_location

#include <map>                          // for map, map<>::value_compare
#include <set>                          // for set
#include <string>                       // for string
#include <utility>                      // for pair
#include <vector>                       // for vector

class gamemap;  // lines 41-41
class team;
class terrain_filter;  // lines 43-43
class unit_map;
class unit_type;  // lines 46-46
namespace wfl { class variant; }
namespace ai { class ai_context; }  // lines 51-51
namespace ai { class unit_advancements_aspect; }
namespace ai { template <typename T> class typesafe_aspect; }
namespace boost { template <class T> class shared_ptr; }
namespace pathfind { struct paths; }
struct battle_context_unit_stats;  // lines 39-39

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

typedef ai_context* ai_context_ptr;


// recursion counter
class recursion_counter {
public:
	recursion_counter(int counter)
		: counter_(++counter)
	{
		if (counter > MAX_COUNTER_VALUE ) {
			throw game::game_error("maximum recursion depth reached!");
		}
	}


	/**
	 * Get the current value of the recursion counter
	 */
	int get_count() const
	{
		return counter_;
	}


	//max recursion depth
	static const int MAX_COUNTER_VALUE = 100;


	/**
	 * Check if more recursion is allowed
	 */
	bool is_ok() const
	{
		return counter_ < MAX_COUNTER_VALUE;
	}
private:

	// recursion counter value
	int counter_;
};

//defensive position

struct defensive_position {
	defensive_position() :
		loc(),
		chance_to_hit(0),
		vulnerability(0.0),
		support(0.0)
		{}

	map_location loc;
	int chance_to_hit;
	double vulnerability, support;
};

// keeps cache
class keeps_cache : public events::observer
{
public:
	keeps_cache();
	~keeps_cache();
	void handle_generic_event(const std::string& event_name);
	void clear();
	const std::set<map_location>& get();
	void init(const gamemap &map);
private:
	const gamemap *map_;
	std::set<map_location> keeps_;
};

// side context

class side_context;

class side_context{
public:

	/**
	 * Get the side number
	 */
	virtual side_number get_side() const  = 0;


	/**
	 * Set the side number
	 */
	virtual void set_side(side_number side) = 0;


	/**
	 * empty destructor
	 */
	virtual ~side_context(){}


	/**
	 * empty constructor
	 */
	side_context() {}


	/**
	 * unwrap
	 */
	virtual side_context& get_side_context() = 0;


	/**
	 * serialize this context to config
	 */
	virtual config to_side_context_config() const = 0;


	/**
	 * Get the value of the recursion counter
	 */
	virtual int get_recursion_count() const = 0;


};

class readonly_context;
class readonly_context : public virtual side_context {
public:
	readonly_context(){}
	virtual ~readonly_context(){}
	virtual readonly_context& get_readonly_context() = 0;
	virtual void on_readonly_context_create() = 0;
	virtual const team& current_team() const = 0;
	virtual void diagnostic(const std::string& msg) = 0;
	virtual void log_message(const std::string& msg) = 0;
	virtual attack_result_ptr check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon) = 0;
	virtual move_result_ptr check_move_action(const map_location& from, const map_location& to, bool remove_movement=true, bool unreach_is_ok=false) = 0;
	virtual recall_result_ptr check_recall_action(const std::string& id, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) = 0;
	virtual recruit_result_ptr check_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) = 0;
	virtual stopunit_result_ptr check_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false) = 0;
	virtual synced_command_result_ptr check_synced_command_action(const std::string& lua_code, const map_location& location = map_location::null_location()) = 0;
	virtual void calculate_possible_moves(std::map<map_location,pathfind::paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, bool enemy,
		bool assume_full_movement=false,
		const terrain_filter* remove_destinations=nullptr) const = 0;
	virtual void calculate_moves(const unit_map& units,
		std::map<map_location,pathfind::paths>& possible_moves, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement=false,
		const terrain_filter* remove_destinations=nullptr,
		bool see_all=false) const = 0;

	virtual const game_info& get_info() const = 0;


	//@note: following part is in alphabetic order
	virtual defensive_position const& best_defensive_position(const map_location& unit,
			const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc) const = 0;


	virtual std::map<map_location,defensive_position>& defensive_position_cache() const = 0;


	virtual const unit_advancements_aspect& get_advancements() const = 0;


	virtual double get_aggression() const = 0;


	virtual int get_attack_depth() const = 0;


	virtual const aspect_map& get_aspects() const = 0;


	virtual aspect_map& get_aspects() = 0;


	virtual void add_facet(const std::string &id, const config &cfg) const = 0;


	virtual void add_aspects(std::vector< aspect_ptr > &aspects ) = 0;


	virtual const attacks_vector& get_attacks() const = 0;


	virtual const wfl::variant& get_attacks_as_variant() const = 0;


	virtual const terrain_filter& get_avoid() const = 0;


	virtual double get_caution() const = 0;


	virtual const move_map& get_dstsrc() const = 0;


	virtual const move_map& get_enemy_dstsrc() const = 0;


	virtual const moves_map& get_enemy_possible_moves() const = 0;


	virtual const move_map& get_enemy_srcdst() const = 0;

	/**
	 * get engine by cfg, creating it if it is not created yet but known
	 */
	virtual engine_ptr get_engine_by_cfg(const config& cfg) = 0;


	virtual const std::vector<engine_ptr>& get_engines() const = 0;


	virtual std::vector<engine_ptr>& get_engines() = 0;


	virtual std::string get_grouping() const = 0;


	virtual const std::vector<goal_ptr>& get_goals() const = 0;


	virtual std::vector<goal_ptr>& get_goals() = 0;


	virtual double get_leader_aggression() const = 0;


	virtual config get_leader_goal() const = 0;


	virtual bool get_leader_ignores_keep() const = 0;


	virtual double get_leader_value() const = 0;


	virtual bool get_passive_leader() const = 0;


	virtual bool get_passive_leader_shares_keep() const = 0;


	virtual const moves_map& get_possible_moves() const = 0;


	virtual const std::vector<unit_ptr>& get_recall_list() const = 0;


	virtual double get_recruitment_diversity() const = 0;


	virtual const config get_recruitment_instructions() const = 0;


	virtual const std::vector<std::string> get_recruitment_more() const = 0;


	virtual const std::vector<std::string> get_recruitment_pattern() const = 0;


	virtual int get_recruitment_randomness() const = 0;


	virtual const config get_recruitment_save_gold() const = 0;


	virtual double get_scout_village_targeting() const = 0;


	virtual bool get_simple_targeting() const = 0;


	virtual const move_map& get_srcdst() const = 0;


	virtual bool get_support_villages() const = 0;


	virtual double get_village_value() const = 0;


	virtual int get_villages_per_scout() const = 0;



	virtual bool is_active(const std::string &time_of_day, const std::string &turns) const = 0;

	virtual bool is_dst_src_valid_lua() const = 0;

	virtual bool is_dst_src_enemy_valid_lua() const = 0;

	virtual bool is_src_dst_valid_lua() const = 0;

	virtual bool is_src_dst_enemy_valid_lua() const = 0;

	virtual void invalidate_defensive_position_cache() const = 0;


	virtual void invalidate_move_maps() const = 0;


	virtual void invalidate_keeps_cache() const= 0;


	virtual const std::set<map_location>& keeps() const= 0;


	virtual bool leader_can_reach_keep() const = 0;


	virtual const map_location& nearest_keep(const map_location& loc) const = 0;


	/**
	 * Function which finds how much 'power' a side can attack a certain location with.
	 * This is basically the maximum hp of damage that can be inflicted upon a unit on loc
	 * by full-health units, multiplied by the defense these units will have.
	 * (if 'use_terrain' is false, then it will be multiplied by 0.5)
	 *
	 * Example: 'loc' can be reached by two units, one of whom has a 10-3 attack
	 * and has 48/48 hp, and can defend at 40% on the adjacent grassland.
	 * The other has a 8-2 attack, and has 30/40 hp, and can defend at 60% on the adjacent mountain.
	 * The rating will be 10*3*1.0*0.4 + 8*2*0.75*0.6 = 19.2
	 */
	virtual double power_projection(const map_location& loc, const move_map& dstsrc) const = 0;


	virtual void raise_user_interact() const = 0;


	virtual void recalculate_move_maps() const = 0;


	virtual void recalculate_move_maps_enemy() const = 0;

	virtual void set_src_dst_valid_lua() = 0;
	virtual void set_src_dst_enemy_valid_lua() = 0;
	virtual void set_dst_src_valid_lua() = 0;
	virtual void set_dst_src_enemy_valid_lua() = 0;

	/** get most suitable keep for leader - nearest free that can be reached in 1 turn, if none - return nearest occupied that can be reached in 1 turn, if none - return nearest keep, if none - return null_location */
	virtual const map_location& suitable_keep( const map_location& leader_location, const pathfind::paths& leader_paths ) = 0;

	/**
	 * serialize to config
	 */
	virtual config to_readonly_context_config() const = 0;


	typedef std::map<std::pair<map_location,const unit_type *>,
	                 std::pair<battle_context_unit_stats,battle_context_unit_stats> >
	        unit_stats_cache_t;
	virtual unit_stats_cache_t & unit_stats_cache() const = 0;

};

class readwrite_context;
class readwrite_context : public virtual readonly_context {
public:
	readwrite_context(){}


	virtual ~readwrite_context(){}


	virtual readwrite_context& get_readwrite_context() = 0;


	virtual attack_result_ptr execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon) = 0;


	virtual move_result_ptr execute_move_action(const map_location& from, const map_location& to, bool remove_movement=true, bool unreach_is_ok=false) = 0;


	virtual recall_result_ptr execute_recall_action(const std::string& id, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) = 0;


	virtual recruit_result_ptr execute_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) = 0;


	virtual stopunit_result_ptr execute_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false) = 0;


	virtual synced_command_result_ptr execute_synced_command_action(const std::string& lua_code, const map_location& location = map_location::null_location()) = 0;


	virtual team& current_team_w() = 0;


	virtual void raise_gamestate_changed() const = 0;


	virtual game_info& get_info_w() = 0;


	/**
	 * serialize this context to config
	 */
	virtual config to_readwrite_context_config() const = 0;

};

//proxies

class side_context_proxy : public virtual side_context {
public:
	side_context_proxy()
		: target_(nullptr)
	{
	}

	virtual ~side_context_proxy(){}


	void init_side_context_proxy(side_context &target)
	{
		target_= &target.get_side_context();
	}

	virtual side_number get_side() const override
	{
		return target_->get_side();
	}

	virtual void set_side(side_number side) override
	{
		return target_->set_side(side);
	}

	virtual side_context& get_side_context() override
	{
		return target_->get_side_context();
	}

	virtual int get_recursion_count() const override
	{
		return target_->get_recursion_count();
	}


	virtual config to_side_context_config() const override
	{
		return target_->to_side_context_config();
	}


private:
	side_context *target_;
};


class readonly_context_proxy : public virtual readonly_context, public virtual side_context_proxy {
public:
	readonly_context_proxy()
		: target_(nullptr)
	{
	}

	virtual ~readonly_context_proxy() {}

	void init_readonly_context_proxy(readonly_context &target)
	{
		init_side_context_proxy(target);
		target_ = &target.get_readonly_context();
	}

	virtual readonly_context& get_readonly_context() override
	{
		return target_->get_readonly_context();
	}


	virtual void on_readonly_context_create() override
	{
		return target_->on_readonly_context_create();
	}


	virtual const team& current_team() const override
	{
		return target_->current_team();
	}

	virtual void diagnostic(const std::string& msg) override
	{
		target_->diagnostic(msg);
	}

	virtual void log_message(const std::string& msg) override
	{
		target_->log_message(msg);
	}

	virtual attack_result_ptr check_attack_action(const map_location &attacker_loc, const map_location &defender_loc, int attacker_weapon) override
	{
		return target_->check_attack_action(attacker_loc, defender_loc, attacker_weapon);
	}

	virtual move_result_ptr check_move_action(const map_location &from, const map_location &to, bool remove_movement=true, bool unreach_is_ok=false) override
	{
		return target_->check_move_action(from, to, remove_movement, unreach_is_ok);
	}


	virtual recall_result_ptr check_recall_action(const std::string &id, const map_location &where = map_location::null_location(),
			const map_location &from = map_location::null_location()) override
	{
		return target_->check_recall_action(id, where, from);
	}


	virtual recruit_result_ptr check_recruit_action(const std::string &unit_name, const map_location &where = map_location::null_location(),
			const map_location &from = map_location::null_location()) override
	{
		return target_->check_recruit_action(unit_name, where, from);
	}

	virtual stopunit_result_ptr check_stopunit_action(const map_location &unit_location, bool remove_movement = true, bool remove_attacks = false) override
	{
		return target_->check_stopunit_action(unit_location, remove_movement, remove_attacks);
	}

	virtual synced_command_result_ptr check_synced_command_action(const std::string& lua_code, const map_location& location = map_location::null_location()) override
	{
		return target_->check_synced_command_action(lua_code, location);
	}

	virtual void calculate_possible_moves(std::map<map_location,pathfind::paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, bool enemy,
		bool assume_full_movement=false,
		const terrain_filter* remove_destinations=nullptr) const override
	{
		target_->calculate_possible_moves(possible_moves, srcdst, dstsrc, enemy, assume_full_movement, remove_destinations);
	}

	virtual void calculate_moves(const unit_map& units,
		std::map<map_location,pathfind::paths>& possible_moves, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement=false,
		const terrain_filter* remove_destinations=nullptr,
		bool see_all=false) const override
	{
		target_->calculate_moves(units, possible_moves, srcdst, dstsrc, enemy, assume_full_movement, remove_destinations, see_all);
	}

	virtual const game_info& get_info() const override
	{
		return target_->get_info();
	}

	virtual void raise_user_interact() const override
	{
		target_->raise_user_interact();
	}


	virtual int get_recursion_count() const override
	{
		return target_->get_recursion_count();
	}

	//@note: following part is in alphabetic order
	defensive_position const& best_defensive_position(const map_location& unit,
			const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc) const override
	{
		return target_->best_defensive_position(unit,dstsrc,srcdst,enemy_dstsrc);
	}


	virtual std::map<map_location,defensive_position>& defensive_position_cache() const override
	{
		return target_->defensive_position_cache();
	}


	virtual const unit_advancements_aspect& get_advancements() const override
	{
		return target_->get_advancements();
	}


	virtual double get_aggression() const override
	{
		return target_->get_aggression();
	}


	virtual int get_attack_depth() const override
	{
		return target_->get_attack_depth();
	}


	virtual const aspect_map& get_aspects() const override
	{
		return target_->get_aspects();
	}


	virtual aspect_map& get_aspects() override
	{
		return target_->get_aspects();
	}


	virtual void add_aspects(std::vector< aspect_ptr > &aspects ) override
	{
		return target_->add_aspects(aspects);
	}


	virtual void add_facet(const std::string &id, const config &cfg) const override
	{
		target_->add_facet(id,cfg);
	}



	virtual const attacks_vector& get_attacks() const override
	{
		return target_->get_attacks();
	}



	virtual const wfl::variant& get_attacks_as_variant() const override
	{
		return target_->get_attacks_as_variant();
	}


	virtual const terrain_filter& get_avoid() const override
	{
		return target_->get_avoid();
	}


	virtual double get_caution() const override
	{
		return target_->get_caution();
	}


	virtual const move_map& get_dstsrc() const override
	{
		return target_->get_dstsrc();
	}


	virtual const move_map& get_enemy_dstsrc() const override
	{
		return target_->get_enemy_dstsrc();
	}


	virtual const moves_map& get_enemy_possible_moves() const override
	{
		return target_->get_enemy_possible_moves();
	}


	virtual const move_map& get_enemy_srcdst() const override
	{
		return target_->get_enemy_srcdst();
	}


	virtual engine_ptr get_engine_by_cfg(const config &cfg) override
	{
		return target_->get_engine_by_cfg(cfg);
	}


	virtual const std::vector<engine_ptr>& get_engines() const override
	{
		return target_->get_engines();
	}


	virtual std::vector<engine_ptr>& get_engines() override
	{
		return target_->get_engines();
	}


	virtual std::string get_grouping() const override
	{
		return target_->get_grouping();
	}


	virtual const std::vector<goal_ptr>& get_goals() const override
	{
		return target_->get_goals();
	}


	virtual std::vector<goal_ptr>& get_goals() override
	{
		return target_->get_goals();
	}


	virtual double get_leader_aggression() const override
	{
		return target_->get_leader_aggression();
	}



	virtual config get_leader_goal() const override
	{
		return target_->get_leader_goal();
	}


	virtual bool get_leader_ignores_keep() const override
	{
		return target_->get_leader_ignores_keep();
	}


	virtual double get_leader_value() const override
	{
		return target_->get_leader_value();
	}


	virtual bool get_passive_leader() const override
	{
		return target_->get_passive_leader();
	}


	virtual bool get_passive_leader_shares_keep() const override
	{
		return target_->get_passive_leader_shares_keep();
	}


	virtual const moves_map& get_possible_moves() const
	{
		return target_->get_possible_moves();
	}


	virtual double power_projection(const map_location& loc, const move_map& dstsrc) const override
	{
		return target_->power_projection(loc,dstsrc);
	}


	virtual const std::vector<unit_ptr>& get_recall_list() const override
	{
		return target_->get_recall_list();
	}


	virtual double get_recruitment_diversity() const override
	{
		return target_->get_recruitment_diversity();
	}


	virtual const config get_recruitment_instructions() const override
	{
		return target_->get_recruitment_instructions();
	}


	virtual const std::vector<std::string> get_recruitment_more() const override
	{
		return target_->get_recruitment_more();
	}


	virtual const std::vector<std::string> get_recruitment_pattern() const override
	{
		return target_->get_recruitment_pattern();
	}


	virtual int get_recruitment_randomness() const override
	{
		return target_->get_recruitment_randomness();
	}


	virtual const config get_recruitment_save_gold() const override
	{
		return target_->get_recruitment_save_gold();
	}


	virtual const move_map& get_srcdst() const override
	{
		return target_->get_srcdst();
	}


	virtual double get_scout_village_targeting() const override
	{
		return target_->get_scout_village_targeting();
	}


	virtual bool get_simple_targeting() const override
	{
		return target_->get_simple_targeting();
	}


	virtual bool get_support_villages() const override
	{
		return target_->get_support_villages();
	}


	virtual double get_village_value() const override
	{
		return target_->get_village_value();
	}


	virtual int get_villages_per_scout() const override
	{
		return target_->get_villages_per_scout();
	}



	virtual bool is_active(const std::string &time_of_day, const std::string &turns) const override
	{
		return target_->is_active(time_of_day, turns);
	}

	virtual bool is_dst_src_valid_lua() const override
	{
		return target_->is_dst_src_valid_lua();
	}

	virtual bool is_dst_src_enemy_valid_lua() const override
	{
		return target_->is_dst_src_enemy_valid_lua();
	}

	virtual bool is_src_dst_valid_lua() const override
	{
		return target_->is_src_dst_valid_lua();
	}

	virtual bool is_src_dst_enemy_valid_lua() const override
	{
		return target_->is_src_dst_enemy_valid_lua();
	}

	virtual void invalidate_defensive_position_cache() const override
	{
		return target_->invalidate_defensive_position_cache();
	}


	virtual void invalidate_move_maps() const override
	{
		target_->invalidate_move_maps();
	}


	virtual void invalidate_keeps_cache() const override
	{
		return target_->invalidate_keeps_cache();
	}


	virtual const std::set<map_location>& keeps() const override
	{
		return target_->keeps();
	}


	virtual bool leader_can_reach_keep() const override
	{
		return target_->leader_can_reach_keep();
	}


	virtual const map_location& nearest_keep( const map_location& loc ) const override
	{
		return target_->nearest_keep(loc);
	}


	virtual void recalculate_move_maps() const override
	{
		target_->recalculate_move_maps();
	}


	virtual void recalculate_move_maps_enemy() const override
	{
		target_->recalculate_move_maps_enemy();
	}

	virtual void set_dst_src_valid_lua() override
	{
		target_->set_dst_src_valid_lua();
	}

	virtual void set_dst_src_enemy_valid_lua() override
	{
		target_->set_dst_src_enemy_valid_lua();
	}

	virtual void set_src_dst_valid_lua() override
	{
		target_->set_src_dst_valid_lua();
	}

	virtual void set_src_dst_enemy_valid_lua() override
	{
		target_->set_src_dst_enemy_valid_lua();
	}

	virtual const map_location& suitable_keep( const map_location& leader_location, const pathfind::paths& leader_paths ) override
	{
		return target_->suitable_keep(leader_location, leader_paths);
	}


	virtual config to_readonly_context_config() const override
	{
		return target_->to_readonly_context_config();
	}


	virtual unit_stats_cache_t & unit_stats_cache() const override
	{
		return target_->unit_stats_cache();
	}


private:
	readonly_context *target_;
};


class readwrite_context_proxy : public virtual readwrite_context, public virtual readonly_context_proxy {
public:
	readwrite_context_proxy()
		: target_(nullptr)
	{
	}


	void init_readwrite_context_proxy(readwrite_context &target)
	{
		init_readonly_context_proxy(target);
		target_ = &target.get_readwrite_context();
	}


	virtual readwrite_context& get_readwrite_context() override
	{
		return target_->get_readwrite_context();
	}


	virtual attack_result_ptr execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon) override
	{
		return target_->execute_attack_action(attacker_loc,defender_loc,attacker_weapon);
	}


	virtual move_result_ptr execute_move_action(const map_location& from, const map_location& to, bool remove_movement=true, bool unreach_is_ok=false) override
	{
		return target_->execute_move_action(from, to, remove_movement, unreach_is_ok);
	}


	virtual recall_result_ptr execute_recall_action(const std::string& id, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) override
	{
		return target_->execute_recall_action(id,where,from);
	}


	virtual recruit_result_ptr execute_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) override
	{
		return target_->execute_recruit_action(unit_name,where,from);
	}


	virtual stopunit_result_ptr execute_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false) override
	{
		return target_->execute_stopunit_action(unit_location,remove_movement,remove_attacks);
	}


	virtual synced_command_result_ptr execute_synced_command_action(const std::string& lua_code, const map_location& location = map_location::null_location()) override
	{
		return target_->execute_synced_command_action(lua_code,location);
	}


	virtual team& current_team_w() override
	{
		return target_->current_team_w();
	}


	virtual void raise_gamestate_changed() const override
	{
		target_->raise_gamestate_changed();
	}


	virtual game_info& get_info_w() override
	{
		return target_->get_info_w();
	}


	virtual int get_recursion_count() const override
	{
		return target_->get_recursion_count();
	}


	virtual config to_readwrite_context_config() const override
	{
		return target_->to_readwrite_context_config();
	}

private:
	readwrite_context *target_;
};


//implementation
class side_context_impl : public side_context {
public:
	side_context_impl(side_number side, const config  &/*cfg*/)
		: side_(side), recursion_counter_(0)
	{
	}

	virtual ~side_context_impl(){}

	virtual side_number get_side() const override
	{
		return side_;
	}

	virtual void set_side(side_number side) override
	{
		side_ = side;
	}


	virtual side_context& get_side_context() override
	{
		return *this;
	}


	virtual int get_recursion_count() const override;


	virtual config to_side_context_config() const override;

private:
	side_number side_;
	recursion_counter recursion_counter_;
};


class readonly_context_impl : public virtual side_context_proxy, public readonly_context, public events::observer {
public:

	/**
	 * Constructor
	 */
	readonly_context_impl(side_context &context, const config &cfg);


	/**
	 * Destructor
	 */
	virtual ~readonly_context_impl();


	/**
	 * Unwrap - this class is not a proxy, so return *this
:w
	 */
	virtual readonly_context& get_readonly_context() override
	{
		return *this;
	}


	virtual void on_readonly_context_create() override;


	/** Handle generic event */
	virtual void handle_generic_event(const std::string& event_name) override;


	/** Return a reference to the 'team' object for the AI. */
	const team& current_team() const override;


	/** Show a diagnostic message on the screen. */
	void diagnostic(const std::string& msg) override;


	/** Display a debug message as a chat message. */
	void log_message(const std::string& msg) override;


	/**
	 * Check if it is possible to attack enemy defender using our unit attacker from attackers current location,
	 * @param attacker_loc location of attacker
	 * @param defender_loc location of defender
	 * @param attacker_weapon weapon of attacker
	 * @retval possible result: ok
	 * @retval possible result: something wrong
	 * @retval possible result: attacker and/or defender are invalid
	 * @retval possible result: attacker and/or defender are invalid
	 * @retval possible result: attacker doesn't have the specified weapon
	 */
	attack_result_ptr check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon) override;


	/**
	 * Check if it is possible to move our unit from location 'from' to location 'to'
	 * @param from location of our unit
	 * @param to where to move
	 * @param remove_movement set unit movement to 0 in case of successful move
	 * @retval possible result: ok
	 * @retval possible result: something wrong
	 * @retval possible result: move is interrupted
	 * @retval possible result: move is impossible
	 */
	move_result_ptr check_move_action(const map_location& from, const map_location& to, bool remove_movement=true, bool unreach_is_ok=false) override;



	/**
	 * Check if it is possible to recall a unit for us on specified location
	 * @param id the id of the unit to be recruited.
	 * @param where location where the unit is to be recruited.
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: leader not on keep
	 * @retval possible_result: no free space on keep
	 * @retval possible_result: not enough gold
	 */
	recall_result_ptr check_recall_action(const std::string& id, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) override;


	/**
	 * Check if it is possible to recruit a unit for us on specified location
	 * @param unit_name the name of the unit to be recruited.
	 * @param where location where the unit is to be recruited.
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: leader not on keep
	 * @retval possible_result: no free space on keep
	 * @retval possible_result: not enough gold
	 */
	recruit_result_ptr check_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) override;


	/**
	 * Check if it is possible to remove unit movements and/or attack
	 * @param unit_location the location of our unit
	 * @param remove_movement set remaining movements to 0
	 * @param remove_attacks set remaining attacks to 0
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	stopunit_result_ptr check_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false) override;


	/**
	 * Check if it is possible to run Lua code
	 * @param lua_code the code to be run
	 * @param location location to be passed to the code as x1/y1
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	synced_command_result_ptr check_synced_command_action(const std::string& lua_code, const map_location& location = map_location::null_location()) override;


	/**
	 * Calculate the moves units may possibly make.
	 *
	 * @param possible_moves      A map which will be filled with the paths
	 *                            each unit can take to get to every possible
	 *                            destination. You probably don't want to use
	 *                            this object at all, except to pass to
	 *                            'move_unit'.
	 * @param srcdst              A map of units to all their possible
	 *                            destinations.
	 * @param dstsrc              A map of destinations to all the units that
	 *                            can move to that destination.
	 * @param enemy               if true, a map of possible moves for enemies
	 *                            will be calculated. If false, a map of
	 *                            possible moves for units on the AI's side
	 *                            will be calculated.  The AI's own leader will
	 *                            not be included in this map.
	 * @param assume_full_movement
	 *                            If true, the function will operate on the
	 *                            assumption that all units can move their full
	 *                            movement allotment.
	 * @param remove_destinations a pointer to a terrain filter for possible destinations
	 *                            to omit.
	 */
	void calculate_possible_moves(std::map<map_location,pathfind::paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, bool enemy,
		bool assume_full_movement=false,
		const terrain_filter* remove_destinations=nullptr) const override;

 	/**
	 * A more fundamental version of calculate_possible_moves which allows the
	 * use of a speculative unit map.
	 * NOTE: Support for a speculative map is broken (not used when pathfinding)
	 *       and has not been used since (probably) r38610 (September 2009).
	 *       (See the todo in the implementation.)
	 */
	void calculate_moves(const unit_map& units,
		std::map<map_location,pathfind::paths>& possible_moves, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement=false,
		const terrain_filter* remove_destinations=nullptr,
		bool see_all=false) const override;


	virtual const game_info& get_info() const override;

	/**
	 * Function which should be called frequently to allow the user to interact
	 * with the interface. This function will make sure that interaction
	 * doesn't occur too often, so there is no problem with calling it very
	 * regularly.
	 */
	void raise_user_interact() const override;


	virtual int get_recursion_count() const override;


	//@note: following functions are in alphabetic order
	defensive_position const& best_defensive_position(const map_location& unit,
			const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc) const override;


	virtual std::map<map_location,defensive_position>& defensive_position_cache() const override;


	virtual const unit_advancements_aspect& get_advancements() const override;


	virtual double get_aggression() const override;


	virtual int get_attack_depth() const override;


	virtual const aspect_map& get_aspects() const override;


	virtual aspect_map& get_aspects() override;


	virtual const attacks_vector& get_attacks() const override;


	virtual const wfl::variant& get_attacks_as_variant() const override;


	virtual const terrain_filter& get_avoid() const override;


	virtual double get_caution() const override;


	virtual const move_map& get_dstsrc() const override;


	virtual const move_map& get_enemy_dstsrc() const override;


	virtual const moves_map& get_enemy_possible_moves() const override;


	virtual const move_map& get_enemy_srcdst() const override;


	virtual engine_ptr get_engine_by_cfg(const config& cfg) override;


	virtual const std::vector<engine_ptr>& get_engines() const override;


	virtual std::vector<engine_ptr>& get_engines() override;


	virtual std::string get_grouping() const override;


	virtual const std::vector<goal_ptr>& get_goals() const override;


	virtual std::vector<goal_ptr>& get_goals() override;


	virtual double get_leader_aggression() const override;


	virtual config get_leader_goal() const override;


	virtual bool get_leader_ignores_keep() const override;


	virtual double get_leader_value() const override;


	virtual bool get_passive_leader() const override;


	virtual bool get_passive_leader_shares_keep() const override;


	virtual const moves_map& get_possible_moves() const override;


	virtual const std::vector<unit_ptr>& get_recall_list() const override;


	virtual double get_recruitment_diversity() const override;


	virtual const config get_recruitment_instructions() const override;


	virtual const std::vector<std::string> get_recruitment_more() const override;


	virtual const std::vector<std::string> get_recruitment_pattern() const override;


	virtual int get_recruitment_randomness() const override;


	virtual const config get_recruitment_save_gold() const override;


	virtual double get_scout_village_targeting() const override;


	virtual bool get_simple_targeting() const override;


	virtual const move_map& get_srcdst() const override;


	virtual bool get_support_villages() const override;


	virtual double get_village_value() const override;


	virtual int get_villages_per_scout() const override;


	virtual bool is_active(const std::string &time_of_day, const std::string &turns) const override;

	virtual bool is_dst_src_valid_lua() const override;

	virtual bool is_dst_src_enemy_valid_lua() const override;

	virtual bool is_src_dst_valid_lua() const override;

	virtual bool is_src_dst_enemy_valid_lua() const override;

	virtual void invalidate_defensive_position_cache() const override;


	virtual void invalidate_move_maps() const override;


	virtual void invalidate_keeps_cache() const override;


	virtual const std::set<map_location>& keeps() const override;


	virtual bool leader_can_reach_keep() const override;


	virtual const map_location& nearest_keep(const map_location& loc) const override;


	virtual double power_projection(const map_location& loc, const move_map& dstsrc) const override;


	virtual void recalculate_move_maps() const override;


	virtual void recalculate_move_maps_enemy() const override;


	virtual void add_aspects(std::vector< aspect_ptr > &aspects) override;


	virtual void add_facet(const std::string &id, const config &cfg) const override;


	void on_create();

	virtual void set_dst_src_valid_lua() override;

	virtual void set_dst_src_enemy_valid_lua() override;

	virtual void set_src_dst_valid_lua() override;

	virtual void set_src_dst_enemy_valid_lua() override;

	virtual const map_location& suitable_keep( const map_location& leader_location, const pathfind::paths& leader_paths ) override;


	virtual config to_readonly_context_config() const override;


	virtual unit_stats_cache_t & unit_stats_cache() const override;

private:
	template<typename T>
	void add_known_aspect(const std::string &name, std::shared_ptr< typesafe_aspect <T> >& where);

	const config cfg_;

	/**
	 * AI Support Engines
	 */
	std::vector< engine_ptr > engines_;

	known_aspect_map known_aspects_;

	aspect_type< unit_advancements_aspect >::typesafe_ptr advancements_;
	aspect_type<double>::typesafe_ptr aggression_;
	aspect_type<int>::typesafe_ptr attack_depth_;
	aspect_map aspects_;
	aspect_type< attacks_vector >::typesafe_ptr attacks_;
	mutable aspect_type<terrain_filter>::typesafe_ptr avoid_;
	aspect_type<double>::typesafe_ptr caution_;
	mutable std::map<map_location,defensive_position> defensive_position_cache_;
	mutable move_map dstsrc_;
	mutable move_map enemy_dstsrc_;
	mutable moves_map enemy_possible_moves_;
	mutable move_map enemy_srcdst_;
	aspect_type< std::string >::typesafe_ptr grouping_;
	std::vector< goal_ptr > goals_;
	mutable keeps_cache keeps_;
	aspect_type<double>::typesafe_ptr leader_aggression_;
	aspect_type< config >::typesafe_ptr leader_goal_;
	aspect_type<bool>::typesafe_ptr leader_ignores_keep_;
	aspect_type< double >::typesafe_ptr leader_value_;
	mutable bool move_maps_enemy_valid_;
	mutable bool move_maps_valid_;
	mutable bool dst_src_valid_lua_;
	mutable bool dst_src_enemy_valid_lua_;
	mutable bool src_dst_valid_lua_;
	mutable bool src_dst_enemy_valid_lua_;
	aspect_type<bool>::typesafe_ptr passive_leader_;
	aspect_type<bool>::typesafe_ptr passive_leader_shares_keep_;
	mutable moves_map possible_moves_;
	aspect_type< double >::typesafe_ptr recruitment_diversity_;
	aspect_type< config >::typesafe_ptr recruitment_instructions_;
	aspect_type< std::vector<std::string> >::typesafe_ptr recruitment_more_;
	aspect_type< std::vector<std::string> >::typesafe_ptr recruitment_pattern_;
	aspect_type< int >::typesafe_ptr recruitment_randomness_;
	aspect_type< config >::typesafe_ptr recruitment_save_gold_;
	recursion_counter recursion_counter_;
	aspect_type< double >::typesafe_ptr scout_village_targeting_;
	aspect_type< bool >::typesafe_ptr simple_targeting_;
	mutable move_map srcdst_;
	aspect_type< bool >::typesafe_ptr support_villages_;
	mutable unit_stats_cache_t unit_stats_cache_;
	aspect_type< double >::typesafe_ptr village_value_;
	aspect_type< int >::typesafe_ptr villages_per_scout_;


};

class readwrite_context_impl : public virtual readonly_context_proxy, public readwrite_context {
public:
	/**
	 * Unwrap - this class is not a proxy, so return *this
	 */
	virtual readwrite_context& get_readwrite_context() override
	{
		return *this;
	}


	/**
	 * Ask the game to attack an enemy defender using our unit attacker from attackers current location,
	 * @param attacker_loc location of attacker
	 * @param defender_loc location of defender
	 * @param attacker_weapon weapon of attacker
	 * @retval possible result: ok
	 * @retval possible result: something wrong
	 * @retval possible result: attacker and/or defender are invalid
	 * @retval possible result: attacker and/or defender are invalid
	 * @retval possible result: attacker doesn't have the specified weapon
	 */
	virtual attack_result_ptr execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon) override;


	/**
	 * Ask the game to move our unit from location 'from' to location 'to', optionally - doing a partial move
	 * @param from location of our unit
	 * @param to where to move
	 * @param remove_movement set unit movement to 0 in case of successful move
	 * @retval possible result: ok
	 * @retval possible result: something wrong
	 * @retval possible result: move is interrupted
	 * @retval possible result: move is impossible
	 */
	virtual move_result_ptr execute_move_action(const map_location& from, const map_location& to, bool remove_movement=true, bool unreach_is_ok=false) override;


	/**
	 * Ask the game to recall a unit for us on specified location
	 * @param id the id of the unit to be recalled.
	 * @param where location where the unit is to be recalled.
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: leader not on keep
	 * @retval possible_result: no free space on keep
	 * @retval possible_result: not enough gold
	 */
	virtual recall_result_ptr execute_recall_action(const std::string& id, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location()) override;


	/**
	 * Ask the game to recruit a unit for us on specified location
	 * @param unit_name the name of the unit to be recruited.
	 * @param where location where the unit is to be recruited.
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: leader not on keep
	 * @retval possible_result: no free space on keep
	 * @retval possible_result: not enough gold
	 */
	virtual recruit_result_ptr execute_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location(), const map_location &from = map_location::null_location());


	/**
	 * Ask the game to remove unit movements and/or attack
	 * @param unit_location the location of our unit
	 * @param remove_movement set remaining movements to 0
	 * @param remove_attacks set remaining attacks to 0
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	virtual stopunit_result_ptr execute_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false) override;


	/**
	 * Ask the game to run Lua code
	 * @param lua_code the code to be run
	 * @param location location to be passed to the code as x1/y1
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	virtual synced_command_result_ptr execute_synced_command_action(const std::string& lua_code, const map_location& location = map_location::null_location()) override;


	/** Return a reference to the 'team' object for the AI. */
	virtual team& current_team_w() override;


	/** Notifies all interested observers of the event respectively. */
	void raise_gamestate_changed() const;


	/**
	 * Constructor.
	 */
	readwrite_context_impl(readonly_context &context, const config &/*cfg*/)
		: recursion_counter_(context.get_recursion_count())
	{
		init_readonly_context_proxy(context);
	}


	virtual ~readwrite_context_impl()
	{
	}

	/**
	 * Functions to retrieve the 'info' object.
	 * Used by derived classes to discover all necessary game information.
	 */
	virtual game_info& get_info_w() override;


	virtual int get_recursion_count() const override;


	virtual config to_readwrite_context_config() const override;

private:
	recursion_counter recursion_counter_;
};


} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
