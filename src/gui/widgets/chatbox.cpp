/*
   Copyright (C) 2016 The Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/multi_page.hpp"

#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "wesnothd_connection.hpp"
#include "config_assign.hpp"
#include "game_preferences.hpp"
#include "lobby_preferences.hpp"
#include "log.hpp"
#include "scripting/plugins/manager.hpp"

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)


namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(chatbox)

tchatbox::tchatbox()
	: tcontainer_(1)
	, roomlistbox_(nullptr)
	, chat_log_container_(nullptr)
	, chat_input_(nullptr)
	, active_window_(0)
	, active_window_changed_callback_()
	, lobby_info_(nullptr)
	, wesnothd_connection_(nullptr)
{
}

void tchatbox::active_window_changed()
{
	tlobby_chat_window& t = open_windows_[active_window_];

	// Clear pending messages notification in room listbox
	tgrid* grid = roomlistbox_->get_row_grid(active_window_);

	find_widget<timage>(grid, "pending_messages", false).set_visible(twidget::tvisible::hidden);

	t.pending_messages = 0;

	if(active_window_changed_callback_) {
		active_window_changed_callback_();
	}
}

void tchatbox::switch_to_window(tlobby_chat_window* t)
{
	switch_to_window(t - &open_windows_[0]);
}

void tchatbox::switch_to_window(size_t id)
{
	active_window_ = id;

	assert(active_window_ < open_windows_.size());
	chat_log_container_->select_page(active_window_);
	roomlistbox_->select_row(active_window_);

	active_window_changed();
}

void tchatbox::finalize_setup()
{
	roomlistbox_ = find_widget<tlistbox>(this, "room_list", false, true);

	roomlistbox_->set_callback_value_change([this](twidget&) {
		switch_to_window(roomlistbox_->get_selected_row());
	});

	chat_log_container_ = find_widget<tmulti_page>(this, "chat_log_container", false, true);

	chat_input_ = find_widget<ttext_box>(this, "chat_input", false, true);

	connect_signal_pre_key_press(*chat_input_,
		std::bind(&tchatbox::chat_input_keypress_callback, this, _3, _4, _5));
}

void tchatbox::send_message_button_callback()
{
	const std::string& input = chat_input_->get_value();
	if(input.empty()) {
		return;
	}

	if(input[0] == '/') {
		// TODO: refactor do_speak so it uses context information about
		//      opened window, so e.g. /ignore in a whisper session ignores
		//      the other party without having to specify it's nick.
		chat_handler::do_speak(input);
	} else {
		tlobby_chat_window& t = open_windows_[active_window_];

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
}


void tchatbox::chat_input_keypress_callback(bool& handled, bool& halt, const SDL_Keycode key)
{
	if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
		send_message_button_callback();
		handled = true;
		halt = true;
	} else if (key == SDLK_TAB) {
		std::string text = chat_input_->get_value();

		std::vector<std::string> matches;
		for(const auto& ui : lobby_info().users())	{
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

void tchatbox::append_to_chatbox(const std::string& text, const bool force_scroll)
{
	append_to_chatbox(text, active_window_, force_scroll);
}

void tchatbox::append_to_chatbox(const std::string& text, size_t id, const bool force_scroll)
{
	tgrid& grid = chat_log_container_->page_grid(id);

	tscroll_label& log = find_widget<tscroll_label>(&grid, "log_text", false);

	const std::string new_text = formatter()
		<< log.label() << "\n" << "<span color='#bcb088'>" << preferences::get_chat_timestamp(time(0)) << text << "</span>";

	log.set_use_markup(true);
	log.set_label(new_text);

	const bool chatbox_at_end = log.vertical_scrollbar_at_end();
	const unsigned chatbox_position = log.get_vertical_scrollbar_item_position();

	if(chatbox_at_end || force_scroll) {
		log.scroll_vertical_scrollbar(tscrollbar_::END);
	} else {
		log.set_vertical_scrollbar_item_position(chatbox_position);
	}
}

void tchatbox::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool tchatbox::get_active() const
{
	return true;
}

const std::string& tchatbox::get_control_type() const
{
	static const std::string type = "chatbox";
	return type;
}

void tchatbox::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

void tchatbox::send_chat_message(const std::string& message,
	bool /*allies_only*/)
{
	::config c = config_of("message", config_of("message", message)("sender", preferences::login()));
	add_chat_message(time(nullptr), preferences::login(), 0, message);

	if(wesnothd_connection_) {
		wesnothd_connection_->send_data(c);
	}
}

void tchatbox::user_relation_changed(const std::string& /*name*/)
{
	if(active_window_changed_callback_) {
		active_window_changed_callback_();
	}
}

void tchatbox::add_chat_message(const time_t& /*time*/,
	const std::string& speaker,
	int /*side*/,
	const std::string& message,
	events::chat_handler::MESSAGE_TYPE /*type*/)
{
	const std::string text = formatter() << "<b>" << speaker << ":</b> " << font::escape_text(message);
	append_to_chatbox(text);
}

void tchatbox::add_whisper_sent(const std::string& receiver, const std::string& message)
{
	if(whisper_window_active(receiver)) {
		add_active_window_message(preferences::login(), message, true);
	} else if(tlobby_chat_window* t = whisper_window_open(receiver, preferences::auto_open_whisper_windows())) {
		switch_to_window(t);
		add_active_window_message(preferences::login(), message, true);
	} else {
		add_active_window_whisper(vgettext("whisper to $receiver", {{"receiver", receiver}}), message, true);
	}

	lobby_info().get_whisper_log(receiver).add_message(preferences::login(), message);
}

void tchatbox::add_whisper_received(const std::string& sender, const std::string& message)
{
	bool can_go_to_active = !preferences::whisper_friends_only() || preferences::is_friend(sender);
	bool can_open_new = preferences::auto_open_whisper_windows() && can_go_to_active;

	lobby_info().get_whisper_log(sender).add_message(sender, message);

	if(whisper_window_open(sender, can_open_new)) {
		if(whisper_window_active(sender)) {
			add_active_window_message(sender, message);

			do_notify(NOTIFY_WHISPER, sender, message);
		} else {
			add_whisper_window_whisper(sender, message);
			increment_waiting_whsipers(sender);

			do_notify(NOTIFY_WHISPER_OTHER_WINDOW, sender, message);
		}
	} else if(can_go_to_active) {
		add_active_window_whisper(sender, message);
		do_notify(NOTIFY_WHISPER, sender, message);
	} else {
		LOG_LB << "Ignoring whisper from " << sender << "\n";
	}
}

void tchatbox::add_chat_room_message_sent(const std::string& room,
	const std::string& message)
{
	tlobby_chat_window* t = room_window_open(room, false);
	if(!t) {
		LOG_LB << "Cannot add sent message to ui for room " << room << ", player not in the room\n";
		return;
	}

	// Do not open room window here. The player should be in the room before sending messages
	room_info* ri = lobby_info().get_room(room);
	assert(ri);

	if(!room_window_active(room)) {
		switch_to_window(t);
	}

	ri->log().add_message(preferences::login(), message);
	add_active_window_message(preferences::login(), message, true);
}

void tchatbox::add_chat_room_message_received(const std::string& room,
	const std::string& speaker,
	const std::string& message)
{
	room_info* ri = lobby_info().get_room(room);
	if(!ri) {
		LOG_LB << "Discarding message to room " << room << " from " << speaker << " (room not open)\n";
		return;
	}

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
	} else if (utils::word_match(message, preferences::login())) {
		notify_mode = NOTIFY_OWN_NICK;
	} else if (preferences::is_friend(speaker)) {
		notify_mode = NOTIFY_FRIEND_MESSAGE;
	}

	do_notify(notify_mode, speaker, message);
}

bool tchatbox::whisper_window_active(const std::string& name)
{
	const tlobby_chat_window& t = open_windows_[active_window_];
	return t.name == name && t.whisper == true;
}

bool tchatbox::room_window_active(const std::string& room)
{
	const tlobby_chat_window& t = open_windows_[active_window_];
	return t.name == room && t.whisper == false;
}

tlobby_chat_window* tchatbox::room_window_open(const std::string& room, bool open_new)
{
	return search_create_window(room, false, open_new);
}

tlobby_chat_window* tchatbox::whisper_window_open(const std::string& name, bool open_new)
{
	return search_create_window(name, true, open_new);
}

tlobby_chat_window* tchatbox::search_create_window(const std::string& name,
	bool whisper,
	bool open_new)
{
	for(auto& t : open_windows_) {
		if(t.name == name && t.whisper == whisper) {
			return &t;
		}
	}

	if(!open_new) {
		return nullptr;
	}

	open_windows_.push_back(tlobby_chat_window(name, whisper));

	std::map<std::string, string_map> data;
	string_map item;

	item["use_markup"] = "true";

	if(whisper) {
		item["label"] = vgettext("Whisper session with $name started. "
			"If you do not want to receive messages from this user, type <i>/ignore $name</i>\n", {{"name", name}});
		data.emplace("log_text", item);
	} else {
		item["label"] = vgettext("<i>Room</i> $name <i>joined</i>", {{"name", name}});
		data.emplace("log_text", item);

		lobby_info().open_room(name);
	}

	chat_log_container_->add_page(data);

	data.clear();

	item["label"] = whisper ? font::escape_text("<" + name + ">") : name;
	data.emplace("room", item);

	tgrid& row_grid = roomlistbox_->add_row(data);

	tbutton& close_button = find_widget<tbutton>(&row_grid, "close_window", false);
	connect_signal_mouse_left_click(close_button,
		std::bind(&tchatbox::close_window_button_callback, this, open_windows_.back(), _3, _4));

	if(name == "lobby") {
		close_button.set_visible(tcontrol::tvisible::hidden);
	}

	return &open_windows_.back();
}

void tchatbox::close_window_button_callback(tlobby_chat_window& chat_window, bool& handled, bool& halt)
{
	const int index = std::find_if(open_windows_.begin(), open_windows_.end(), [&chat_window](const tlobby_chat_window& room) {
		return room.name == chat_window.name;
	}) - open_windows_.begin();

	close_window(index);

	handled = true;
	halt = true;
}

void tchatbox::send_to_server(const ::config& cfg)
{
	if(wesnothd_connection_) {
		wesnothd_connection_->send_data(cfg);
	}
}

void tchatbox::increment_waiting_whsipers(const std::string& name)
{
	if(tlobby_chat_window* t = whisper_window_open(name, false)) {
		t->pending_messages++;
		if(t->pending_messages == 1) {
			DBG_LB << "do whisper pending mark row " << (t - &open_windows_[0]) << " with " << t->name << "\n";
			tgrid* grid = roomlistbox_->get_row_grid(t - &open_windows_[0]);
			// this breaks for some reason
			// tlabel& label = grid->get_widget<tlabel>("room", false);
			// label.set_use_markup(true);
			// label.set_label(colorize("<" + t->name + ">", "red"));
			find_widget<timage>(grid, "pending_messages", false).set_visible(twidget::tvisible::visible);
		}
	}
}

void tchatbox::increment_waiting_messages(const std::string& room)
{
	if(tlobby_chat_window* t = room_window_open(room, false)) {
		t->pending_messages++;
		if(t->pending_messages == 1) {
			int idx = t - &open_windows_[0];
			DBG_LB << "do room pending mark row " << idx << " with " << t->name << "\n";
			tgrid* grid = roomlistbox_->get_row_grid(idx);
			// this breaks for some reason
			// tlabel& label = grid->get_widget<tlabel>("room", false);
			// label.set_use_markup(true);
			// label.set_label(colorize(t->name, "red"));
			find_widget<timage>(grid, "pending_messages", false).set_visible(twidget::tvisible::visible);
		}
	}
}

void tchatbox::add_whisper_window_whisper(const std::string& sender, const std::string& message)
{
	tlobby_chat_window* t = whisper_window_open(sender, false);
	if(!t) {
		ERR_LB << "Whisper window not open in add_whisper_window_whisper for " << sender << "\n";
		return;
	}

	const std::string text = formatter() << "<b>" << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, t - &open_windows_[0], false);
}

void tchatbox::add_active_window_whisper(const std::string& sender,
	const std::string& message,
	const bool force_scroll)
{
	const std::string text = formatter() << "<b>" << "whisper: " << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, force_scroll);
}

void tchatbox::close_window(size_t idx)
{
	const tlobby_chat_window& t = open_windows_[idx];
	bool active_changed = idx == active_window_;
	DBG_LB << "Close window " << idx << " - " << t.name << "\n";

	if((t.name == "lobby" && t.whisper == false) || open_windows_.size() == 1) {
		return;
	}

	if(t.whisper == false) {
		// closing a room window -- send a part to the server
		::config data, msg;
		msg["room"] = t.name;
		msg["player"] = preferences::login();
		data.add_child("room_part", msg);
		send_to_server(data);
	}

	if(active_window_ == open_windows_.size() - 1) {
		active_window_--;
	}

	if(t.whisper) {
		lobby_info().get_whisper_log(t.name).clear();
	} else {
		lobby_info().close_room(t.name);
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

void tchatbox::add_room_window_message(const std::string& room,
	const std::string& sender,
	const std::string& message)
{
	tlobby_chat_window* t = room_window_open(room, false);
	if(!t) {
		ERR_LB << "Room window not open in add_room_window_message for " << room << "\n";
		return;
	}

	const std::string text = formatter() << "<b>" << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, t - &open_windows_[0], false);
}

void tchatbox::add_active_window_message(const std::string& sender,
	const std::string& message,
	const bool force_scroll)
{
	const std::string text = formatter() << "<b>" << sender << ":</b> " << font::escape_text(message);
	append_to_chatbox(text, force_scroll);
}


room_info* tchatbox::active_window_room()
{
	const tlobby_chat_window& t = open_windows_[active_window_];
	if(t.whisper) {
		return nullptr;
	}

	return lobby_info().get_room(t.name);
}

void tchatbox::process_room_join(const ::config& data)
{
	const std::string& room = data["room"];
	const std::string& player = data["player"];

	DBG_LB << "room join: " << room << " " << player << "\n";

	room_info* r = lobby_info().get_room(room);
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
			tlobby_chat_window* t = room_window_open(room, true);
			lobby_info().open_room(room);
			r = lobby_info().get_room(room);
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

void tchatbox::process_room_part(const ::config& data)
{
	// TODO: close room window when the part message is sent
	const std::string& room = data["room"];
	const std::string& player = data["player"];

	DBG_LB << "Room part: " << room << " " << player << "\n";

	if(room_info* r = lobby_info().get_room(room)) {
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

void tchatbox::process_room_query_response(const ::config& data)
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
				room_info* r = lobby_info().get_room(room);
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

void tchatbox::process_message(const ::config& data, bool whisper /*= false*/)
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

bool tchatbox::process_network_data(const ::config& data)
{
	if(const ::config& c = data.child("message")) {
		process_message(c);
	} else if(const ::config& c = data.child("whisper")) {
		process_message(c, true);
	} else if(const ::config& c = data.child("room_join")) {
		process_room_join(c);
	} else if(const ::config& c = data.child("room_part")) {
		process_room_part(c);
	} else if(const ::config& c = data.child("room_query_response")) {
		process_room_query_response(c);
	}

	return false;
}

// }---------- DEFINITION ---------{

tchatbox_definition::tchatbox_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	load_resolutions<tresolution>(cfg);
}

tchatbox_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg), grid()
{
	state.push_back(tstate_definition(cfg.child("background")));
	state.push_back(tstate_definition(cfg.child("foreground")));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<tbuilder_grid>(child);
}
// }---------- BUILDER -----------{

namespace implementation
{

tbuilder_chatbox::tbuilder_chatbox(const config& cfg)
	: tbuilder_control(cfg)
{
}

twidget* tbuilder_chatbox::build() const
{
	tchatbox* widget = new tchatbox();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed unit preview pane '" << id
			  << "' with definition '" << definition << "'.\n";

	std::shared_ptr<const tchatbox_definition::tresolution> conf
		= std::static_pointer_cast<
			const tchatbox_definition::tresolution>(widget->config());

	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
