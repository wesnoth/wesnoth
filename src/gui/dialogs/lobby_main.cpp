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
#define DBG_NW LOG_STREAM(info, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)


namespace gui2 {

namespace {
	//TODO move this and other MP prefs to a MP_prefs header and possibly cpp
	const char* prefkey_whisper_friends_only = "lobby_whisper_friends_only";
	const char* prefkey_auto_open_whisper_windows = "lobby_auto_open_whisper_windows";
	bool whisper_friends_only() {
		return utils::string_bool(preferences::get(prefkey_whisper_friends_only));
	}
	bool auto_open_whisper_windows() {
		return utils::string_bool(preferences::get(prefkey_auto_open_whisper_windows), true);
	}
}

void tlobby_main::send_chat_message(const std::string& message, bool /*allies_only*/)
{
	config data, msg;
	msg["message"] = message;
	msg["sender"] = preferences::login();
	data.add_child("message", msg);

	add_chat_message(time(NULL), preferences::login(), 0, message); //local echo
	network::send_data(data, 0, true);
}

void tlobby_main::add_chat_message(const time_t& /*time*/, const std::string& speaker,
	int /*side*/, const std::string& message, events::chat_handler::MESSAGE_TYPE /*type*/)
{
	std::stringstream ss;
	ss << "<" << speaker << "> ";
	ss << message;
	append_to_chatbox(ss.str());
}


void tlobby_main::add_whisper_sent(const std::string& receiver, const std::string& message)
{
	if (whisper_window_active(receiver)) {
		add_active_window_message(preferences::login(), message);
	} else if (tlobby_chat_window* t =  whisper_window_open(receiver, utils::string_bool(prefkey_auto_open_whisper_windows))) {
		switch_to_window(t);
		add_active_window_message(preferences::login(), message);
	} else {
		add_active_window_whisper("whisper to " + receiver, message);
	}
	lobby_info_.get_whisper_log(receiver).add_message(preferences::login(), message);
}

void tlobby_main::add_whisper_received(const std::string& sender, const std::string& message)
{
	bool can_go_to_active = !whisper_friends_only() || preferences::is_friend(sender);
	bool can_open_new = utils::string_bool(preferences::get(prefkey_auto_open_whisper_windows)) && can_go_to_active;
	lobby_info_.get_whisper_log(sender).add_message(sender, message);
	if (whisper_window_open(sender, can_open_new)) {
		if (whisper_window_active(sender)) {
			add_active_window_message(sender, message);
			do_notify(NOTIFY_WHISPER);
		} else {
			add_whisper_window_whisper(sender, message);
			increment_waiting_whsipers(sender);
			do_notify(NOTIFY_WHISPER_OTHER_WINDOW);
		}
	} else if (can_go_to_active) {
		add_active_window_whisper(sender, message);
		do_notify(NOTIFY_WHISPER);
	} else {
		DBG_NW << "Ignoring whisper from " << sender << "\n";
	}
}

/** inherited form chat_handler */
void tlobby_main::add_chat_room_message_sent(const std::string& room,
	const std::string& message)
{
	//do not open room window here, player should be in the room before sending
	//messages yo it should be allowed to happen
	if (tlobby_chat_window* t = room_window_open(room, false)) {
		room_info* ri = lobby_info_.get_room(room);
		assert(ri);
		if (!room_window_active(room)) {
			switch_to_window(t);
		}
		ri->log().add_message(preferences::login(), message);
		add_active_window_message(preferences::login(), message);
	} else {
		DBG_NW << "Cannot add sent message to ui for room " << room
			<< ", player not in the room\n";
	}
}

void tlobby_main::add_chat_room_message_received(const std::string& room,
	const std::string& speaker, const std::string& message)
{
	if (room_info* ri = lobby_info_.get_room(room)) {
		t_notify_mode notify_mode = NOTIFY_NONE;
		ri->log().add_message(speaker, message);
		if (room_window_active(room)) {
			add_active_window_message(speaker, message);
			notify_mode = NOTIFY_MESSAGE;
		} else {
			add_room_window_message(room, speaker, message);
			increment_waiting_messages(room);
			notify_mode = NOTIFY_MESSAGE_OTHER_WINDOW;
		}
		if (speaker == "server") {
			notify_mode = NOTIFY_SERVER_MESSAGE;
		} else if (utils::word_match(message, preferences::login())) {
			notify_mode = NOTIFY_OWN_NICK;
		} else if (preferences::is_friend(speaker)) {
			notify_mode = NOTIFY_FRIEND_MESSAGE;
		}
		do_notify(notify_mode);
	} else {
		LOG_NW << "Discarding message to room " << room
			<< " from " << speaker << " (room not open)\n";
	}
}

void tlobby_main::append_to_chatbox(const std::string& text)
{
	chat_log_->set_label(chat_log_->label() + "\n" + text);
	window_->invalidate_layout();
}

void tlobby_main::do_notify(t_notify_mode mode)
{
	switch (mode) {
		case NOTIFY_MESSAGE:
			break;
		default:
			break;
	}
}

tlobby_main::tlobby_main(const config& game_config, lobby_info& info)
: legacy_result_(QUIT)
, game_config_(game_config)
, gamelistbox_(NULL), chat_log_(NULL)
, chat_input_(NULL), window_(NULL)
, lobby_info_(info), preferences_callback_(NULL)
, open_windows_(), active_window_(0)
{
	room_window_open("lobby", true);
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
	GUI2_EASY_BUTTON_CALLBACK(next_window, tlobby_main);
}

void tlobby_main::post_show(twindow& /*window*/)
{
	window_ = NULL;
}


tlobby_chat_window* tlobby_main::room_window_open(const std::string& room, bool open_new)
{
	return search_create_window(room, false, open_new);
}

tlobby_chat_window* tlobby_main::whisper_window_open(const std::string& name, bool open_new)
{
	return search_create_window(name, true, open_new);
}

tlobby_chat_window* tlobby_main::search_create_window(const std::string& name, bool whisper, bool open_new)
{
	foreach (tlobby_chat_window& t, open_windows_) {
		if (t.name == name && t.whisper == whisper) return &t;
	}
	if (open_new) {
		open_windows_.push_back(tlobby_chat_window(name, whisper));
		if (!whisper) {
			lobby_info_.open_room(name);
		}
		return &open_windows_.back();
	}
	return NULL;
}

bool tlobby_main::whisper_window_active(const std::string& name)
{
	const tlobby_chat_window& t = open_windows_[active_window_];
	return t.name == name && t.whisper == true;
}

bool tlobby_main::room_window_active(const std::string& room)
{
	const tlobby_chat_window& t = open_windows_[active_window_];
	return t.name == room && t.whisper == false;
}

void tlobby_main::increment_waiting_whsipers(const std::string& name)
{
	if (tlobby_chat_window* t = whisper_window_open(name, false)) {
		t->pending_messages++;
	}
}

void tlobby_main::increment_waiting_messages(const std::string& room)
{
	if (tlobby_chat_window* t = room_window_open(room, false)) {
		t->pending_messages++;
	}
}

void tlobby_main::add_whisper_window_whisper(const std::string& /*sender*/, const std::string& /*message*/)
{
}

void tlobby_main::add_active_window_whisper(const std::string& sender, const std::string& message)
{
	std::stringstream ss;
	ss << "<" << "whisper: " << sender << ">" << message;
	append_to_chatbox(ss.str());
}

void tlobby_main::add_room_window_message(const std::string& /*room*/,
	const std::string& /*sender*/, const std::string& /*message*/)
{
}

void tlobby_main::add_active_window_message(const std::string& sender, const std::string& message)
{
	std::stringstream ss;
	ss << "<" << sender << ">" << message;
	append_to_chatbox(ss.str());
}

void tlobby_main::next_active_window()
{
	if (open_windows_.size() < 2) return;
	if (active_window_ == open_windows_.size() - 1) {
		active_window_ = 0;
	} else {
		active_window_++;
	}
	active_window_changed();
}

void tlobby_main::switch_to_window(tlobby_chat_window* t)
{
	active_window_ = t - &open_windows_[0];
	assert(active_window_ < open_windows_.size());
	active_window_changed();
}

void tlobby_main::active_window_changed()
{
	tlabel* header = dynamic_cast<tlabel*>(window_->find_widget("chat_log_header", false));
	const tlobby_chat_window& t = open_windows_[active_window_];
	if (t.whisper) {
		header->set_label("<" + t.name + ">");
		chat_log_->set_label(lobby_info_.get_whisper_log(t.name).assemble_text());
	} else {
		header->set_label(t.name);
		assert(lobby_info_.get_room(t.name));
		chat_log_->set_label(lobby_info_.get_room(t.name)->log().assemble_text());
	}
	window_->invalidate_layout();
}


void tlobby_main::close_active_window()
{
	return close_window(active_window_);
}

void tlobby_main::close_window(size_t idx)
{
	const tlobby_chat_window& t = open_windows_[idx];
	bool active_changed = idx == active_window_;
	if (t.name == "lobby" && t.whisper == false) return;
	if (open_windows_.size() == 1) return;
	if (active_window_ == open_windows_.size() - 1) {
		active_window_--;
	}
	if (t.whisper) {
		lobby_info_.get_whisper_log(t.name).clear();
	} else {
		lobby_info_.close_room(t.name);
	}
	open_windows_.erase(open_windows_.begin() + idx);
	if (active_changed) active_window_changed();
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

void tlobby_main::process_message(const config &data, bool whisper /*= false*/)
{
	std::string sender = data["sender"];
	if (preferences::is_ignored(sender)) return;
	const std::string& message = data["message"];
	if (whisper) {
		add_whisper_received(sender, message);
	} else {
		std::string room = data["room"];
		if (room.empty()) {
			LOG_NW << "Message without a room from " << sender << ", assuming lobby\n";
			room = "lobby";
		}
		add_chat_room_message_received(room, sender, message);
	}
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

void tlobby_main::process_room_join(const config &data)
{
	const std::string& room = data["room"];
	const std::string& player = data["player"];
	room_info* r = lobby_info_.get_room(room);
	if (r) {
		if (player == preferences::login()) {
			//parse full userlist
		} else {
			r->add_member(player);
			//update userlists
			//check the show-lobby-joins param
		}
	} else {
		if (player == preferences::login()) {
			tlobby_chat_window* t = room_window_open(room, true);
			switch_to_window(t);
			lobby_info_.open_room(room);
		} else {
			LOG_NW << "Discarding join info for a room the player is not in\n";
		}
	}
}

void tlobby_main::process_room_part(const config &data)
{
	//todo close room window when the part message is sent
	const std::string& room = data["room"];
	const std::string& player = data["player"];
	room_info* r = lobby_info_.get_room(room);
	if (r) {
		r->remove_member(player);
	} else {
		LOG_NW << "Discarding part info for a room the player is not in\n";
	}
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
	if (input[0] == '/') {
		//TODO: refactor do_speak so it uses context information about
		//      opened window, so e.g. /ignore in a whisper session ignores
		//      the other party without having to specify it's nick.
		chat_handler::do_speak(input);
	} else {
		config msg;
		send_message_to_active_window(input);
	}
	chat_input_->set_value("");
}

void tlobby_main::send_message_to_active_window(const std::string& input)
{
	tlobby_chat_window& t = open_windows_[active_window_];
	if (t.whisper) {
		send_whisper(t.name, input);
		add_whisper_sent(t.name, input);
	} else {
		send_chat_room_message(t.name, input);
		add_chat_room_message_sent(t.name, input);
	}
}

void tlobby_main::next_window_button_callback(twindow& /*window*/)
{
	next_active_window();
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


void tlobby_main::show_preferences_button_callback(gui2::twindow& /*window*/)
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
