/*
   Copyright (C) 2016 - 2018 The Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/chatbox.hpp"

#include "gui/auxiliary/find_widget.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "font/pango/escape.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences/credentials.hpp"
#include "preferences/game.hpp"
#include "preferences/lobby.hpp"
#include "scripting/plugins/manager.hpp"
#include "wesnothd_connection.hpp"

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(chatbox)

chatbox::chatbox(const implementation::builder_chatbox& builder)
	: container_base(builder, get_control_type())
	, roomlistbox_(nullptr)
	, chat_log_container_(nullptr)
	, chat_input_(nullptr)
	, active_window_(0)
	, active_window_changed_callback_()
	, lobby_info_(nullptr)
	, wesnothd_connection_(nullptr)
{
	// We only implement a RECEIVE_KEYBOARD_FOCUS handler; LOSE_KEYBOARD_FOCUS
	// isn't needed. This handler forwards focus to the input textbox, meaning
	// if keyboard_focus is called on a chatbox, distributor::keyboard_focus_
	// will immediately be set to the input textbox, which will then handle focus
	// loss itself when applicable. Nothing else happens in the interim while
	// keyboard_focus_ equals `this` to warrent cleanup.
	connect_signal<event::RECEIVE_KEYBOARD_FOCUS>(
		std::bind(&chatbox::signal_handler_receive_keyboard_focus, this, _2));
}

void chatbox::finalize_setup()
{
	roomlistbox_ = find_widget<listbox>(this, "room_list", false, true);

	// We need to bind a lambda here since switch_to_window is overloaded.
	// A lambda alone would be more verbose because it'd need to specify all the parameters.
	connect_signal_notify_modified(*roomlistbox_,
		std::bind([this]() { switch_to_window(roomlistbox_->get_selected_row()); }));

	chat_log_container_ = find_widget<multi_page>(this, "chat_log_container", false, true);

	chat_input_ = find_widget<text_box>(this, "chat_input", false, true);

	connect_signal_pre_key_press(*chat_input_,
		std::bind(&chatbox::chat_input_keypress_callback, this, _5));
}

void chatbox::active_window_changed()
{
	lobby_chat_window& t = open_windows_[active_window_];

	// Clear pending messages notification in room listbox
	grid* grid = roomlistbox_->get_row_grid(active_window_);
	find_widget<image>(grid, "pending_messages", false).set_visible(widget::visibility::hidden);

	t.pending_messages = 0;

	if(active_window_changed_callback_) {
		active_window_changed_callback_();
	}
}

void chatbox::switch_to_window(lobby_chat_window* t)
{
	switch_to_window(t - &open_windows_[0]);
}

void chatbox::switch_to_window(size_t id)
{
	active_window_ = id;
	assert(active_window_ < open_windows_.size());

	chat_log_container_->select_page(active_window_);
	roomlistbox_->select_row(active_window_);

	active_window_changed();
}

void chatbox::chat_input_keypress_callback(const SDL_Keycode key)
{
	std::string input = chat_input_->get_value();
	if(input.empty()) {
		return;
	}

	switch(key) {
	case SDLK_RETURN:
	case SDLK_KP_ENTER: {
		if(input[0] == '/') {
			// TODO: refactor do_speak so it uses context information about
			//       opened window, so e.g. /ignore in a whisper session ignores
			//       the other party without having to specify it's nick.
			chat_handler::do_speak(input);
		} else {
			lobby_chat_window& t = open_windows_[active_window_];

			if(t.whisper) {
				send_whisper(t.name, input);
				add_whisper_sent(t.name, input);
			} else {
				send_chat_room_message(t.name, input);
				add_chat_room_message_sent(t.name, input);
			}
		}

		chat_input_->save_to_history();
		chat_input_->set_value("");

		break;
	}

	case SDLK_TAB: {
		// TODO: very inefficient! Very! D:
		std::vector<std::string> matches;
		for(const auto& ui : lobby_info_->users()) {
			if(ui.name != preferences::login()) {
				matches.push_back(ui.name);
			}
		}

		const bool line_start = utils::word_completion(input, matches);

		if(matches.empty()) {
			return;
		}

		if(matches.size() == 1) {
			input.append(line_start ? ": " : " ");
		} else {
			std::string completion_list = utils::join(matches, " ");
			append_to_chatbox(completion_list);
		}

		chat_input_->set_value(input);

		break;
	}

	default:
		break;
	}
}

void chatbox::append_to_chatbox(const std::string& text, const bool force_scroll)
{
	append_to_chatbox(text, active_window_, force_scroll);
}

void chatbox::append_to_chatbox(const std::string& text, size_t id, const bool force_scroll)
{
	grid& grid = chat_log_container_->page_grid(id);

	scroll_label& log = find_widget<scroll_label>(&grid, "log_text", false);
	const bool chatbox_at_end = log.vertical_scrollbar_at_end();

	const std::string new_text = formatter()
		<< log.get_label() << "\n" << "<span color='#bcb088'>" << preferences::get_chat_timestamp(time(0)) << text << "</span>";

	log.set_use_markup(true);
	log.set_label(new_text);

	const unsigned chatbox_position = log.get_vertical_scrollbar_item_position();

	if(chatbox_at_end || force_scroll) {
		log.scroll_vertical_scrollbar(scrollbar_base::END);
	} else {
		log.set_vertical_scrollbar_item_position(chatbox_position);
	}
}

void chatbox::send_chat_message(const std::string& message, bool /*allies_only*/)
{
	add_chat_message(time(nullptr), preferences::login(), 0, message);

	::config c {"message", ::config {"message", message, "sender", preferences::login()}};
	send_to_server(c);
}

void chatbox::user_relation_changed(const std::string& /*name*/)
{
	if(active_window_changed_callback_) {
		active_window_changed_callback_();
	}
}

void chatbox::add_chat_message(const time_t& /*time*/,
	const std::string& speaker,
	int /*side*/,
	const std::string& message,
	events::chat_handler::MESSAGE_TYPE /*type*/)
{
	std::string text;

	// FIXME: the chat_command_handler class (which handles chat commands) dispatches a
	// message consisting of '/me insert text here' in the case the '/me' or '/emote'
	// commands are used, so we need to do some manual preprocessing here.
	if(message.compare(0, 4, "/me ") == 0) {
		text = formatter() << "<i>" << speaker << " " << font::escape_text(message.substr(4)) << "</i>";
	} else {
		text = formatter() << "<b>" << speaker << ":</b> " << font::escape_text(message);
	}

	append_to_chatbox(text);
}

void chatbox::add_whisper_sent(const std::string& receiver, const std::string& message)
{
	if(whisper_window_active(receiver)) {
		add_active_window_message(preferences::login(), message, true);
	} else if(lobby_chat_window* t = whisper_window_open(receiver, preferences::auto_open_whisper_windows())) {
		switch_to_window(t);
		add_active_window_message(preferences::login(), message, true);
	} else {
		add_active_window_whisper(vgettext("whisper to $receiver", {{"receiver", receiver}}), message, true);
	}

	lobby_info_->get_whisper_log(receiver).add_message(preferences::login(), message);
}

void chatbox::add_whisper_received(const std::string& sender, const std::string& message)
{
	bool can_go_to_active = !preferences::whisper_friends_only() || preferences::is_friend(sender);
	bool can_open_new = preferences::auto_open_whisper_windows() && can_go_to_active;

	lobby_info_->get_whisper_log(sender).add_message(sender, message);

	if(whisper_window_open(sender, can_open_new)) {
		if(whisper_window_active(sender)) {
			add_active_window_message(sender, message);

			do_notify(mp::NOTIFY_WHISPER, sender, message);
		} else {
			add_whisper_window_whisper(sender, message);
			increment_waiting_whispers(sender);

			do_notify(mp::NOTIFY_WHISPER_OTHER_WINDOW, sender, message);
		}
	} else if(can_go_to_active) {
		add_active_window_whisper(sender, message);
		do_notify(mp::NOTIFY_WHISPER, sender, message);
	} else {
		LOG_LB << "Ignoring whisper from " << sender << "\n";
	}
}

void chatbox::add_chat_room_message_sent(const std::string& room, const std::string& message)
{
	lobby_chat_window* t = room_window_open(room, false);
	if(!t) {
		LOG_LB << "Cannot add sent message to ui for room " << room << ", player not in the room\n";
		return;
	}

	// Do not open room window here. The player should be in the room before sending messages
	mp::room_info* ri = lobby_info_->get_room(room);
	assert(ri);

	if(!room_window_active(room)) {
		switch_to_window(t);
	}

	ri->log().add_message(preferences::login(), message);
	add_active_window_message(preferences::login(), message, true);
}

void chatbox::add_chat_room_message_received(const std::string& room,
	const std::string& speaker,
	const std::string& message)
{
	mp::room_info* ri = lobby_info_->get_room(room);
	if(!ri) {
		LOG_LB << "Discarding message to room " << room << " from " << speaker << " (room not open)\n";
		return;
	}

	mp::notify_mode notify_mode = mp::NOTIFY_NONE;
	ri->log().add_message(speaker, message);

	if(room_window_active(room)) {
		add_active_window_message(speaker, message);
		notify_mode = mp::NOTIFY_MESSAGE;
	} else {
		add_room_window_message(room, speaker, message);
		increment_waiting_messages(room);
		notify_mode = mp::NOTIFY_MESSAGE_OTHER_WINDOW;
	}

	if(speaker == "server") {
		notify_mode = mp::NOTIFY_SERVER_MESSAGE;
	} else if (utils::word_match(message, preferences::login())) {
		notify_mode = mp::NOTIFY_OWN_NICK;
	} else if (preferences::is_friend(speaker)) {
		notify_mode = mp::NOTIFY_FRIEND_MESSAGE;
	}

	do_notify(notify_mode, speaker, message);
}

bool chatbox::whisper_window_active(const std::string& name)
{
	const lobby_chat_window& t = open_windows_[active_window_];
	return t.name == name && t.whisper == true;
}

bool chatbox::room_window_active(const std::string& room)
{
	const lobby_chat_window& t = open_windows_[active_window_];
	return t.name == room && t.whisper == false;
}

lobby_chat_window* chatbox::room_window_open(const std::string& room, const bool open_new, const bool allow_close)
{
	return find_or_create_window(room, false, open_new, allow_close);
}

lobby_chat_window* chatbox::whisper_window_open(const std::string& name, bool open_new)
{
	return find_or_create_window(name, true, open_new, true);
}

lobby_chat_window* chatbox::find_or_create_window(const std::string& name,
	const bool whisper,
	const bool open_new,
	const bool allow_close)
{
	for(auto& t : open_windows_) {
		if(t.name == name && t.whisper == whisper) {
			return &t;
		}
	}

	if(!open_new) {
		return nullptr;
	}

	open_windows_.emplace_back(name, whisper);

	//
	// Add a new chat log page.
	//
	std::map<std::string, string_map> data;
	string_map item;

	item["use_markup"] = "true";

	if(whisper) {
		item["label"] = vgettext("Whisper session with <i>“$name”</i> started. "
			"If you do not want to receive messages from this user, type <i>/ignore $name</i>\n", {{"name", name}});
		data.emplace("log_text", item);
	} else {
		item["label"] = vgettext("Room <i>“$name”</i> joined", {{"name", name}});
		data.emplace("log_text", item);

		lobby_info_->open_room(name);
	}

	chat_log_container_->add_page(data);

	//
	// Add a new room window tab.
	//
	data.clear();

	item["label"] = whisper ? font::escape_text("<" + name + ">") : name;
	data.emplace("room", item);

	grid& row_grid = roomlistbox_->add_row(data);

	//
	// Set up the Close Window button.
	//
	button& close_button = find_widget<button>(&row_grid, "close_window", false);

	if(!allow_close) {
		close_button.set_visible(widget::visibility::hidden);
	} else {
		connect_signal_mouse_left_click(close_button,
			std::bind(&chatbox::close_window_button_callback, this, open_windows_.back().name, _3, _4));
	}

	return &open_windows_.back();
}

void chatbox::close_window_button_callback(std::string room_name, bool& handled, bool& halt)
{
	const int index = std::find_if(open_windows_.begin(), open_windows_.end(),
		[&room_name](const lobby_chat_window& room) { return room.name == room_name; }
	) - open_windows_.begin();

	close_window(index);

	handled = halt = true;
}

void chatbox::send_to_server(const ::config& cfg)
{
	if(wesnothd_connection_) {
		wesnothd_connection_->send_data(cfg);
	}
}

void chatbox::increment_waiting_whispers(const std::string& name)
{
	if(lobby_chat_window* t = whisper_window_open(name, false)) {
		++t->pending_messages;

		if(t->pending_messages == 1) {
			DBG_LB << "do whisper pending mark row " << (t - &open_windows_[0]) << " with " << t->name << "\n";

			grid* grid = roomlistbox_->get_row_grid(t - &open_windows_[0]);
			find_widget<image>(grid, "pending_messages", false).set_visible(widget::visibility::visible);
		}
	}
}

void chatbox::increment_waiting_messages(const std::string& room)
{
	if(lobby_chat_window* t = room_window_open(room, false)) {
		++t->pending_messages;

		if(t->pending_messages == 1) {
			int idx = t - &open_windows_[0];

			DBG_LB << "do room pending mark row " << idx << " with " << t->name << "\n";

			grid* grid = roomlistbox_->get_row_grid(idx);
			find_widget<image>(grid, "pending_messages", false).set_visible(widget::visibility::visible);
		}
	}
}

void chatbox::add_whisper_window_whisper(const std::string& sender, const std::string& message)
{
	lobby_chat_window* t = whisper_window_open(sender, false);
	if(!t) {
		ERR_LB << "Whisper window not open in add_whisper_window_whisper for " << sender << "\n";
		return;
	}

	const std::string text = formatter() << "<b>" << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, t - &open_windows_[0], false);
}

void chatbox::add_active_window_whisper(const std::string& sender,
	const std::string& message,
	const bool force_scroll)
{
	const std::string text = formatter() << "<b>" << "whisper: " << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, force_scroll);
}

void chatbox::close_window(size_t idx)
{
	const lobby_chat_window& t = open_windows_[idx];

	DBG_LB << "Close window " << idx << " - " << t.name << "\n";

	// Can't close the lobby!
	if((t.name == "lobby" && t.whisper == false) || open_windows_.size() == 1) {
		return;
	}

	if(t.whisper == false) {
		// closing a room window -- send a part to the server
		::config data, msg;
		msg["room"] = t.name;
		msg["player"] = preferences::login();
		data.add_child("room_part", std::move(msg));

		send_to_server(data);
	}

	// Check if we're closing the currently-active window.
	const bool active_changed = idx == active_window_;

	if(active_window_ == open_windows_.size() - 1) {
		--active_window_;
	}

	if(t.whisper) {
		lobby_info_->get_whisper_log(t.name).clear();
	} else {
		lobby_info_->close_room(t.name);
	}

	open_windows_.erase(open_windows_.begin() + idx);

	roomlistbox_->remove_row(idx);
	roomlistbox_->select_row(active_window_);

	chat_log_container_->remove_page(idx);
	chat_log_container_->select_page(active_window_);

	if(active_changed) {
		active_window_changed();
	}
}

void chatbox::add_room_window_message(const std::string& room,
	const std::string& sender,
	const std::string& message)
{
	lobby_chat_window* t = room_window_open(room, false);
	if(!t) {
		ERR_LB << "Room window not open in add_room_window_message for " << room << "\n";
		return;
	}

	const std::string text = formatter() << "<b>" << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, t - &open_windows_[0], false);
}

void chatbox::add_active_window_message(const std::string& sender,
	const std::string& message,
	const bool force_scroll)
{
	const std::string text = formatter() << "<b>" << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, force_scroll);
}

mp::room_info* chatbox::active_window_room()
{
	const lobby_chat_window& t = open_windows_[active_window_];
	if(t.whisper) {
		return nullptr;
	}

	return lobby_info_->get_room(t.name);
}

void chatbox::process_room_join(const ::config& data)
{
	const std::string& room = data["room"];
	const std::string& player = data["player"];

	DBG_LB << "room join: " << room << " " << player << "\n";

	mp::room_info* r = lobby_info_->get_room(room);
	if(r) {
		if(player == preferences::login()) {
			if(const auto& members = data.child("members")) {
				r->process_room_members(members);
			}
		} else {
			r->add_member(player);

			/* TODO: add/use preference */
			add_room_window_message(room, "server", vgettext("$player has entered the room", {{"player", player}}));
		}

		if(r == active_window_room()) {
			active_window_changed_callback_();
		}
	} else {
		if(player == preferences::login()) {
			lobby_chat_window* t = room_window_open(room, true);

			lobby_info_->open_room(room);
			r = lobby_info_->get_room(room);
			assert(r);

			if(const auto& members = data.child("members")) {
				r->process_room_members(members);
			}

			switch_to_window(t);

			const std::string& topic = data["topic"];
			if(!topic.empty()) {
				add_chat_room_message_received("room", "server", room + ": " + topic);
			}
		} else {
			LOG_LB << "Discarding join info for a room the player is not in\n";
		}
	}
}

void chatbox::process_room_part(const ::config& data)
{
	// TODO: close room window when the part message is sent
	const std::string& room = data["room"];
	const std::string& player = data["player"];

	DBG_LB << "Room part: " << room << " " << player << "\n";

	if(mp::room_info* r = lobby_info_->get_room(room)) {
		r->remove_member(player);

		/* TODO: add/use preference */
		add_room_window_message(room, "server", vgettext("$player has left the room", {{"player", player}}));
		if(active_window_room() == r) {
			active_window_changed_callback_();
		}
	} else {
		LOG_LB << "Discarding part info for a room the player is not in\n";
	}
}

void chatbox::process_room_query_response(const ::config& data)
{
	const std::string& room = data["room"];
	const std::string& message = data["message"];

	DBG_LB << "room query response: " << room << " " << message << "\n";

	if(room.empty()) {
		if(!message.empty()) {
			add_active_window_message("server", message);
		}

		if(const ::config& rooms = data.child("rooms")) {
			// TODO: this should really open a nice join room dialog instead
			std::stringstream ss;
			ss << "Rooms:";

			for(const auto & r : rooms.child_range("room")) {
				ss << " " << r["name"];
			}

			add_active_window_message("server", ss.str());
		}
	} else {
		if(room_window_open(room, false)) {
			if(!message.empty()) {
				add_chat_room_message_received(room, "server", message);
			}

			if(const ::config& members = data.child("members")) {
				mp::room_info* r = lobby_info_->get_room(room);
				assert(r);
				r->process_room_members(members);
				if(r == active_window_room()) {
					active_window_changed_callback_();
				}
			}
		} else {
			if(!message.empty()) {
				add_active_window_message("server", room + ": " + message);
			}
		}
	}
}

void chatbox::process_message(const ::config& data, bool whisper /*= false*/)
{
	std::string sender = data["sender"];
	DBG_LB << "process message from " << sender << " " << (whisper ? "(w)" : "")
		<< ", len " << data["message"].str().size() << '\n';

	if(preferences::is_ignored(sender)) {
		return;
	}

	const std::string& message = data["message"];
	preferences::parse_admin_authentication(sender, message);

	if(whisper) {
		add_whisper_received(sender, message);
	} else {
		std::string room = data["room"];
		if(room.empty()) {
			LOG_LB << "Message without a room from " << sender << ", assuming lobby\n";
			room = "lobby";
		}

		add_chat_room_message_received(room, sender, message);
	}

	// Notify plugins about the message
	::config plugin_data = data;
	plugin_data["whisper"] = whisper;
	plugins_manager::get()->notify_event("chat", plugin_data);
}

void chatbox::process_network_data(const ::config& data)
{
	if(const ::config& message = data.child("message")) {
		process_message(message);
	} else if(const ::config& whisper = data.child("whisper")) {
		process_message(whisper, true);
	} else if(const ::config& room_join = data.child("room_join")) {
		process_room_join(room_join);
	} else if(const ::config& room_part = data.child("room_part")) {
		process_room_part(room_part);
	} else if(const ::config& room_query_response = data.child("room_query_response")) {
		process_room_query_response(room_query_response);
	}
}

void chatbox::signal_handler_receive_keyboard_focus(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	// Forward focus to the input textbox.
	get_window()->keyboard_capture(chat_input_);
}

// }---------- DEFINITION ---------{

chatbox_definition::chatbox_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	load_resolutions<resolution>(cfg);
}

chatbox_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid()
{
	state.emplace_back(cfg.child("background"));
	state.emplace_back(cfg.child("foreground"));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}
// }---------- BUILDER -----------{

namespace implementation
{

builder_chatbox::builder_chatbox(const config& cfg)
	: builder_styled_widget(cfg)
{
}

widget* builder_chatbox::build() const
{
	chatbox* widget = new chatbox(*this);

	DBG_GUI_G << "Window builder: placed unit preview pane '" << id
			  << "' with definition '" << definition << "'.\n";

	const auto conf = widget->cast_config_to<chatbox_definition>();
	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
