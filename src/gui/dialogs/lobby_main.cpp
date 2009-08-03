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
#include "gui/dialogs/lobby_player_info.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"

#include "foreach.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "lobby_data.hpp"
#include "lobby_preferences.hpp"
#include "log.hpp"
#include "network.hpp"
#include "playmp_controller.hpp"
#include "sound.hpp"

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

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(info, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)


namespace gui2 {

void tsub_player_list::init(gui2::twindow &w, const std::string &id)
{
	list = &w.get_widget<tlistbox>(id, false);
	show_toggle = &w.get_widget<ttoggle_button>(id + "_show_toggle", false);
	show_toggle->set_icon_name("lobby/group-expanded.png");
	show_toggle->set_callback_state_change(
		boost::bind(&tsub_player_list::show_toggle_callback, this, _1));
	count = &w.get_widget<tlabel>(id + "_count", false);
	label = &w.get_widget<tlabel>(id + "_label", false);
}

void tsub_player_list::show_toggle_callback(gui2::twidget* /*widget*/)
{
	if (show_toggle->get_value()) {
		list->set_visible(twidget::INVISIBLE);
		show_toggle->set_icon_name("lobby/group-folded.png");
	} else {
		list->set_visible(twidget::VISIBLE);
		show_toggle->set_icon_name("lobby/group-expanded.png");
	}
}

void tsub_player_list::auto_hide()
{
	std::stringstream ss;
	ss << "(" << list->get_item_count() << ")";
	count->set_label(ss.str());
	if (list->get_item_count() == 0) {
		list->set_visible(twidget::INVISIBLE);
		show_toggle->set_visible(twidget::INVISIBLE);
		label->set_visible(twidget::INVISIBLE);
		count->set_visible(twidget::INVISIBLE);
	} else {
		list->set_visible(show_toggle->get_value() ? twidget::INVISIBLE : twidget::VISIBLE);
		show_toggle->set_visible(twidget::VISIBLE);
		label->set_visible(twidget::VISIBLE);
		count->set_visible(twidget::VISIBLE);
	}
}

void tplayer_list::init(gui2::twindow &w)
{
	active_game.init(w, "active_game");
	active_room.init(w, "active_room");
	other_rooms.init(w, "other_rooms");
	other_games.init(w, "other_games");
	sort_by_name = &w.get_widget<ttoggle_button>("player_list_sort_name", false);
	sort_by_relation = &w.get_widget<ttoggle_button>("player_list_sort_relation", false);
}

void tplayer_list::update_sort_icons()
{
	if (sort_by_name->get_value()) {
		sort_by_name->set_icon_name("lobby/sort-az.png");
	} else {
		sort_by_name->set_icon_name("lobby/sort-az-off.png");
	}
	if (sort_by_relation->get_value()) {
		sort_by_relation->set_icon_name("lobby/sort-friend.png");
	} else {
		sort_by_relation->set_icon_name("lobby/sort-friend-off.png");
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

void tlobby_main::user_relation_changed(const std::string& /*name*/)
{
	player_list_dirty_ = true;
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
	} else if (tlobby_chat_window* t =  whisper_window_open(receiver, preferences::auto_open_whisper_windows())) {
		switch_to_window(t);
		add_active_window_message(preferences::login(), message);
	} else {
		add_active_window_whisper("whisper to " + receiver, message);
	}
	lobby_info_.get_whisper_log(receiver).add_message(preferences::login(), message);
}

void tlobby_main::add_whisper_received(const std::string& sender, const std::string& message)
{
	bool can_go_to_active = !preferences::whisper_friends_only() || preferences::is_friend(sender);
	bool can_open_new = preferences::auto_open_whisper_windows() && can_go_to_active;
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
		LOG_LB << "Ignoring whisper from " << sender << "\n";
	}
}

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
		LOG_LB << "Cannot add sent message to ui for room " << room
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
		LOG_LB << "Discarding message to room " << room
			<< " from " << speaker << " (room not open)\n";
	}
}

void tlobby_main::append_to_chatbox(const std::string& text)
{
	append_to_chatbox(text, active_window_);
}

void tlobby_main::append_to_chatbox(const std::string& text, size_t id)
{
	tgrid& grid = chat_log_container_->page_grid(id);
	tcontrol& log = grid.get_widget<tcontrol>("log_text", false);
	log.set_label(log.label() + "\n" + preferences::get_chat_timestamp(time(0)) + text);
	window_->invalidate_layout();
}

void tlobby_main::do_notify(t_notify_mode mode)
{
	if (preferences::lobby_sounds()) {
		switch (mode) {
			case NOTIFY_WHISPER:
			case NOTIFY_WHISPER_OTHER_WINDOW:
			case NOTIFY_OWN_NICK:
				sound::play_UI_sound(game_config::sounds::receive_message_highlight);
				break;
			case NOTIFY_FRIEND_MESSAGE:
				sound::play_UI_sound(game_config::sounds::receive_message_friend);
				break;
			case NOTIFY_SERVER_MESSAGE:
				sound::play_UI_sound(game_config::sounds::receive_message_server);
				break;
			case NOTIFY_LOBBY_QUIT:
				sound::play_UI_sound(game_config::sounds::user_leave);
				break;
			case NOTIFY_LOBBY_JOIN:
				sound::play_UI_sound(game_config::sounds::user_arrive);
				break;
			case NOTIFY_MESSAGE:
				break;
			default:
				break;
		}
	}
}

tlobby_main::tlobby_main(const config& game_config, lobby_info& info, display& disp)
: legacy_result_(QUIT)
, game_config_(game_config)
, gamelistbox_(NULL), chat_log_container_(NULL)
, chat_input_(NULL), window_(NULL)
, lobby_info_(info), preferences_callback_(NULL)
, open_windows_(), active_window_(0), selected_game_id_()
, player_list_(), player_list_dirty_(false), disp_(disp)
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

void add_tooltip_data(std::map<std::string, string_map>& map,
					const std::string& key, const std::string& label)
{
	string_map item;
	item["tooltip"] = label;
	map.insert(std::make_pair(key, item));
}

void set_visible_if_exists(tgrid* grid, const char* id, bool visible)
{
	twidget* w = grid->find_widget(id, false);
	if (w) {
		w->set_visible(visible ? twidget::VISIBLE : twidget::INVISIBLE);
	}
}

std::string colorize(const std::string& str, const std::string& color)
{
	return "<span color=\"" + color +"\">" + str + "</span>";
}

std::string tag(const std::string& str, const std::string& tag)
{
	return "<" + tag + ">" + str + "</" + tag + ">";
}

} //end anonymous namespace

void tlobby_main::update_gamelist()
{
	gamelistbox_->clear();
	lobby_info_.apply_game_filter();
	std::stringstream ss;
	ss << "Games: Showing " << lobby_info_.games_filtered().size()
		<< " out of " << lobby_info_.games().size();
	gamelistbox_->get_widget<tlabel>("map", false).set_label(ss.str());
	for (unsigned i = 0; i < lobby_info_.games_filtered().size(); ++i) {
		const game_info& game = *lobby_info_.games_filtered()[i];
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
		if (game.observers) {
			add_label_data(data, "observer_icon", "misc/eye.png");
			add_tooltip_data(data, "observer_icon", _("Observers allowed"));
		} else {
			add_label_data(data, "observer_icon", "misc/no_observer.png");
			add_tooltip_data(data, "observer_icon", _("Observers not allowed"));
		}

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
		add_tooltip_data(data, "vision_icon", game.vision);

		gamelistbox_->add_row(data);
		tgrid* grid = gamelistbox_->get_row_grid(gamelistbox_->get_item_count() - 1);
		ttoggle_panel& row_panel = grid->get_widget<ttoggle_panel>("panel", false);
		row_panel.set_callback_mouse_left_double_click(boost::bind(
			&tlobby_main::join_or_observe, this, i));

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
		set_visible_if_exists(grid, "no_era", !game.have_era);

		tbutton* join_button = dynamic_cast<tbutton*>(grid->find_widget("join", false));
		if (join_button) {
			join_button->set_callback_mouse_left_click(
				dialog_callback<tlobby_main, &tlobby_main::join_button_callback>);
			join_button->set_active(game.can_join());
		}
		tbutton* observe_button = dynamic_cast<tbutton*>(grid->find_widget("observe", false));
		if (observe_button) {
			observe_button->set_callback_mouse_left_click(
				dialog_callback<tlobby_main, &tlobby_main::observe_button_callback>);
			observe_button->set_active(game.can_observe());
		}
		tminimap* minimap = dynamic_cast<tminimap*>(grid->find_widget("minimap", false));
		if (minimap) {
			minimap->set_config(&game_config_);
			minimap->set_map_data(game.map_data);
		}
	}
	update_selected_game();
}

void tlobby_main::update_playerlist()
{
	lobby_info_.update_user_statuses(selected_game_id_, active_window_room());
	lobby_info_.sort_users(player_list_.sort_by_name->get_value(), player_list_.sort_by_relation->get_value());

	bool lobby = false;
	if (room_info* ri = active_window_room()) {
		if (ri->name() == "lobby") {
			lobby = true;
		}
	}
	player_list_.active_game.list->clear();
	player_list_.active_room.list->clear();
	player_list_.other_rooms.list->clear();
	player_list_.other_games.list->clear();
	foreach (user_info* userptr, lobby_info_.users_sorted())
	{
		user_info& user = *userptr;
		tsub_player_list* target_list(NULL);
		std::map<std::string, string_map> data;
		std::stringstream icon_ss;
		std::string name = user.name;
		icon_ss << "lobby/status";
		switch (user.state) {
			case user_info::SEL_ROOM:
				icon_ss << "-lobby";
				target_list = &player_list_.active_room;
				if (lobby) {
					target_list = &player_list_.other_rooms;
				}
				break;
			case user_info::LOBBY:
				icon_ss << "-lobby";
				target_list = &player_list_.other_rooms;
				break;
			case user_info::SEL_GAME:
				name = colorize(name, "cyan");
				icon_ss << (user.observing ? "-obs" : "-playing");
				target_list = &player_list_.active_game;
				break;
			case user_info::GAME:
				name = colorize(name, "red");
				icon_ss << (user.observing ? "-obs" : "-playing");
				target_list = &player_list_.other_games;
				break;
			default:
				ERR_LB << "Bad user state in lobby: " << user.name << ": " << user.state << "\n";
				continue;
		}
		switch (user.relation) {
			case user_info::ME:
				/* fall through */
			case user_info::NEUTRAL:
				icon_ss << "-n";
				break;
			case user_info::FRIEND:
				icon_ss << "-f";
				break;
			case user_info::IGNORED:
				icon_ss << "-i";
				break;
			default:
				ERR_LB << "Bad user relation in lobby: " << user.relation << "\n";
		}
		if (user.registered) {
			name = tag(name, "b");
		}
		icon_ss << ".png";
		add_label_data(data, "player", name);
		add_label_data(data, "main_icon", icon_ss.str());
		if (!preferences::playerlist_group_players()) {
			target_list = &player_list_.other_rooms;
		}
		target_list->list->add_row(data);

		tgrid* grid = target_list->list->get_row_grid(target_list->list->get_item_count() - 1);
		tlabel& name_label = grid->get_widget<tlabel>("player", false);
		name_label.set_markup_mode(tcontrol::PANGO_MARKUP);
		ttoggle_panel& panel = grid->get_widget<ttoggle_panel>("userpanel", false);
		panel.set_callback_mouse_left_double_click(boost::bind(
			&tlobby_main::user_dialog_callback, this, userptr));

	}
	player_list_.active_game.auto_hide();
	player_list_.active_room.auto_hide();
	player_list_.other_rooms.auto_hide();
	player_list_.other_games.auto_hide();

	window_->invalidate_layout();
	player_list_dirty_ = false;
}

void tlobby_main::update_selected_game()
{
	int idx = gamelistbox_->get_selected_row();
	bool can_join = false, can_observe = false;
	if (idx >= 0) {
		const game_info& game = *lobby_info_.games_filtered()[idx];
		can_observe = game.can_observe();
		can_join = game.can_join();
		selected_game_id_ = game.id;
	} else {
		selected_game_id_ = 0;
	}
	window_->get_widget<tbutton>("observe_global", false).set_active(can_observe);
	window_->get_widget<tbutton>("join_global", false).set_active(can_join);
	update_playerlist();
}

void tlobby_main::pre_show(CVideo& /*video*/, twindow& window)
{
	roomlistbox_ = &window.get_widget<tlistbox>("room_list", false);
	VALIDATE(roomlistbox_, missing_widget("room_list"));
	roomlistbox_->set_callback_value_change(dialog_callback<tlobby_main, &tlobby_main::room_switch_callback>);

	gamelistbox_ = dynamic_cast<tlistbox*>(window.find_widget("game_list", false));
	VALIDATE(gamelistbox_, missing_widget("game_list"));
	gamelistbox_->set_callback_value_change(dialog_callback<tlobby_main, &tlobby_main::gamelist_change_callback>);

	player_list_.init(window);

	player_list_.sort_by_name->set_value(preferences::playerlist_sort_name());
	player_list_.sort_by_relation->set_value(preferences::playerlist_sort_relation());
	player_list_.update_sort_icons();

	player_list_.sort_by_name->set_callback_state_change(boost::bind(
		&tlobby_main::player_filter_callback, this, _1));
	player_list_.sort_by_relation->set_callback_state_change(boost::bind(
		&tlobby_main::player_filter_callback, this, _1));


	chat_log_container_ = dynamic_cast<tmulti_page*>(window.find_widget("chat_log_container", false));
	VALIDATE(chat_log_container_, missing_widget("chat_log_container_"));

	window.set_enter_disabled(true);
	window.set_event_loop_pre_callback(boost::bind(&tlobby_main::network_handler, this));
	window_ = &window;

	chat_input_ = dynamic_cast<ttext_box*>(window.find_widget("chat_input", false));
	VALIDATE(chat_input_, missing_widget("chat_input"));
	chat_input_->set_key_press_callback(boost::bind(&tlobby_main::chat_input_keypress_callback, this, _1, _2, _3, _4));

	GUI2_EASY_BUTTON_CALLBACK(send_message, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(create, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(show_help, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(refresh, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(show_preferences, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(join_global, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(observe_global, tlobby_main);
	GUI2_EASY_BUTTON_CALLBACK(close_window, tlobby_main);

	ttoggle_button& skip_replay = window.get_widget<ttoggle_button>("skip_replay", false);
	skip_replay.set_value(preferences::skip_mp_replay());
	skip_replay.set_callback_state_change(boost::bind(&tlobby_main::skip_replay_changed_callback, this, _1));

	filter_friends_ = &window.get_widget<ttoggle_button>("filter_with_friends", false);
	filter_ignored_ = &window.get_widget<ttoggle_button>("filter_without_ignored", false);
	filter_slots_ = &window.get_widget<ttoggle_button>("filter_vacant_slots", false);
	filter_invert_ = &window.get_widget<ttoggle_button>("filter_invert", false);
	filter_text_= &window.get_widget<ttext_box>("filter_text", false);

	filter_friends_->set_callback_state_change(
		boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	filter_ignored_->set_callback_state_change(
		boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	filter_slots_->set_callback_state_change(
		boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	filter_invert_->set_callback_state_change(
		boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	filter_text_->set_key_press_callback(
		boost::bind(&tlobby_main::game_filter_keypress_callback, this, _1, _2, _3, _4));

	room_window_open("lobby", true);
	active_window_changed();
	game_filter_reload();
}

void tlobby_main::post_show(twindow& /*window*/)
{
	window_ = NULL;
}

room_info* tlobby_main::active_window_room()
{
	const tlobby_chat_window& t = open_windows_[active_window_];
	if (t.whisper) return NULL;
	return lobby_info_.get_room(t.name);
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
		std::map<std::string, string_map> data;
		utils::string_map symbols;
		symbols["name"] = name;
		if (whisper) {
			add_label_data(data, "log_header", "<" + name + ">");
			add_label_data(data, "log_text", vgettext(
				"Whisper session with $name started. "
				"If you don't want to receive messages from this user, "
				"type /ignore $name\n", symbols));
		} else {
			add_label_data(data, "log_header", name);
			add_label_data(data, "log_text", vgettext(
				"Room $name joined", symbols));
			lobby_info_.open_room(name);
		}
		chat_log_container_->add_page(data);
		std::map<std::string, string_map> data2;
		add_label_data(data2, "room", whisper ? "<" + name + ">" : name);
		roomlistbox_->add_row(data2);

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
		if (t->pending_messages == 1) {
			DBG_LB << "do whisper pending mark row "
				<< (t - &open_windows_[0]) << " with " << t->name << "\n";
			tgrid* grid = roomlistbox_->get_row_grid(t - &open_windows_[0]);
			//this breaks for some reason
			//tlabel& label = grid->get_widget<tlabel>("room", false);
			//label.set_markup_mode(tcontrol::PANGO_MARKUP);
			//label.set_label(colorize("<" + t->name + ">", "red"));
			grid->get_widget<timage>("pending_messages", false).set_visible(twidget::VISIBLE);
		}
	}
}

void tlobby_main::increment_waiting_messages(const std::string& room)
{
	if (tlobby_chat_window* t = room_window_open(room, false)) {
		t->pending_messages++;
		if (t->pending_messages == 1) {
			int idx = t - &open_windows_[0];
			DBG_LB << "do room pending mark row "
				<< idx << " with " << t->name << "\n";
			tgrid* grid = roomlistbox_->get_row_grid(idx);
			//this breaks for some reason
			//tlabel& label = grid->get_widget<tlabel>("room", false);
			//label.set_markup_mode(tcontrol::PANGO_MARKUP);
			//label.set_label(colorize(t->name, "red"));
			grid->get_widget<timage>("pending_messages", false).set_visible(twidget::VISIBLE);
		}
	}
}

void tlobby_main::add_whisper_window_whisper(const std::string& sender, const std::string& message)
{
	std::stringstream ss;
	ss << "<" << sender << ">" << message;
	tlobby_chat_window* t = whisper_window_open(sender, false);
	if (!t) {
		ERR_LB << "Whisper window not open in add_whisper_window_whisper for " << sender << "\n";
		return;
	}
	append_to_chatbox(ss.str(), t - &open_windows_[0]);
}

void tlobby_main::add_active_window_whisper(const std::string& sender, const std::string& message)
{
	std::stringstream ss;
	ss << "<" << "whisper: " << sender << ">" << message;
	append_to_chatbox(ss.str());
}

void tlobby_main::add_room_window_message(const std::string& room,
	const std::string& sender, const std::string& message)
{
	std::stringstream ss;
	ss << "<" << sender << ">" << message;
	tlobby_chat_window* t = room_window_open(room, false);
	if (!t) {
		ERR_LB << "Room window not open in add_room_window_message for " << room << "\n";
		return;
	}
	append_to_chatbox(ss.str(), t - &open_windows_[0]);
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
	switch_to_window(t - &open_windows_[0]);
}

void tlobby_main::switch_to_window(size_t id)
{
	active_window_ = id;
	assert(active_window_ < open_windows_.size());
	chat_log_container_->select_page(active_window_);
	roomlistbox_->select_row(active_window_);
	active_window_changed();
}

void tlobby_main::active_window_changed()
{
	tlabel& header = chat_log_container_->
		page_grid(active_window_).get_widget<tlabel>("log_header", false);
	tlobby_chat_window& t = open_windows_[active_window_];
	std::string expected_label;
	if (t.whisper) {
		expected_label = "<" + t.name + ">";
	} else {
		expected_label = t.name;
	}
	if (header.label() != expected_label) {
		ERR_LB << "Chat log header not what it should be! "
			<< header.label() << " vs " << expected_label << "\n";
	}

	DBG_LB << "active window changed to " << active_window_ << " "
		<< (t.whisper ? "w" : "r") << " "
		<< t.name << " " << t.pending_messages << " : " << expected_label << "\n";

	//clear pending messages notification in room listbox
	tgrid* grid = roomlistbox_->get_row_grid(active_window_);
	//this breaks for some reason
	//tlabel& label = grid->get_widget<tlabel>("room", false);
	//label.set_label(expected_label);
	grid->get_widget<timage>("pending_messages", false).set_visible(twidget::HIDDEN);
	t.pending_messages = 0;

	window_->get_widget<tbutton>("close_window", false).set_active(t.whisper || t.name != "lobby");
	update_playerlist();
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
	if (t.whisper == false) {
		//closing a room window -- send a part to the server
		config data, msg;
		msg["room"] = t.name;
		msg["player"] = preferences::login();
		data.add_child("room_part", msg);
		network::send_data(data, 0, true);
	}
	if (active_window_ == open_windows_.size() - 1) {
		active_window_--;
	}
	if (t.whisper) {
		lobby_info_.get_whisper_log(t.name).clear();
	} else {
		lobby_info_.close_room(t.name);
	}
	open_windows_.erase(open_windows_.begin() + idx);
	roomlistbox_->remove_row(idx);
	roomlistbox_->select_row(active_window_);
	chat_log_container_->remove_page(idx);
	chat_log_container_->select_page(active_window_);
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
		LOG_LB << "caught network::error in network_handler: " << e.message << "\n";
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
	DBG_LB << "process message from " << sender << " "
		<< (whisper ? "(w)" : "") << ", len " << data["message"].size() << "\n";
	if (preferences::is_ignored(sender)) return;
	const std::string& message = data["message"];
	preferences::parse_admin_authentication(sender, message);
	if (whisper) {
		add_whisper_received(sender, message);
	} else {
		std::string room = data["room"];
		if (room.empty()) {
			LOG_LB << "Message without a room from " << sender << ", assuming lobby\n";
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
	int joined = data.child_count("insert_child");
	int left = data.child_count("remove_child");
	if (joined > 0 || left > 0) {
		if (left > joined) {
			do_notify(NOTIFY_LOBBY_QUIT);
		} else {
			do_notify(NOTIFY_LOBBY_JOIN);
		}
	}
}

void tlobby_main::process_room_join(const config &data)
{
	const std::string& room = data["room"];
	const std::string& player = data["player"];
	room_info* r = lobby_info_.get_room(room);

	if (r) {
		if (player == preferences::login()) {
			if (const config& members = data.child("members")) {
				r->process_room_members(members);
			}
		} else {
			r->add_member(player);
		}
		if (r == active_window_room()) {
			update_playerlist();
		}
	} else {
		if (player == preferences::login()) {
			tlobby_chat_window* t = room_window_open(room, true);
			lobby_info_.open_room(room);
			r = lobby_info_.get_room(room);
			assert(r);
			if (const config& members = data.child("members")) {
				r->process_room_members(members);
			}
			switch_to_window(t);

			const std::string& topic = data["topic"];
			if (!topic.empty()) {
				add_chat_room_message_received("room", "server", room + ": " + topic);
			}
		} else {
			LOG_LB << "Discarding join info for a room the player is not in\n";
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
		LOG_LB << "Discarding part info for a room the player is not in\n";
	}
}

void tlobby_main::process_room_query_response(const config& data)
{
	const std::string& room = data["room"];
	const std::string& message = data["message"];
	if (room.empty()) {
		if (!message.empty()) {
			add_active_window_message("server", message);
		}
	} else {
		if (room_window_open(room, false)) {
			if (!message.empty()) {
				add_chat_room_message_received(room, "server", message);
			}
			if (const config& members = data.child("members")) {
				room_info* r = lobby_info_.get_room(room);
				assert(r);
				r->process_room_members(members);
				if (r == active_window_room()) {
					update_playerlist();
				}
			}
		} else {
			if (!message.empty()) {
				add_active_window_message("server", room + ": " + message);
			}
		}
	}
}

void tlobby_main::join_button_callback(gui2::twindow &window)
{
	LOG_LB << "join_button_callback\n";
	join_global_button_callback(window);
}

void tlobby_main::observe_button_callback(gui2::twindow &window)
{
	LOG_LB << "observe_button_callback\n";
	observe_global_button_callback(window);
}

void tlobby_main::observe_global_button_callback(gui2::twindow &window)
{
	LOG_LB << "observe_global_button_callback\n";
	if (do_game_join(gamelistbox_->get_selected_row(), true)) {
		legacy_result_ = OBSERVE;
		window.close();
	}
}

void tlobby_main::join_global_button_callback(gui2::twindow &window)
{
	LOG_LB << "join_global_button_callback\n";
	if (do_game_join(gamelistbox_->get_selected_row(), false)) {
		legacy_result_ = JOIN;
		window.close();
	}
}

void tlobby_main::join_or_observe(int idx)
{
	const game_info& game = *lobby_info_.games_filtered()[idx];
	if (do_game_join(idx, !game.can_join())) {
		legacy_result_ = game.can_join() ? JOIN : OBSERVE;
		window_->close();
	}
}

bool tlobby_main::do_game_join(int idx, bool observe)
{
	if (idx < 0 || idx > static_cast<int>(lobby_info_.games_filtered().size())) {
		ERR_LB << "Requested join/observe of a game with index out of range: "
			<< idx << ", games size is " << lobby_info_.games_filtered().size() << "\n";
		return false;
	}
	const game_info& game = *lobby_info_.games_filtered()[idx];
	if (observe) {
		if (!game.can_observe()) {
			ERR_LB << "Requested observe of a game with observers disabled\n";
			return false;
		}
	} else {
		if (!game.can_join()) {
			ERR_LB << "Requested join to a game with no vacant slots\n";
			return false;
		}
	}
	config response;
	config& join = response.add_child("join");
	join["id"] = lexical_cast<std::string>(game.id);
	join["observe"] = observe ? "yes" : "no";
	if (join && game.password_required) {
		std::string password;
		//TODO replace with a gui2 dialog
		const int res = gui::show_dialog(disp_, NULL, _("Password Required"),
		          _("Joining this game requires a password."),
		          gui::OK_CANCEL, NULL, NULL, _("Password: "), &password);
		if (res != 0) {
			return false;
		}
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
		if (player_list_dirty_) update_gamelist();
	} else {
		config msg;
		send_message_to_active_window(input);
	}
	chat_input_->save_to_history();
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

void tlobby_main::close_window_button_callback(twindow& /*window*/)
{
	close_active_window();
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

void tlobby_main::room_switch_callback(twindow& /*window*/)
{
	switch_to_window(roomlistbox_->get_selected_row());
}

bool tlobby_main::chat_input_keypress_callback(twidget* widget, SDLKey key,
	SDLMod /*mod*/, Uint16 /*unicode*/)
{
	if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		send_message_button_callback(*widget->get_window());
		return true;
	}
	return false;
}

void tlobby_main::game_filter_reload()
{
	lobby_info_.clear_game_filter();

	foreach (const std::string& s, utils::split(filter_text_->get_value(), ' ')) {
		lobby_info_.add_game_filter(new game_filter_general_string_part(s));
	}
	//TODO: make changing friend/ignore lists trigger a refresh
	if (filter_friends_->get_value()) {
		lobby_info_.add_game_filter(
			new game_filter_value<bool, &game_info::has_friends>(true));
	}
	if (filter_ignored_->get_value()) {
		lobby_info_.add_game_filter(
			new game_filter_value<bool, &game_info::has_ignored>(false));
	}
	if (filter_slots_->get_value()) {
		lobby_info_.add_game_filter(
			new game_filter_value<size_t, &game_info::vacant_slots, std::greater<size_t> >(0));
	}
	lobby_info_.set_game_filter_invert(filter_invert_->get_value());
}

bool tlobby_main::game_filter_keypress_callback(twidget* /*widget*/, SDLKey key,
	SDLMod /*mod*/, Uint16 /*unicode*/)
{
	if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		game_filter_reload();
		update_gamelist();
	}
	return false;
}

void tlobby_main::game_filter_change_callback(gui2::twidget* /*widget*/)
{
	game_filter_reload();
	update_gamelist();
}

void tlobby_main::gamelist_change_callback(gui2::twindow &/*window*/)
{
	update_selected_game();
}

void tlobby_main::player_filter_callback(gui2::twidget* /*widget*/)
{
	player_list_.update_sort_icons();
	preferences::set_playerlist_sort_relation(player_list_.sort_by_relation->get_value());
	preferences::set_playerlist_sort_name(player_list_.sort_by_name->get_value());
	update_gamelist();
}

void tlobby_main::user_dialog_callback(user_info* info)
{
	tlobby_player_info dlg(*this, *info, lobby_info_);
	dlg.show(window_->video());
	if (dlg.result_open_whisper()) {
		tlobby_chat_window* t = whisper_window_open(info->name, true);
		switch_to_window(t);
	}
	update_gamelist();
}

void tlobby_main::skip_replay_changed_callback(twidget* w)
{
	ttoggle_button* tb = dynamic_cast<ttoggle_button*>(w);
	assert(tb);
	preferences::set_skip_mp_replay(tb->get_value());
}

} // namespace gui2
