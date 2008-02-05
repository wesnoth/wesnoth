/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file replay.hpp
//! Replay control code.

#ifndef REPLAY_H_INCLUDED
#define REPLAY_H_INCLUDED

#include "config.hpp"
#include "gamestatus.hpp"
#include "map.hpp"
#include "random.hpp"

class config_writer;
class game_display;
class terrain_label;
class unit_map;

struct verification_manager
{
	verification_manager(const unit_map& units);
	~verification_manager();
};

class replay: public rng
{
public:
	replay();
	explicit replay(const config& cfg);

	void set_save_info(const game_state& save);
	void set_save_info_completion(const std::string &st);

	void set_skip(bool skip);
	bool is_skipping() const;

	void save_game(const std::string& label, const config& snapshot,
	               const config& starting_pos, bool include_replay = true);

	void add_start();
	void add_recruit(int unit_index, const gamemap::location& loc);
	void add_recall(int unit_index, const gamemap::location& loc);
	void add_disband(int unit_index);
	void add_countdown_update(int value,int team);
	void add_movement(const gamemap::location& a, const gamemap::location& b);
	void add_attack(const gamemap::location& a, const gamemap::location& b,
	                int att_weapon, int def_weapon);
	void choose_option(int index);
	void text_input(std::string input);
	void set_random_value(const std::string& choice);
	void add_label(const terrain_label*);
	void clear_labels(const std::string&);
	void add_rename(const std::string& name, const gamemap::location& loc);
	void end_turn();
	void add_event(const std::string& name,
		const gamemap::location& loc=gamemap::location::null_location);
	void add_unit_checksum(const gamemap::location& loc,config* const cfg);
	void add_checksum_check(const gamemap::location& loc);
	/**
	 * Adds an advancement to the replay, the following option command
	 * determines which advancement option has been choosen
	 */
	void add_advancement(const gamemap::location& loc);
	
	void add_chat_message_location();
	void speak(const config& cfg);
	std::string build_chat_log(const std::string& team);

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

	int ncommands();

	static void throw_error(const std::string& msg);

	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};

	static std::string last_replay_error;
private:
	//generic for add_movement and add_attack
	void add_pos(const std::string& type,
	             const gamemap::location& a, const gamemap::location& b);

	void add_value(const std::string& type, int value);

	void add_chat_log_entry(const config*, std::stringstream&, const std::string&) const;

	const config::child_list& commands() const;
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
	unsigned int pos_;

	config* current_;

	game_state saveInfo_;

	bool skip_;
	
	std::vector<int> message_locations;
};

replay& get_replay_source();

extern replay recorder;

//replays up to one turn from the recorder object
//returns true if it got to the end of the turn without data running out
bool do_replay(game_display& disp, const gamemap& map, const game_data& gameinfo,
	unit_map& units, std::vector<team>& teams, int team_num,
	const gamestatus& state, game_state& state_of_game, replay* obj=NULL);

bool do_replay_handle(game_display& disp, const gamemap& map, const game_data& gameinfo,
					  unit_map& units, std::vector<team>& teams, int team_num,
	   const gamestatus& state, game_state& state_of_game, 
	const std::string& do_untill);

//an object which can be made to undo a recorded move
//unless the transaction is confirmed
struct replay_undo
{
	replay_undo(replay& obj) : obj_(&obj) {}
	~replay_undo() { if(obj_) obj_->undo(); }
	void confirm_transaction() { obj_ = NULL; }

private:
	replay* obj_;
};

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

#endif
