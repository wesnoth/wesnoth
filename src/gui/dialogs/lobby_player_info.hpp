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

#ifndef GUI_DIALOGS_LOBBY_PLAYER_INFO_HPP_INCLUDED
#define GUI_DIALOGS_LOBBY_PLAYER_INFO_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "chat_events.hpp"
#include "gui/dialogs/lobby/lobby_info.hpp"

namespace gui2
{

class tbutton;
class tlabel;
class ttext_box;

class tlobby_player_info : public tdialog
{
public:
	tlobby_player_info(events::chat_handler& chat,
					   user_info& info,
					   const lobby_info& li);

	~tlobby_player_info();

	bool result_open_whisper() const
	{
		return result_open_whisper_;
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void update_relation(twindow& w);

	void add_to_friends_button_callback(twindow& w);

	void add_to_ignores_button_callback(twindow& w);

	void remove_from_list_button_callback(twindow& w);

	void start_whisper_button_callback(twindow& w);

	void check_status_button_callback(twindow& w);

	void kick_button_callback(twindow& w);

	void kick_ban_button_callback(twindow& w);

	void do_kick_ban(bool ban);

	events::chat_handler& chat_;

	user_info& info_;

	ttext_box* reason_;

	ttext_box* time_;

	tlabel* relation_;

	tbutton* add_to_friends_;

	tbutton* add_to_ignores_;

	tbutton* remove_from_list_;

	bool result_open_whisper_;

	const lobby_info& lobby_info_;
};

} // end namespace gui2

#endif
