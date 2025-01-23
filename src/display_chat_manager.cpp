/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "display_chat_manager.hpp"

#include "display.hpp"
#include "floating_label.hpp"
#include "game_board.hpp" // <-- only needed for is_observer()
#include "preferences/preferences.hpp"
#include "log.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "mp_ui_alerts.hpp"
#include "serialization/string_utils.hpp"
#include "color.hpp"
#include "serialization/utf8_exception.hpp"
#include "video.hpp" // only for faked

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {
	const int chat_message_border = 5;
	const int chat_message_x = 10;
	const color_t chat_message_color {255,255,255,SDL_ALPHA_OPAQUE};
	const color_t chat_message_bg    {0,0,0,140};
}

display_chat_manager::chat_message::chat_message(int speaker, int h)
	: speaker_handle(speaker), handle(h), created_at(std::chrono::steady_clock::now())
{}


void display_chat_manager::add_chat_message(const std::time_t& time, const std::string& speaker,
		int side, const std::string& message, events::chat_handler::MESSAGE_TYPE type,
		bool bell)
{
	const bool whisper = speaker.find("whisper: ") == 0;
	std::string sender = speaker;
	if (whisper) {
		sender.assign(speaker, 9, speaker.size());
		add_whisperer( sender );
	}
	//remove disconnected user from whisperer
	std::string::size_type pos = message.find(" has disconnected");
	if (pos != std::string::npos){
		for(std::set<std::string>::const_iterator w = whisperers().begin(); w != whisperers().end(); ++w){
			if (*w == message.substr(0,pos)) remove_whisperer(*w);
		}
	}

	if (!prefs::get().parse_should_show_lobby_join(sender, message)) return;
	if (prefs::get().is_ignored(sender)) return;

	//prefs::get().parse_admin_authentication(sender, message); TODO: replace

	bool is_observer = false;
	{ //TODO: Clean this block up somehow

		const game_board * board = dynamic_cast<const game_board*>(&my_disp_.context());

		if (board) {
			is_observer = board->is_observer();
		}
	}

	if (bell) {
		if ((type == events::chat_handler::MESSAGE_PRIVATE && (!is_observer || whisper))
			|| utils::word_match(message, prefs::get().login())) {
			mp::ui_alerts::private_message(false, sender, message);
		} else if (prefs::get().is_friend(sender)) {
			mp::ui_alerts::friend_message(false, sender, message);
		} else if (sender == "server") {
			mp::ui_alerts::server_message(false, sender, message);
		} else {
			mp::ui_alerts::public_message(false, sender, message);
		}
	}

	bool action = false;

	std::string msg;

	if (message.compare(0,4,"/me ") == 0) {
		msg.assign(message, 4, message.size());
		action = true;
	} else {
		msg = message;
	}

	try {
		// We've had a joker who send an invalid utf-8 message to crash clients
		// so now catch the exception and ignore the message.
		msg = video::headless() ? "" : font::pango_word_wrap(msg,font::SIZE_15,my_disp_.map_outside_area().w*3/4);
	} catch (utf8::invalid_utf8_exception&) {
		ERR_NG << "Invalid utf-8 found, chat message is ignored.";
		return;
	}

	int ypos = chat_message_x;
	for(std::vector<chat_message>::const_iterator m = chat_messages_.begin(); m != chat_messages_.end(); ++m) {
		ypos += std::max(font::get_floating_label_rect(m->handle).h,
			font::get_floating_label_rect(m->speaker_handle).h);
	}
	color_t speaker_color {255,255,255,SDL_ALPHA_OPAQUE};
	if(side >= 1) {
		speaker_color = team::get_side_color_range(side).mid();
	}

	color_t message_color = chat_message_color;
	std::stringstream str;
	std::stringstream message_str;

	if(type ==  events::chat_handler::MESSAGE_PUBLIC) {
		if(action) {
			str << "<" << speaker << " " << msg << ">";
			message_color = speaker_color;
			message_str << " ";
		} else {
			if (!speaker.empty())
				str << "<" << speaker << ">";
			message_str << msg;
		}
	} else {
		if(action) {
			str << "*" << speaker << " " << msg << "*";
			message_color = speaker_color;
			message_str << " ";
		} else {
			if (!speaker.empty())
				str << "*" << speaker << "*";
			message_str << msg;
		}
	}

	// Prepend message with timestamp.
	std::stringstream message_complete;
	message_complete << prefs::get().get_chat_timestamp(std::chrono::system_clock::from_time_t(time)) << str.str();

	const SDL_Rect rect = my_disp_.map_outside_area();

	font::floating_label spk_flabel(message_complete.str());
	spk_flabel.set_font_size(font::SIZE_15);
	spk_flabel.set_color(speaker_color);
	spk_flabel.set_position(rect.x + chat_message_x, rect.y + ypos);
	spk_flabel.set_clip_rect(rect);
	spk_flabel.set_alignment(font::LEFT_ALIGN);
	spk_flabel.set_bg_color(chat_message_bg);
	spk_flabel.set_border_size(chat_message_border);
	spk_flabel.use_markup(false);

	int speaker_handle = font::add_floating_label(spk_flabel);

	font::floating_label msg_flabel(message_str.str());
	msg_flabel.set_font_size(font::SIZE_15);
	msg_flabel.set_color(message_color);
	msg_flabel.set_position(rect.x + chat_message_x + font::get_floating_label_rect(speaker_handle).w,
	rect.y + ypos);
	msg_flabel.set_clip_rect(rect);
	msg_flabel.set_alignment(font::LEFT_ALIGN);
	msg_flabel.set_bg_color(chat_message_bg);
	msg_flabel.set_border_size(chat_message_border);
	msg_flabel.use_markup(false);

	int message_handle = font::add_floating_label(msg_flabel);

	chat_messages_.emplace_back(speaker_handle,message_handle);

	prune_chat_messages();
}

void display_chat_manager::prune_chat_messages(bool remove_all)
{
	//NOTE: prune_chat_messages(false) seems to be only called when a new message is added, which in
	//      particular means the aging feature won't work unless new messages are addded regularly
	const auto message_aging = prefs::get().chat_message_aging();
	const unsigned max_chat_messages = prefs::get().chat_lines();
	const bool enable_aging = message_aging != std::chrono::minutes{0};

	const auto remove_before = enable_aging
		? std::chrono::steady_clock::now() - message_aging
		: std::chrono::steady_clock::time_point::min();

	int movement = 0;

	if(enable_aging || remove_all || chat_messages_.size() > max_chat_messages) {
		while (!chat_messages_.empty() &&
		       (remove_all ||
			chat_messages_.front().created_at < remove_before ||
		        chat_messages_.size() > max_chat_messages))
		{
			const chat_message &old = chat_messages_.front();
			movement += font::get_floating_label_rect(old.handle).h;
			font::remove_floating_label(old.speaker_handle);
			font::remove_floating_label(old.handle);
			chat_messages_.erase(chat_messages_.begin());
		}
	}

	for(const chat_message &cm : chat_messages_) {
		font::move_floating_label(cm.speaker_handle, 0, - movement);
		font::move_floating_label(cm.handle, 0, - movement);
	}
}
