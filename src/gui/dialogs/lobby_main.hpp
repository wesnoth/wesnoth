/*
   Copyright (C) 2009 - 2016 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_LOBBY_HPP_INCLUDED
#define GUI_DIALOGS_LOBBY_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/tree_view.hpp"
#include "chat_events.hpp"
#include "gui/dialogs/lobby/lobby_info.hpp"

#include <boost/scoped_ptr.hpp>

class display;

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#endif
namespace gui2
{

class tgrid;
class tlabel;
#ifndef GUI2_EXPERIMENTAL_LISTBOX
class tlistbox;
#endif
class ttext_box;
class twindow;
class tmulti_page;
class ttoggle_button;

struct tlobby_chat_window
{
	tlobby_chat_window(const std::string& name, bool whisper)
		: name(name), whisper(whisper), pending_messages(0)
	{
	}
	std::string name;
	bool whisper;
	int pending_messages;
};

struct tplayer_list;

struct tsub_player_list
{
	void init(twindow& w, const std::string& id);
	void show_toggle_callback(twidget& widget);
	void auto_hide();
	tlabel* label;
	tlabel* count;
	ttoggle_button* show_toggle;
	tlistbox* list;
	ttree_view_node* tree;
	tlabel* tree_label;
};

struct tplayer_list
{
	void init(twindow& w);
	void update_sort_icons();
	tsub_player_list active_game;
	tsub_player_list active_room;
	tsub_player_list other_rooms;
	tsub_player_list other_games;

	ttoggle_button* sort_by_name;
	ttoggle_button* sort_by_relation;

	ttree_view* tree;
};

class tlobby_main : public tdialog, private events::chat_handler
{
public:
	tlobby_main(const config& game_config, lobby_info& info, CVideo& video);

	~tlobby_main();

	/**
	 * Set the callback used to show the preferences.
	 */
	void set_preferences_callback(boost::function<void()> f);

	void update_gamelist();

protected:
	void update_gamelist_header();

	void update_gamelist_diff();

	void update_gamelist_filter();

	std::map<std::string, string_map> make_game_row_data(const game_info& game);

	void adjust_game_row_contents(const game_info& game, int idx, tgrid* grid);

public:
	void update_playerlist();

	enum legacy_result {
		QUIT,
		JOIN,
		OBSERVE,
		CREATE,
		PREFERENCES
	};

	legacy_result get_legacy_result() const
	{
		return legacy_result_;
	}

	enum t_notify_mode {
		NOTIFY_NONE,
		NOTIFY_MESSAGE,
		NOTIFY_MESSAGE_OTHER_WINDOW,
		NOTIFY_SERVER_MESSAGE,
		NOTIFY_OWN_NICK,
		NOTIFY_FRIEND_MESSAGE,
		NOTIFY_WHISPER,
		NOTIFY_WHISPER_OTHER_WINDOW,
		NOTIFY_LOBBY_JOIN,
		NOTIFY_LOBBY_QUIT,
		NOTIFY_COUNT
	};

	void do_notify(t_notify_mode mode) { do_notify(mode, "", ""); }
	void do_notify(t_notify_mode mode, const std::string & sender, const std::string & message);

protected:
	/** inherited form chat_handler */
	virtual void send_chat_message(const std::string& message,
								   bool /*allies_only*/);

	virtual void user_relation_changed(const std::string& name);

	/** inherited form chat_handler */
	virtual void add_chat_message(const time_t& time,
								  const std::string& speaker,
								  int side,
								  const std::string& message,
								  events::chat_handler::MESSAGE_TYPE type
								  = events::chat_handler::MESSAGE_PRIVATE);

	/** inherited form chat_handler */
	virtual void add_whisper_sent(const std::string& receiver,
								  const std::string& message);

	/** inherited form chat_handler */
	virtual void add_whisper_received(const std::string& sender,
									  const std::string& message);

	/** inherited form chat_handler */
	virtual void add_chat_room_message_sent(const std::string& room,
											const std::string& message);

	/** inherited form chat_handler */
	virtual void add_chat_room_message_received(const std::string& room,
												const std::string& speaker,
												const std::string& message);

private:
	void update_selected_game();

	/**
	 * Append some text to the active chat log
	 */
	void append_to_chatbox(const std::string& text);

	/**
	 * Append some text to the chat log for window "id"
	 */
	void append_to_chatbox(const std::string& text, size_t id);

	/**
	 * Result flag for interfacing with other MP dialogs
	 */
	legacy_result legacy_result_;

	/**
	 * Get the room* corresponding to the currently active window, or NULL
	 * if a whisper window is active at the moment
	 */
	room_info* active_window_room();

	/**
	 * Check if a room window for "room" is open, if open_new is true
	 * then it will be created if not found.
	 * @return valid ptr if the window was found or added, null otherwise
	 */
	tlobby_chat_window* room_window_open(const std::string& room,
										 bool open_new);

	/**
	 * Check if a whisper window for user "name" is open, if open_new is true
	 * then it will be created if not found.
	 * @return valid ptr if the window was found or added, null otherwise
	 */
	tlobby_chat_window* whisper_window_open(const std::string& name,
											bool open_new);

	/**
	 * Helper function to find and open a new window, used by *_window_open
	 */
	tlobby_chat_window*
	search_create_window(const std::string& name, bool whisper, bool open_new);

	/**
	 * @return true if the whisper window for "name" is the active window
	 */
	bool whisper_window_active(const std::string& name);

	/**
	 * @return true if the room window for "room" is the active window
	 */
	bool room_window_active(const std::string& room);

	/**
	 * Mark the whisper window for "name" as having one more pending message
	 */
	void increment_waiting_whsipers(const std::string& name);

	/**
	 * Mark the room window for "room" as having one more pending message
	 */
	void increment_waiting_messages(const std::string& room);

	/**
	 * Add a whisper message to the whisper window
	 */
	void add_whisper_window_whisper(const std::string& sender,
									const std::string& message);

	/**
	 * Add a whisper message to the current window which is not the whisper
	 * window
	 * for "name".
	 */
	void add_active_window_whisper(const std::string& sender,
								   const std::string& message);

	/**
	 * Add a message to the window for room "room"
	 */
	void add_room_window_message(const std::string& room,
								 const std::string& sender,
								 const std::string& message);

	/**
	 * Add a message to the window for room "room"
	 */
	void add_active_window_message(const std::string& sender,
								   const std::string& message);

	/**
	 * Switch to the window given by a valid pointer (e.g. received from a call
	 * to *_window_open)
	 */
	void switch_to_window(tlobby_chat_window* t);

	void switch_to_window(size_t id);

	void active_window_changed();

	void close_active_window();

	void close_window(size_t idx);


	/**
	 * Network polling callback
	 */
	void network_handler();

	void process_network_data(const config& data);

	void process_message(const config& data, bool whisper = false);

	void process_gamelist(const config& data);

	void process_gamelist_diff(const config& data);

	void process_room_join(const config& data);

	void process_room_part(const config& data);

	void process_room_query_response(const config& data);

	void join_button_callback(twindow& window);

	void observe_button_callback(twindow& window);

	void join_global_button_callback(twindow& window);

	void observe_global_button_callback(twindow& window);

	void join_or_observe(int index);

	/**
	 * Assemble and send a game join request. Ask for password if the game
	 * requires one.
	 * @return true iff the request was actually sent, false if not for some
	 *         reason like user canceled the password dialog or idx was invalid.
	 */
	bool do_game_join(int idx, bool observe);

	void send_message_button_callback(twindow& window);

	void send_message_to_active_window(const std::string& input);

	void close_window_button_callback(twindow& window);

	void create_button_callback(twindow& window);

	void show_preferences_button_callback(twindow& window);

	void refresh_button_callback(twindow& window);

	void quit_button_callback(twindow& window);

	void room_switch_callback(twindow& window);

	void chat_input_keypress_callback(bool& handled,
									  bool& halt,
									  const SDLKey key,
									  twindow& window);

	void game_filter_reload();

	void game_filter_change_callback(twidget& widget);

	void game_filter_keypress_callback(const SDLKey key);

	void gamelist_change_callback(twindow& window);

	void player_filter_callback(twidget& widget);

	void user_dialog_callback(user_info* info);

	void skip_replay_changed_callback(twidget& w);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	virtual void post_build(twindow& window);

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	const config& game_config_;

	tlistbox* gamelistbox_;

	tlistbox* userlistbox_;

	tlistbox* roomlistbox_;

	tmulti_page* chat_log_container_;

	ttext_box* chat_input_;

	twindow* window_;

	lobby_info& lobby_info_;

	boost::function<void()> preferences_callback_;

	/**
	 * This represents the open chat windows (rooms and whispers at the moment)
	 * with 1 to 1 correspondence to what the user sees in the interface
	 */
	std::vector<tlobby_chat_window> open_windows_;

	size_t active_window_;

	ttoggle_button* filter_friends_;

	ttoggle_button* filter_ignored_;

	ttoggle_button* filter_slots_;

	ttoggle_button* filter_invert_;

	ttext_box* filter_text_;

	int selected_game_id_;

	tplayer_list player_list_;

	bool player_list_dirty_;

	bool gamelist_dirty_;

	unsigned last_gamelist_update_;

	bool gamelist_diff_update_;

	CVideo& video_;

	/** Timer for updating the lobby. */
	size_t lobby_update_timer_;

	/** Wrapper for the preferences hotkey. */
	boost::function<void()> preferences_wrapper_;

	std::vector<int> gamelist_id_at_row_;

	bool delay_playerlist_update_;

	bool delay_gamelist_update_;

	friend struct lobby_delay_gamelist_update_guard;
};

} // namespace gui2

#endif
