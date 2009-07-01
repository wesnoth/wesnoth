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
 * @file ai/ai_manager.hpp
 * Managing the AIs lifecycle - headers
 * @todo 1.7 Refactor history handling and internal commands.
 * @todo 1.7 Refactor all the mess with those AI parameters.
 * @todo 1.7 AI Interface command to clear the history.
 */

#ifndef AI_MANAGER_HPP_INCLUDED
#define AI_MANAGER_HPP_INCLUDED

#include "../global.hpp"

#include "contexts.hpp"
#include "game_info.hpp"
#include "default/contexts.hpp"

#include <map>
#include <stack>
#include <vector>
#include <deque>


namespace ai {

class interface;

/**
 * Base class that holds the AI and current AI parameters.
 * It is an implementation detail.
 * @todo 1.7.2 move it out of public view
 */
class holder{
public:
	holder(int side, const std::string& ai_algorithm_type);

	void init( int side );

	virtual ~holder();

	interface& get_ai_ref();
	interface& get_ai_ref( int side );

	const std::string& get_ai_algorithm_type() const;
	void set_ai_algorithm_type(const std::string& ai_algorithm_type);

	const config& get_ai_memory() const;
	config& get_ai_memory();
	void set_ai_memory(const config& ai_memory);

	const std::vector<config>& get_ai_parameters() const;
	std::vector<config>& get_ai_parameters();
	void set_ai_parameters(const std::vector<config>& ai_parameters);

	const config& get_ai_effective_parameters() const;
	config& get_ai_effective_parameters();
	void set_ai_effective_parameters(const config& ai_effective_parameters);

	const config& get_ai_global_parameters() const;
	config& get_ai_global_parameters();
	void set_ai_global_parameters(const config& ai_global_parameters);

	const std::string describe_ai();

	//not used in the moment
	bool is_mandate_ok();

private:
	ai_ptr ai_;
	side_context *side_context_;
	readonly_context *readonly_context_;
	readwrite_context *readwrite_context_;
	default_ai_context *default_ai_context_;
	std::string ai_algorithm_type_;
	config ai_effective_parameters_;
	config ai_global_parameters_;
	config ai_memory_;
	std::vector<config> ai_parameters_;
	int side_;

	ai_ptr create_ai( int side );
};

/**
 * AI Command History Item. It is an implementation detail
 */
class command_history_item{
public:
	command_history_item();

	command_history_item(int number, const std::string& command);

	virtual ~command_history_item();

	int get_number() const;
	void set_number(int number);

	const std::string& get_command() const;
	void set_command(const std::string& command);

private:
	int number_;
	std::string command_;

};

/**
 * Class that manages AIs for all sides and manages AI redeployment.
 * This class is responsible for managing the AI lifecycle
 * It can be accessed like this:   ai::manager::foo(...);
 */
class manager
{
public:

	// =======================================================================
	// CONSTANTS
	// =======================================================================

	static const size_t MAX_HISTORY_SIZE = 200;

	static const int AI_TEAM_COMMAND_AI = 0;
	static const int AI_TEAM_FALLBACK_AI = -1;


	static const std::string AI_TYPE_COMPOSITE_AI;
	static const std::string AI_TYPE_SAMPLE_AI;
	static const std::string AI_TYPE_IDLE_AI;
	static const std::string AI_TYPE_FORMULA_AI;
	static const std::string AI_TYPE_DFOOL_AI;
	static const std::string AI_TYPE_AI2;
	static const std::string AI_TYPE_DEFAULT;


	// =======================================================================
	// LIFECYCLE
	// =======================================================================

	/**
	 * Sets AI information.
	 * @param info ai_information to be set.
	 */
	static void set_ai_info(const game_info& info);


	/**
	 * Clears AI information.
	 * Should be called in playsingle_controller 's destructor.
	 */
	static void clear_ai_info();


	/**
	 * Adds observer of game events.
	 * Should be called in playsingle_controller 's constructor.
	 */
	static void add_observer( events::observer* event_observer);


	/**
	 * Removes an observer of game events.
	 * Should be called in playsingle_controller 's destructor.
	 */
	static void remove_observer( events::observer* event_observer );


	/**
	 * Adds observer of game events except user interact event
	 */
	static void add_gamestate_observer( events::observer* event_observer);


	/**
	 * Removes an observer of game events except user interact event
	 */
	static void remove_gamestate_observer( events::observer* event_observer );


	/**
	 * Notifies all observers of 'user interact' event.
	 * Function which should be called frequently to allow the user to interact
	 * with the interface. This function will make sure that interaction
	 * doesn't occur too often, so there is no problem with calling it very
	 * regularly.
	 */
	static void raise_user_interact();


	/**
	 * Notifies all observers of 'unit recruited' event.
	 */
	static void raise_unit_recruited();


	/**
	 * Notifies all observers of 'unit moved' event.
	 */
	static void raise_unit_moved();


	/**
	 * Notifies all observers of 'enemy attacked' event.
	 */
	static void raise_enemy_attacked();


	/**
	 * Notifies all observers of 'turn started' event.
	 */
	static void raise_turn_started();


	/**
	 * Adds an observer of 'user interact' event.
	 */
	static void add_user_interact_observer( events::observer* event_observer );


	/**
	 * Adds an observer of 'unit recruited' event.
	 */
	static void add_unit_recruited_observer( events::observer* event_observer );


	/**
	 * Adds an observer of 'unit moved' event.
	 */
	static void add_unit_moved_observer( events::observer* event_observer );


	/**
	 * Adds an observers of 'enemy attacked' event.
	 */
	static void add_enemy_attacked_observer( events::observer* event_observer );


	/**
	 * Adds an observer of 'turn started' event.
	 */
	static void add_turn_started_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'user interact' event.
	 */
	static void delete_user_interact_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'unit recruited' event.
	 */
	static void delete_unit_recruited_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'unit moved' event.
	 */
	static void delete_unit_moved_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'enemy attacked' event.
	 */
	static void delete_enemy_attacked_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'turn started' event.
	 */
	static void delete_turn_started_observer( events::observer* event_observer );


protected:

	manager();


public:

	virtual ~manager();

	// =======================================================================
	// EVALUATION
	// =======================================================================

	/**
	 * Evaluates a string command using command AI.
	 * @note Running this command may invalidate references previously returned
	 *       by manager. Will intercept those commands which start with '!'
	 *       and '?', and will try to evaluate them as internal commands.
	 * @param side side number (1-based).
	 * @param str string to evaluate.
	 * @return string result of evaluation.
	 */
	static const std::string evaluate_command( int side, const std::string& str );


	// =======================================================================
	// ADD, CREATE AIs, OR LIST AI TYPES
	// =======================================================================

	/**
	 * Adds active AI for specified @a side from @a file.
	 * @note Running this command may invalidate references previously returned
	 *       by manager. AI is not initialized at this point.
	 * @param side side number (1-based, as in game_info).
	 * @param file file name, follows the usual WML convention.
	 * @param replace should new ai replace the current ai or 'be placed on top of it'.
	 * @return true if successful.
	 */
	static bool add_ai_for_side_from_file( int side, const std::string& file, bool replace = true );


	/**
	 * Adds active AI for specified @a side from @a cfg.
	 * @note Running this command may invalidate references previously returned
	 *       by manager. AI is not initialized at this point.
	 * @param side side number (1-based, as in game_info).
	 * @param cfg the config from which all ai parameters are to be read.
	 * @param replace should new ai replace the current ai or 'be placed on top of it'.
	 * @return true if successful.
	 */
	static bool add_ai_for_side_from_config(int side, const config &cfg, bool replace = true);


	/**
	 * Adds active AI for specified @a side from parameters.
	 * @note Running this command may invalidate references previously returned
	 *       by manager. AI is not initialized at this point.
	 * @param side side number (1-based, as in game_info).
	 * @param ai_algorithm_type type of AI algorithm to create.
	 * @param replace should new ai replace the current ai or 'be placed on top of it'.
	 * @return true if successful.
	 */
	static bool add_ai_for_side( int side, const std::string& ai_algorithm_type, bool replace = true);


	/**
	 * Returns a smart pointer to a new AI. 
	 * @param ai_algorithm_type type of AI algorithm to create
	 * @param context context in which this ai is created 
	 * @return the reference to the created AI
	 */
	static ai_ptr create_transient_ai( const std::string& ai_algorithm_type, default_ai_context *ai_context);


	// =======================================================================
	// REMOVE
	// =======================================================================

	/**
	 * Removes top-level AI from @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 */
	static void remove_ai_for_side( int side );


	/**
	 * Removes all AIs from @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 */
	static void remove_all_ais_for_side( int side );


	/**
	 * Clears all the AIs.
	 * @note Running this command may invalidate references previously returned
	 *       by manager. For example, this is called from the destructor of
	 *       playsingle_controller. It is necessary to do this if any of the
	 *       info structures used by the AI goes out of scope.
	 */
	static void clear_ais();

	// =======================================================================
	// GET active AI parameters
	// =======================================================================


	/**
	 * Gets AI parameters for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @return a reference to active AI parameters.
	 * @note This reference may become invalid after specific manager operations.
	 */
	static const std::vector<config>& get_active_ai_parameters_for_side( int side );


	/**
	 * Gets effective AI parameters for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @return a reference to active AI effective parameters.
	 * @note this reference may become invalid after specific manager operations.
	 */
	static const config& get_active_ai_effective_parameters_for_side( int side );


	/**
	 * Get global AI parameters for active AI of the @a given side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @return a reference to active ai global parameters.
	 * @note This reference may become invalid after specific manager operations.
	 */
	static const config& get_active_ai_global_parameters_for_side( int side );


	/**
	 * Gets AI info for active AI of the given @a side.
	 * @param side side number (1-based).
	 * @return a reference to active AI info.
	 */
	static game_info& get_active_ai_info_for_side( int side );


	/**
	 * Gets global AI-game info
	 * @return a reference to the AI-game info.
	 */
	static game_info& get_ai_info();


	/**
	 * Gets AI memory for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @return a reference to active AI memory.
	 * @note This reference may become invalid after specific manager operations.
	 */
	static const config& get_active_ai_memory_for_side( int side );


	/**
	 * Gets AI algorithm type for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @return a reference to active AI algorithm_type.
	 * @note This reference may become invalid after specific manager operations.
	 */
	static const std::string& get_active_ai_algorithm_type_for_side( int side );


	// =======================================================================
	// SET active AI parameters
	// =======================================================================

	/**
	 * Sets AI parameters for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @param ai_parameters AI parameters to be set.
	 */
	static void set_active_ai_parameters_for_side( int side, const std::vector<config>& ai_parameters );


	/**
	 * Sets effective AI parameters for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @param ai_effective_parameters AI effective parameters to be set.
	 * @deprecated Added only for bug-for-bug compatibility with side.cpp.
	 *             Will be refactored away.
	 */
	static void set_active_ai_effective_parameters_for_side( int side, const config& ai_effective_parameters );


	/**
	 * Sets global AI parameters for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @param ai_global_parameters AI global parameters to be set.
	 * @deprecated Added only for bug-for-bug compatibility with side.cpp.
	 *             Will be refactored away.
	 */
	static void set_active_ai_global_parameters_for_side( int side, const config& ai_global_parameters );


	/**
	 * Sets AI memory for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager
	 * @param side side number (1-based, as in game_info).
	 * @param ai_memory AI memory to be set.
	 * @deprecated Added only for bug-for-bug compatibility with side.cpp.
	 *             Will be refactored away.
	 */
	static void set_active_ai_memory_for_side( int side, const config& ai_memory );


	/**
	 * Sets AI algorithm type for active AI of the given @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @param ai_algorithm_type AI algorithm type to be set.
	 */
	static void set_active_ai_algorithm_type_for_side( int side, const std::string& ai_algorithm_type );


	// =======================================================================
	// PROXY
	// =======================================================================

	/**
	 * Plays a turn for the specified side using its active AI.
	 * @param side side number (1-based, as in game_info).
	 */
	static void play_turn(side_number side);


private:

	typedef std::map< int, std::stack< holder > > AI_map_of_stacks;
	static AI_map_of_stacks ai_map_;
	static std::deque< command_history_item > history_;
	static long history_item_counter_;
	static game_info *ai_info_;

	static events::generic_event user_interact_;
	static events::generic_event unit_recruited_;
	static events::generic_event unit_moved_;
	static events::generic_event enemy_attacked_;
	static events::generic_event turn_started_;
	static int last_interact_;
	static int num_interact_;



	// =======================================================================
	// EVALUATION
	// =======================================================================

	/**
	 * Evaluates an internal manager command.
	 * @param side side number (1-based).
	 * @param str string to evaluate.
	 * @return string result of evaluation.
	 * @todo 1.7 rewrite this function to use a fai or lua parser.
	 */
	static const std::string internal_evaluate_command( int side, const std::string& str );

	/**
	 * Determines if the command should be intercepted and evaluated as internal command.
	 * @param str command string to check.
	 * @return true if the command should be intercepted and evaluated.
	 */
	static bool should_intercept( const std::string& str );

	// =======================================================================
	// AI STACKS
	// =======================================================================


	/**
	 * Gets the AI stack for the specified side, create it if it doesn't exist.
	 */
	static std::stack< holder >& get_or_create_ai_stack_for_side(int side);

	// =======================================================================
	// AI HOLDERS
	// =======================================================================


	/**
	 * Gets active holder for specified @a side.
	 */
	static holder& get_active_ai_holder_for_side( int side );

	/**
	 * Gets command holder for specified @a side.
	 */
	static holder& get_command_ai_holder( int side );

	/**
	 * Gets fallback holder for specified @a side.
	 */
	static holder& get_fallback_ai_holder( int side );

	/**
	 * Gets or creates active holder for specified @a side without fallback.
	 */
	static holder& get_or_create_active_ai_holder_for_side_without_fallback(int side, const std::string& ai_algorithm_type);

	// =======================================================================
	// AI POINTERS
	// =======================================================================

	/**
	 * Gets active AI for specified side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 * @return a reference to the active AI.
	 * @note This reference may become invalid after specific manager operations.
	 */
	static interface& get_active_ai_for_side( int side );


	/**
	 * Gets the command AI for the specified @a side.
	 */
	static interface& get_command_ai( int side );


	/**
	 * Gets the fallback AI for the specified @a side.
	 */
	static interface& get_fallback_ai( int side );

	/**
	 * Gets or creates active AI for specified @a side without fallback.
	 */
	static interface& get_or_create_active_ai_for_side_without_fallback( int side, const std::string& ai_algorithm_type );


};

} //end of namespace ai

#endif
