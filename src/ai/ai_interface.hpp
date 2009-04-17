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
 * @file ai/ai_interface.hpp
 * Interface to the AI.
 */

#ifndef AI_AI_INTERFACE_HPP_INCLUDED
#define AI_AI_INTERFACE_HPP_INCLUDED

class game_display;
class gamemap;

#include "../formula_callable.hpp"
#include "../pathfind.hpp"
#include "../gamestatus.hpp"
#include "../playturn.hpp"

class ai_attack_result;
class ai_move_result;
class ai_recruit_result;
class ai_stopunit_result;

class ai_interface : public game_logic::formula_callable {
public:
	/** get the 1-based side number which is controlled by this AI */
	unsigned int get_side() const { return side_;}

	/** get the 'master' flag of the AI. 'master' AI is the top-level-AI. */
	bool get_master() const { return master_;}


	/** A convenient typedef for the often used 'location' object. */
	typedef map_location location;

	/** The standard way in which a map of possible moves is recorded. */
	typedef std::multimap<location,location> move_map;

	/** The standard way in which a map of possible movement routes to location is recorded*/
	typedef std::map<location,paths> moves_map;

	/**
	 * info is structure which holds references to all the important objects
	 * that an AI might need access to, in order to make and implement its
	 * decisions.
	 */
	struct info {
		info(game_display& disp, gamemap& map, unit_map& units,
			std::vector<team>& teams, gamestatus& state, class game_state& game_state)
			: disp(disp), map(map), units(units), teams(teams),
			   state(state), game_state_(game_state)
		{}

		/** The display object, used to draw the moves the AI makes. */
		game_display& disp;

		/** The map of the game -- use this object to find the terrain at any location. */
		gamemap& map;

		/** The map of units. It maps locations -> units. */
		unit_map& units;

		/** A list of the teams in the game. */
		std::vector<team>& teams;

		/** Information about what turn it is, and what time of day. */
		gamestatus& state;

		/** The global game state, because we may set the completion field. */
		class game_state& game_state_;
	};

	/**
	 * The constructor.
	 *
	 * All derived classes should take an argument of type info& which they
	 * should pass to this constructor.
	 */
	ai_interface(int side, bool master) : side_(side), master_(master) {
		add_ref(); //this class shouldn't be reference counted.
	}
	virtual ~ai_interface() {}

	/**
	 * Function that is called when the AI must play its turn.
	 * Derived classes should implement their AI algorithm in this function.
	 */
	virtual void play_turn() = 0;

	/**
	 * Function called when a a new turn is played
	 * Derived persistant AIs should call this function each turn (expect first)
	 */
	virtual void new_turn() {
	}

	/** Return a reference to the 'team' object for the AI. */
	team& current_team() { return get_info().teams[get_side()-1]; }
	const team& current_team() const { return get_info().teams[get_side()-1]; }

	/** Show a diagnostic message on the screen. */
	void diagnostic(const std::string& msg);

	/** Display a debug message as a chat message. */
	void log_message(const std::string& msg);

        /** Set the side */
        virtual void set_side(unsigned int side) { side_ = side; }

        /** Evaluate */
        virtual std::string evaluate(const std::string& /*str*/)
			{ return "evaluate command not implemented by this AI"; }

        /** Return a message with information about the ai. Useful for making debugging ai-independent. */
        virtual std::string describe_self() { return "? ai"; }
protected:

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
	std::auto_ptr<ai_attack_result> execute_attack_action(const location& attacker_loc, const location& defender_loc, int attacker_weapon);
	std::auto_ptr<ai_attack_result> check_attack_action(const location& attacker_loc, const location& defender_loc, int attacker_weapon);


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
	std::auto_ptr<ai_move_result> execute_move_action(const location& from, const location& to, bool remove_movement=true);
	std::auto_ptr<ai_move_result> check_move_action(const location& from, const location& to, bool remove_movement=true);


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
	std::auto_ptr<ai_recruit_result> execute_recruit_action(const std::string& unit_name, const location &where = map_location::null_location);
	std::auto_ptr<ai_recruit_result> check_recruit_action(const std::string& unit_name, const location &where = map_location::null_location);


	/**
	 * Ask the game to remove unit movements and/or attack
	 * @param unit_location the location of our unit
	 * @param remove_movement set remaining movements to 0
	 * @param remove_attacks set remaining attacks to 0
	 * @retval possible result: ok
	 * @retval possible_result: something wrong
	 * @retval possible_result: nothing to do
	 */
	std::auto_ptr<ai_stopunit_result> execute_stopunit_action(const location& unit_location, bool remove_movement = true, bool remove_attacks = false);
	std::auto_ptr<ai_stopunit_result> check_stopunit_action(const location& unit_location, bool remove_movement = true, bool remove_attacks = false);


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
	void attack_enemy(const location u, const location target, int att_weapon, int def_weapon);

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
	location move_unit(location from, location to, std::map<location,paths>& possible_moves);

	/**
	 * @deprecated
	 * Identical to 'move_unit', except that the unit's movement
	 * isn't set to 0 after the move is complete.
	 */
	location move_unit_partial(location from, location t, std::map<location,paths>& possible_moves);

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
	void calculate_possible_moves(std::map<location,paths>& possible_moves,
		move_map& srcdst, move_map& dstsrc, bool enemy,
		bool assume_full_movement=false,
		const std::set<location>* remove_destinations=NULL) const;

 	/**
	 * A more fundamental version of calculate_possible_moves which allows the
	 * use of a speculative unit map.
	 */
	void calculate_moves(const unit_map& units,
		std::map<location,paths>& possible_moves, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement=false,
		const std::set<location>* remove_destinations=NULL,
		bool see_all=false) const;

	/**
	 * Recruit a unit. It will recruit the unit with the given name,
	 * at the given location, or at an available location to recruit units
	 * if 'loc' is not a valid recruiting location.
	 *
	 * @retval false              If recruitment cannot be performed, because
	 *                            there are no available tiles, or not enough
	 *                            money.
	 */
	bool recruit(const std::string& unit_name, location loc=location());

	/**
	 * Functions to retrieve the 'info' object.
	 * Used by derived classes to discover all necessary game information.
	 */
	info& get_info();
	const info& get_info() const;

	/**
	 * Function which should be called frequently to allow the user to interact
	 * with the interface. This function will make sure that interaction
	 * doesn't occur too often, so there is no problem with calling it very
	 * regularly.
	 */
	void raise_user_interact() const;

	/** Notifies all interested observers of the event respectively. */
	void raise_unit_recruited() const;
	void raise_unit_moved() const;
	void raise_enemy_attacked() const;
protected:
	virtual void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	virtual variant get_value(const std::string& key) const;
private:
	unsigned int side_;
	bool master_;
};

#endif
