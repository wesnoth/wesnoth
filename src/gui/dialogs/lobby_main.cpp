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
#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/lobby_main.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text_box.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "network.hpp"

#include <boost/bind.hpp>

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)


namespace gui2 {

tlobby_main::tlobby_main()
: games_(), games_initialized_(false)
, gamelistbox_(NULL), chat_log_(NULL)
{
}

twindow* tlobby_main::build_window(CVideo& video)
{
	return build(video, get_id(LOBBY_MAIN));
}

void tlobby_main::update_gamelist(const config& cfg)
{
	foreach (const config &game, cfg.child("gamelist").child_range("game"))
	{
		std::map<std::string, string_map> data;
		string_map item;
		std::string tmp;

		tmp = game["name"];
		utils::truncate_as_wstring(tmp, 20);
		item["label"] = tmp;
		data.insert(std::make_pair("name", item));

		tmp = game["mp_era"];
		utils::truncate_as_wstring(tmp, 20);
		item["label"] = tmp;
		data.insert(std::make_pair("name", item));

		gamelistbox_->add_row(data);
		tgrid* grid = gamelistbox_->get_row_grid(gamelistbox_->get_item_count() - 1);

		tbutton* join_button = dynamic_cast<tbutton*>(
			grid->find_widget("join", false));
		join_button->set_callback_mouse_left_click(
			dialog_callback<tlobby_main, &tlobby_main::join_button_callback>);

		tbutton* observe_button = dynamic_cast<tbutton*>(
			grid->find_widget("observe", false));
		observe_button->set_callback_mouse_left_click(
			dialog_callback<tlobby_main, &tlobby_main::observe_button_callback>);
	}
}

void tlobby_main::pre_show(CVideo& /*video*/, twindow& window)
{
	gamelistbox_ = dynamic_cast<tlistbox*>(window.find_widget("game_list", false));
	VALIDATE(gamelistbox_, missing_widget("game_list"));

	chat_log_ = dynamic_cast<ttext_box*>(window.find_widget("chat_log", false));
	VALIDATE(chat_log_, missing_widget("chat_log"));

	window.set_event_loop_pre_callback(boost::bind(&tlobby_main::network_handler, this));
}

void tlobby_main::network_handler()
{
	config data;
	try {
		const network::connection sock = network::receive_data(data);
		if(sock) {
			process_network_data(data);
		}
	} catch(network::error& e) {
		ERR_NW << "caught network::error in network_handler: " << e.message << "\n";
		throw;
	}
}

void tlobby_main::process_network_data(const config &data)
{
	if (const config &c = data.child("error")) {
		throw network::error(c["message"]);
	} else if (const config &c = data.child("message")) {
		process_message(c);
	} else if (const config &c = data.child("whisper")) {
		process_message(c, true);
	} else if(data.child("gamelist")) {
		process_gamelist(data);
	} else if (const config &c = data.child("gamelist_diff")) {
		process_gamelist_diff(c);
	} else if (const config &c = data.child("room_join")) {
		process_room_join(c);
	} else if (const config &c = data.child("room_part")) {
		process_room_part(c);
	} else if (const config &c = data.child("room_query_response")) {
		process_room_query_response(c);
	}
}

void tlobby_main::process_message(const config &data, bool /*whisper / *= false*/)
{
	const std::string& sender = data["sender"];
	const std::string& message = data["message"];
	const std::string& room = data["room"];
	std::stringstream ss;
	ss << "<";
	if (!room.empty()) {
		ss << room << ": ";
	}
	ss << sender << "> ";
	ss << message;
	LOG_NW << "Message: " << ss.str() << std::endl;
	//chat_log_->set_value(chat_log_->text() + "\n" + ss.str());
}

void tlobby_main::process_gamelist(const config &/*data*/)
{
	games_ = data;
	games_initialized_ = true;
	update_gamelist(games_);
}

void tlobby_main::process_gamelist_diff(const config &/*data*/)
{
	if (!games_initialized_) return;
	try {
		games_.apply_diff(data);
	} catch(config::error& e) {
		ERR_CF << "Error while applying the gamelist diff: '"
			<< e.message << "' Getting a new gamelist.\n";
		network::send_data(config("refresh_lobby"), 0, true);
		return;
	}
	update_gamelist(games_);
}

void tlobby_main::process_room_join(const config &/*data*/)
{
}

void tlobby_main::process_room_part(const config &/*data*/)
{
}

void tlobby_main::process_room_query_response(const config &/*data*/)
{
}

void tlobby_main::join_button_callback(gui2::twindow &window)
{
	LOG_NW << "join_button_callback\n";
}

void tlobby_main::observe_button_callback(gui2::twindow &window)
{
	LOG_NW << "observe_button_callback\n";
}

} // namespace gui2
