/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 *  @file
 *  Replay control code.
 */

#ifndef REPLAY_H_INCLUDED
#define REPLAY_H_INCLUDED

#include "config.hpp"
#include "map_location.hpp"

#include <deque>
#include <map>
#include <set>
class game_display;
class terrain_label;
class unit_map;
class play_controller;
struct time_of_day;

class chat_msg {
public:
	const std::string &text() const { return text_; }
	const std::string &nick() const { return nick_; }
	const std::string &color() const { return color_; }
	const time_t &time() const { return time_; }
	chat_msg(const config &cfg);
	virtual ~chat_msg();
private:
	std::string color_;
	std::string nick_;
	std::string text_;
	time_t time_;
};

class replay
{
public:
	replay();
	explicit replay(const config& cfg);

	void append(const config& cfg);

	void set_skip(bool skip);
	bool is_skipping() const;

	void add_start();
	void add_countdown_update(int value,int team);

	void add_synced_command(const std::string& name, const config& command);
	void init_side();
	/*
		returns a reference to the newest config that us not dependent or has undo =no
	
	*/
	config& get_last_real_command();
	/**
		adds a user_input to the replay
		@param from_side the side that had to make the decision, -1 for 'server'
	*/
	void user_input(const std::string &, const config &, int from_side);
	void add_label(const terrain_label*);
	void clear_labels(const std::string&, bool);
	void add_rename(const std::string& name, const map_location& loc);
	void end_turn();
	void add_unit_checksum(const map_location& loc,config* const cfg);
	void add_checksum_check(const map_location& loc);
	void add_log_data(const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const config& c);


	void add_chat_message_location();
	void speak(const config& cfg);
	const std::vector<chat_msg>& build_chat_log();

	//get data range will get a range of moves from the replay system.
	//if data_type is 'ALL_DATA' then it will return all data in this range
	//except for undoable data that has already been sent. If data_type is
	//NON_UNDO_DATA, then it will only retrieve undoable data, and will mark
	//it as already sent.
	//undoable data includes moves such as placing a label or speaking, which is
	//ignored by the undo system.
	enum DATA_TYPE { ALL_DATA, NON_UNDO_DATA };
	config get_data_range(int cmd_start, int cmd_end, DATA_TYPE data_type=ALL_DATA);
	config get_last_turn(int num_turns=1);
	const config& get_replay_data() const { return cfg_; }
	
	void undo();
	/*
	undoes the last move and puts it into given config to be reone with redo
	this is good, because even undoable commands can have dependent commands, which would otherwise get lost causing oos.
	*/
	void undo_cut(config& dst);
	/*
		puts the given config which was cut with undo_cut back in the replay.
	*/
	void redo(const config& dst);

	void start_replay();
	void revert_action();
	config* get_next_action();

	bool at_end() const;
	void set_to_end();

	void clear();
	bool empty();

	enum MARK_SENT { MARK_AS_UNSENT, MARK_AS_SENT };
	void add_config(const config& cfg, MARK_SENT mark=MARK_AS_UNSENT);

	int ncommands() const;

	static void process_error(const std::string& msg);

private:

	void add_chat_log_entry(const config &speak, std::back_insert_iterator< std::vector<chat_msg> > &i) const;

	config &command(int);
	void remove_command(int);
	/** Adds a new empty command to the command list.
	 *
	 * @return a pointer to the added command
	 */
	config* add_command();
	config cfg_;
	int pos_;

	bool skip_;

	std::vector<int> message_locations;

	/*a leftover from rng*/
	void set_random(config*) {}
};

replay& get_replay_source();

extern replay recorder;

enum REPLAY_RETURN 
{
	REPLAY_RETURN_AT_END,
	REPLAY_FOUND_DEPENDENT,
	REPLAY_FOUND_END_TURN
};
//replays up to one turn from the recorder object
//returns true if it got to the end of the turn without data running out
REPLAY_RETURN do_replay(int side_num);

REPLAY_RETURN do_replay_handle(int side_num);

class replay_network_sender
{
public:
	replay_network_sender(replay& obj);
	~replay_network_sender();

	void sync_non_undoable();
	void commit_and_sync();
private:
	replay& obj_;
	int upto_;
};

namespace mp_sync {

/**
 * Interface for querying local choices.
 * It has to support querying the user and making a random choice
 */
struct user_choice
{
	virtual ~user_choice() {}
	virtual config query_user(int side) const = 0;
	virtual config random_choice(int side) const = 0;
	///whether the choice is visible for the user like an advacement choice
	///a non-visible choice is for example get_global_variable
	virtual bool is_visible() const { return true; }
};

/**
 * Performs a choice for WML events.
 *
 * The choice is synchronized across all the multiplayer clients and
 * stored into the replay. The function object is called if the local
 * client is responsible for making the choice.
 * otherwise this function waits for a remote choice and returns it when it is received.
 * information about the choice made is saved in replay with dependent=true
 * 
 * @param name Tag used for storing the choice into the replay.
 * @param side The number of the side responsible for making the choice.
 *             If zero, it defaults to the currently active side.
 *
 * @note In order to prevent issues with sync, crash, or infinite loop, a
 *       number of precautions must be taken when getting a choice from a
 *       specific side.
 *       - The server must recognize @name replay commands as legal from
 *         non-active players. Preferably the server should be notified
 *         about which player the data is expected from, and discard data
 *         from unexpected players.
 */
config get_user_choice(const std::string &name, const user_choice &uch,
	int side = 0);
/**
 * Performs a choice for mutiple sides for WML events.
 * uch is called on all sies specified in sides, this in done simulaniously on all those sides (or one after another if one client controlls mutiple sides)
 * and after all calls are executed the results are returned.
 */
std::map<int, config> get_user_choice_multiple_sides(const std::string &name, const user_choice &uch,
	std::set<int> sides);

}

#endif
