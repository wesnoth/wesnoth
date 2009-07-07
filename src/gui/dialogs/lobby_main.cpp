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
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/text_box.hpp"

#include "foreach.hpp"
#include "lobby_data.hpp"
#include "log.hpp"
#include "network.hpp"
#include "game_preferences.hpp"
#include "playmp_controller.hpp"
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


void tlobby_main::send_chat_message(const std::string& message, bool /*allies_only*/)
{
	config data, msg;
	msg["message"] = message;
	msg["sender"] = preferences::login();
	data.add_child("message", msg);

	add_chat_message(time(NULL), preferences::login(), 0, message);	//local echo
	network::send_data(data, 0, true);
}

void tlobby_main::add_chat_message(const time_t& /*time*/, const std::string& speaker,
	int /*side*/, const std::string& message, events::chat_handler::MESSAGE_TYPE /*type*/)
{
	//chat_.add_message(time, speaker, message);
	//chat_.update_textbox(chat_textbox_);
	std::stringstream ss;
	ss << "<" << speaker << "> ";
	ss << message;
	LOG_NW << "Message: " << ss.str() << std::endl;
	chat_log_->set_label(chat_log_->label() + "\n" + ss.str());
	window_->invalidate_layout();
}

tlobby_main::tlobby_main(const config& game_config, lobby_info& info)
: legacy_result_(QUIT)
, game_config_(game_config)
, gamelistbox_(NULL), chat_log_(NULL)
, chat_input_(NULL), window_(NULL)
, lobby_info_(info), preferences_callback_(NULL)
{
}

void tlobby_main::set_preferences_callback(boost::function<void ()> cb)
{
	preferences_callback_ = cb;
}

tlobby_main::~tlobby_main()
{
}

twindow* tlobby_main::build_window(CVideo& video)
{
	return build(video, get_id(LOBBY_MAIN));
}

namespace {

void add_label_data(std::map<std::string, string_map>& map,
					const std::string& key, const std::string& label)
{
	string_map item;
	item["label"] = label;
	map.insert(std::make_pair(key, item));
}

void set_visible_if_exists(tgrid* grid, const char* id, bool visible)
{
	twidget* w = grid->find_widget(id, false);
	if (w) {
		w->set_visible(visible ? twidget::VISIBLE : twidget::INVISIBLE);
	}
}

} //end anonymous namespace

void tlobby_main::update_gamelist()
{
	gamelistbox_->clear();
	foreach (const game_info &game, lobby_info_.games())
	{
		std::map<std::string, string_map> data;

		add_label_data(data, "name", game.name);
		add_label_data(data, "era", game.era);
		add_label_data(data, "era_short", game.era_short);
		add_label_data(data, "map_info", game.map_info);
		add_label_data(data, "scenario", game.scenario);
		add_label_data(data, "map_size_text", game.map_size_info);
		add_label_data(data, "time_limit", game.time_limit);
		add_label_data(data, "status", game.status);
		add_label_data(data, "gold_text", game.gold);
		add_label_data(data, "xp_text", game.xp);
		add_label_data(data, "vision_text", game.vision);
		add_label_data(data, "time_limit_text", game.time_limit);
		add_label_data(data, "status", game.status);
		add_label_data(data, "observer_icon", game.observers ? "misc/eye.png" : "misc/no_observer.png");
		const char* vision_icon;
		if (game.fog) {
			if (game.shroud) {
				vision_icon = "misc/vision-fog-shroud.png";
			} else {
				vision_icon = "misc/vision-fog.png";
			}
		} else {
			if (game.shroud) {
				vision_icon = "misc/vision-shroud.png";
			} else {
				vision_icon = "misc/vision-none.png";
			}
		}
		add_label_data(data, "vision_icon", vision_icon);

		gamelistbox_->add_row(data);
		tgrid* grid = gamelistbox_->get_row_grid(gamelistbox_->get_item_count() - 1);

		set_visible_if_exists(grid, "time_limit_icon", !game.time_limit.empty());
		set_visible_if_exists(grid, "vision_fog", game.fog);
		set_visible_if_exists(grid, "vision_shroud", game.shroud);
		set_visible_if_exists(grid, "vision_none", !(game.fog || game.shroud));
		set_visible_if_exists(grid, "observers_yes", game.observers);
		set_visible_if_exists(grid, "observers_no", !game.observers);
		set_visible_if_exists(grid, "needs_password", game.password_required);
		set_visible_if_exists(grid, "reloaded", game.reloaded);
		set_visible_if_exists(grid, "started", game.started);
		set_visible_if_exists(grid, "use_map_settings", game.use_map_settings);

		tbutton* join_button = dynamic_cast<tbutton*>(grid->find_widget("join", false));
		if (join_button) {
			join_button->set_callback_mouse_left_click(
				dialog_callback<tlobby_main, &tlobby_main::join_button_callback>);
		}
		tbutton* observe_button = dynamic_cast<tbutton*>(grid->find_widget("observe", false));
		if (observe_button) {
			observe_button->set_callback_mouse_left_click(
				dialog_callback<tlobby_main, &tlobby_main::observe_button_callback>);
		}
		tminimap* minimap = dynamic_cast<tminimap*>(grid->find_widget("minimap", false));
		if (minimap) {
			minimap->set_config(&game_config_);
			minimap->set_map_data(game.map_data);
		}
	}
	userlistbox_->clear();
	foreach (const user_info& user, lobby_info_.users())
	{
		std::map<std::string, string_map> data;
		add_label_data(data, "player", user.name);
		userlistbox_->add_row(data);
	}
	window_->invalidate_layout();
}

void tlobby_main::pre_show(CVideo& /*video*/, twindow& window)
{
	gamelistbox_ = dynamic_cast<tlistbox*>(window.find_widget("game_list", false));
	VALIDATE(gamelistbox_, missing_widget("game_list"));

	userlistbox_ = dynamic_cast<tlistbox*>(window.find_widget("user_list", false));
	VALIDATE(userlistbox_, missing_widget("user_list"));

	chat_log_ = dynamic_cast<tlabel*>(window.find_widget("chat_log", false));
	VALIDATE(chat_log_, missing_widget("chat_log"));

	window.set_event_loop_pre_callback(boost::bind(&tlobby_main::network_handler, this));
	window_ = &window;

	chat_input_ = dynamic_cast<ttext_box*>(window.find_widget("chat_input", false));
	VALIDATE(chat_input_, missing_widget("chat_input"));

	GUI2_EASY_BUTTON_CALLBACK(send_message, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(create, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(show_help, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(refresh, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(show_preferences, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(join_global, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(observe_global, tlobby_main);
}

void tlobby_main::post_show(twindow& /*window*/)
{
	window_ = NULL;
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
	std::string sender = data["sender"];
	const std::string& message = data["message"];
	const std::string& room = data["room"];
	if (!room.empty()) {
		sender = room + ": " + sender;
	}
	add_chat_message(time(0), sender, 0, message);
}

void tlobby_main::process_gamelist(const config &data)
{
	lobby_info_.process_gamelist(data);
	update_gamelist();
}

void tlobby_main::process_gamelist_diff(const config &data)
{
	if (lobby_info_.process_gamelist_diff(data)) {
		update_gamelist();
	}
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
	join_global_button_callback(window);
}

void tlobby_main::observe_button_callback(gui2::twindow &window)
{
	LOG_NW << "observe_button_callback\n";
	observe_global_button_callback(window);
}

void tlobby_main::observe_global_button_callback(gui2::twindow &window)
{
	LOG_NW << "observe_global_button_callback\n";
	do_game_join(gamelistbox_->get_selected_row(), true);
	legacy_result_ = OBSERVE;
	window.close();
}

void tlobby_main::join_global_button_callback(gui2::twindow &window)
{
	LOG_NW << "join_global_button_callback\n";
	do_game_join(gamelistbox_->get_selected_row(), false);
	legacy_result_ = JOIN;
	window.close();
}

bool tlobby_main::do_game_join(int idx, bool observe)
{
	if (idx < 0 || idx > static_cast<int>(lobby_info_.games().size())) {
		ERR_NG << "Requested join/observe of a game with index out of range: "
			<< idx << ", games size is " << lobby_info_.games().size() << "\n";
		return false;
	}
	const game_info& game = lobby_info_.games()[idx];

	config response;
	config& join = response.add_child("join");
	join["id"] = game.id;
	join["observe"] = observe ? "yes" : "no";
	if (join && game.password_required) {
		std::string password;
		/*
		const int res = gui::show_dialog(disp_, NULL, _("Password Required"),
		          _("Joining this game requires a password."),
		          gui::OK_CANCEL, NULL, NULL, _("Password: "), &password);
		if (res != 0) {
			return false;
		}
		*/
		if(!password.empty()) {
			join["password"] = password;
		}
	}
	network::send_data(response, 0, true);
	if (observe && game.started) {
		playmp_controller::set_replay_last_turn(game.current_turn);
	}
	return true;
}

void tlobby_main::send_message_button_callback(gui2::twindow &/*window*/)
{
	const std::string& input = chat_input_->get_value();
	if (input.empty()) return;
	chat_handler::do_speak(input);
	chat_input_->set_value("");
}

void tlobby_main::create_button_callback(gui2::twindow& window)
{
	legacy_result_ = CREATE;
	window.close();
}

void tlobby_main::refresh_button_callback(gui2::twindow& /*window*/)
{
	network::send_data(config("refresh_lobby"), 0, true);
}


void tlobby_main::show_preferences_button_callback(gui2::twindow& window)
{
	if (preferences_callback_) {
		preferences_callback_();
		network::send_data(config("refresh_lobby"), 0, true);
	}
}

void tlobby_main::show_help_button_callback(gui2::twindow& /*window*/)
{
}

} // namespace gui2
