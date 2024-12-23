/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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
 *  @file
 *  Replay control code.
 */

#pragma once

#include "map/location.hpp"

#include <chrono>
#include <iterator>
class replay_recorder_base;
class terrain_label;
class config;

class chat_msg {
public:
	const std::string &text() const { return text_; }
	const std::string &nick() const { return nick_; }
	const std::string &color() const { return color_; }
	const auto& time() const { return time_; }
	chat_msg(const config &cfg);
	virtual ~chat_msg();
private:
	std::string color_;
	std::string nick_;
	std::string text_;
	std::chrono::system_clock::time_point time_;
};

enum class REPLAY_ACTION_TYPE
{
	// Chat and similar actions that don't change the gamestate
	UNSYNCED,
	// Local choices
	DEPENDENT,
	// Commands the invoke a synced user actions
	SYNCED,
	// The actions has a wrong format.
	INVALID
};

class replay
{
public:
	explicit replay(replay_recorder_base& base);


	void add_start();
	void add_surrender(int side_number);
	void add_countdown_update(int value,int team);

	void add_synced_command(const std::string& name, const config& command);
	void init_side();
	/*
		returns a reference to the newest config that us not dependent or has undo =no

	*/
	config& get_last_real_command();
	/**
	 * adds a user_input to the replay
	 * @param name The tag name of the config to add
	 * @param input the contents of the config to add
	 * @param from_side the side that had to make the decision, -1 for 'server'
	*/
	void user_input(const std::string &name, const config &input, int from_side);
	void add_label(const terrain_label*);
	void clear_labels(const std::string&, bool);
	void add_rename(const std::string& name, const map_location& loc);
	void end_turn(int next_player_number);
	void add_unit_checksum(const map_location& loc,config& cfg);
	void add_log_data(const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const config& c);

	/**
		adds a chat message if it wasn't added yet.
		@returns true if a message location was added
	*/
	bool add_chat_message_location();
	bool add_chat_message_location(int pos);
	void speak(const config& cfg);
	const std::vector<chat_msg>& build_chat_log() const;

	//get data range will get a range of moves from the replay system.
	//if data_type is 'ALL_DATA' then it will return all data in this range
	//except for undoable data that has already been sent. If data_type is
	//NON_UNDO_DATA, then it will only retrieve undoable data, and will mark
	//it as already sent.
	//undoable data includes moves such as placing a label or speaking, which is
	//ignored by the undo system.
	enum DATA_TYPE { ALL_DATA, NON_UNDO_DATA };
	config get_unsent_commands(DATA_TYPE data_type);

	void undo();
	/*
		undoes the last move and puts it into given config to be reone with redo
		The retuned config also contains the depended commands for that user action.
		This is needed be because we also want to readd those dependent commands to the replay when redoing the command.
	*/
	void undo_cut(config& dst);
	/*
		puts the given config which was cut with undo_cut back in the replay.
	*/
	void redo(const config& dst, bool set_to_end = false);

	void start_replay();
	void revert_action();
	config* get_next_action();
	config* peek_next_action();

	bool at_end() const;
	void set_to_end();

	bool empty() const;

	enum MARK_SENT { MARK_AS_UNSENT, MARK_AS_SENT };
	void add_config(const config& cfg, MARK_SENT mark=MARK_AS_UNSENT);

	int ncommands() const;

	static void process_error(const std::string& msg);
	/*
		adds a [start] at the begnning of the replay if there is none.
		returns true if a [start] was added.
	*/
	bool add_start_if_not_there_yet();
	void delete_upcoming_commands();
private:

	void add_chat_log_entry(const config &speak, std::back_insert_iterator< std::vector<chat_msg>> &i) const;

	config &command(int) const;
	void remove_command(int);
	/** Adds a new empty command to the command list at the end.
	 *
	 * @return a reference to the added command
	 */
	config& add_command();
	/**
	 * adds a new command to the command list at the current position.
	 *
	 * @return a reference to the added command
	 */
	config& add_nonundoable_command();
	replay_recorder_base* base_;
	int sent_upto_;
	std::vector<int> message_locations;
};

enum REPLAY_RETURN
{
	REPLAY_RETURN_AT_END,
	REPLAY_FOUND_DEPENDENT,
	REPLAY_FOUND_END_TURN,
	REPLAY_FOUND_INIT_TURN,
	REPLAY_FOUND_END_MOVE,
	REPLAY_FOUND_END_LEVEL
};

REPLAY_ACTION_TYPE get_replay_action_type(const config& command);

//replays up to one turn from the recorder object
//returns true if it got to the end of the turn without data running out
REPLAY_RETURN do_replay(bool one_move = false);

REPLAY_RETURN do_replay_handle(bool one_move = false);
