/* $Id$ */
/*
   Copyright (C) 2005
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "display.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "log.hpp"
#include "multiplayer_ui.hpp"
#include "network.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "replay.hpp"

#define LOG_NW LOG_STREAM(info, network)
#define ERR_NW LOG_STREAM(err, network)

namespace mp {

void check_response(network::connection res, const config& data)
{
	if(!res) {
		throw network::error(_("Connection timed out"));
	}

	const config* err = data.child("error");
	if(err != NULL) {
		throw network::error((*err)["message"]);
	}
}

void level_to_gamestate(config& level, game_state& state, bool saved_game)
{
	config * const replay_data = level.child("replay");
	config replay_data_store;
	if(replay_data != NULL) {
		replay_data_store = *replay_data;
		LOG_NW << "setting replay\n";
		state.replay_data = *replay_data;
		recorder = replay(replay_data_store);
		if(!recorder.empty()) {
			recorder.set_skip(false);
			recorder.set_to_end();
		}
	}

	//adds the starting pos to the level
	if(level.child("replay_start") == NULL){
		level.add_child("replay_start", level);
	}
	//this is important, if it does not happen, the starting position is missing and
	//will be drawn from the snapshot instead (which is not what we want since we have
	//all needed information here already)
	state.starting_pos = *(level.child("replay_start"));

	level["campaign_type"] = "multiplayer";
	state.campaign_type = "multiplayer";
	state.version = level["version"];

	// [variables] might be used in a normal scenario too
	const config* const vars = level.child("variables");
	if(vars != NULL) {
		state.variables = *vars;
	}
	//If we start a fresh game, there won't be any snapshot information. If however this
	//is a savegame, we got a valid snapshot here.
	if (saved_game){
		state.snapshot = *(level.child("snapshot"));

		if (state.snapshot.child("variables") != NULL){
			state.variables = *state.snapshot.child("variables");
		}

		//We also need to take into account, that the reload could take place with different players.
		//If so, they are substituted now.
		const config::child_list& snapshot_sides = state.snapshot.get_children("side");
		const config::child_list& level_sides = level.get_children("side");
		for(config::child_list::const_iterator side = snapshot_sides.begin(); side != snapshot_sides.end(); ++side) {
			for(config::child_list::const_iterator lside = level_sides.begin(); lside != level_sides.end(); ++lside) {
				if ( ((**side)["side"] == (**lside)["side"]) 
					&& ((**side)["current_player"] != (**lside)["current_player"]) ){
					(**side)["current_player"] = (**lside)["current_player"];
					(**side)["description"] = (**lside)["description"];
					(**side)["save_id"] = (**lside)["save_id"];
					(**side)["controller"] = (**lside)["controller"];
					break;
				}
			}
		}
	}

}

std::string get_colour_string(int id)
{
	std::string prefix("\033[3 m");
	prefix[3] = lexical_cast<char, int>(id + 1);
	std::map<int, std::string>::iterator name = game_config::team_rgb_name.find(id+1);
	if(name != game_config::team_rgb_name.end()){
	  return prefix + _(name->second.c_str());
	}else{
	  return prefix + _("Invalid Color");
	}
}

chat::chat()
{
}

void chat::add_message(const std::string& user, const std::string& message)
{
	message_history_.push_back(msg(user, message));

	while (message_history_.size() > 1024) {
		message_history_.pop_front();

		if (last_update_ > 0)
			last_update_--;
	}
}

void chat::init_textbox(gui::textbox& textbox)
{
	std::string s;

	for(msg_hist::const_iterator itor = message_history_.begin();
			itor != message_history_.end(); ++itor) {
		s.append(format_message(*itor));
	}

	textbox.set_text(s);
	last_update_ = message_history_.size();
	textbox.scroll_to_bottom();
}

void chat::update_textbox(gui::textbox& textbox)
{
	std::string s;

	for(msg_hist::const_iterator itor = message_history_.begin() + last_update_;
			itor != message_history_.end(); ++itor) {
		s.append(format_message(*itor));
	}

	textbox.append_text(s,true);

	last_update_ = message_history_.size();
}

std::string chat::format_message(const msg& message)
{
	if(message.message.substr(0,3) == "/me") {
		return "<" + message.user + " " + message.message.substr(3) + ">\n";
	} else {
		return "<" + message.user + ">" + message.message + "\n";
	}
}

ui::ui(display& disp, const std::string& title, const config& cfg, chat& c, config& gamelist) :
	gui::widget(disp.video()),
	disp_(disp),
	initialized_(false),
	gamelist_initialized_(false),

	hotkey_handler_(&disp),
	disp_manager_(&disp),

	game_config_(cfg),
	chat_(c),
	gamelist_(gamelist),

	title_(disp.video(), title, font::SIZE_LARGE, font::TITLE_COLOUR),
	chat_textbox_(disp.video(), 100, "", false),
	entry_textbox_(disp.video(), 100),
	users_menu_(disp.video(), std::vector<std::string>(), false, -1, -1, NULL, &gui::menu::slateborder_style),

	result_(CONTINUE)
{
	const SDL_Rect area = { 0, 0, disp.video().getx(), disp.video().gety() };
	users_menu_.set_numeric_keypress_selection(false);
	set_location(area);
}

void ui::process_network()
{
	config data;

	try {
		const network::connection sock = network::receive_data(data);

		if(sock) {
			process_network_data(data, sock);
		}
	} catch(network::error& e) {
		process_network_error(e);
	}

	if (accept_connections()) {
		network::connection sock = network::accept_connection();
		if(sock) {
			LOG_NW << "Received connection\n";

			process_network_connection(sock);
		}
	}
}

ui::result ui::get_result()
{
	return result_;
}

ui::result ui::set_result(ui::result res)
{
	result_ = res;
	return res;
}

int ui::xscale(int x) const
{
	return (x * width())/1024;
}

int ui::yscale(int y) const
{
	return (y * height())/768;
}

SDL_Rect ui::client_area() const
{
	SDL_Rect res;

	res.x = xscale(10) + 10;
	res.y = yscale(38) + 10;
	res.w = xscale(828) > 12 ? xscale(828) - 12 : 0;
	res.h = yscale(520) > 12 ? yscale(520) - 12 : 0;

	return res;
}

const config& ui::game_config() const
{
	return game_config_;
}

void ui::draw_contents()
{
	hide_children();

	surface background(image::get_image("misc/lobby.png",image::UNSCALED));
	background = scale_surface(background, video().getx(), video().gety());
	if(background == NULL)
		return;
	SDL_BlitSurface(background, NULL, video().getSurface(), NULL);
	update_whole_screen();

	hide_children(false);
}

void ui::set_location(const SDL_Rect& rect)
{
	hide_children();
	widget::set_location(rect);
	layout_children(rect);
	if(!initialized_) {
		chat_textbox_.set_wrap(true);
		chat_.init_textbox(chat_textbox_);
		initialized_ = true;
	}
	hide_children(false);
}

void ui::process_event()
{
}

void ui::handle_event(const SDL_Event& event)
{
	if(event.type == SDL_KEYDOWN) {
		handle_key_event(event.key);
	}
}

void ui::send_chat_query(const std::string& args)
{
	config data;
	data.add_child("query")["type"] = args;
	network::send_data(data);
}

void ui::add_chat_message(const std::string& speaker, int /*side*/, const std::string& message, display::MESSAGE_TYPE /*type*/)
{
	chat_.add_message(speaker,message);
	chat_.update_textbox(chat_textbox_);
}

void ui::send_chat_message(const std::string& message, bool /*allies_only*/)
{
	config data, msg;
	msg["message"] = message;
	msg["sender"] = preferences::login();
	data.add_child("message", msg);

	add_chat_message(preferences::login(),0, message);	//local echo
	network::send_data(data);
}


void ui::handle_key_event(const SDL_KeyboardEvent& event)
{
	//On enter, adds the current chat message to the chat textbox.
	if(event.keysym.sym == SDLK_RETURN && !entry_textbox_.text().empty()) {

		chat_handler::do_speak(entry_textbox_.text());
		entry_textbox_.clear();

	} else if(event.keysym.sym == SDLK_TAB ) {
		std::string text = entry_textbox_.text();
		std::string semiword;
		bool beginning;

		const size_t last_space = text.rfind(" ");

		//if last character is a space return
		if(last_space == text.size() -1) {
			return;
		}

		if(last_space == std::string::npos) {
			beginning = true;
			semiword = text;
		}else{
			beginning = false;
			semiword.assign(text,last_space+1,text.size());
		}


		std::vector<std::string> matches;
		std::string best_match = semiword;
		std::vector<std::string>& users = user_list_;
		std::sort<std::vector<std::string>::iterator>(users.begin(), users.end());
		for(std::vector<std::string>::const_iterator i = users.begin(); i != users.end(); ++i) {
			if( i->size() >= semiword.size() &&
					std::equal(semiword.begin(),semiword.end(),i->begin(),chars_equal_insensitive)) {
				if(matches.empty()) {
					best_match = *i;
				} else {
					int j= 0;;
					while(toupper(best_match[j]) == toupper((*i)[j])) j++;
					best_match.erase(best_match.begin()+j,best_match.end());
				}
				matches.push_back(*i);
			}
		}

		if(!matches.empty()) {
			std::string add = beginning ? ": " : " ";
			text.replace(last_space+1, semiword.size(), best_match);
			if(matches.size() == 1) {
				text.append(add);
			} else {
				std::string completion_list;
				std::vector<std::string>::iterator it;
				for(it =matches.begin();it!=matches.end();it++) {
					completion_list += " ";
					completion_list += *it;
				}
				chat_.add_message("",completion_list);
				chat_.update_textbox(chat_textbox_);
			}
			entry_textbox_.set_text(text);
		}

	}
}

void ui::process_network_data(const config& data, const network::connection /*sock*/)
{
	if(data.child("error")) {
		throw network::error((*data.child("error"))["message"]);
	} else {
		if(data.child("message")) {
			const config& msg = *data.child("message");
			config* cignore;
			bool ignored = false;
			if ((cignore = preferences::get_prefs()->child("ignore"))){
				for(std::map<std::string,t_string>::const_iterator i = cignore->values.begin();
				i != cignore->values.end(); ++i){
					if(msg["sender"] == i->first){
						if (i->second == "yes"){
							ignored = true;
						}
					}
				}
			}

			if (!ignored){
				sound::play_sound(game_config::sounds::receive_message);

				chat_.add_message(msg["sender"], msg["message"]);
				chat_.update_textbox(chat_textbox_);
			}
		}

		if(data.child("whisper")){
			const config& cwhisper = *data.child("whisper");

			config* cignore;
			bool ignored = false;
			if ((cignore = preferences::get_prefs()->child("ignore"))){
				for(std::map<std::string,t_string>::const_iterator i = cignore->values.begin();
				i != cignore->values.end(); ++i){
					if(cwhisper["sender"] == i->first){
						if (i->second == "yes"){
							ignored = true;
						}
					}
				}
			}

			if (!ignored){
				sound::play_sound(game_config::sounds::receive_message);
				chat_.add_message("whisper: "+cwhisper["sender"], cwhisper["message"]);
				chat_.update_textbox(chat_textbox_);
			}
		}

		if(data.child("gamelist")) {
			if(!gamelist_initialized_)
				gamelist_initialized_ = true;
			gamelist_ = data;
			gamelist_updated(false);
		} else if(data.child("gamelist_diff")) {
			if(gamelist_initialized_) {
				gamelist_.apply_diff(*data.child("gamelist_diff"));
				gamelist_updated(false);
			}
		}
	}
}

void ui::process_network_error(network::error& error)
{
	ERR_NW << "Caught networking error: " << error.message << "\n";

	// Default behaviour is to re-throw the error. May be overridden.
	throw error;
}

void ui::process_network_connection(const network::connection /*sock*/)
{
	LOG_NW << "Caught network connection.\n";
}

void ui::hide_children(bool hide)
{
	title_.hide(hide);
	chat_textbox_.hide(hide);
	entry_textbox_.hide(hide);
	users_menu_.hide(hide);
}

void ui::layout_children(const SDL_Rect& /*rect*/)
{
	title_.set_location(xscale(12) + 8, yscale(38) + 8);
	users_menu_.set_width(xscale(159));
	users_menu_.set_max_width(xscale(159));
	users_menu_.set_location(xscale(856), yscale(42));
	users_menu_.set_height(yscale(715));
	users_menu_.set_max_height(yscale(715));
	chat_textbox_.set_location(xscale(11) + 4, yscale(573) + 4);
	chat_textbox_.set_measurements(xscale(833) - 8, yscale(143) - 8);
	entry_textbox_.set_location(xscale(11) + 4, yscale(732));
	entry_textbox_.set_width(xscale(833) - 8);
}

void ui::gamelist_updated(bool silent)
{
	std::vector<std::string> user_strings;
	config::child_list users = gamelist_.get_children("user");
	config::child_iterator user;
	for (user = users.begin(); user != users.end(); ++user) {
		const std::string prefix = (**user)["available"] == "no" ? "#" : "";
		std::string suffix = "";
		if(!(**user)["location"].empty()) {
			suffix = std::string(" (") + (**user)["location"] + std::string(")");
		}
		user_strings.push_back(prefix + (**user)["name"].str() + suffix);
	}
	set_user_list(user_strings, silent);
}

void ui::set_user_list(const std::vector<std::string>& list, bool silent)
{
	if(!silent) {
		if(list.size() < user_list_.size()) {
			sound::play_sound(game_config::sounds::user_leave);
		} else if(list.size() > user_list_.size()) {
			sound::play_sound(game_config::sounds::user_arrive);
		}
	}

	user_list_ = list;

	users_menu_.set_items(user_list_,true,true);
}

const gui::widget& ui::title() const
{
	return title_;
}


}
