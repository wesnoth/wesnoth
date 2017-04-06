/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MENU_EVENTS_H_INCLUDED
#define MENU_EVENTS_H_INCLUDED

#include "chat_events.hpp"
#include "floating_textbox.hpp"
#include "units/map.hpp"
#include "lua_jailbreak_exception.hpp"

#include <vector>

class game_state;
class gamemap;
class game_data;
class game_board;
class gamemap;
class play_controller;
class team;
class unit_map;

namespace events {
	class mouse_handler;
}

struct fallback_ai_to_human_exception : public lua_jailbreak_exception {IMPLEMENT_LUA_JAILBREAK_EXCEPTION(fallback_ai_to_human_exception)};

namespace events {

class menu_handler : private chat_handler {
public:
	menu_handler(game_display* gui, play_controller & pc,
		const config& game_config);
	virtual ~menu_handler();

	gui::floating_textbox& get_textbox();
	void set_gui(game_display* gui) { gui_ = gui; }

	void objectives();
	void show_statistics(int side_num);
	void unit_list();
	void status_table();
	void save_map();
	void preferences();
	void show_chat_log();
	void show_help();
	void speak();
	void whisper();
	void shout();
	void recruit(int side_num, const map_location &last_hex);
	void repeat_recruit(int side_num, const map_location &last_hex);
	void recall(int side_num, const map_location& last_hex);
	void show_enemy_moves(bool ignore_units, int side_num);
	void toggle_shroud_updates(int side_num);
	void update_shroud_now(int side_num);
	bool end_turn(int side_num);
	void goto_leader(int side_num);
	void unit_description();
	void terrain_description(mouse_handler& mousehandler);
	void rename_unit();
	void create_unit(mouse_handler& mousehandler);
	void change_side(mouse_handler& mousehandler);
	void kill_unit(mouse_handler& mousehandler);
	void label_terrain(mouse_handler& mousehandler, bool team_only);
	void clear_labels();
	void label_settings();
	void continue_move(mouse_handler &mousehandler, int side_num);
	void execute_gotos(mouse_handler &mousehandler, int side_num);
	void toggle_ellipses();
	void toggle_grid();
	void unit_hold_position(mouse_handler &mousehandler, int side_num);
	void end_unit_turn(mouse_handler &mousehandler, int side_num);
	void search();
	void request_control_change(int side_num, const std::string &player);
	void user_command();
	void custom_command();
	void ai_formula();
	void clear_messages();
	std::vector<std::string> get_commands_list();

	unit_map::iterator current_unit();
	unit_map::const_iterator current_unit() const
	{ return const_cast<menu_handler *>(this)->current_unit(); }
	void move_unit_to_loc(const unit_map::iterator& ui, const map_location& target,
		bool continue_move, int side_num, mouse_handler &mousehandler);
	///@return Whether or not the recruit was successful
	bool do_recruit(const std::string& name, int side_num, const map_location& last_hex);
	void do_speak();
	void do_search(const std::string& new_search);
	void do_command(const std::string &str);
	void do_ai_formula(const std::string &str, int side_num, mouse_handler &mousehandler);
	void send_to_server(const config& cfg) override;
protected:
	void add_chat_message(const time_t& time, const std::string& speaker,
			int side, const std::string& message,
			events::chat_handler::MESSAGE_TYPE type=events::chat_handler::MESSAGE_PRIVATE) override;
	void send_chat_message(const std::string& message, bool allies_only=false) override;
private:
	//console_handler is basically a sliced out part of menu_handler
	//and as such needs access to menu_handler's privates
	friend class console_handler;

	//void do_speak(const std::string& message, bool allies_only);
//	std::vector<std::string> create_unit_table(const statistics::stats::str_int_map& m,unsigned int team);
	bool has_friends() const;

	game_display* gui_;
	play_controller & pc_;

	game_state & gamestate() const;
	game_data & gamedata();
	game_board & board() const;
	unit_map& units();
	std::vector<team>& teams() const;
	const gamemap& map();

	const config& game_config_;

	gui::floating_textbox textbox_info_;
	std::string last_search_;
	map_location last_search_hit_;
};

}
#endif
