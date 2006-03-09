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

void level_to_gamestate(config& level, game_state& state)
{
	//any replay data is only temporary and should be removed from
	//the level data in case we want to save the game later
	config * const replay_data = level.child("replay");
	config replay_data_store;
	if(replay_data != NULL) {
		replay_data_store = *replay_data;
		LOG_NW << "setting replay\n";
		recorder = replay(replay_data_store);
		if(!recorder.empty()) {
			recorder.set_skip(false);
		}

		level.clear_children("replay");
	}

	//adds the starting pos to the level
	if(level.child("replay_start") == NULL)
		level.add_child("replay_start") = level;

	level["campaign_type"] = "multiplayer";
	state.campaign_type = "multiplayer";
	state.snapshot = level;

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
	users_menu_(disp.video(), std::vector<std::string>()),

	result_(CONTINUE)
{
	const SDL_Rect area = { 0, 0, disp.video().getx(), disp.video().gety() };
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

void ui::handle_key_event(const SDL_KeyboardEvent& event)
{
	//On enter, adds the current chat message to the chat textbox.
	if(event.keysym.sym == SDLK_RETURN && !entry_textbox_.text().empty()) {

		const std::string& text = entry_textbox_.text();

		//if the text starts with '/query' it's a query to the server.
		//otherwise it's just a chat message
		static const std::string query = "/query ";
		static const std::string whisper = "/msg ";
		static const std::string ignore = "/ignore ";
		
		config data;

		if(text.size() >= query.size() && std::equal(query.begin(),query.end(),text.begin())) {
			const std::string args = text.substr(query.size());

			data.add_child("query")["type"] = args;
			
		} else if (text.size() >= whisper.size() && std::equal(whisper.begin(),whisper.end(),text.begin())) {
		
			int pos;
			pos = text.find(" ",whisper.size());
			
			const std::string message = text.substr((pos+1),text.size());
			
			const std::string receiver = text.substr(whisper.size(),
			(text.size()-message.size()-whisper.size()-1));
			
			config cwhisper;
			cwhisper["message"] = message;
			cwhisper["sender"] = preferences::login();
			cwhisper["receiver"] = receiver;
			data.add_child("whisper", cwhisper);

			chat_.add_message(("whisper to "+cwhisper["receiver"]),
			(entry_textbox_.text().substr(whisper.size()+receiver.size()+1)));
			
			chat_.update_textbox(chat_textbox_);
			
		} else if (text.size() >= ignore.size() && std::equal(ignore.begin(),ignore.end(),text.begin())) {
		
			static const std::string add = "add";
			static const std::string remove = "remove";
			static const std::string list = "list";
			static const std::string clear = "clear";
			
			int pos;
			pos = text.find(" ",ignore.size());
			
			const std::string arg = text.substr((pos+1),text.find_first_of(' '));
			
			const std::string command = text.substr(ignore.size(),
			(text.size()-arg.size()-ignore.size()-1));
			
			config* cignore;
			
			if (std::equal(add.begin(),add.end(),command.begin())){
				if (!preferences::get_prefs()->child("ignore")){
					preferences::get_prefs()->add_child("ignore");
				}
				cignore = preferences::get_prefs()->child("ignore");
				(*cignore)[arg] = "yes";
				chat_.add_message("ignores list", "Added "+arg+" to ignore list.");
				
			} else if (std::equal(remove.begin(),remove.end(),command.begin())){
				if ((cignore = preferences::get_prefs()->child("ignore"))){
					(*cignore)[arg] = "no";
					chat_.add_message("ignores list", "Removed "+arg+" from ignore list.");
				}	
			} else if (std::equal(list.begin(),list.end(),command.begin())){
				std::string message = " ";
				if ((cignore = preferences::get_prefs()->child("ignore"))){
					std::map<std::string,t_string>::const_iterator i ;
					for( i = cignore->values.begin(); i != cignore->values.end(); ++i){
						if (i->second == "yes"){
							message+=i->first+",";
						}
					}
					message.erase(message.length()-1,1);
				}	
				chat_.add_message("ignores list", message);
			} else if (std::equal(clear.begin(),clear.end(),command.begin())){

				if ((cignore = preferences::get_prefs()->child("ignore"))){
					string_map::iterator nick;
					for(nick= cignore->values.begin() ; nick!= cignore->values.end(); nick++) {
						(*cignore)[nick->first] = "no";
						chat_.add_message("ignores list", "Removed "+nick->first+" from ignore list.");
					}
				}	
			} else {				
				chat_.add_message("ignores list", "unknown command "+command+".");	
			}
			chat_.update_textbox(chat_textbox_);
		} else {

			// Sends the message to the network
			config msg;
			msg["message"] = text;
			msg["sender"] = preferences::login();
			data.add_child("message", msg);

			chat_.add_message(preferences::login(), entry_textbox_.text());
			chat_.update_textbox(chat_textbox_);

		}

		network::send_data(data);
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
					while(best_match[j] == (*i)[j]) j++;
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
			sound::play_sound(game_config::sounds::receive_message);
			
			const config& cwhisper = *data.child("whisper");
			chat_.add_message("whisper: "+cwhisper["sender"], cwhisper["message"]);
			chat_.update_textbox(chat_textbox_);
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
