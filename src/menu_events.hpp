/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MENU_EVENTS_H_INCLUDED
#define MENU_EVENTS_H_INCLUDED

#include "global.hpp"
#include "actions.hpp"
#include "display.hpp"
#include "game_display.hpp"
#include "show_dialog.hpp"
#include "floating_textbox.hpp"
#include "widgets/textbox.hpp"

class game_state;
class gamestatus;
class tod_manager;

namespace events {
	class mouse_handler;
}

struct fallback_ai_to_human_exception {};

namespace events{

class chat_handler {
public:
	chat_handler();
	virtual ~chat_handler();

protected:
	void do_speak(const std::string& message, bool allies_only=false);

	//called from do_speak
	virtual void add_chat_message(const time_t& time,
			const std::string& speaker, int side, const std::string& message,
			game_display::MESSAGE_TYPE type=game_display::MESSAGE_PRIVATE)=0;
	virtual void send_chat_message(const std::string& message, bool allies_only=false)=0;
	void send_command(const std::string& cmd, const std::string& args="");
	void change_logging(const std::string& data);
	friend class chat_command_handler;
};

class menu_handler : private chat_handler {
public:
	menu_handler(game_display* gui, unit_map& units, std::vector<team>& teams,
		const config& level, const gamemap& map,
		const config& game_config, const gamestatus& status, const tod_manager& tod_mng, game_state& gamestate,
		undo_list& undo_stack, undo_list& redo_stack);
	virtual ~menu_handler();

	const undo_list& get_undo_list() const;
	gui::floating_textbox& get_textbox();
	void set_gui(game_display* gui) { gui_ = gui; }

	std::string get_title_suffix(int side_num);
	void objectives(int side_num);
	void show_statistics(int side_num);
	void unit_list();
	void status_table(int selected=0);
	void save_map();
	void preferences();
	void show_chat_log();
	void show_help();
	void speak();
	void whisper();
	void shout();
	void recruit(bool browse, int side_num, const map_location &last_hex);
	void repeat_recruit(int side_num, const map_location &last_hex);
	void recall(int side_num, const map_location& last_hex);
	void undo(int side_num);
	void redo(int side_num);
	void show_enemy_moves(bool ignore_units, int side_num);
	void toggle_shroud_updates(int side_num);
	void update_shroud_now(int side_num);
	bool end_turn(int side_num);
	void goto_leader(int side_num);
	void unit_description(mouse_handler& mousehandler);
	void rename_unit(mouse_handler& mousehandler);
	void create_unit(mouse_handler& mousehandler);
	void change_side(mouse_handler& mousehandler);
	void label_terrain(mouse_handler& mousehandler, bool team_only);
	void clear_labels();
	void continue_move(mouse_handler &mousehandler, int side_num);
	void toggle_grid();
	void unit_hold_position(mouse_handler &mousehandler, int side_num);
	void end_unit_turn(mouse_handler &mousehandler, int side_num);
	void search();
	void user_command();
	void custom_command(mouse_handler& mousehandler, int side_num);
	void ai_formula();
	void clear_messages();
#ifdef USRCMD2
	void user_command_2();
	void user_command_3();
#endif

	unit_map::iterator current_unit(mouse_handler& mousehandler);
	unit_map::const_iterator current_unit(const mouse_handler& mousehandler) const;
	void move_unit_to_loc(const unit_map::const_iterator& ui, const map_location& target,
		bool continue_move, int side_num, mouse_handler &mousehandler);
	void do_speak();
	void do_search(const std::string& new_search);
	void do_command(const std::string &str, int side_num, mouse_handler &mousehandler);
	void do_ai_formula(const std::string &str, int side_num, mouse_handler &mousehandler);
	void clear_undo_stack(int side_num);
	bool has_team() const;
protected:
	void add_chat_message(const time_t& time, const std::string& speaker,
			int side, const std::string& message,
			game_display::MESSAGE_TYPE type=game_display::MESSAGE_PRIVATE);
	void send_chat_message(const std::string& message, bool allies_only=false);
private:
	//console_handler is basically a sliced out part of menu_handler
	//and as such needs access to menu_handler's privates
	friend class console_handler;

	//void do_speak(const std::string& message, bool allies_only);
	void do_recruit(const std::string& name, int side_num, const map_location& last_hex);
//	std::vector<std::string> create_unit_table(const statistics::stats::str_int_map& m,unsigned int team);
	bool has_friends() const;
	bool clear_shroud(int side_num);
	static void change_controller(const std::string& side, const std::string& controller);
	static void change_side_controller(const std::string& side, const std::string& player, bool own_side=false);
	void scenario_settings_table(int selected=0);

	game_display* gui_;
	unit_map& units_;
	std::vector<team>& teams_;
	const config& level_;
	const gamemap& map_;
	const config& game_config_;
	const gamestatus& status_;
	const tod_manager& tod_manager_;
	game_state& gamestate_;

	undo_list& undo_stack_;
	undo_list& redo_stack_;
	gui::floating_textbox textbox_info_;
	std::string last_search_;
	map_location last_search_hit_;

	std::string last_recruit_;
};

}
#endif
