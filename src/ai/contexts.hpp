/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/contexts.hpp
 * Helper functions for the object which operates in the context of AI for specific side
 * this is part of AI interface
 */

#ifndef AI_CONTEXTS_HPP_INCLUDED
#define AI_CONTEXTS_HPP_INCLUDED

class game_display;
class gamemap;

#include "actions.hpp"
#include "game_info.hpp"
#include "../game_display.hpp"
#include "../gamestatus.hpp"
#include "../pathfind.hpp"
#include "../playturn.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

class interface;

typedef boost::shared_ptr< interface > ai_ptr;


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
	virtual const team& current_team() const = 0;
	virtual void diagnostic(const std::string& msg) = 0;
	virtual void log_message(const std::string& msg) = 0;
	virtual attack_result_ptr check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon) = 0;
	virtual move_result_ptr check_move_action(const map_location& from, const map_location& to, bool remove_movement=true) = 0;
	virtual recruit_result_ptr check_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location) = 0;
	virtual stopunit_result_ptr check_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false) = 0;
	virtual void calculate_possible_moves(std::map<map_location,paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, bool enemy,
		bool assume_full_movement=false,
		const std::set<map_location>* remove_destinations=NULL) const = 0;
	virtual void calculate_moves(const unit_map& units,
		std::map<map_location,paths>& possible_moves, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement=false,
		const std::set<map_location>* remove_destinations=NULL,
		bool see_all=false) const = 0;

	const virtual game_info& get_info() const = 0;


	virtual void raise_user_interact() const = 0;


	virtual const std::set<map_location>& avoided_locations() = 0;


	virtual const move_map& get_dstsrc() const = 0;


	virtual const move_map& get_enemy_dstsrc() const = 0;


	virtual const moves_map& get_enemy_possible_moves() const = 0;


	virtual const move_map& get_enemy_srcdst() const = 0;


	virtual const moves_map& get_possible_moves() const = 0;


	virtual const move_map& get_srcdst() const = 0;


	virtual void invalidate_avoided_locations_cache() = 0;


	virtual void recalculate_move_maps() = 0;

};

class readwrite_context;
class readwrite_context : public virtual readonly_context {
public:
	readwrite_context(){}
	virtual ~readwrite_context(){}
	virtual readwrite_context& get_readwrite_context() = 0;
	virtual attack_result_ptr execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon) = 0;
	virtual move_result_ptr execute_move_action(const map_location& from, const map_location& to, bool remove_movement=true) = 0;
	virtual recruit_result_ptr execute_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location) = 0;
	virtual stopunit_result_ptr execute_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false) = 0;
	virtual team& current_team_w() = 0;
	virtual void attack_enemy(const map_location u, const map_location target, int att_weapon, int def_weapon) = 0;
	virtual map_location move_unit(map_location from, map_location to, std::map<map_location,paths>& possible_moves) = 0;
	virtual map_location move_unit_partial(map_location from, map_location to, std::map<map_location,paths>& possible_moves) = 0;
	virtual bool recruit(const std::string& unit_name, map_location loc=map_location()) = 0;
	virtual void raise_unit_recruited() const = 0;
	virtual void raise_unit_moved() const = 0;
	virtual void raise_enemy_attacked() const = 0;
	virtual game_info& get_info_w() = 0;
};

class ai_default_context;
class ai_default_context : public virtual readwrite_context {
public:
	ai_default_context(){}
	virtual ~ai_default_context(){}
	virtual ai_default_context& get_ai_default_context() = 0;
};


//proxies

class side_context_proxy : public virtual side_context {
public:
	side_context_proxy()
		: target_(NULL)
	{
	}

	virtual ~side_context_proxy(){}


	void init_side_context_proxy(side_context &target)
	{
		target_= &target.get_side_context();
	}

	virtual side_number get_side() const
	{
		return target_->get_side();
	}

	virtual void set_side(side_number side)
	{
		return target_->set_side(side);
	}

	virtual side_context& get_side_context()
	{
		return target_->get_side_context();
	}

	virtual int get_recursion_count(){
		return target_->get_recursion_count();
	}

private:
	side_context *target_;
};


class readonly_context_proxy : public virtual readonly_context, public virtual side_context_proxy {
public:
	readonly_context_proxy()
		: target_(NULL)
	{
	}

	virtual ~readonly_context_proxy() {}

	void init_readonly_context_proxy(readonly_context &target)
	{
		init_side_context_proxy(target);
		target_ = &target.get_readonly_context();
	}

	virtual readonly_context& get_readonly_context()
	{
		return target_->get_readonly_context();
	}

	virtual const team& current_team() const
	{
		return target_->current_team();
	}

	virtual void diagnostic(const std::string& msg)
	{
		target_->diagnostic(msg);
	}

	virtual void log_message(const std::string& msg)
	{
		target_->log_message(msg);
	}

	virtual attack_result_ptr check_attack_action(const map_location &attacker_loc, const map_location &defender_loc, int attacker_weapon)
	{
		return target_->check_attack_action(attacker_loc, defender_loc, attacker_weapon);
	}

	virtual move_result_ptr check_move_action(const map_location &from, const map_location &to, bool remove_movement=true)
	{
		return target_->check_move_action(from, to, remove_movement);
	}

	virtual recruit_result_ptr check_recruit_action(const std::string &unit_name, const map_location &where = map_location::null_location)
	{
		return target_->check_recruit_action(unit_name, where);
	}

	virtual stopunit_result_ptr check_stopunit_action(const map_location &unit_location, bool remove_movement = true, bool remove_attacks = false)
	{
		return target_->check_stopunit_action(unit_location, remove_movement, remove_attacks);
	}

	virtual void calculate_possible_moves(std::map<map_location,paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, bool enemy,
		bool assume_full_movement=false,
		const std::set<map_location>* remove_destinations=NULL) const
	{
		target_->calculate_possible_moves(possible_moves, srcdst, dstsrc, enemy, assume_full_movement, remove_destinations);
	}

	virtual void calculate_moves(const unit_map& units,
		std::map<map_location,paths>& possible_moves, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement=false,
		const std::set<map_location>* remove_destinations=NULL,
		bool see_all=false) const
	{
		target_->calculate_moves(units, possible_moves, srcdst, dstsrc, enemy, assume_full_movement, remove_destinations, see_all);
	}

	const virtual game_info& get_info() const
	{
		return target_->get_info();
	}

	virtual void raise_user_interact() const
	{
		target_->raise_user_interact();
	}


	virtual const std::set<map_location>& avoided_locations()
	{
		return target_->avoided_locations();
	}


	virtual int get_recursion_count() const
	{
		return target_->get_recursion_count();
	}


	virtual const move_map& get_dstsrc() const
	{
		return target_->get_dstsrc();
	}


	virtual const move_map& get_enemy_dstsrc() const
	{
		return target_->get_enemy_dstsrc();
	}


	virtual const moves_map& get_enemy_possible_moves() const
	{
		return target_->get_enemy_possible_moves();
	}


	virtual const move_map& get_enemy_srcdst() const
	{
		return target_->get_enemy_srcdst();
	}


	virtual const moves_map& get_possible_moves() const
	{
		return target_->get_possible_moves();
	}


	virtual const move_map& get_srcdst() const
	{
		return target_->get_srcdst();
	}


	virtual void invalidate_avoided_locations_cache()
	{
		target_->invalidate_avoided_locations_cache();
	}


	virtual void recalculate_move_maps()
	{
		target_->recalculate_move_maps();
	}

private:
	readonly_context *target_;
};


class readwrite_context_proxy : public virtual readwrite_context, public virtual readonly_context_proxy {
public:
	readwrite_context_proxy()
		: target_(NULL)
	{
	}


	void init_readwrite_context_proxy(readwrite_context &target)
	{
		init_readonly_context_proxy(target);
		target_ = &target.get_readwrite_context();
	}


	virtual readwrite_context& get_readwrite_context()
	{
		return target_->get_readwrite_context();
	}


	virtual attack_result_ptr execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon)
	{
		return target_->execute_attack_action(attacker_loc,defender_loc,attacker_weapon);
	}


	virtual move_result_ptr execute_move_action(const map_location& from, const map_location& to, bool remove_movement=true)
	{
		return target_->execute_move_action(from, to, remove_movement);
	}


	virtual recruit_result_ptr execute_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location)
	{
		return target_->execute_recruit_action(unit_name,where);
	}


	virtual stopunit_result_ptr execute_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false)
	{
		return target_->execute_stopunit_action(unit_location,remove_movement,remove_attacks);
	}


	virtual team& current_team_w()
	{
		return target_->current_team_w();
	}


	virtual void attack_enemy(const map_location u, const map_location target, int att_weapon, int def_weapon)
	{
		target_->attack_enemy(u, target, att_weapon, def_weapon);
	}


	virtual map_location move_unit(map_location from, map_location to, std::map<map_location,paths>& possible_moves)
	{
		return target_->move_unit(from, to, possible_moves);
	}


	virtual map_location move_unit_partial(map_location from, map_location to, std::map<map_location,paths>& possible_moves)
	{
		return target_->move_unit_partial(from, to, possible_moves);
	}


	virtual bool recruit(const std::string& unit_name, map_location loc=map_location())
	{
		return target_->recruit(unit_name, loc);
	}


	virtual void raise_unit_recruited() const
	{
		target_->raise_unit_recruited();
	}


	virtual void raise_unit_moved() const
	{
		target_->raise_unit_moved();
	}


	virtual void raise_enemy_attacked() const
	{
		target_->raise_enemy_attacked();
	}


	virtual game_info& get_info_w()
	{
		return target_->get_info_w();
	}


	virtual int get_recursion_count()
	{
		return target_->get_recursion_count();
	}
private:
	readwrite_context *target_;
};


//implementation
class side_context_impl : public side_context {
public:
	side_context_impl(side_number side)
		: side_(side), recursion_counter_(0)
	{
	}

	virtual ~side_context_impl(){}

	virtual side_number get_side() const
	{
		return side_;
	}

	virtual void set_side(side_number side)
	{
		side_ = side;
	}


	virtual side_context& get_side_context()
	{
		return *this;
	}


	virtual int get_recursion_count() const;

private:
	side_number side_;
	recursion_counter recursion_counter_;
};


//@todo: public game_logic::formula_callable
class readonly_context_impl : public virtual side_context_proxy, public readonly_context {
public:

	/**
	 * Constructor
	 */
	readonly_context_impl(side_context &context)
		: recursion_counter_(context.get_recursion_count()),dstsrc_(),enemy_dstsrc_(),enemy_possible_moves_(),enemy_srcdst_(),possible_moves_(),srcdst_(),avoided_locations_()
	{
		init_side_context_proxy(context);
	}


	/**
	 * Destructor
	 */
	virtual ~readonly_context_impl() {}


	/**
	 * Unwrap - this class is not a proxy, so return *this
:w
	 */
	virtual readonly_context& get_readonly_context()
	{
		return *this;
	}


	/** Return a reference to the 'team' object for the AI. */
	const team& current_team() const { return get_info().teams[get_side()-1]; }


	/** Show a diagnostic message on the screen. */
	void diagnostic(const std::string& msg);


	/** Display a debug message as a chat message. */
	void log_message(const std::string& msg);


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
	attack_result_ptr check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon);


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
	move_result_ptr check_move_action(const map_location& from, const map_location& to, bool remove_movement=true);


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
	recruit_result_ptr check_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location);


	/**
	 * Check if it is possible to remove unit movements and/or attack
	 * @param unit_location the location of our unit
	 * @param remove_movement set remaining movements to 0
	 * @param remove_attacks set remaining attacks to 0
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	stopunit_result_ptr check_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false);


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
	 * @param remove_destinations a pointer to a set of possible destinations
	 *                            to omit.
	 */
	void calculate_possible_moves(std::map<map_location,paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, bool enemy,
		bool assume_full_movement=false,
		const std::set<map_location>* remove_destinations=NULL) const;

 	/**
	 * A more fundamental version of calculate_possible_moves which allows the
	 * use of a speculative unit map.
	 */
	void calculate_moves(const unit_map& units,
		std::map<map_location,paths>& possible_moves, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement=false,
		const std::set<map_location>* remove_destinations=NULL,
		bool see_all=false) const;


	const virtual game_info& get_info() const;

	/**
	 * Function which should be called frequently to allow the user to interact
	 * with the interface. This function will make sure that interaction
	 * doesn't occur too often, so there is no problem with calling it very
	 * regularly.
	 */
	void raise_user_interact() const;


	virtual const std::set<map_location>& avoided_locations();


	virtual int get_recursion_count() const;


	virtual const move_map& get_dstsrc() const;


	virtual const move_map& get_enemy_dstsrc() const;


	virtual const moves_map& get_enemy_possible_moves() const;


	virtual const move_map& get_enemy_srcdst() const;


	virtual const moves_map& get_possible_moves() const;


	virtual const move_map& get_srcdst() const;


	virtual void invalidate_avoided_locations_cache();


	virtual void recalculate_move_maps();


private:
	recursion_counter recursion_counter_;
	move_map dstsrc_;
	move_map enemy_dstsrc_;
	moves_map enemy_possible_moves_;
	move_map enemy_srcdst_;
	moves_map possible_moves_;
	move_map srcdst_;
	std::set<map_location> avoided_locations_;

};

class readwrite_context_impl : public virtual readonly_context_proxy, public readwrite_context {
public:
	/**
	 * Unwrap - this class is not a proxy, so return *this
	 */
	virtual readwrite_context& get_readwrite_context()
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
	virtual attack_result_ptr execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon);


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
	virtual move_result_ptr execute_move_action(const map_location& from, const map_location& to, bool remove_movement=true);


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
	virtual recruit_result_ptr execute_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location);


	/**
	 * Ask the game to remove unit movements and/or attack
	 * @param unit_location the location of our unit
	 * @param remove_movement set remaining movements to 0
	 * @param remove_attacks set remaining attacks to 0
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	virtual stopunit_result_ptr execute_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false);


	/** Return a reference to the 'team' object for the AI. */
	virtual team& current_team_w() { return get_info_w().teams[get_side()-1]; }


	/**
	 * This function should be called to attack an enemy.
	 *
	 * @deprecated
	 * @param u            The location of the attacking unit. (Note this shouldn't
	 *                     be a reference since attack::attack() can invalidate the
	 *                     unit_map and references to the map are also invalid then.)
	 * @param target       The location of the target unit. This unit must be in
	 *                     range of the attacking unit's weapon. (See note at param u.)
	 * @param weapon       The number of the weapon (0-based) which should be used
	 *                     by the attacker. (It must be a valid weapon of the attacker.)
	 * @param def_weapon   The number of the weapon (0-based) which should be used
	 *                     by the defender. (It must be a valid weapon of the defender.)
	 */
	virtual void attack_enemy(const map_location u, const map_location target, int att_weapon, int def_weapon);


	/**
	 * This function should be called to move a unit.
	 *
	 * @deprecated
	 * Once the unit has been moved, its movement allowance is set to 0.
	 * @param from                The location of the unit being moved.
	 * @param to                  The location to be moved to. This must be a
	 *                            valid move for the unit.
	 * @param possible_moves      The map of possible moves, as obtained from
	 *                            'calculate_possible_moves'.
	 */
	virtual map_location move_unit(map_location from, map_location to, std::map<map_location,paths>& possible_moves);

	/**
	 * @deprecated
	 * Identical to 'move_unit', except that the unit's movement
	 * isn't set to 0 after the move is complete.
	 */
	map_location move_unit_partial(map_location from, map_location to, std::map<map_location,paths>& possible_moves);


	/**
	 * Recruit a unit. It will recruit the unit with the given name,
	 * at the given location, or at an available location to recruit units
	 * if 'loc' is not a valid recruiting location.
	 *
	 * @retval false              If recruitment cannot be performed, because
	 *                            there are no available tiles, or not enough
	 *                            money.
	 */
	bool recruit(const std::string& unit_name, map_location loc=map_location());

	/** Notifies all interested observers of the event respectively. */
	void raise_unit_recruited() const;
	void raise_unit_moved() const;
	void raise_enemy_attacked() const;


	/**
	 * Constructor.
	 */
	readwrite_context_impl(readonly_context &context)
		: recursion_counter_(context.get_recursion_count())
	{
		init_readonly_context_proxy(context);
	}


	virtual ~readwrite_context_impl() {}

	/**
	 * Functions to retrieve the 'info' object.
	 * Used by derived classes to discover all necessary game information.
	 */
	virtual game_info& get_info_w();


	virtual int get_recursion_count() const;

private:
	recursion_counter recursion_counter_;
};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
