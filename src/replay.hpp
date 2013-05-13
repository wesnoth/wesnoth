/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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
#include "rng.hpp"

#include <deque>

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

class replay: public rand_rng::rng
{
public:
	replay();
	explicit replay(const config& cfg);

	void append(const config& cfg);

	void set_skip(bool skip);
	bool is_skipping() const;

	void add_start();
	void add_recruit(const std::string& type_id, const map_location& loc, const map_location& from);
	void add_recall(const std::string& unit_id, const map_location& loc, const map_location& from);
	void add_disband(const std::string& unit_id);
	void add_countdown_update(int value,int team);
	/// Records a move that follows the provided @a steps.
	void add_movement(const std::vector<map_location>& steps);
	/// Modifies the most recently recorded move to indicate that it
	/// stopped early (due to unforseen circumstances, such as an ambush).
	void limit_movement(const map_location& early_stop);
	void add_attack(const map_location& a, const map_location& b,
		int att_weapon, int def_weapon, const std::string& attacker_type_id,
		const std::string& defender_type_id, int attacker_lvl,
		int defender_lvl, const size_t turn, const time_of_day &t);
	void add_auto_shroud(bool turned_on);
	void update_shroud();
	void add_seed(const char* child_name, int seed);
	void user_input(const std::string &, const config &);
	void add_label(const terrain_label*);
	void clear_labels(const std::string&, bool);
	void add_rename(const std::string& name, const map_location& loc);
	void init_side();
	void end_turn();
	void add_event(const std::string& name,
		const map_location& loc=map_location::null_location);
	void add_unit_checksum(const map_location& loc,config* const cfg);
	void add_checksum_check(const map_location& loc);
	void add_log_data(const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const std::string &var);
	void add_log_data(const std::string &category, const std::string &key, const config& c);

	/**
	 * Mark an expected advancement adding it to the queue
	 */
	void add_expected_advancement(const map_location& loc);

	/**
	 * Access to the expected advancements queue.
	 */
	const std::deque<map_location>& expected_advancements() const;

	/**
	 * Remove the front expected advancement from the queue
	 */
	void pop_expected_advancement();

	/**
	 * Adds an advancement to the replay, the following option command
	 * determines which advancement option has been chosen
	 */
	void add_advancement(const map_location& loc);

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

	void start_replay();
	void revert_action();
	config* get_next_action();
	void pre_replay();

	bool at_end() const;
	void set_to_end();

	void clear();
	bool empty();

	enum MARK_SENT { MARK_AS_UNSENT, MARK_AS_SENT };
	void add_config(const config& cfg, MARK_SENT mark=MARK_AS_UNSENT);

	int ncommands() const;

	static void process_error(const std::string& msg);

private:
	//generic for add_movement and add_attack
	void add_pos(const std::string& type,
	             const map_location& a, const map_location& b);

	void add_chat_log_entry(const config &speak, std::back_insert_iterator< std::vector<chat_msg> > &i) const;

	config &command(int);
	void remove_command(int);
	/** Adds a new empty command to the command list.
	 *
	 * @param update_random_context  If set to false, do not update the
	 *           random context variables: all random generation will take
	 *           place in the previous random context. Used for commands
	 *           for which "random context" is pointless, and which can be
	 *           issued while some other commands are still taking place,
	 *           like, for example, messages during combats.
	 *
	 * @return a pointer to the added command
	 */
	config* add_command(bool update_random_context=true);
	config cfg_;
	int pos_;

	config* current_;

	bool skip_;

	std::vector<int> message_locations;

	/**
	 * A queue of units (locations) that are supposed to advance but the
	 * relevant advance (choice) message has not yet been received
	 */
	std::deque<map_location> expected_advancements_;
};

replay& get_replay_source();

extern replay recorder;

//replays up to one turn from the recorder object
//returns true if it got to the end of the turn without data running out
bool do_replay(int side_num, replay *obj = NULL);

bool do_replay_handle(int side_num, const std::string &do_untill);

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
 * It has to support querying the user and making a random choice from a
 * preseeded random generator.
 */
struct user_choice
{
	virtual ~user_choice() {}
	virtual config query_user() const = 0;
	virtual config random_choice(rand_rng::simple_rng &) const = 0;
};

/**
 * Performs a choice for WML events.
 *
 * The choice is synchronized across all the multiplayer clients and
 * stored into the replay. The function object is called if the local
 * client is responsible for making the choice.
 * @param name Tag used for storing the choice into the replay.
 * @param side The number of the side responsible for making the choice.
 *             If zero, it defaults to the currently active side.
 * @param force_sp If true, user choice will happen in prestart and start
 *                 events too. But if used for these events in multiplayer,
 *                 an exception will be thrown instead.
 *
 * @note In order to prevent issues with sync, crash, or infinite loop, a
 *       number of precautions must be taken when getting a choice from a
 *       specific side.
 *       - The calling function must enter a loop to wait for network sync
 *         if the side is non-local. This loop must end when a response is
 *         received in the replay.
 *       - The server must recognize @name replay commands as legal from
 *         non-active players. Preferably the server should be notified
 *         about which player the data is expected from, and discard data
 *         from unexpected players.
 *       - do_replay_handle must ignore the @name replay command when the
 *         originating player's turn is reached.
 */
config get_user_choice(const std::string &name, const user_choice &uch,
	int side = 0, bool force_sp = false);

}

#endif
