/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Managing the AIs lifecycle - headers
 * TODO: Refactor history handling and internal commands.
 * TODO: AI Interface command to clear the history.
 */

#pragma once

#include "ai/game_info.hpp"       // for side_number, ai_ptr
#include "config.hpp"             // for config, etc
#include "generic_event.hpp"      // for generic_event, etc

#include <deque>                        // for deque
#include <map>                          // for map, map<>::value_compare
#include <stack>                        // for stack
#include <string>                       // for string

class game_launcher;
namespace ai { class ai_composite; }  // lines 45-45
namespace ai { class ai_context; }  // lines 42-42
namespace ai { class component; }  // lines 43-43
namespace ai { class default_ai_context; }  // lines 41-41
namespace ai { class readonly_context; }  // lines 39-39
namespace ai { class readwrite_context; }  // lines 40-40
namespace ai { class side_context; }  // lines 38-38
namespace events { class generic_event; }
namespace events { class observer; }


namespace ai
{

/**
 * Base class that holds the AI and current AI parameters.
 * It is an implementation detail.
 */
class holder{
public:
	holder(side_number side, const config &cfg);

	virtual ~holder();

	ai_composite& get_ai_ref();

	const std::string describe_ai();

	config to_config() const;

	void modify_ai(const config& cfg);


	void append_ai(const config& cfg);


	const std::string get_ai_overview();


	const std::string get_ai_structure();


	const std::string get_ai_identifier() const;

	component* get_component(component *root, const std::string &path); // Ai debug method

private:
	void init( side_number side );


	composite_ai_ptr ai_;
	std::unique_ptr<side_context> side_context_;
	std::unique_ptr<readonly_context> readonly_context_;
	std::unique_ptr<readwrite_context> readwrite_context_;
	std::unique_ptr<default_ai_context> default_ai_context_;
	side_number side_;
	config cfg_;
};

/**
 * AI Command History Item. It is an implementation detail. See TODOs above.
 */
class command_history_item{
public:

	command_history_item(int number, const std::string &command)
		: number_(number), command_(command)
	{}

	int get_number() const { return number_; }

	const std::string& get_command() const { return command_; }

private:
	int number_;
	std::string command_;

};

/**
 * Class that manages AIs for all sides and manages AI redeployment.
 * This class is responsible for managing the AI lifecycle.
 */
class manager
{
public:

	// =======================================================================
	// CONSTANTS
	// =======================================================================

	static const std::size_t MAX_HISTORY_SIZE = 200;

	static const std::string AI_TYPE_COMPOSITE_AI;
	static const std::string AI_TYPE_SAMPLE_AI;
	static const std::string AI_TYPE_IDLE_AI;
	static const std::string AI_TYPE_FORMULA_AI;
	static const std::string AI_TYPE_DFOOL_AI;
	static const std::string AI_TYPE_AI2;
	static const std::string AI_TYPE_DEFAULT;


	// =======================================================================
	// CONSTRUCTORS AND DESTRUCTORS
	// =======================================================================

	manager();

	/* The singleton can't be set to null in the destructor because member objects
	(which access the singleton) are destroyed *after* the destructor has been run. */
	~manager() = default;

	// =======================================================================
	// ACCESS TO MANAGER
	// =======================================================================

	static manager& get_singleton()
	{
		assert(singleton_ != nullptr);
		return *singleton_;
	}

	static bool has_manager()
	{
		return singleton_ != nullptr;
	}

	// =======================================================================
	// LIFECYCLE
	// =======================================================================


	/**
	 * Adds observer of game events.
	 * Should be called in playsingle_controller 's constructor.
	 */
	void add_observer( events::observer* event_observer);


	/**
	 * Removes an observer of game events.
	 * Should be called in playsingle_controller 's destructor.
	 */
	void remove_observer( events::observer* event_observer );


	/**
	 * Adds observer of game events except ai_user_interact event and ai_sync_network event
	 */
	void add_gamestate_observer( events::observer* event_observer);


	/**
	 * Removes an observer of game events except ai_user_interact event and ai_sync_network event
	 */
	void remove_gamestate_observer( events::observer* event_observer );


	/**
	 * Notifies all observers of 'ai_user_interact' event.
	 * Function which should be called frequently to allow the user to interact
	 * with the interface. This function will make sure that interaction
	 * doesn't occur too often, so there is no problem with calling it very
	 * regularly.
	 */
	void raise_user_interact();

	/**
	 * Notifies all observers of 'ai_sync_network' event.
	 * Basically a request from the AI to sync the network.
	 */
	void raise_sync_network();


	/**
	 * Notifies all observers of 'ai_gamestate_changed' event.
	 */
	void raise_gamestate_changed();


	/**
	 * Notifies all observers of 'ai_tod_changed' event.
	 */
	void raise_tod_changed();


	/**
	 * Notifies all observers of 'ai_recruit_list_changed' event.
	 */
	void raise_recruit_list_changed();


	/**
	 * Notifies all observers of 'ai_turn_started' event.
	 */
	void raise_turn_started();


	/**
	 * Notifies all observers of 'ai_map_changed' event.
	 */
	void raise_map_changed();


	/**
	 * Adds an observer of 'ai_map_changed' event.
	 */
	void add_map_changed_observer( events::observer* event_observer );


	/**
	 * Adds an observer of 'ai_recruit_list_changed' event.
	 */
	void add_recruit_list_changed_observer( events::observer* event_observer );


	/**
	 * Adds an observer of 'ai_turn_started' event.
	 */
	void add_turn_started_observer( events::observer* event_observer );


	/**
	 * Adds an observer of 'ai_tod_changed' event.
	 */
	void add_tod_changed_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'ai_map_changed' event.
	 */
	void remove_map_changed_observer( events::observer* event_observer );



	/**
	 * Deletes an observer of 'ai_recruit_list_changed' event.
	 */
	void remove_recruit_list_changed_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'ai_turn_started' event.
	 */
	void remove_turn_started_observer( events::observer* event_observer );


	/**
	 * Deletes an observer of 'ai_tod_changed' event.
	 */
	void remove_tod_changed_observer( events::observer* event_observer );


public:

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
	const std::string evaluate_command( side_number side, const std::string& str );


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
	bool add_ai_for_side_from_file( side_number side, const std::string& file, bool replace = true );


	/**
	 * Adds active AI for specified @a side from @a cfg.
	 * @note Running this command may invalidate references previously returned
	 *       by manager. AI is not initialized at this point.
	 * @param side side number (1-based, as in game_info).
	 * @param cfg the config from which all ai parameters are to be read.
	 * @param replace should new ai replace the current ai or 'be placed on top of it'.
	 * @return true if successful.
	 */
	bool add_ai_for_side_from_config(side_number side, const config &cfg, bool replace = true);


	// =======================================================================
	// REMOVE
	// =======================================================================

	/**
	 * Removes top-level AI from @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 */
	void remove_ai_for_side( side_number side );


	/**
	 * Removes all AIs from @a side.
	 * @note Running this command may invalidate references previously returned
	 *       by manager.
	 * @param side side number (1-based, as in game_info).
	 */
	void remove_all_ais_for_side( side_number side );


	/**
	 * Clears all the AIs.
	 * @note Running this command may invalidate references previously returned
	 *       by manager. For example, this is called from the destructor of
	 *       playsingle_controller. It is necessary to do this if any of the
	 *       info structures used by the AI goes out of scope.
	 */
	void clear_ais();

	// =======================================================================
	// GET active AI parameters
	// =======================================================================


	/**
	 * Gets AI info for active AI of the given @a side.
	 * @param side side number (1-based).
	 * @return a reference to active AI info.
	 */
	game_info& get_active_ai_info_for_side( side_number side );


	/**
	 * Gets AI Overview for active AI of the given @a side
	 * @param side side number (1-based)
	 * @return an ai overview
	 */
	std::string get_active_ai_overview_for_side( side_number side );


	/**
	 * Gets AI Structure for active AI of the given @a side
	 * @param side side number (1-based)
	 * @return an ai structure
	 */
	std::string get_active_ai_structure_for_side( side_number side );

	/**
	 * Gets AI algorithm identifier for active AI of the given @a side.
	 * @param side side number (1-based).
	 * @return ai identifier for the active AI
	 */
	std::string get_active_ai_identifier_for_side( side_number side );

	/**
	 * Gets the active AI holder for debug purposes.
	 * Will only work in debug mode, otherwise returns a reference to an empty holder
	 * @param side side number(1-based)
	 * @return debug ? active holder : empty holder
	 */
	ai::holder& get_active_ai_holder_for_side_dbg(side_number side);

	/**
	 * Gets AI config for active AI of the given @a side.
	 * @param side side number (1-based).
	 * @return a config object for the active AI
	 */
	config to_config( side_number side );


	/**
	 * Gets global AI-game info
	 * @return a reference to the AI-game info.
	 */
	game_info& get_ai_info();


	// =======================================================================
	// SET active AI parameters
	// =======================================================================

	/**
	 * Modifies AI parameters for active AI of the given @a side.
	 * This function is a backend for [modify_ai] tag
	 * @param side side_number (1-based, as in game_info).
	 * @param cfg - content of [modify_ai] tag
	 */

	void modify_active_ai_for_side( ai::side_number side, const config &cfg );

	/**
	 * Appends AI parameters to active AI of the given @a side.
	 * This function is a backend for [modify_side][ai] tag
	 * @param side side_number (1-based, as in game_info).
	 * @param cfg - content of [modify_side][ai] tag
	 */

	void append_active_ai_for_side( ai::side_number side, const config &cfg );

	// =======================================================================
	// PROXY
	// =======================================================================

	/**
	 * Plays a turn for the specified side using its active AI.
	 * @param side side number (1-based, as in game_info).
	 */
	void play_turn(side_number side);


private:

	typedef std::map< side_number, std::stack< holder >> AI_map_of_stacks;

	std::deque< command_history_item > history_;
	long history_item_counter_;
	game_info ai_info_;

	events::generic_event map_changed_;
	events::generic_event recruit_list_changed_;
	events::generic_event user_interact_;
	events::generic_event sync_network_;
	events::generic_event tod_changed_;
	events::generic_event gamestate_changed_;
	events::generic_event turn_started_;
	int last_interact_;
	int num_interact_;

	AI_map_of_stacks ai_map_;

	static manager* singleton_;


	// =======================================================================
	// EVALUATION
	// =======================================================================

	/**
	 * Evaluates an internal manager command.
	 * @param side side number (1-based).
	 * @param str string to evaluate.
	 * @return string result of evaluation.
	 * TODO: rewrite this function to use a fai or lua parser.
	 */
	const std::string internal_evaluate_command( side_number side, const std::string& str );

	/**
	 * Determines if the command should be intercepted and evaluated as internal command.
	 * @param str command string to check.
	 * @return true if the command should be intercepted and evaluated.
	 */
	bool should_intercept( const std::string& str ) const;

	// =======================================================================
	// AI STACKS
	// =======================================================================


	/**
	 * Gets the AI stack for the specified side, create it if it doesn't exist.
	 */
	std::stack< holder >& get_or_create_ai_stack_for_side(side_number side);

	// =======================================================================
	// AI HOLDERS
	// =======================================================================


	/**
	 * Gets active holder for specified @a side.
	 */
	holder& get_active_ai_holder_for_side( side_number side );

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
	ai_composite& get_active_ai_for_side( side_number side );


	friend class ::game_launcher;
};

} //end of namespace ai
