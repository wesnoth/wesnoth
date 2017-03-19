/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "display_chat_manager.hpp"

#include "desktop/notifications.hpp"
#include "display.hpp"
#include "floating_label.hpp"
#include "game_board.hpp" // <-- only needed for is_observer()
#include "game_preferences.hpp"
#include "log.hpp"
#include "font/marked-up_text.hpp"
#include "mp_ui_alerts.hpp"
#include "serialization/string_utils.hpp"
#include "color.hpp"

#include <SDL_timer.h>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {
	const int chat_message_border = 5;
	const int chat_message_x = 10;
	const color_t chat_message_color = {255,255,255,SDL_ALPHA_OPAQUE};
	const color_t chat_message_bg     = {0,0,0,140};
}

display_chat_manager::chat_message::chat_message(int speaker, int h)
	: speaker_handle(speaker), handle(h), created_at(SDL_GetTicks())
{}


void display_chat_manager::add_chat_message(const time_t& time, const std::string& speaker,
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

	if (!preferences::parse_should_show_lobby_join(sender, message)) return;
	if (preferences::is_ignored(sender)) return;

	preferences::parse_admin_authentication(sender, message);

	bool is_observer = false;
	{ //TODO: Clean this block up somehow

		const game_board * board = dynamic_cast<const game_board*>(&my_disp_.get_disp_context());

		if (board) {
			is_observer = board->is_observer();
		}
	}

	if (bell) {
		if ((type == events::chat_handler::MESSAGE_PRIVATE && (!is_observer || whisper))
			|| utils::word_match(message, preferences::login())) {
			mp_ui_alerts::private_message(false, sender, message);
		} else if (preferences::is_friend(sender)) {
			mp_ui_alerts::friend_message(false, sender, message);
		} else if (sender == "server") {
			mp_ui_alerts::server_message(false, sender, message);
		} else {
			mp_ui_alerts::public_message(false, sender, message);
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
		msg = my_disp_.video().faked() ? "" : font::word_wrap_text(msg,font::SIZE_SMALL,my_disp_.map_outside_area().w*3/4);
	} catch (utf8::invalid_utf8_exception&) {
		ERR_NG << "Invalid utf-8 found, chat message is ignored." << std::endl;
		return;
	}

	int ypos = chat_message_x;
	for(std::vector<chat_message>::const_iterator m = chat_messages_.begin(); m != chat_messages_.end(); ++m) {
		ypos += std::max(font::get_floating_label_rect(m->handle).h,
			font::get_floating_label_rect(m->speaker_handle).h);
	}
	color_t speaker_color = {255,255,255,SDL_ALPHA_OPAQUE};
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
	message_complete << preferences::get_chat_timestamp(time) << str.str();

	const SDL_Rect rect = my_disp_.map_outside_area();

	font::floating_label spk_flabel(message_complete.str());
	spk_flabel.set_font_size(font::SIZE_SMALL);
	spk_flabel.set_color(speaker_color);
	spk_flabel.set_position(rect.x + chat_message_x, rect.y + ypos);
	spk_flabel.set_clip_rect(rect);
	spk_flabel.set_alignment(font::LEFT_ALIGN);
	spk_flabel.set_bg_color(chat_message_bg);
	spk_flabel.set_border_size(chat_message_border);
	spk_flabel.use_markup(false);

	int speaker_handle = font::add_floating_label(spk_flabel);

	font::floating_label msg_flabel(message_str.str());
	msg_flabel.set_font_size(font::SIZE_SMALL);
	msg_flabel.set_color(message_color);
	msg_flabel.set_position(rect.x + chat_message_x + font::get_floating_label_rect(speaker_handle).w,
	rect.y + ypos);
	msg_flabel.set_clip_rect(rect);
	msg_flabel.set_alignment(font::LEFT_ALIGN);
	msg_flabel.set_bg_color(chat_message_bg);
	msg_flabel.set_border_size(chat_message_border);
	msg_flabel.use_markup(false);

	int message_handle = font::add_floating_label(msg_flabel);

	chat_messages_.push_back(chat_message(speaker_handle,message_handle));

	prune_chat_messages();
}

void display_chat_manager::prune_chat_messages(bool remove_all)
{
	const unsigned message_aging = preferences::chat_message_aging();
	const unsigned message_ttl = remove_all ? 0 : message_aging * 60 * 1000;
	const unsigned max_chat_messages = preferences::chat_lines();
	int movement = 0;

	if(message_aging != 0 || remove_all || chat_messages_.size() > max_chat_messages) {
		while (!chat_messages_.empty() &&
		       (chat_messages_.front().created_at + message_ttl < SDL_GetTicks() ||
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



