/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/lobby_player_info.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"

#include "game_preferences.hpp"
#include "gettext.hpp"

namespace gui2 {

REGISTER_WINDOW(lobby_player_info)

tlobby_player_info::tlobby_player_info(events::chat_handler& chat, user_info& info, const lobby_info& li)
: chat_(chat), info_(info), reason_(NULL), time_(NULL), relation_(NULL),
add_to_friends_(NULL), add_to_ignores_(NULL), remove_from_list_(NULL),
result_open_whisper_(false), lobby_info_(li)
{
}

tlobby_player_info::~tlobby_player_info()
{
}

void tlobby_player_info::pre_show(CVideo& /*video*/, twindow& window)
{
	relation_ = find_widget<tlabel>(&window, "relation_info", false, true);
	GUI2_EASY_BUTTON_CALLBACK(start_whisper, tlobby_player_info);

	GUI2_EASY_BUTTON_CALLBACK(add_to_friends, tlobby_player_info);
	GUI2_EASY_BUTTON_CALLBACK(add_to_ignores, tlobby_player_info);
	GUI2_EASY_BUTTON_CALLBACK(remove_from_list, tlobby_player_info);
	add_to_friends_ = add_to_friends_btn;
	add_to_ignores_ = add_to_ignores_btn;
	remove_from_list_ = remove_from_list_btn;

	GUI2_EASY_BUTTON_CALLBACK(check_status, tlobby_player_info);
	GUI2_EASY_BUTTON_CALLBACK(kick, tlobby_player_info);
	GUI2_EASY_BUTTON_CALLBACK(kick_ban, tlobby_player_info);

	find_widget<tlabel>(&window, "player_name", false)
			.set_label(info_.name);

	std::stringstream loc;
	const game_info* game = lobby_info_.get_game_by_id(info_.game_id);
	if (game != NULL) {
		loc << _("In game:") << " " << game->name << " ";
		if (info_.observing) {
			loc << _("(observing)");
		} else {
			loc << _("(playing)");
		}
	} else {
		loc << _("In lobby");
	}

	find_widget<tlabel>(&window, "location_info", false).set_label(loc.str());

	update_relation(window);

	if (!preferences::is_authenticated()) {
		twidget* aw = window.find("admin", false);
		aw->set_visible(twidget::INVISIBLE);
	}
}

void tlobby_player_info::post_show(twindow& /*window*/)
{
}

void tlobby_player_info::update_relation(twindow& w)
{
	add_to_friends_->set_active(false);
	add_to_ignores_->set_active(false);
	remove_from_list_->set_active(false);
	switch (info_.relation) {
		case user_info::FRIEND:
			relation_->set_label(_("On friends list"));
			add_to_ignores_->set_active(true);
			remove_from_list_->set_active(true);
			break;
		case user_info::IGNORED:
			relation_->set_label(_("On ignores list"));
			add_to_friends_->set_active(true);
			remove_from_list_->set_active(true);
			break;
		case user_info::NEUTRAL:
			relation_->set_label(_("Neither a friend nor ignored"));
			add_to_friends_->set_active(true);
			add_to_ignores_->set_active(true);
			break;
		case user_info::ME:
			relation_->set_label(_("You"));
			break;
		default:
			relation_->set_label(_("Error"));
	}
	w.invalidate_layout();
}

void tlobby_player_info::add_to_friends_button_callback(twindow& w)
{
	preferences::add_friend(info_.name);
	info_.relation = user_info::FRIEND;
	update_relation(w);
}

void tlobby_player_info::add_to_ignores_button_callback(twindow& w)
{
	preferences::add_ignore(info_.name);
	info_.relation = user_info::IGNORED;
	update_relation(w);
}

void tlobby_player_info::remove_from_list_button_callback(twindow& w)
{
	preferences::remove_friend(info_.name);
	preferences::remove_ignore(info_.name);
	info_.relation = user_info::NEUTRAL;
	update_relation(w);
}

void tlobby_player_info::start_whisper_button_callback(twindow& w)
{
	result_open_whisper_ = true;
	w.close();
}

void tlobby_player_info::check_status_button_callback(twindow& w)
{
	chat_.send_command("query", "status " + info_.name);
	w.close();
}

	void tlobby_player_info::kick_button_callback(twindow& w)
{
	do_kick_ban(false);
	w.close();
}

void tlobby_player_info::kick_ban_button_callback(twindow& w)
{
	do_kick_ban(true);
	w.close();
}

void tlobby_player_info::do_kick_ban(bool ban)
{
	std::stringstream ss;
	ss << (ban ? "kban" : "kick ") << info_.name;
	if (ban && !time_->get_value().empty()) {
		ss << " " << time_->get_value();
	}
	if (!reason_->get_value().empty()) {
		ss << " " << reason_->get_value();
	}

	chat_.send_command("query", ss.str());
}

} //end namespace gui2
