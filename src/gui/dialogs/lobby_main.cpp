/*
   Copyright (C) 2009 - 2014 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/lobby_main.hpp"
#include "gui/dialogs/lobby_player_info.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/timer.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/tree_view_node.hpp"

#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "lobby_preferences.hpp"
#include "log.hpp"
#include "network.hpp"
#include "playmp_controller.hpp"
#include "preferences_display.hpp"
#include "mp_ui_sounds.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)
#define SCOPE_LB log_scope2(log_lobby, __func__)


namespace gui2
{

REGISTER_DIALOG(lobby_main)

void tsub_player_list::init(gui2::twindow& w, const std::string& id)
{
	list = find_widget<tlistbox>(&w, id, false, true);
	show_toggle
			= find_widget<ttoggle_button>(&w, id + "_show_toggle", false, true);
	show_toggle->set_icon_name("lobby/group-expanded.png");
	show_toggle->set_callback_state_change(
			boost::bind(&tsub_player_list::show_toggle_callback, this, _1));
	count = find_widget<tlabel>(&w, id + "_count", false, true);
	label = find_widget<tlabel>(&w, id + "_label", false, true);

	ttree_view& parent_tree = find_widget<ttree_view>(&w, "player_tree", false);

	string_map tree_group_field;
	std::map<std::string, string_map> tree_group_item;

	tree_group_field["label"] = id;
	tree_group_item["tree_view_node_label"] = tree_group_field;
	tree = &parent_tree.add_node("player_group", tree_group_item);

	tree_label = find_widget<tlabel>(tree, "tree_view_node_label", false, true);

	tree_label->set_label(label->label());
}

void tsub_player_list::show_toggle_callback(gui2::twidget& /*widget*/)
{
	if(show_toggle->get_value()) {
		list->set_visible(twidget::tvisible::invisible);
		show_toggle->set_icon_name("lobby/group-folded.png");
	} else {
		list->set_visible(twidget::tvisible::visible);
		show_toggle->set_icon_name("lobby/group-expanded.png");
	}
}

void tsub_player_list::auto_hide()
{
	assert(tree);
	assert(tree_label);
	if(tree->empty()) {
		/**
		 * @todo Make sure setting visible resizes the widget.
		 *
		 * It doesn't work here since invalidate_layout is blocked, but the
		 * widget should also be able to handle it itself. Once done the
		 * setting of the label text can also be removed.
		 */
		assert(label);
		tree_label->set_label(label->label() + " (0)");
		// tree_label->set_visible(twidget::tvisible::invisible);
	} else {
		assert(label);
		std::stringstream ss;
		ss << label->label() << " (" << tree->size() << ")";
		tree_label->set_label(ss.str());
		// tree_label->set_visible(twidget::tvisible::visible);
	}
}

void tplayer_list::init(gui2::twindow& w)
{
	active_game.init(w, "active_game");
	active_room.init(w, "active_room");
	other_rooms.init(w, "other_rooms");
	other_games.init(w, "other_games");
	sort_by_name = find_widget<ttoggle_button>(
			&w, "player_list_sort_name", false, true);
	sort_by_relation = find_widget<ttoggle_button>(
			&w, "player_list_sort_relation", false, true);

	tree = find_widget<ttree_view>(&w, "player_tree", false, true);

	find_widget<twidget>(&w, "old_player_list", false)
			.set_visible(twidget::tvisible::invisible);

	/**
	 * @todo This is a hack to fold the items.
	 *
	 * This hack can be removed when folding is properly implemented.
	 */
	assert(active_room.tree);
	find_widget<ttoggle_button>(active_room.tree, "tree_view_node_icon", false)
			.set_value(true);
	assert(other_rooms.tree);
	find_widget<ttoggle_button>(other_rooms.tree, "tree_view_node_icon", false)
			.set_value(true);
	assert(other_games.tree);
	find_widget<ttoggle_button>(other_games.tree, "tree_view_node_icon", false)
			.set_value(true);
}

void tplayer_list::update_sort_icons()
{
	if(sort_by_name->get_value()) {
		sort_by_name->set_icon_name("lobby/sort-az.png");
	} else {
		sort_by_name->set_icon_name("lobby/sort-az-off.png");
	}
	if(sort_by_relation->get_value()) {
		sort_by_relation->set_icon_name("lobby/sort-friend.png");
	} else {
		sort_by_relation->set_icon_name("lobby/sort-friend-off.png");
	}
}
void tlobby_main::send_chat_message(const std::string& message,
									bool /*allies_only*/)
{
	config data, msg;
	msg["message"] = message;
	msg["sender"] = preferences::login();
	data.add_child("message", msg);

	add_chat_message(time(NULL), preferences::login(), 0, message); // local
																	// echo
	network::send_data(data, 0);
}

void tlobby_main::user_relation_changed(const std::string& /*name*/)
{
	player_list_dirty_ = true;
}

void tlobby_main::add_chat_message(const time_t& /*time*/,
								   const std::string& speaker,
								   int /*side*/,
								   const std::string& message,
								   events::chat_handler::MESSAGE_TYPE /*type*/)
{
	std::stringstream ss;
	ss << "<" << speaker << "> ";
	ss << message;
	append_to_chatbox(ss.str());
}


void tlobby_main::add_whisper_sent(const std::string& receiver,
								   const std::string& message)
{
	if(whisper_window_active(receiver)) {
		add_active_window_message(preferences::login(), message);
	} else if(tlobby_chat_window* t = whisper_window_open(
					  receiver, preferences::auto_open_whisper_windows())) {
		switch_to_window(t);
		add_active_window_message(preferences::login(), message);
	} else {
		utils::string_map symbols;
		symbols["receiver"] = receiver;
		add_active_window_whisper(VGETTEXT("whisper to $receiver", symbols),
								  message);
	}
	lobby_info_.get_whisper_log(receiver)
			.add_message(preferences::login(), message);
}

void tlobby_main::add_whisper_received(const std::string& sender,
									   const std::string& message)
{
	bool can_go_to_active = !preferences::whisper_friends_only()
							|| preferences::is_friend(sender);
	bool can_open_new = preferences::auto_open_whisper_windows()
						&& can_go_to_active;
	lobby_info_.get_whisper_log(sender).add_message(sender, message);
	if(whisper_window_open(sender, can_open_new)) {
		if(whisper_window_active(sender)) {
			add_active_window_message(sender, message);
			do_notify(NOTIFY_WHISPER);
		} else {
			add_whisper_window_whisper(sender, message);
			increment_waiting_whsipers(sender);
			do_notify(NOTIFY_WHISPER_OTHER_WINDOW);
		}
	} else if(can_go_to_active) {
		add_active_window_whisper(sender, message);
		do_notify(NOTIFY_WHISPER);
	} else {
		LOG_LB << "Ignoring whisper from " << sender << "\n";
	}
}

void tlobby_main::add_chat_room_message_sent(const std::string& room,
											 const std::string& message)
{
	// do not open room window here, player should be in the room before sending
	// messages yo it should be allowed to happen
	if(tlobby_chat_window* t = room_window_open(room, false)) {
		room_info* ri = lobby_info_.get_room(room);
		assert(ri);
		if(!room_window_active(room)) {
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
												 const std::string& speaker,
												 const std::string& message)
{
	if(room_info* ri = lobby_info_.get_room(room)) {
		t_notify_mode notify_mode = NOTIFY_NONE;
		ri->log().add_message(speaker, message);
		if(room_window_active(room)) {
			add_active_window_message(speaker, message);
			notify_mode = NOTIFY_MESSAGE;
		} else {
			add_room_window_message(room, speaker, message);
			increment_waiting_messages(room);
			notify_mode = NOTIFY_MESSAGE_OTHER_WINDOW;
		}
		if(speaker == "server") {
			notify_mode = NOTIFY_SERVER_MESSAGE;
		} else if(utils::word_match(message, preferences::login())) {
			notify_mode = NOTIFY_OWN_NICK;
		} else if(preferences::is_friend(speaker)) {
			notify_mode = NOTIFY_FRIEND_MESSAGE;
		}
		do_notify(notify_mode);
	} else {
		LOG_LB << "Discarding message to room " << room << " from " << speaker
			   << " (room not open)\n";
	}
}

void tlobby_main::append_to_chatbox(const std::string& text)
{
	append_to_chatbox(text, active_window_);
}

void tlobby_main::append_to_chatbox(const std::string& text, size_t id)
{
	tgrid& grid = chat_log_container_->page_grid(id);
	tscroll_label& log = find_widget<tscroll_label>(&grid, "log_text", false);
	log.set_label(log.label() + "\n" + preferences::get_chat_timestamp(time(0))
				  + text);
	log.scroll_vertical_scrollbar(tscrollbar_::END);
}

void tlobby_main::do_notify(t_notify_mode mode)
{
	if(preferences::lobby_sounds()) {
		switch(mode) {
			case NOTIFY_WHISPER:
			case NOTIFY_WHISPER_OTHER_WINDOW:
			case NOTIFY_OWN_NICK:
				mp_ui_sounds::private_message(true);
				break;
			case NOTIFY_FRIEND_MESSAGE:
				mp_ui_sounds::friend_message(true);
				break;
			case NOTIFY_SERVER_MESSAGE:
				mp_ui_sounds::server_message(true);
				break;
			case NOTIFY_LOBBY_QUIT:
				mp_ui_sounds::player_leaves(true);
				break;
			case NOTIFY_LOBBY_JOIN:
				mp_ui_sounds::player_joins(true);
				break;
			case NOTIFY_MESSAGE:
				mp_ui_sounds::public_message(true);
				break;
			default:
				break;
		}
	}
}

tlobby_main::tlobby_main(const config& game_config,
						 lobby_info& info,
						 display& disp)
	: legacy_result_(QUIT)
	, game_config_(game_config)
	, gamelistbox_(NULL)
	, userlistbox_(NULL)
	, roomlistbox_(NULL)
	, chat_log_container_(NULL)
	, chat_input_(NULL)
	, window_(NULL)
	, lobby_info_(info)
	, preferences_callback_(NULL)
	, open_windows_()
	, active_window_(0)
	, filter_friends_(NULL)
	, filter_ignored_(NULL)
	, filter_slots_(NULL)
	, filter_invert_(NULL)
	, filter_text_(NULL)
	, selected_game_id_()
	, player_list_()
	, player_list_dirty_(false)
	, gamelist_dirty_(false)
	, last_gamelist_update_(0)
	, gamelist_diff_update_(true)
	, disp_(disp)
	, lobby_update_timer_(0)
	, preferences_wrapper_()
	, gamelist_id_at_row_()
	, delay_playerlist_update_(false)
	, delay_gamelist_update_(false)
{
}

struct lobby_delay_gamelist_update_guard
{
	lobby_delay_gamelist_update_guard(tlobby_main& l) : l(l)
	{
		l.delay_gamelist_update_ = true;
	}
	~lobby_delay_gamelist_update_guard()
	{
		l.delay_gamelist_update_ = false;
	}
	tlobby_main& l;
};

void tlobby_main::set_preferences_callback(boost::function<void()> cb)
{
	preferences_callback_ = cb;
}

tlobby_main::~tlobby_main()
{
	if(lobby_update_timer_) {
		remove_timer(lobby_update_timer_);
	}
}

static void signal_handler_sdl_video_resize(const event::tevent event,
											bool& handled,
											bool& halt,
											const tpoint& new_size,
											CVideo& video)
{
	DBG_GUI_E << __func__ << ": " << event << ".\n";

	if(new_size.x < preferences::min_allowed_width()
	   || new_size.y < preferences::min_allowed_height()) {

		DBG_GUI_E << __func__ << ": resize aborted, too small.\n";
		handled = true;
		halt = true;
		return;
	}

	if(new_size.x == static_cast<int>(settings::screen_width)
	   && new_size.y == static_cast<int>(settings::screen_height)) {

		DBG_GUI_E << __func__ << ": resize not needed.\n";
		handled = true;
		return;
	}

	if(!preferences::set_resolution(video, new_size.x, new_size.y)) {

		LOG_GUI_E << __func__ << ": resize aborted, resize failed.\n";
	}
}

static bool fullscreen(CVideo& video)
{
	preferences::set_fullscreen(video, !preferences::fullscreen());

	// Setting to fullscreen doesn't seem to generate a resize event.
	const SDL_Rect& rect = screen_area();

	SDL_Event event;
#if SDL_VERSION_ATLEAST(2, 0, 0)
	event.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_RESIZED;
	event.window.data1 = rect.w;
	event.window.data2 = rect.h;
#else
	event.type = SDL_VIDEORESIZE;
	event.resize.type = SDL_VIDEORESIZE;
	event.resize.w = rect.w;
	event.resize.h = rect.h;
#endif

	SDL_PushEvent(&event);

	return true;
}

void tlobby_main::post_build(CVideo& video, twindow& window)
{
	/** @todo Should become a global hotkey after 1.8, then remove it here. */
	window.register_hotkey(hotkey::HOTKEY_FULLSCREEN,
						   boost::bind(fullscreen, boost::ref(video)));


	/** @todo Remove this code once the resizing in twindow is finished. */
	window.connect_signal<event::SDL_VIDEO_RESIZE>(
			boost::bind(&signal_handler_sdl_video_resize,
						_2,
						_3,
						_4,
						_5,
						boost::ref(video)),
			event::tdispatcher::front_child);

	/*** Local hotkeys. ***/
	preferences_wrapper_
			= boost::bind(&tlobby_main::show_preferences_button_callback,
						  this,
						  boost::ref(window));

	window.register_hotkey(
			hotkey::HOTKEY_PREFERENCES,
			boost::bind(function_wrapper<bool, boost::function<void()> >,
						true,
						boost::cref(preferences_wrapper_)));
}

namespace
{

void add_label_data(std::map<std::string, string_map>& map,
					const std::string& key,
					const std::string& label)
{
	string_map item;
	item["label"] = label;
	map.insert(std::make_pair(key, item));
}

void add_tooltip_data(std::map<std::string, string_map>& map,
					  const std::string& key,
					  const std::string& label)
{
	string_map item;
	item["tooltip"] = label;
	map.insert(std::make_pair(key, item));
}

void modify_grid_with_data(tgrid* grid,
						   const std::map<std::string, string_map>& map)
{
	FOREACH(const AUTO & v, map)
	{
		const std::string& key = v.first;
		const string_map& strmap = v.second;
		twidget* w = grid->find(key, false);
		if(w == NULL)
			continue;
		tcontrol* c = dynamic_cast<tcontrol*>(w);
		if(c == NULL)
			continue;
		FOREACH(const AUTO & vv, strmap)
		{
			if(vv.first == "label") {
				c->set_label(vv.second);
			} else if(vv.first == "tooltip") {
				c->set_tooltip(vv.second);
			}
		}
	}
}

void set_visible_if_exists(tgrid* grid, const char* id, bool visible)
{
	twidget* w = grid->find(id, false);
	if(w) {
		w->set_visible(visible ? twidget::tvisible::visible
							   : twidget::tvisible::invisible);
	}
}

std::string colorize(const std::string& str, const std::string& color)
{
	return "<span color=\"" + color + "\">" + str + "</span>";
}

std::string tag(const std::string& str, const std::string& tag)
{
	return "<" + tag + ">" + str + "</" + tag + ">";
}

} // end anonymous namespace

void tlobby_main::update_gamelist()
{
	SCOPE_LB;
	gamelistbox_->clear();
	gamelist_id_at_row_.clear();
	lobby_info_.make_games_vector();
	int select_row = -1;
	for(unsigned i = 0; i < lobby_info_.games().size(); ++i) {
		const game_info& game = *lobby_info_.games()[i];
		if(game.id == selected_game_id_) {
			select_row = i;
		}
		gamelist_id_at_row_.push_back(game.id);
		LOG_LB << "Adding game to listbox (1)" << game.id << "\n";
		gamelistbox_->add_row(make_game_row_data(game));
		tgrid* grid = gamelistbox_->get_row_grid(gamelistbox_->get_item_count()
												 - 1);
		adjust_game_row_contents(
				game, gamelistbox_->get_item_count() - 1, grid);
	}
	if(select_row >= 0 && select_row != gamelistbox_->get_selected_row()) {
		gamelistbox_->select_row(select_row);
	}
	update_selected_game();
	gamelist_dirty_ = false;
	last_gamelist_update_ = SDL_GetTicks();
	lobby_info_.sync_games_display_status();
	lobby_info_.apply_game_filter();
	update_gamelist_header();
	gamelistbox_->set_row_shown(lobby_info_.games_visibility());
}

void tlobby_main::update_gamelist_diff()
{
	SCOPE_LB;
	lobby_info_.make_games_vector();
	int select_row = -1;
	unsigned list_i = 0;
	int list_rows_deleted = 0;
	std::vector<int> next_gamelist_id_at_row;
	for(unsigned i = 0; i < lobby_info_.games().size(); ++i) {
		const game_info& game = *lobby_info_.games()[i];
		if(game.display_status == game_info::NEW) {
			LOG_LB << "Adding game to listbox " << game.id << "\n";
			if(list_i != gamelistbox_->get_item_count()) {
				gamelistbox_->add_row(make_game_row_data(game), list_i);
				DBG_LB << "Added a game listbox row not at the end" << list_i
					   << " " << gamelistbox_->get_item_count() << "\n";
				list_rows_deleted--;
			} else {
				gamelistbox_->add_row(make_game_row_data(game));
			}
			tgrid* grid = gamelistbox_->get_row_grid(
					gamelistbox_->get_item_count() - 1);
			adjust_game_row_contents(
					game, gamelistbox_->get_item_count() - 1, grid);
			list_i++;
			next_gamelist_id_at_row.push_back(game.id);
		} else {
			if(list_i >= gamelistbox_->get_item_count()) {
				ERR_LB << "Ran out of listbox items -- triggering a full "
						  "refresh\n";
				network::send_data(config("refresh_lobby"), 0);
				return;
			}
			if(list_i + list_rows_deleted >= gamelist_id_at_row_.size()) {
				ERR_LB << "gamelist_id_at_row_ overflow! " << list_i << " + "
					   << list_rows_deleted
					   << " >= " << gamelist_id_at_row_.size()
					   << " -- triggering a full refresh\n";
				network::send_data(config("refresh_lobby"), 0);
				return;
			}
			int listbox_game_id
					= gamelist_id_at_row_[list_i + list_rows_deleted];
			if(game.id != listbox_game_id) {
				ERR_LB << "Listbox game id does not match expected id "
					   << listbox_game_id << " " << game.id << " (row "
					   << list_i << ")\n";
				network::send_data(config("refresh_lobby"), 0);
				return;
			}
			if(game.display_status == game_info::UPDATED) {
				LOG_LB << "Modifying game in listbox " << game.id << " (row "
					   << list_i << ")\n";
				tgrid* grid = gamelistbox_->get_row_grid(list_i);
				modify_grid_with_data(grid, make_game_row_data(game));
				adjust_game_row_contents(game, list_i, grid);
				++list_i;
				next_gamelist_id_at_row.push_back(game.id);
			} else if(game.display_status == game_info::DELETED) {
				LOG_LB << "Deleting game from listbox " << game.id << " (row "
					   << list_i << ")\n";
				gamelistbox_->remove_row(list_i);
				++list_rows_deleted;
			} else {
				// clean
				LOG_LB << "Clean game in listbox " << game.id << " (row "
					   << list_i << ")\n";
				next_gamelist_id_at_row.push_back(game.id);
				++list_i;
			}
		}
	}
	for(unsigned i = 0; i < next_gamelist_id_at_row.size(); ++i) {
		if(next_gamelist_id_at_row[i] == selected_game_id_) {
			select_row = i;
		}
	}
	next_gamelist_id_at_row.swap(gamelist_id_at_row_);
	if(select_row >= static_cast<int>(gamelistbox_->get_item_count())) {
		ERR_LB << "Would select a row beyond the listbox" << select_row << " "
			   << gamelistbox_->get_item_count() << "\n";
		select_row = gamelistbox_->get_item_count() - 1;
	}
	if(select_row >= 0 && select_row != gamelistbox_->get_selected_row()) {
		gamelistbox_->select_row(select_row);
	}
	update_selected_game();
	gamelist_dirty_ = false;
	last_gamelist_update_ = SDL_GetTicks();
	lobby_info_.sync_games_display_status();
	lobby_info_.apply_game_filter();
	update_gamelist_header();
	gamelistbox_->set_row_shown(lobby_info_.games_visibility());
}

void tlobby_main::update_gamelist_header()
{
#ifndef GUI2_EXPERIMENTAL_LISTBOX
	utils::string_map symbols;
	symbols["num_shown"]
			= lexical_cast<std::string>(lobby_info_.games_filtered().size());
	symbols["num_total"]
			= lexical_cast<std::string>(lobby_info_.games().size());
	std::string games_string
			= VGETTEXT("Games: showing $num_shown out of $num_total", symbols);
	find_widget<tlabel>(gamelistbox_, "map", false).set_label(games_string);
#endif
}

std::map<std::string, string_map>
tlobby_main::make_game_row_data(const game_info& game)
{
	std::map<std::string, string_map> data;

	const char* color_string;
	if(game.vacant_slots > 0) {
		if(game.reloaded || game.started) {
			color_string = "yellow";
		} else {
			color_string = "green";
		}
	} else {
		if(game.observers) {
			color_string = "#ddd";
		} else {
			color_string = "red";
		}
	}
	if(!game.have_era && (game.vacant_slots > 0 || game.observers)) {
		color_string = "#444";
	}

	add_label_data(data, "status", colorize(game.status, color_string));
	add_label_data(data, "name", colorize(game.name, color_string));

	add_label_data(data, "era", game.era);
	add_label_data(data, "era_short", game.era_short);
	add_label_data(data, "map_info", game.map_info);
	add_label_data(data, "scenario", game.scenario);
	add_label_data(data, "map_size_text", game.map_size_info);
	add_label_data(data, "time_limit", game.time_limit);
	add_label_data(data, "gold_text", game.gold);
	add_label_data(data, "xp_text", game.xp);
	add_label_data(data, "vision_text", game.vision);
	add_label_data(data, "time_limit_text", game.time_limit);
	add_label_data(data, "status", game.status);
	if(game.observers) {
		add_label_data(data, "observer_icon", "misc/eye.png");
		add_tooltip_data(data, "observer_icon", _("Observers allowed"));
	} else {
		add_label_data(data, "observer_icon", "misc/no_observer.png");
		add_tooltip_data(data, "observer_icon", _("Observers not allowed"));
	}

	const char* vision_icon;
	if(game.fog) {
		if(game.shroud) {
			vision_icon = "misc/vision-fog-shroud.png";
		} else {
			vision_icon = "misc/vision-fog.png";
		}
	} else {
		if(game.shroud) {
			vision_icon = "misc/vision-shroud.png";
		} else {
			vision_icon = "misc/vision-none.png";
		}
	}
	add_label_data(data, "vision_icon", vision_icon);
	add_tooltip_data(data, "vision_icon", game.vision);
	return data;
}

void tlobby_main::adjust_game_row_contents(const game_info& game,
										   int idx,
										   tgrid* grid)
{
	find_widget<tcontrol>(grid, "name", false).set_use_markup(true);

	find_widget<tcontrol>(grid, "status", false).set_use_markup(true);

	ttoggle_panel& row_panel = find_widget<ttoggle_panel>(grid, "panel", false);

	row_panel.set_callback_mouse_left_double_click(
			boost::bind(&tlobby_main::join_or_observe, this, idx));

	set_visible_if_exists(grid, "time_limit_icon", !game.time_limit.empty());
	set_visible_if_exists(grid, "vision_fog", game.fog);
	set_visible_if_exists(grid, "vision_shroud", game.shroud);
	set_visible_if_exists(grid, "vision_none", !(game.fog || game.shroud));
	set_visible_if_exists(grid, "observers_yes", game.observers);
	set_visible_if_exists(grid, "shuffle_sides_icon", game.shuffle_sides);
	set_visible_if_exists(grid, "observers_no", !game.observers);
	set_visible_if_exists(grid, "needs_password", game.password_required);
	set_visible_if_exists(grid, "reloaded", game.reloaded);
	set_visible_if_exists(grid, "started", game.started);
	set_visible_if_exists(grid, "use_map_settings", game.use_map_settings);
	set_visible_if_exists(grid, "no_era", !game.have_era);


	tbutton* join_button = dynamic_cast<tbutton*>(grid->find("join", false));
	if(join_button) {
		connect_signal_mouse_left_click(
				*join_button,
				boost::bind(&tlobby_main::join_button_callback,
							this,
							boost::ref(*window_)));
		join_button->set_active(game.can_join());
	}
	tbutton* observe_button
			= dynamic_cast<tbutton*>(grid->find("observe", false));
	if(observe_button) {
		connect_signal_mouse_left_click(
				*observe_button,
				boost::bind(&tlobby_main::observe_button_callback,
							this,
							boost::ref(*window_)));
		observe_button->set_active(game.can_observe());
	}
	tminimap* minimap = dynamic_cast<tminimap*>(grid->find("minimap", false));
	if(minimap) {
		minimap->set_config(&game_config_);
		minimap->set_map_data(game.map_data);
	}
}

void tlobby_main::update_gamelist_filter()
{
	DBG_LB << "tlobby_main::update_gamelist_filter\n";
	lobby_info_.apply_game_filter();
	DBG_LB << "Games in lobby_info: " << lobby_info_.games().size()
		   << ", games in listbox: " << gamelistbox_->get_item_count() << "\n";
	assert(lobby_info_.games().size() == gamelistbox_->get_item_count());
	gamelistbox_->set_row_shown(lobby_info_.games_visibility());
}


void tlobby_main::update_playerlist()
{
	if(delay_playerlist_update_)
		return;
	SCOPE_LB;
	DBG_LB << "Playerlist update: " << lobby_info_.users().size() << "\n";
	lobby_info_.update_user_statuses(selected_game_id_, active_window_room());
	lobby_info_.sort_users(player_list_.sort_by_name->get_value(),
						   player_list_.sort_by_relation->get_value());

	bool lobby = false;
	if(room_info* ri = active_window_room()) {
		if(ri->name() == "lobby") {
			lobby = true;
		}
	}
	player_list_.active_game.list->clear();
	player_list_.active_room.list->clear();
	player_list_.other_rooms.list->clear();
	player_list_.other_games.list->clear();

	assert(player_list_.active_game.tree);
	assert(player_list_.active_room.tree);
	assert(player_list_.other_games.tree);
	assert(player_list_.other_rooms.tree);

	player_list_.active_game.tree->clear();
	player_list_.active_room.tree->clear();
	player_list_.other_games.tree->clear();
	player_list_.other_rooms.tree->clear();

	FOREACH(AUTO userptr, lobby_info_.users_sorted())
	{
		user_info& user = *userptr;
		tsub_player_list* target_list(NULL);
		std::map<std::string, string_map> data;
		std::stringstream icon_ss;
		std::string name = user.name;
		icon_ss << "lobby/status";
		switch(user.state) {
			case user_info::SEL_ROOM:
				icon_ss << "-lobby";
				target_list = &player_list_.active_room;
				if(lobby) {
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
				ERR_LB << "Bad user state in lobby: " << user.name << ": "
					   << user.state << "\n";
				continue;
		}
		switch(user.relation) {
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
				ERR_LB << "Bad user relation in lobby: " << user.relation
					   << "\n";
		}
		if(user.registered) {
			name = tag(name, "b");
		}
		icon_ss << ".png";
		add_label_data(data, "player", name);
		add_label_data(data, "main_icon", icon_ss.str());
		if(!preferences::playerlist_group_players()) {
			target_list = &player_list_.other_rooms;
		}

		assert(target_list->tree);

		string_map tree_group_field;
		std::map<std::string, string_map> tree_group_item;

		/*** Add tree item ***/
		tree_group_field["label"] = icon_ss.str();
		tree_group_item["icon"] = tree_group_field;

		tree_group_field["label"] = name;
		tree_group_field["use_markup"] = "true";
		tree_group_item["name"] = tree_group_field;

		ttree_view_node& player
				= target_list->tree->add_child("player", tree_group_item);

		find_widget<ttoggle_panel>(&player, "tree_view_node_label", false)
				.set_callback_mouse_left_double_click(boost::bind(
						 &tlobby_main::user_dialog_callback, this, userptr));
	}
	player_list_.active_game.auto_hide();
	player_list_.active_room.auto_hide();
	player_list_.other_rooms.auto_hide();
	player_list_.other_games.auto_hide();

	player_list_dirty_ = false;
}

void tlobby_main::update_selected_game()
{
	int idx = gamelistbox_->get_selected_row();
	bool can_join = false, can_observe = false;
	if(idx >= 0) {
		const game_info& game = *lobby_info_.games()[idx];
		can_observe = game.can_observe();
		can_join = game.can_join();
		selected_game_id_ = game.id;
	} else {
		selected_game_id_ = 0;
	}
	find_widget<tbutton>(window_, "observe_global", false)
			.set_active(can_observe);

	find_widget<tbutton>(window_, "join_global", false).set_active(can_join);

	player_list_dirty_ = true;
}

void tlobby_main::pre_show(CVideo& /*video*/, twindow& window)
{
	SCOPE_LB;
	roomlistbox_ = find_widget<tlistbox>(&window, "room_list", false, true);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(
			*roomlistbox_,
			boost::bind(&tlobby_main::room_switch_callback,
						*this,
						boost::ref(window)));
#else
	roomlistbox_->set_callback_value_change(
			dialog_callback<tlobby_main, &tlobby_main::room_switch_callback>);
#endif

	gamelistbox_ = find_widget<tlistbox>(&window, "game_list", false, true);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(
			*gamelistbox_,
			boost::bind(&tlobby_main::gamelist_change_callback,
						*this,
						boost::ref(window)));
#else
	gamelistbox_->set_callback_value_change(
			dialog_callback<tlobby_main,
							&tlobby_main::gamelist_change_callback>);
#endif

	player_list_.init(window);

	player_list_.sort_by_name->set_value(preferences::playerlist_sort_name());
	player_list_.sort_by_relation->set_value(
			preferences::playerlist_sort_relation());
	player_list_.update_sort_icons();

	player_list_.sort_by_name->set_callback_state_change(
			boost::bind(&tlobby_main::player_filter_callback, this, _1));
	player_list_.sort_by_relation->set_callback_state_change(
			boost::bind(&tlobby_main::player_filter_callback, this, _1));

	chat_log_container_ = find_widget<tmulti_page>(
			&window, "chat_log_container", false, true);

	window.set_enter_disabled(true);

	window_ = &window;

	chat_input_ = find_widget<ttext_box>(&window, "chat_input", false, true);
	assert(chat_input_);
	connect_signal_pre_key_press(
			*chat_input_,
			boost::bind(&tlobby_main::chat_input_keypress_callback,
						this,
						_3,
						_4,
						_5,
						boost::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "send_message", false),
			boost::bind(&tlobby_main::send_message_button_callback,
						this,
						boost::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "create", false),
			boost::bind(&tlobby_main::create_button_callback,
						this,
						boost::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "refresh", false),
			boost::bind(&tlobby_main::refresh_button_callback,
						this,
						boost::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "show_preferences", false),
			boost::bind(&tlobby_main::show_preferences_button_callback,
						this,
						boost::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "join_global", false),
			boost::bind(&tlobby_main::join_global_button_callback,
						this,
						boost::ref(window)));
	find_widget<tbutton>(&window, "join_global", false).set_active(false);

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "observe_global", false),
			boost::bind(&tlobby_main::observe_global_button_callback,
						this,
						boost::ref(window)));
	find_widget<tbutton>(&window, "observe_global", false).set_active(false);

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "close_window", false),
			boost::bind(&tlobby_main::close_window_button_callback,
						this,
						boost::ref(window)));

	ttoggle_button& skip_replay
			= find_widget<ttoggle_button>(&window, "skip_replay", false);
	skip_replay.set_value(preferences::skip_mp_replay());
	skip_replay.set_callback_state_change(
			boost::bind(&tlobby_main::skip_replay_changed_callback, this, _1));

	filter_friends_ = find_widget<ttoggle_button>(
			&window, "filter_with_friends", false, true);
	filter_ignored_ = find_widget<ttoggle_button>(
			&window, "filter_without_ignored", false, true);
	filter_slots_ = find_widget<ttoggle_button>(
			&window, "filter_vacant_slots", false, true);
	filter_invert_ = find_widget<ttoggle_button>(
			&window, "filter_invert", false, true);
	filter_text_ = find_widget<ttext_box>(&window, "filter_text", false, true);

	filter_friends_->set_callback_state_change(
			boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	filter_ignored_->set_callback_state_change(
			boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	filter_slots_->set_callback_state_change(
			boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	filter_invert_->set_callback_state_change(
			boost::bind(&tlobby_main::game_filter_change_callback, this, _1));
	connect_signal_pre_key_press(
			*filter_text_,
			boost::bind(&tlobby_main::game_filter_keypress_callback, this, _5));

	room_window_open("lobby", true);
	active_window_changed();
	game_filter_reload();

	// Force first update to be directly.
	tlobby_main::network_handler();
	lobby_update_timer_
			= add_timer(game_config::lobby_network_timer,
						boost::bind(&tlobby_main::network_handler, this),
						true);
}

void tlobby_main::post_show(twindow& /*window*/)
{
	window_ = NULL;
	remove_timer(lobby_update_timer_);
	lobby_update_timer_ = 0;
}

room_info* tlobby_main::active_window_room()
{
	const tlobby_chat_window& t = open_windows_[active_window_];
	if(t.whisper)
		return NULL;
	return lobby_info_.get_room(t.name);
}

tlobby_chat_window* tlobby_main::room_window_open(const std::string& room,
												  bool open_new)
{
	return search_create_window(room, false, open_new);
}

tlobby_chat_window* tlobby_main::whisper_window_open(const std::string& name,
													 bool open_new)
{
	return search_create_window(name, true, open_new);
}

tlobby_chat_window* tlobby_main::search_create_window(const std::string& name,
													  bool whisper,
													  bool open_new)
{
	FOREACH(AUTO & t, open_windows_)
	{
		if(t.name == name && t.whisper == whisper)
			return &t;
	}
	if(open_new) {
		open_windows_.push_back(tlobby_chat_window(name, whisper));
		std::map<std::string, string_map> data;
		utils::string_map symbols;
		symbols["name"] = name;
		if(whisper) {
			add_label_data(data, "log_header", "<" + name + ">");
			add_label_data(data,
						   "log_text",
						   VGETTEXT("Whisper session with $name started. "
									"If you don’t want to receive messages "
									"from this user, "
									"type /ignore $name\n",
									symbols));
		} else {
			add_label_data(data, "log_header", name);
			add_label_data(
					data, "log_text", VGETTEXT("Room $name joined", symbols));
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
	if(tlobby_chat_window* t = whisper_window_open(name, false)) {
		t->pending_messages++;
		if(t->pending_messages == 1) {
			DBG_LB << "do whisper pending mark row " << (t - &open_windows_[0])
				   << " with " << t->name << "\n";
			tgrid* grid = roomlistbox_->get_row_grid(t - &open_windows_[0]);
			// this breaks for some reason
			// tlabel& label = grid->get_widget<tlabel>("room", false);
			// label.set_use_markup(true);
			// label.set_label(colorize("<" + t->name + ">", "red"));
			find_widget<timage>(grid, "pending_messages", false)
					.set_visible(twidget::tvisible::visible);
		}
	}
}

void tlobby_main::increment_waiting_messages(const std::string& room)
{
	if(tlobby_chat_window* t = room_window_open(room, false)) {
		t->pending_messages++;
		if(t->pending_messages == 1) {
			int idx = t - &open_windows_[0];
			DBG_LB << "do room pending mark row " << idx << " with " << t->name
				   << "\n";
			tgrid* grid = roomlistbox_->get_row_grid(idx);
			// this breaks for some reason
			// tlabel& label = grid->get_widget<tlabel>("room", false);
			// label.set_use_markup(true);
			// label.set_label(colorize(t->name, "red"));
			find_widget<timage>(grid, "pending_messages", false)
					.set_visible(twidget::tvisible::visible);
		}
	}
}

void tlobby_main::add_whisper_window_whisper(const std::string& sender,
											 const std::string& message)
{
	std::stringstream ss;
	ss << "<" << sender << ">" << message;
	tlobby_chat_window* t = whisper_window_open(sender, false);
	if(!t) {
		ERR_LB << "Whisper window not open in add_whisper_window_whisper for "
			   << sender << "\n";
		return;
	}
	append_to_chatbox(ss.str(), t - &open_windows_[0]);
}

void tlobby_main::add_active_window_whisper(const std::string& sender,
											const std::string& message)
{
	std::stringstream ss;
	ss << "<"
	   << "whisper: " << sender << ">" << message;
	append_to_chatbox(ss.str());
}

void tlobby_main::add_room_window_message(const std::string& room,
										  const std::string& sender,
										  const std::string& message)
{
	std::stringstream ss;
	ss << "<" << sender << ">" << message;
	tlobby_chat_window* t = room_window_open(room, false);
	if(!t) {
		ERR_LB << "Room window not open in add_room_window_message for " << room
			   << "\n";
		return;
	}
	append_to_chatbox(ss.str(), t - &open_windows_[0]);
}

void tlobby_main::add_active_window_message(const std::string& sender,
											const std::string& message)
{
	std::stringstream ss;
	ss << "<" << sender << ">" << message;
	append_to_chatbox(ss.str());
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
	tlabel& header = find_widget<tlabel>(
			&chat_log_container_->page_grid(active_window_),
			"log_header",
			false);

	tlobby_chat_window& t = open_windows_[active_window_];
	std::string expected_label;
	if(t.whisper) {
		expected_label = "<" + t.name + ">";
	} else {
		expected_label = t.name;
	}
	if(header.label() != expected_label) {
		ERR_LB << "Chat log header not what it should be! " << header.label()
			   << " vs " << expected_label << "\n";
	}

	bool close_button_active = (t.whisper || (t.name != "lobby"));

	DBG_LB << "active window changed to " << active_window_ << " "
		   << (t.whisper ? "w" : "r") << " " << t.name << " "
		   << t.pending_messages << " : " << expected_label
		   << " close button:" << close_button_active << "\n";

	// clear pending messages notification in room listbox
	tgrid* grid = roomlistbox_->get_row_grid(active_window_);
	// this breaks for some reason
	// tlabel& label = grid->get_widget<tlabel>("room", false);
	// label.set_label(expected_label);
	find_widget<timage>(grid, "pending_messages", false)
			.set_visible(twidget::tvisible::hidden);
	t.pending_messages = 0;

	find_widget<tbutton>(window_, "close_window", false)
			.set_active(close_button_active);
	player_list_dirty_ = true;
}


void tlobby_main::close_active_window()
{
	DBG_LB << "Close window button clicked\n";
	return close_window(active_window_);
}

void tlobby_main::close_window(size_t idx)
{
	const tlobby_chat_window& t = open_windows_[idx];
	bool active_changed = idx == active_window_;
	DBG_LB << "Close window " << idx << " - " << t.name << "\n";
	if(t.name == "lobby" && t.whisper == false)
		return;
	if(open_windows_.size() == 1)
		return;
	if(t.whisper == false) {
		// closing a room window -- send a part to the server
		config data, msg;
		msg["room"] = t.name;
		msg["player"] = preferences::login();
		data.add_child("room_part", msg);
		network::send_data(data, 0);
	}
	if(active_window_ == open_windows_.size() - 1) {
		active_window_--;
	}
	if(t.whisper) {
		lobby_info_.get_whisper_log(t.name).clear();
	} else {
		lobby_info_.close_room(t.name);
	}
	open_windows_.erase(open_windows_.begin() + idx);
	roomlistbox_->remove_row(idx);
	roomlistbox_->select_row(active_window_);
	chat_log_container_->remove_page(idx);
	chat_log_container_->select_page(active_window_);
	if(active_changed)
		active_window_changed();
}

void tlobby_main::network_handler()
{
	if(gamelist_dirty_ && !delay_gamelist_update_
	   && (SDL_GetTicks() - last_gamelist_update_
		   > game_config::lobby_refresh)) {
		if(gamelist_diff_update_) {
			update_gamelist_diff();
		} else {
			update_gamelist();
			gamelist_diff_update_ = true;
		}
	}
	if(player_list_dirty_) {
		update_gamelist_filter();
		update_playerlist();
	}
	try
	{
		config data;
		const network::connection sock = network::receive_data(data);
		if(sock) {
			process_network_data(data);
		}
	}
	catch(network::error& e)
	{
		LOG_LB << "caught network::error in network_handler: " << e.message
			   << "\n";
		throw;
	}
}

void tlobby_main::process_network_data(const config& data)
{
	if(const config& c = data.child("error")) {
		throw network::error(c["message"]);
	} else if(const config& c = data.child("message")) {
		process_message(c);
	} else if(const config& c = data.child("whisper")) {
		process_message(c, true);
	} else if(data.child("gamelist")) {
		process_gamelist(data);
	} else if(const config& c = data.child("gamelist_diff")) {
		process_gamelist_diff(c);
	} else if(const config& c = data.child("room_join")) {
		process_room_join(c);
	} else if(const config& c = data.child("room_part")) {
		process_room_part(c);
	} else if(const config& c = data.child("room_query_response")) {
		process_room_query_response(c);
	}
}

void tlobby_main::process_message(const config& data, bool whisper /*= false*/)
{
	std::string sender = data["sender"];
	DBG_LB << "process message from " << sender << " " << (whisper ? "(w)" : "")
		   << ", len " << data["message"].str().size() << '\n';
	if(preferences::is_ignored(sender))
		return;
	const std::string& message = data["message"];
	preferences::parse_admin_authentication(sender, message);
	if(whisper) {
		add_whisper_received(sender, message);
	} else {
		std::string room = data["room"];
		if(room.empty()) {
			LOG_LB << "Message without a room from " << sender
				   << ", assuming lobby\n";
			room = "lobby";
		}
		add_chat_room_message_received(room, sender, message);
	}
}

void tlobby_main::process_gamelist(const config& data)
{
	lobby_info_.process_gamelist(data);
	DBG_LB << "Received gamelist\n";
	gamelist_dirty_ = true;
	gamelist_diff_update_ = false;
}

void tlobby_main::process_gamelist_diff(const config& data)
{
	if(lobby_info_.process_gamelist_diff(data)) {
		DBG_LB << "Received gamelist diff\n";
		gamelist_dirty_ = true;
	} else {
		ERR_LB << "process_gamelist_diff failed!" << std::endl;
	}
	int joined = data.child_count("insert_child");
	int left = data.child_count("remove_child");
	if(joined > 0 || left > 0) {
		if(left > joined) {
			do_notify(NOTIFY_LOBBY_QUIT);
		} else {
			do_notify(NOTIFY_LOBBY_JOIN);
		}
	}
}

void tlobby_main::process_room_join(const config& data)
{
	const std::string& room = data["room"];
	const std::string& player = data["player"];
	room_info* r = lobby_info_.get_room(room);
	DBG_LB << "room join: " << room << " " << player << " "
		   << static_cast<void*>(r) << "\n";

	if(r) {
		if(player == preferences::login()) {
			if(const config& members = data.child("members")) {
				r->process_room_members(members);
			}
		} else {
			r->add_member(player);
			/* TODO: add/use preference */
			utils::string_map symbols;
			symbols["player"] = player;
			add_room_window_message(
					room,
					"server",
					VGETTEXT("$player has entered the room", symbols));
		}
		if(r == active_window_room()) {
			player_list_dirty_ = true;
		}
	} else {
		if(player == preferences::login()) {
			tlobby_chat_window* t = room_window_open(room, true);
			lobby_info_.open_room(room);
			r = lobby_info_.get_room(room);
			assert(r);
			if(const config& members = data.child("members")) {
				r->process_room_members(members);
			}
			switch_to_window(t);

			const std::string& topic = data["topic"];
			if(!topic.empty()) {
				add_chat_room_message_received(
						"room", "server", room + ": " + topic);
			}
		} else {
			LOG_LB << "Discarding join info for a room the player is not in\n";
		}
	}
}

void tlobby_main::process_room_part(const config& data)
{
	// todo close room window when the part message is sent
	const std::string& room = data["room"];
	const std::string& player = data["player"];
	DBG_LB << "Room part: " << room << " " << player << "\n";
	room_info* r = lobby_info_.get_room(room);
	if(r) {
		r->remove_member(player);
		/* TODO: add/use preference */
		utils::string_map symbols;
		symbols["player"] = player;
		add_room_window_message(
				room, "server", VGETTEXT("$player has left the room", symbols));
		if(active_window_room() == r) {
			player_list_dirty_ = true;
		}
	} else {
		LOG_LB << "Discarding part info for a room the player is not in\n";
	}
}

void tlobby_main::process_room_query_response(const config& data)
{
	const std::string& room = data["room"];
	const std::string& message = data["message"];
	DBG_LB << "room query response: " << room << " " << message << "\n";
	if(room.empty()) {
		if(!message.empty()) {
			add_active_window_message("server", message);
		}
		if(const config& rooms = data.child("rooms")) {
			// TODO: this should really open a nice join room dialog instead
			std::stringstream ss;
			ss << "Rooms:";
			FOREACH(const AUTO & r, rooms.child_range("room"))
			{
				ss << " " << r["name"];
			}
			add_active_window_message("server", ss.str());
		}
	} else {
		if(room_window_open(room, false)) {
			if(!message.empty()) {
				add_chat_room_message_received(room, "server", message);
			}
			if(const config& members = data.child("members")) {
				room_info* r = lobby_info_.get_room(room);
				assert(r);
				r->process_room_members(members);
				if(r == active_window_room()) {
					player_list_dirty_ = true;
				}
			}
		} else {
			if(!message.empty()) {
				add_active_window_message("server", room + ": " + message);
			}
		}
	}
}

void tlobby_main::join_button_callback(gui2::twindow& window)
{
	LOG_LB << "join_button_callback\n";
	join_global_button_callback(window);
}

void tlobby_main::observe_button_callback(gui2::twindow& window)
{
	LOG_LB << "observe_button_callback\n";
	observe_global_button_callback(window);
}

void tlobby_main::observe_global_button_callback(gui2::twindow& window)
{
	LOG_LB << "observe_global_button_callback\n";
	if(do_game_join(gamelistbox_->get_selected_row(), true)) {
		legacy_result_ = OBSERVE;
		window.close();
	}
}

void tlobby_main::join_global_button_callback(gui2::twindow& window)
{
	LOG_LB << "join_global_button_callback\n";
	if(do_game_join(gamelistbox_->get_selected_row(), false)) {
		legacy_result_ = JOIN;
		window.close();
	}
}

void tlobby_main::join_or_observe(int idx)
{
	const game_info& game = *lobby_info_.games()[idx];
	if(do_game_join(idx, !game.can_join())) {
		legacy_result_ = game.can_join() ? JOIN : OBSERVE;
		window_->close();
	}
}

bool tlobby_main::do_game_join(int idx, bool observe)
{
	if(idx < 0 || idx > static_cast<int>(lobby_info_.games().size())) {
		ERR_LB << "Requested join/observe of a game with index out of range: "
			   << idx << ", games size is " << lobby_info_.games().size()
			   << "\n";
		return false;
	}
	const game_info& game = *lobby_info_.games()[idx];
	if(observe) {
		if(!game.can_observe()) {
			ERR_LB << "Requested observe of a game with observers disabled"
				   << std::endl;
			return false;
		}
	} else {
		if(!game.can_join()) {
			ERR_LB << "Requested join to a game with no vacant slots"
				   << std::endl;
			return false;
		}
	}
	config response;
	config& join = response.add_child("join");
	join["id"] = lexical_cast<std::string>(game.id);
	join["observe"] = observe;
	if(join && !observe && game.password_required) {
		std::string password;
		// TODO replace with a gui2 dialog
		const int res
				= gui::show_dialog(disp_,
								   NULL,
								   _("Password Required"),
								   _("Joining this game requires a password."),
								   gui::OK_CANCEL,
								   NULL,
								   NULL,
								   _("Password: "),
								   &password);
		if(res != 0) {
			return false;
		}
		if(!password.empty()) {
			join["password"] = password;
		}
	}
	network::send_data(response, 0);
	if(observe && game.started) {
		playmp_controller::set_replay_last_turn(game.current_turn);
	}
	return true;
}

void tlobby_main::send_message_button_callback(gui2::twindow& /*window*/)
{
	const std::string& input = chat_input_->get_value();
	if(input.empty())
		return;
	if(input[0] == '/') {
		// TODO: refactor do_speak so it uses context information about
		//      opened window, so e.g. /ignore in a whisper session ignores
		//      the other party without having to specify it's nick.
		chat_handler::do_speak(input);
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
	if(t.whisper) {
		send_whisper(t.name, input);
		add_whisper_sent(t.name, input);
	} else {
		send_chat_room_message(t.name, input);
		add_chat_room_message_sent(t.name, input);
	}
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
	network::send_data(config("refresh_lobby"), 0);
}


void tlobby_main::show_preferences_button_callback(gui2::twindow& window)
{
	if(preferences_callback_) {
		preferences_callback_();

		/**
		 * The screen size might have changed force an update of the size.
		 *
		 * @todo This might no longer be needed when gui2 is done.
		 */
		window.invalidate_layout();

		network::send_data(config("refresh_lobby"), 0);
	}
}

void tlobby_main::room_switch_callback(twindow& /*window*/)
{
	switch_to_window(roomlistbox_->get_selected_row());
}

void tlobby_main::chat_input_keypress_callback(bool& handled,
											   bool& halt,
											   const SDLKey key,
											   twindow& window)
{
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		send_message_button_callback(window);
		handled = true;
		halt = true;
	} else if(key == SDLK_TAB) {
		std::string text = chat_input_->get_value();
		const std::vector<user_info>& match_infos = lobby_info_.users();
		std::vector<std::string> matches;

		FOREACH(const AUTO & ui, match_infos)
		{
			if(ui.name != preferences::login()) {
				matches.push_back(ui.name);
			}
		}
		const bool line_start = utils::word_completion(text, matches);

		if(matches.empty()) {
			return;
		}

		if(matches.size() == 1) {
			text.append(line_start ? ": " : " ");
		} else {
			std::string completion_list = utils::join(matches, " ");
			append_to_chatbox(completion_list);
		}
		chat_input_->set_value(text);
		handled = true;
		halt = true;
	}
}

void tlobby_main::game_filter_reload()
{
	lobby_info_.clear_game_filter();

	FOREACH(const AUTO & s, utils::split(filter_text_->get_value(), ' '))
	{
		lobby_info_.add_game_filter(new game_filter_general_string_part(s));
	}
	// TODO: make changing friend/ignore lists trigger a refresh
	if(filter_friends_->get_value()) {
		lobby_info_.add_game_filter(
				new game_filter_value<bool, &game_info::has_friends>(true));
	}
	if(filter_ignored_->get_value()) {
		lobby_info_.add_game_filter(
				new game_filter_value<bool, &game_info::has_ignored>(false));
	}
	if(filter_slots_->get_value()) {
		lobby_info_.add_game_filter(
				new game_filter_value<size_t,
									  &game_info::vacant_slots,
									  std::greater<size_t> >(0));
	}
	lobby_info_.set_game_filter_invert(filter_invert_->get_value());
}

void tlobby_main::game_filter_keypress_callback(const SDLKey key)
{
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		game_filter_reload();
		update_gamelist_filter();
	}
}

void tlobby_main::game_filter_change_callback(gui2::twidget& /*widget*/)
{
	game_filter_reload();
	update_gamelist_filter();
}

void tlobby_main::gamelist_change_callback(gui2::twindow& /*window*/)
{
	update_selected_game();
}

void tlobby_main::player_filter_callback(gui2::twidget& /*widget*/)
{
	player_list_.update_sort_icons();
	preferences::set_playerlist_sort_relation(
			player_list_.sort_by_relation->get_value());
	preferences::set_playerlist_sort_name(
			player_list_.sort_by_name->get_value());
	player_list_dirty_ = true;
	// window_->invalidate_layout();
}

void tlobby_main::user_dialog_callback(user_info* info)
{
	tlobby_player_info dlg(*this, *info, lobby_info_);
	lobby_delay_gamelist_update_guard g(*this);
	dlg.show(window_->video());
	delay_playerlist_update_ = true;
	if(dlg.result_open_whisper()) {
		tlobby_chat_window* t = whisper_window_open(info->name, true);
		switch_to_window(t);
		window_->invalidate_layout();
	}
	selected_game_id_ = info->game_id;
	// the commented out code below should be enough, but that'd delete the
	// playerlist and the widget calling this callback, so instead the game
	// will be selected on the next gamelist update.
	/*
	if (info->game_id != 0) {
		for (unsigned i = 0; i < lobby_info_.games_filtered().size(); ++i) {
		game_info& g = *lobby_info_.games_filtered()[i];
			if (info->game_id == g.id) {
			gamelistbox_->select_row(i);
				update_selected_game();
				break;
			}
		}
	}
	*/
	// do not update here as it can cause issues with removing the widget
	// from within it's event handler. Should get updated as soon as possible
	// update_gamelist();
	delay_playerlist_update_ = false;
	player_list_dirty_ = true;
	network::send_data(config("refresh_lobby"), 0);
}

void tlobby_main::skip_replay_changed_callback(twidget& w)
{
	ttoggle_button& tb = dynamic_cast<ttoggle_button&>(w);
	preferences::set_skip_mp_replay(tb.get_value());
}

} // namespace gui2
