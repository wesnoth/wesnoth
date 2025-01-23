/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "gui/core/register_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_text.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

#include "font/pango/escape.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_initialization/multiplayer.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "scripting/plugins/manager.hpp"
#include "serialization/markup.hpp"
#include "wml_exception.hpp"

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
	: container_base(builder, type())
	, roomlistbox_(nullptr)
	, chat_log_container_(nullptr)
	, chat_input_(nullptr)
	, active_window_(0)
	, active_window_changed_callback_()
	, log_(nullptr)
{
	// We only implement a RECEIVE_KEYBOARD_FOCUS handler; LOSE_KEYBOARD_FOCUS
	// isn't needed. This handler forwards focus to the input textbox, meaning
	// if keyboard_focus is called on a chatbox, distributor::keyboard_focus_
	// will immediately be set to the input textbox, which will then handle focus
	// loss itself when applicable. Nothing else happens in the interim while
	// keyboard_focus_ equals `this` to warrent cleanup.
	connect_signal<event::RECEIVE_KEYBOARD_FOCUS>(
		std::bind(&chatbox::signal_handler_receive_keyboard_focus, this, std::placeholders::_2));
}

void chatbox::finalize_setup()
{
	roomlistbox_ = find_widget<listbox>("room_list", false, true);

	// We need to bind a lambda here since switch_to_window is overloaded.
	// A lambda alone would be more verbose because it'd need to specify all the parameters.
	connect_signal_notify_modified(*roomlistbox_,
		[this](auto&&...) { switch_to_window(roomlistbox_->get_selected_row()); });

	chat_log_container_ = find_widget<multi_page>("chat_log_container", false, true);

	chat_input_ = find_widget<text_box>("chat_input", false, true);

	connect_signal_pre_key_press(*chat_input_,
		std::bind(&chatbox::chat_input_keypress_callback, this, std::placeholders::_5));
}

void chatbox::load_log(std::map<std::string, chatroom_log>& log, bool show_lobby)
{
	const std::string new_tip = formatter()
		<< "\n"
		// TRANSLATORS: This is the new chat text indicator
		<< markup::span_color("#FF0000", "============", _("NEW"), "============");

	for(auto& l : log) {
		const bool is_lobby = l.first == "lobby";

		if(!show_lobby && is_lobby && !l.second.whisper) {
			continue;
		}

		const std::size_t new_tip_index = l.second.log.find(new_tip);

		if(new_tip_index != std::string::npos) {
			l.second.log.replace(new_tip_index, new_tip.length(), "");
		}
		find_or_create_window(l.first, l.second.whisper, true, !is_lobby, l.second.log + new_tip);
	}

	log_ = &log;
}

void chatbox::active_window_changed()
{
	lobby_chat_window& t = open_windows_[active_window_];

	// Clear pending messages notification in room listbox
	grid* grid = roomlistbox_->get_row_grid(active_window_);
	grid->find_widget<image>("pending_messages").set_visible(widget::visibility::hidden);

	t.pending_messages = 0;

	if(active_window_changed_callback_) {
		active_window_changed_callback_();
	}
}

void chatbox::switch_to_window(lobby_chat_window* t)
{
	switch_to_window(t - &open_windows_[0]);
}

void chatbox::switch_to_window(std::size_t id)
{
	active_window_ = id;
	assert(active_window_ < open_windows_.size());

	chat_log_container_->select_page(active_window_);
	roomlistbox_->select_row(active_window_);

	// Grab input focus
	get_window()->keyboard_capture(chat_input_);

	active_window_changed();
}

void chatbox::chat_input_keypress_callback(const SDL_Keycode key)
{
	std::string input = chat_input_->get_value();
	if(input.empty() || chat_input_->is_composing()) {
		return;
	}

	lobby_chat_window& t = open_windows_[active_window_];

	switch(key) {
	case SDLK_RETURN:
	case SDLK_KP_ENTER: {
		if(input[0] == '/') {
			// TODO: refactor do_speak so it uses context information about
			//       opened window, so e.g. /ignore in a whisper session ignores
			//       the other party without having to specify it's nick.
			chat_handler::do_speak(input);
		} else {
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
		auto* li = mp::get_lobby_info();
		if(!li) {
			break;
		}

		std::vector<std::string> matches;
		for(const auto& ui : li->users()) {
			if(ui.name != prefs::get().login()) {
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

void chatbox::append_to_chatbox(const std::string& text, std::size_t id, const bool force_scroll)
{
	grid& grid = chat_log_container_->page_grid(id);
	scroll_text& log = grid.find_widget<scroll_text>("log_text");

	const bool chatbox_at_end = log.vertical_scrollbar_at_end();
	const unsigned chatbox_position = log.get_vertical_scrollbar_item_position();

	const std::string before_message = log.get_value().empty() ? "" : "\n";
	const std::string new_text = formatter()
		<< log.get_value() << before_message << markup::span_color("#bcb088", prefs::get().get_chat_timestamp(std::chrono::system_clock::now()), text);

	log.set_use_markup(true);
	log.set_value(new_text);

	if(log_ != nullptr) {
		try {
			const std::string& room_name = open_windows_[id].name;
			log_->at(room_name).log = new_text;
		} catch(const std::out_of_range&) {
		}
	}

	if(chatbox_at_end || force_scroll) {
		log.scroll_vertical_scrollbar(scrollbar_base::END);
	} else {
		log.set_vertical_scrollbar_item_position(chatbox_position);
	}
}

void chatbox::send_chat_message(const std::string& message, bool /*allies_only*/)
{
	add_chat_message(std::time(nullptr), prefs::get().login(), 0, message);

	::config c {"message", ::config {"message", message, "sender", prefs::get().login()}};
	send_to_server(c);
}

void chatbox::clear_messages()
{
	const auto id = active_window_;
	grid& grid = chat_log_container_->page_grid(id);
	scroll_text& log = grid.find_widget<scroll_text>("log_text");
	log.set_label("");
}

void chatbox::user_relation_changed(const std::string& /*name*/)
{
	if(active_window_changed_callback_) {
		active_window_changed_callback_();
	}
}

void chatbox::add_chat_message(const std::time_t& /*time*/,
	const std::string& speaker,
	int /*side*/,
	const std::string& message,
	events::chat_handler::MESSAGE_TYPE /*type*/)
{
	std::string text;
	if(message.compare(0, 4, "/me ") == 0) {
		text = formatter() << markup::italic(speaker, " ", font::escape_text(message.substr(4)));
	} else {
		text = formatter() << markup::bold(speaker, ":") << font::escape_text(message);
	}

	append_to_chatbox(text);
}

void chatbox::add_whisper_sent(const std::string& receiver, const std::string& message)
{
	if(whisper_window_active(receiver)) {
		add_active_window_message(prefs::get().login(), message, true);
	} else if(lobby_chat_window* t = whisper_window_open(receiver, prefs::get().auto_open_whisper_windows())) {
		switch_to_window(t);
		add_active_window_message(prefs::get().login(), message, true);
	} else {
		add_active_window_whisper(VGETTEXT("whisper to $receiver", {{"receiver", receiver}}), message, true);
	}
}

void chatbox::add_whisper_received(const std::string& sender, const std::string& message)
{
	bool can_go_to_active = !prefs::get().lobby_whisper_friends_only() || prefs::get().is_friend(sender);
	bool can_open_new = prefs::get().auto_open_whisper_windows() && can_go_to_active;

	if(whisper_window_open(sender, can_open_new)) {
		if(whisper_window_active(sender)) {
			add_active_window_message(sender, message);

			do_notify(mp::notify_mode::whisper, sender, message);
		} else {
			add_whisper_window_whisper(sender, message);
			increment_waiting_whispers(sender);

			do_notify(mp::notify_mode::whisper_other_window, sender, message);
		}
	} else if(can_go_to_active) {
		add_active_window_whisper(sender, message);
		do_notify(mp::notify_mode::whisper, sender, message);
	} else {
		LOG_LB << "Ignoring whisper from " << sender;
	}
}

void chatbox::add_chat_room_message_sent(const std::string& room, const std::string& message)
{
	lobby_chat_window* t = room_window_open(room, false);
	if(!t) {
		LOG_LB << "Cannot add sent message to ui for room " << room << ", player not in the room";
		return;
	}

	if(!room_window_active(room)) {
		switch_to_window(t);
	}

	add_active_window_message(prefs::get().login(), message, true);
}

void chatbox::add_chat_room_message_received(const std::string& room,
	const std::string& speaker,
	const std::string& message)
{
	mp::notify_mode notify_mode = mp::notify_mode::none;

	if(room_window_active(room)) {
		add_active_window_message(speaker, message);
		notify_mode = mp::notify_mode::message;
	} else {
		add_room_window_message(room, speaker, message);
		increment_waiting_messages(room);
		notify_mode = mp::notify_mode::message_other_window;
	}

	if(speaker == "server") {
		notify_mode = mp::notify_mode::server_message;
	} else if (utils::word_match(message, prefs::get().login())) {
		notify_mode = mp::notify_mode::own_nick;
	} else if (prefs::get().is_friend(speaker)) {
		notify_mode = mp::notify_mode::friend_message;
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
	return find_or_create_window(room, false, open_new, allow_close,
		VGETTEXT("Joined <i>$name</i>", { { "name", translation::dsgettext("wesnoth-lib", room.c_str()) } }));
}

lobby_chat_window* chatbox::whisper_window_open(const std::string& name, bool open_new)
{
	return find_or_create_window(name, true, open_new, true,
		VGETTEXT("Started private message with <i>$name</i>. "
		"If you do not want to receive messages from this player, type <i>/ignore $name</i>", { { "name", name } }));
}

lobby_chat_window* chatbox::find_or_create_window(const std::string& name,
	const bool whisper,
	const bool open_new,
	const bool allow_close,
	const std::string& initial_text)
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
	widget_item item;
	item["use_markup"] = "true";
	item["label"] = initial_text;
	widget_data data{{"log_text", item}};

	if(log_ != nullptr) {
		log_->emplace(name, chatroom_log{item["label"], whisper});
	}

	chat_log_container_->add_page(data);

	//
	// Add a new room window tab.
	//
	data.clear();
	item.clear();

	if(!whisper) {
		item["label"] = translation::dsgettext("wesnoth-lib", name.c_str());
	} else {
		item["label"] = "<" + name + ">";
	}

	data.emplace("room", item);

	grid& row_grid = roomlistbox_->add_row(data);

	//
	// Set up the Close Window button.
	//
	button& close_button = row_grid.find_widget<button>("close_window");

	if(!allow_close) {
		close_button.set_visible(widget::visibility::hidden);
	} else {
		connect_signal_mouse_left_click(close_button,
			std::bind(&chatbox::close_window_button_callback, this, open_windows_.back().name, std::placeholders::_3, std::placeholders::_4));
	}

	return &open_windows_.back();
}

void chatbox::close_window_button_callback(std::string room_name, bool& handled, bool& halt)
{
	const int index = std::distance(open_windows_.begin(), std::find_if(open_windows_.begin(), open_windows_.end(),
		[&room_name](const lobby_chat_window& room) { return room.name == room_name; }
	));

	close_window(index);

	handled = halt = true;
}

void chatbox::send_to_server(const ::config& cfg)
{
	mp::send_to_server(cfg);
}

void chatbox::increment_waiting_whispers(const std::string& name)
{
	if(lobby_chat_window* t = whisper_window_open(name, false)) {
		++t->pending_messages;

		if(t->pending_messages == 1) {
			DBG_LB << "do whisper pending mark row " << (t - &open_windows_[0]) << " with " << t->name;

			grid* grid = roomlistbox_->get_row_grid(t - &open_windows_[0]);
			grid->find_widget<image>("pending_messages").set_visible(widget::visibility::visible);
		}
	}
}

void chatbox::increment_waiting_messages(const std::string& room)
{
	if(lobby_chat_window* t = room_window_open(room, false)) {
		++t->pending_messages;

		if(t->pending_messages == 1) {
			int idx = t - &open_windows_[0];

			DBG_LB << "do room pending mark row " << idx << " with " << t->name;

			grid* grid = roomlistbox_->get_row_grid(idx);
			grid->find_widget<image>("pending_messages").set_visible(widget::visibility::visible);
		}
	}
}

void chatbox::add_whisper_window_whisper(const std::string& sender, const std::string& message)
{
	lobby_chat_window* t = whisper_window_open(sender, false);
	if(!t) {
		ERR_LB << "Whisper window not open in add_whisper_window_whisper for " << sender;
		return;
	}

	const std::string text = formatter() << markup::bold(sender, ": ") << font::escape_text(message);
	append_to_chatbox(text, t - &open_windows_[0], false);
}

void chatbox::add_active_window_whisper(const std::string& sender,
	const std::string& message,
	const bool force_scroll)
{
	const std::string text = formatter() << markup::bold("whisper: ", sender, ": ") << font::escape_text(message);
	append_to_chatbox(text, force_scroll);
}

void chatbox::close_window(std::size_t idx)
{
	const lobby_chat_window& t = open_windows_[idx];

	DBG_LB << "Close window " << idx << " - " << t.name;

	// Can't close the lobby!
	if((t.name == "lobby" && t.whisper == false) || open_windows_.size() == 1) {
		return;
	}

	// Check if we're closing the currently-active window.
	const bool active_changed = idx == active_window_;

	if(active_window_ == open_windows_.size() - 1) {
		--active_window_;
	}

	if(log_ != nullptr) {
		log_->erase(t.name);
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
		ERR_LB << "Room window not open in add_room_window_message for " << room;
		return;
	}

	const std::string text = formatter() << markup::bold(sender, ": ") << font::escape_text(message);
	append_to_chatbox(text, t - &open_windows_[0], false);
}

void chatbox::add_active_window_message(const std::string& sender,
	const std::string& message,
	const bool force_scroll)
{
	const std::string text = formatter() << markup::bold(sender, ": ") << font::escape_text(message);
	append_to_chatbox(text, force_scroll);
}

void chatbox::process_message(const ::config& data, bool whisper /*= false*/)
{
	std::string sender = data["sender"];
	DBG_LB << "process message from " << sender << " " << (whisper ? "(w)" : "")
		<< ", len " << data["message"].str().size();

	if(prefs::get().is_ignored(sender)) {
		return;
	}

	const std::string& message = data["message"];
	//prefs::get().parse_admin_authentication(sender, message); TODO: replace

	if(whisper) {
		add_whisper_received(sender, message);
	} else {
		if (!prefs::get().parse_should_show_lobby_join(sender, message)) return;

		std::string room = data["room"];

		// Attempt to send to the currently active room first.
		if(room.empty()) {
			LOG_LB << "Message without a room from " << sender << ", falling back to active window";
			room = open_windows_[active_window_].name;
		}

		// If we still don't have a name, fall back to lobby.
		if(room.empty()) {
			LOG_LB << "Message without a room from " << sender << ", assuming lobby";
			room = "lobby";
		}

		if(log_ != nullptr && data["type"].str() == "motd") {
			if(log_->at("lobby").received_motd == message) {
				LOG_LB << "Ignoring repeated motd";
				return;
			} else {
				log_->at("lobby").received_motd = message;
			}
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
	if(const auto message = data.optional_child("message")) {
		process_message(*message);
	} else if(const auto whisper = data.optional_child("whisper")) {
		process_message(*whisper, true);
	}
}

void chatbox::signal_handler_receive_keyboard_focus(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

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
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "background", missing_mandatory_wml_tag("chatbox_definition][resolution", "background")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "foreground", missing_mandatory_wml_tag("chatbox_definition][resolution", "foreground")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", missing_mandatory_wml_tag("chatbox_definition][resolution", "grid"));
	grid = std::make_shared<builder_grid>(child);
}
// }---------- BUILDER -----------{

namespace implementation
{

builder_chatbox::builder_chatbox(const config& cfg)
	: builder_styled_widget(cfg)
{
}

std::unique_ptr<widget> builder_chatbox::build() const
{
	auto widget = std::make_unique<chatbox>(*this);

	DBG_GUI_G << "Window builder: placed unit preview pane '" << id
			  << "' with definition '" << definition << "'.";

	const auto conf = widget->cast_config_to<chatbox_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);
	widget->finalize_setup();

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
