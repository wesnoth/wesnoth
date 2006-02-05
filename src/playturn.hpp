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
#ifndef PLAYTURN_HPP_INCLUDED
#define PLAYTURN_HPP_INCLUDED

class replay_network_sender;

#include "actions.hpp"
#include "config.hpp"
#include "display.hpp"
#include "events.hpp"
#include "gamestatus.hpp"
#include "hotkeys.hpp"
#include "key.hpp"
#include "scoped_resource.hpp"
#include "pathfind.hpp"
#include "show_dialog.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "unit_types.hpp"
#include "unit.hpp"

#include "widgets/button.hpp"
#include "widgets/textbox.hpp"

#include <deque>
#include <map>
#include <vector>

struct command_disabler
{
	command_disabler();
	~command_disabler();
};

class turn_info : public hotkey::command_executor, public events::handler
{
public:
	//this keeps track of any textbox that might be used to do searching
	//and send messages
	struct floating_textbox
	{
		util::scoped_ptr<gui::textbox> box;
		util::scoped_ptr<gui::button> check;

		enum MODE { TEXTBOX_NONE, TEXTBOX_SEARCH, TEXTBOX_MESSAGE,
		            TEXTBOX_COMMAND };
		MODE mode;

		std::string label_string;
		int label;

		floating_textbox() : box(NULL), check(NULL), mode(TEXTBOX_NONE), label(0)
		{}

		bool active() const { return box.get() != NULL; }
	};

	enum TURN_MODE { PLAY_TURN, BROWSE_NETWORKED, BROWSE_AI };

	turn_info(const game_data& gameinfo, game_state& state_of_game,
	          const gamestatus& status, const config& terrain_config,
		  const config& level, CKey& key, display& gui, gamemap& map,
		  std::vector<team>& teams, unsigned int team_num, unit_map& units,
		  TURN_MODE mode, floating_textbox& textbox,
		  replay_network_sender& network_sender);

	void turn_slice();

	bool turn_over() const;

	void send_data();

	undo_list& undos() { return undo_stack_; }

	bool can_execute_command(hotkey::HOTKEY_COMMAND command) const;
	bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

	void move_unit_to_loc(const unit_map::const_iterator& ui, const gamemap::location& target, bool continue_move);
	void start_interactive_turn();

	void save_game(const std::string& message,gui::DIALOG_TYPE dialog_type);

	enum PROCESS_DATA_RESULT { PROCESS_CONTINUE, PROCESS_RESTART_TURN, PROCESS_END_TURN };

	//function which will process incoming network data, and act on it. If there is
	//more data than a single turn's worth, excess data will be placed into 'backlog'.
	//No more than one turn's worth of data will be placed into a single backlog item,
	//so it is safe to assume that backlog won't be touched if cfg is a member of a previous
	//backlog.
	//data will be forwarded to all peers other than 'from', unless 'from' is null, in
	//which case data will not be forwarded
	PROCESS_DATA_RESULT process_network_data(const config& cfg,network::connection from,std::deque<config>& backlog);




private:
	//convenience functions
	team& current_team() { return teams_[team_num_-1]; }
	const team& current_team() const { return teams_[team_num_-1]; }

	team& viewing_team() { return teams_[gui_.viewing_team()]; }
	const team& viewing_team() const { return teams_[gui_.viewing_team()]; }

	unit_map::const_iterator find_unit(const gamemap::location& hex) const;
	unit_map::iterator find_unit(const gamemap::location& hex);

	unit_map::const_iterator selected_unit() const;
	unit_map::iterator selected_unit();

	unit_map::iterator current_unit();
	unit_map::const_iterator current_unit() const;

	void write_game_snapshot(config& cfg) const;

	bool unit_in_cycle(unit_map::const_iterator it) const;

	//overridden from command_executor
	virtual void cycle_units();
	virtual void cycle_back_units();
	virtual void end_turn();
	virtual void goto_leader();
	virtual void unit_hold_position();
	virtual void end_unit_turn();
	virtual void undo();
	virtual void redo();
	virtual void unit_description();
	virtual void rename_unit();
	virtual void save_game();
	virtual void load_game();
	virtual void toggle_grid();
	virtual void status_table();
	virtual void recruit();
	virtual void repeat_recruit();
	virtual void recall();
	virtual void speak();
	virtual void create_unit();
	virtual void change_unit_side();
	virtual void preferences();
	virtual void objectives();
	virtual void unit_list();
	virtual void show_statistics();
	virtual void label_terrain();
	virtual void clear_labels();
	virtual void show_enemy_moves(bool ignore_units);
	virtual void toggle_shroud_updates();
	virtual void update_shroud_now();
	virtual void continue_move();
	virtual void search();
	virtual void show_help();
	virtual void show_chat_log();
	virtual void user_command();
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command) const;

	void do_search(const std::string& str);
	void do_command(const std::string& str);
	void do_speak(const std::string& str, bool allies_only);
	bool has_friends() const;

	void do_recruit(const std::string& name);

	void handle_event(const SDL_Event& event);
	void mouse_motion(const SDL_MouseMotionEvent& event);
	void mouse_motion(int x, int y);
	void mouse_press(const SDL_MouseButtonEvent& event);

	void left_click(const SDL_MouseButtonEvent& event);
	void show_menu(const std::vector<std::string>& items, int xloc, int yloc, bool context_menu);

	void show_attack_options(unit_map::const_iterator u);

	//function which, given the location of a potential enemy to attack, will return the location
	//that the currently selected unit would move to and attack it from this turn. Returns an
	//invalid location if not possible.
	gamemap::location current_unit_attacks_from(const gamemap::location& loc, const gamemap::location::DIRECTION preferred, const gamemap::location::DIRECTION second_preferred) const;

	bool attack_enemy(unit_map::iterator attacker, unit_map::iterator defender);
	bool move_unit_along_current_route(bool check_shroud=true);

	bool clear_shroud();
	void clear_undo_stack();

	std::vector<std::string> create_unit_table(const statistics::stats::str_int_map& m);

	bool enemies_visible() const;

	void change_side_controller(const std::string& side, const std::string& player, bool orphan_side=false);

	const game_data& gameinfo_;
	game_state& state_of_game_;
	const gamestatus& status_;
	const config& terrain_config_;
	const config& level_;
	CKey key_;
	display& gui_;
	gamemap& map_;
	std::vector<team>& teams_;
	unsigned int team_num_;
	unit_map& units_;

	const unit_map& visible_units() const;
	mutable unit_map visible_units_;
	bool browse_;
	bool allow_network_commands_;

	bool left_button_, right_button_, middle_button_;
	bool minimap_scrolling_;
	gamemap::location next_unit_;
	paths current_paths_, all_paths_;
	paths::route current_route_;
	bool enemy_paths_;
	gamemap::location last_hex_;
	gamemap::location::DIRECTION last_nearest_, last_second_nearest_;
	gamemap::location selected_hex_;
	undo_list undo_stack_;
	undo_list redo_stack_;
	int path_turns_;

	//cached value indicating whether any enemy units are visible.
	//computed with enemies_visible()
	bool enemies_visible_;

	bool end_turn_;
	int start_ncmd_;

	std::string last_recruit_;

	std::string last_search_;
	gamemap::location last_search_hit_;

	floating_textbox& textbox_;

	void update_textbox_location();
	void create_textbox(floating_textbox::MODE mode, const std::string& label, const std::string& check_label="", bool checked=false);
	void close_textbox();
	void enter_textbox();
	void tab_textbox();

	replay_network_sender& replay_sender_;
};

void play_turn(const game_data& gameinfo, game_state& state_of_game,
               const gamestatus& status, const config& terrain_config,
			   const config& level,
               CKey& key, display& gui, gamemap& map,
               std::vector<team>& teams, unsigned int team_num,
               std::map<gamemap::location,unit>& units,
               turn_info::floating_textbox& textbox,
               replay_network_sender& network_sender);


#endif
