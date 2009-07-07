/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_LOBBY_HPP_INCLUDED
#define GUI_DIALOGS_LOBBY_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "config.hpp"
#include "chat_events.hpp"
#include "lobby_data.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>


namespace gui2 {

class tlabel;
class tlistbox;
class ttext_box;
class twindow;

class tlobby_main : public tdialog, private events::chat_handler
{
public:
	tlobby_main(const config& game_config, lobby_info& info);

	~tlobby_main();

	/**
	 * Set the callback used to show the preferences.
	 */
	void set_preferences_callback(boost::function<void ()> f);

	void update_gamelist();

	enum legacy_result { QUIT, JOIN, OBSERVE, CREATE, PREFERENCES };

	legacy_result get_legacy_result() const { return legacy_result_; }
protected:
	void send_chat_message(const std::string& message, bool /*allies_only*/);
	void add_chat_message(const time_t& time, const std::string& speaker,
		int side, const std::string& message,
		events::chat_handler::MESSAGE_TYPE type = events::chat_handler::MESSAGE_PRIVATE);
private:
	legacy_result legacy_result_;

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

	/**
	 * Assemble and send a game join request. Ask for password if the game
	 * requires one.
	 * @return true iif the request was actually sent, false if not for some
	 *         reason like user canceled the password dialog or idx was invalid.
	 */
	bool do_game_join(int idx, bool observe);

	void send_message_button_callback(twindow& window);

	void create_button_callback(twindow& window);

	void show_preferences_button_callback(twindow& window);

	void show_help_button_callback(twindow& window);

	void refresh_button_callback(twindow& window);

	void quit_button_callback(twindow& window);

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	const config& game_config_;

	tlistbox* gamelistbox_;

	tlistbox* userlistbox_;

	tlabel* chat_log_;

	ttext_box* chat_input_;

	twindow* window_;

	lobby_info& lobby_info_;

	boost::function<void ()> preferences_callback_;
};

} // namespace gui2

#endif

