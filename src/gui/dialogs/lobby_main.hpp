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

class config;

namespace gui2 {

class tlabel;
class tlistbox;
class ttext_box;
class twindow;

class tlobby_main : public tdialog, private events::chat_handler
{
public:
	tlobby_main();

	~tlobby_main();

	void update_gamelist(const config& cfg);
protected:
	void send_chat_message(const std::string& message, bool /*allies_only*/);
	void add_chat_message(const time_t& time, const std::string& speaker,
		int side, const std::string& message,
		events::chat_handler::MESSAGE_TYPE type = events::chat_handler::MESSAGE_PRIVATE);
private:
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

	void send_message_button_callback(twindow& window);


	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	config games_;

	bool games_initialized_;

	tlistbox* gamelistbox_;

	tlabel* chat_log_;

	ttext_box* chat_input_;

	twindow* window_;
};

} // namespace gui2

#endif

