/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef REPLAY_H_INCLUDED
#define REPLAY_H_INCLUDED

#include "config.hpp"
#include "gamestatus.hpp"
#include "map.hpp"
#include "random.hpp"
#include "unit.hpp"

class display;

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

	config& get_config();

	void set_save_info(const game_state& save);
	const game_state& get_save_info() const;

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
	                int weapon);
	void choose_option(int index);
	void add_label(const std::string& text, const gamemap::location& loc);
	void clear_labels();
	void add_rename(const std::string& name, const gamemap::location& loc);
	void end_turn();

	void speak(const config& cfg);
	std::string build_chat_log(const std::string& team) const;

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
	config* get_next_action();
	void pre_replay();

	bool at_end() const;
	void set_to_end();

	void clear();
	bool empty();

	enum MARK_SENT { MARK_AS_UNSENT, MARK_AS_SENT };
	void add_config(const config& cfg, MARK_SENT mark=MARK_AS_UNSENT);

	int ncommands();

	void mark_current();

	struct error {};

private:
	//generic for add_movement and add_attack
	void add_pos(const std::string& type,
	             const gamemap::location& a, const gamemap::location& b);

	void add_value(const std::string& type, int value);

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
};

replay& get_replay_source();

extern replay recorder;

//replays up to one turn from the recorder object
//returns true if it got to the end of the turn without data running out
bool do_replay(display& disp, const gamemap& map, const game_data& gameinfo,
               std::map<gamemap::location,unit>& units,
	       std::vector<team>& teams, int team_num, const gamestatus& state,
	       game_state& state_of_game, replay* obj=NULL);

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
