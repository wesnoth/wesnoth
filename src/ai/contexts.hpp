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

#include "ai_interface.hpp"
#include "game_info.hpp"
#include "../game_display.hpp"
#include "../gamestatus.hpp"
#include "../pathfind.hpp"
#include "../playturn.hpp"

class ai_attack_result;
class ai_move_result;
class ai_recruit_result;
class ai_stopunit_result;

class ai_readonly_context: public game_logic::formula_callable, public ai_interface {
public:
	/** A convenient typedef for the often used 'location' object. */
	typedef map_location location;

	/** The standard way in which a map of possible moves is recorded. */
	typedef std::multimap<location,location> move_map;

	/** The standard way in which a map of possible movement routes to location is recorded*/
	typedef std::map<location,paths> moves_map;

	/**
	 * The constructor.
	 */
	ai_readonly_context(unsigned int side, bool master) : ai_interface(side,master) {
		add_ref(); //this class shouldn't be reference counted.
	}
	virtual ~ai_readonly_context() {}

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
	std::auto_ptr<ai_attack_result> check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon);


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
	std::auto_ptr<ai_move_result> check_move_action(const map_location& from, const map_location& to, bool remove_movement=true);


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
	std::auto_ptr<ai_recruit_result> check_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location);


	/**
	 * Check if it is possible to remove unit movements and/or attack
	 * @param unit_location the location of our unit
	 * @param remove_movement set remaining movements to 0
	 * @param remove_attacks set remaining attacks to 0
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	std::auto_ptr<ai_stopunit_result> check_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false);


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
	 *                         edonly_ai_context  If true, the function will operate on the
	 *                            assumption thareadonly_ai_contextt all units can move their full
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


	const virtual ai_game_info& get_info() const;

	/**
	 * Function which should be called frequently to allow the user to interact
	 * with the interface. This function will make sure that interaction
	 * doesn't occur too often, so there is no problem with calling it very
	 * regularly.
	 */
	void raise_user_interact() const;

	virtual void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	virtual variant get_value(const std::string& key) const;

};

class ai_readwrite_context : public ai_readonly_context {
public:


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
	std::auto_ptr<ai_attack_result> execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon);


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
	std::auto_ptr<ai_move_result> execute_move_action(const map_location& from, const map_location& to, bool remove_movement=true);


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
	std::auto_ptr<ai_recruit_result> execute_recruit_action(const std::string& unit_name, const map_location &where = map_location::null_location);


	/**
	 * Ask the game to remove unit movements and/or attack
	 * @param unit_location the location of our unit
	 * @param remove_movement set remaining movements to 0
	 * @param remove_attacks set remaining attacks to 0
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	std::auto_ptr<ai_stopunit_result> execute_stopunit_action(const map_location& unit_location, bool remove_movement = true, bool remove_attacks = false);


	/** Return a reference to the 'team' object for the AI. */
	team& current_team() { return get_info().teams[get_side()-1]; }
	const team& current_team() const { return get_info().teams[get_side()-1]; }

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
	void attack_enemy(const map_location u, const map_location target, int att_weapon, int def_weapon);

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
	map_location move_unit(map_location from, map_location to, std::map<map_location,paths>& possible_moves);

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
	 * The constructor.
	 */
	ai_readwrite_context(unsigned int side, bool master) : ai_readonly_context(side,master){
	}
	virtual ~ai_readwrite_context() {}

	/**
	 * Functions to retrieve the 'info' object.
	 * Used by derived classes to discover all necessary game information.
	 */
	const virtual ai_game_info& get_info() const;
	virtual ai_game_info& get_info();
};

#endif
