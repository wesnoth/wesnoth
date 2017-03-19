/*
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_LOBBY_PLAYER_INFO_HPP_INCLUDED
#define GUI_DIALOGS_LOBBY_PLAYER_INFO_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "chat_events.hpp"
#include "game_initialization/lobby_info.hpp"

namespace gui2
{

class button;
class label;
class text_box;

namespace dialogs
{

class lobby_player_info : public modal_dialog
{
public:
	lobby_player_info(events::chat_handler& chat,
					   mp::user_info& info,
					   const mp::lobby_info& li);

	~lobby_player_info();

	bool result_open_whisper() const
	{
		return result_open_whisper_;
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);

	void update_relation();

	void add_to_friends_button_callback();

	void add_to_ignores_button_callback();

	void remove_from_list_button_callback();

	void start_whisper_button_callback(window& w);

	void check_status_button_callback(window& w);

	void kick_button_callback(window& w);

	void kick_ban_button_callback(window& w);

	void do_kick_ban(bool ban);

	events::chat_handler& chat_;

	mp::user_info& info_;

	text_box* reason_;

	text_box* time_;

	label* relation_;

	button* add_to_friends_;

	button* add_to_ignores_;

	button* remove_from_list_;

	bool result_open_whisper_;

	const mp::lobby_info& lobby_info_;
};

} // namespace dialogs
} // end namespace gui2

#endif
