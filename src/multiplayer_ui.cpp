/* $Id$ */
/*
   Copyright (C) 2005 - 2008
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "construct_dialog.hpp"
#include "game_display.hpp"
#include "font.hpp"
#include "marked-up_text.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "log.hpp"
#include "multiplayer_ui.hpp"
#include "network.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "replay.hpp"
#include "wml_separators.hpp"

#define LOG_NG LOG_STREAM(info, engine)
#define ERR_NG LOG_STREAM(err, engine)
#define ERR_CF LOG_STREAM(err, config)
#define DBG_NW LOG_STREAM(debug, network)
#define LOG_NW LOG_STREAM(info, network)
#define ERR_NW LOG_STREAM(err, network)

namespace {

	class user_menu_style : public gui::menu::imgsel_style {
	public:
		user_menu_style() : gui::menu::imgsel_style("misc/selection", false,
										   0x000000, 0x4a4440, 0x999999,
										   0.0, 0.2, 0.2),
										   item_size_(empty_rect)
		{}
		virtual void init();
		virtual SDL_Rect item_size(const std::string& /*item*/) { return item_size_; }
		void set_width(const int width) { item_size_.w = width; }
	private:
		SDL_Rect item_size_;
	};

	void user_menu_style::init()
	{
		imgsel_style::init();
		item_size_.h = font::get_max_height(font_size_);
		scale_images(-1, item_size_.h);
		item_size_.h += 2 * thickness_;
	}

	user_menu_style umenu_style;
	
} // anon namespace

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
	//any replay data is only temporary and should be removed from
	//the level data in case we want to save the game later
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

	//set random
	const std::string seed = level["random_seed"];
	if(! seed.empty()) {
		const unsigned calls = lexical_cast_default<unsigned>(level["random_calls"]);
		state.seed_random(lexical_cast<int>(seed), calls);
	} else {
		ERR_NG << "No random seed found, random "
			"events will probably be out of sync.\n";
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

	const config* const vars = level.child("variables");
	if(vars != NULL) {
		state.set_variables(*vars);
	}
	state.set_menu_items(level.get_children("menu_item"));

	//If we start a fresh game, there won't be any snapshot information. If however this
	//is a savegame, we got a valid snapshot here.
	if (saved_game){
		state.snapshot = *(level.child("snapshot"));
		if (state.snapshot.child("variables") != NULL){
			state.set_variables(*state.snapshot.child("variables"));
		}
		state.set_menu_items(state.snapshot.get_children("menu_item"));

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
	if(state.get_variables().empty()) {
		LOG_NG << "No variables were found for the game_state." << std::endl;
	} else {
		LOG_NG << "Variables found and loaded into game_state:" << std::endl;
		LOG_NG << state.get_variables();
	}
}

std::string get_colour_string(int id)
{
	std::string prefix = team::get_side_highlight(id);
	std::stringstream side_id;
	side_id << (id + 1);
	std::map<std::string, t_string>::iterator name = game_config::team_rgb_name.find(side_id.str());
	if(name != game_config::team_rgb_name.end()){
		return prefix + name->second;
	}else{
		return prefix + _("Invalid Color");
	}
}

chat::chat()
{
}

void chat::add_message(const time_t& time, const std::string& user,
		const std::string& message)
{
	message_history_.push_back(msg(time, user, message));

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
		return preferences::get_chat_timestamp(message.time) + "<" + message.user
				+ message.message.substr(3) + ">\n";
	} else {
		return preferences::get_chat_timestamp(message.time) + "<" + message.user
				+ "> " + message.message + "\n";
	}
}

ui::ui(game_display& disp, const std::string& title, const config& cfg, chat& c, config& gamelist) :
	gui::widget(disp.video()),
	disp_(disp),
	initialized_(false),
	gamelist_initialized_(false),

	hotkey_handler_(&disp),
	disp_manager_(&disp),

	game_config_(cfg),
	chat_(c),
	gamelist_(gamelist),

#ifdef USE_TINY_GUI
	title_(disp.video(), title, font::SIZE_SMALL, font::TITLE_COLOUR),
#else
	title_(disp.video(), title, font::SIZE_LARGE, font::TITLE_COLOUR),
	entry_textbox_(disp.video(), 100),
#endif
	chat_textbox_(disp.video(), 100, "", false),
	users_menu_(disp.video(), std::vector<std::string>(), false, -1, -1, NULL, &umenu_style),

	selected_game_(""),

	result_(CONTINUE),
	gamelist_refresh_(false),
	lobby_clock_(0)
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

	//apply diffs at a set interval
	if(gamelist_refresh_ && SDL_GetTicks() - lobby_clock_ > game_config::lobby_refresh)
	{
		const cursor::setter cursor_setter(cursor::WAIT);
		gamelist_updated(false);
		gamelist_refresh_ = false;
		lobby_clock_ = SDL_GetTicks();
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

const int ui::xscale_base = 1024;
const int ui::yscale_base =  768;

int ui::xscale(int x) const
{
	return (x * width())/ui::xscale_base;
}

int ui::yscale(int y) const
{
	return (y * height())/ui::yscale_base;
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

#ifdef USE_TINY_GUI
	surface background(image::get_image("misc/lobby_tiny.png"));
#else
	surface background(image::get_image("misc/lobby.png"));
#endif
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
    if(users_menu_.double_clicked()) {
		std::string usr_text = user_list_[users_menu_.selection()];
		std::string caption = _("Send a private message to ") + usr_text;
		gui::dialog d(disp(), _("Whisper"), caption, gui::OK_CANCEL);
		d.set_textbox( _("Message: "));
		Uint32 show_time = SDL_GetTicks();
		if (!(d.show() || d.textbox_text().empty())) {
			std::stringstream msg;
			msg << "/msg " << usr_text << ' ' << d.textbox_text();
			chat_handler::do_speak(msg.str());
        }
		if(show_time + 60000 < SDL_GetTicks()) {
			//if the dialog has been open for a long time, refresh the lobby
			config request;
			request.add_child("refresh_lobby");
			network::send_data(request, 0, true);
		}
	}
}

void ui::add_chat_message(const time_t& time, const std::string& speaker, int /*side*/, const std::string& message, game_display::MESSAGE_TYPE /*type*/)
{
	chat_.add_message(time, speaker, message);
	chat_.update_textbox(chat_textbox_);
}

void ui::send_chat_message(const std::string& message, bool /*allies_only*/)
{
	config data, msg;
	msg["message"] = message;
	msg["sender"] = preferences::login();
	data.add_child("message", msg);

	add_chat_message(time(NULL), preferences::login(),0, message);	//local echo
	network::send_data(data, 0, true);
}


void ui::handle_key_event(const SDL_KeyboardEvent& event)
{
#ifndef USE_TINY_GUI
	//On enter, adds the current chat message to the chat textbox.
	if((event.keysym.sym == SDLK_RETURN || event.keysym.sym == SDLK_KP_ENTER) && !entry_textbox_.text().empty()) {

		chat_handler::do_speak(entry_textbox_.text());
		entry_textbox_.clear();
	// nick tab-completion
	} else if(event.keysym.sym == SDLK_TAB ) {
		std::string text = entry_textbox_.text();
		std::vector<std::string> matches = user_list_;
		// Exclude own nick from tab-completion.
		matches.erase(std::remove(matches.begin(), matches.end(),
				preferences::login()), matches.end());
		const bool line_start = utils::word_completion(text, matches);

		if (matches.empty()) return;

		if (matches.size() == 1) {
			text.append(line_start ? ": " : " ");
		} else {
			std::string completion_list = utils::join(matches, ' ');
			chat_.add_message(time(NULL), "", completion_list);
			chat_.update_textbox(chat_textbox_);
		}
		entry_textbox_.set_text(text);
	}
#endif
}

void ui::process_message(const config& msg, const bool whisper) {
	const std::string& sender = msg["sender"];
	const std::string& message = msg["message"];
	if (!preferences::show_lobby_join(sender, message)) return;
	if (preferences::is_ignored(sender)) return;

	if (whisper || utils::word_match(message, preferences::login())) {
		sound::play_UI_sound(game_config::sounds::receive_message_highlight);
	} else if (preferences::is_friend(sender)) {
		sound::play_UI_sound(game_config::sounds::receive_message_friend);
	} else if (sender == "server") {
		sound::play_UI_sound(game_config::sounds::receive_message_server);
	} else {
		sound::play_UI_sound(game_config::sounds::receive_message);
	}
	chat_.add_message(time(NULL), (whisper ? "whisper: " : "") + msg["sender"],
			msg["message"]);
	chat_.update_textbox(chat_textbox_);
}

void ui::process_network_data(const config& data, const network::connection /*sock*/)
{
	if(data.child("error")) {
		throw network::error((*data.child("error"))["message"]);
	} else if (data.child("message")) {
		process_message(*data.child("message"));
	} else if(data.child("whisper")){
		process_message(*data.child("whisper"), true);
	} else if(data.child("gamelist")) {
		const cursor::setter cursor_setter(cursor::WAIT);
		gamelist_initialized_ = true;
		gamelist_ = data;
		gamelist_updated(false);
		gamelist_refresh_ = false;
		lobby_clock_ = SDL_GetTicks();
	} else if(data.child("gamelist_diff")) {
		if(gamelist_initialized_) {
			try {
				gamelist_.apply_diff(*data.child("gamelist_diff"));
			} catch(config::error& e) {
				ERR_CF << "Error while applying the gamelist diff: '"
					<< e.message << "' Getting a new gamelist.\n";
				network::send_data(config("refresh_lobby"), 0, true);
			}
			gamelist_refresh_ = true;
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
#ifndef USE_TINY_GUI
	entry_textbox_.hide(hide);
#endif
	users_menu_.hide(hide);
}

void ui::layout_children(const SDL_Rect& /*rect*/)
{
	title_.set_location(xscale(12) + 8, yscale(38) + 8);
	umenu_style.set_width(xscale(159));
	users_menu_.set_width(xscale(159));
	users_menu_.set_max_width(xscale(159));
	users_menu_.set_location(xscale(856), yscale(42));
	users_menu_.set_height(yscale(715));
	users_menu_.set_max_height(yscale(715));
#ifdef USE_TINY_GUI
	chat_textbox_.set_location(xscale(11) + 4, yscale(625) + 4);
	chat_textbox_.set_measurements(xscale(833) - 8, yscale(143) - 8);

#else
	chat_textbox_.set_location(xscale(11) + 4, yscale(573) + 4);
	chat_textbox_.set_measurements(xscale(833) - 8, yscale(143) - 8);
	entry_textbox_.set_location(xscale(11) + 4, yscale(732));
	entry_textbox_.set_width(xscale(833) - 8);
#endif
}

bool ui::user_info::operator> (const user_info& b) const {
	user_info const& a = *this;

	// ME always on top
	if (a.relation == ME) {
		return true;
	}
	if (b.relation == ME) {
		return false;
	}

	// friends next, sorted by location
	if ((a.relation == FRIEND) && (b.relation == FRIEND)) {
		if (a.state != b.state) {
			return a.state < b.state;
		}
		return a.name < b.name;
	}
	if (a.relation == FRIEND) {
		return true;
	}
	if (b.relation == FRIEND) {
		return false;
	}

	// players in the selected game next, sorted by relation (friends/neutral/ignored)
	if ((a.state == SEL_GAME) && (b.state == SEL_GAME)) {
		if (a.relation != b.relation) {
			return a.relation < b.relation;
		}
		return a.name < b.name;
	}
	if (a.state == SEL_GAME) {
		return true;
	}
	if (b.state == SEL_GAME) {
		return false;
	}

	// all others grouped by relation
	if (a.relation != b.relation) {
		return a.relation < b.relation;
	}
	if (a.state != b.state) {
		return a.state < b.state;
	}
	return a.name < b.name;
}

void ui::gamelist_updated(bool silent)
{
	config::child_list users = gamelist_.get_children("user");
	config::child_iterator user;
	std::list<user_info> u_list;

	for (user = users.begin(); user != users.end(); ++user) {
		user_info u_elem;
		u_elem.name = (**user)["name"];
		u_elem.game_id = "";
		u_elem.location = "";
		u_elem.state = (**user)["available"] == "no" ? GAME : LOBBY;
		if(!(**user)["game_id"].empty()) {
			u_elem.game_id = (**user)["game_id"];
			if (u_elem.game_id == selected_game_) {
				u_elem.state = SEL_GAME;
			}
		}
		if(!(**user)["location"].empty()) {
			u_elem.location = (**user)["location"];
		}
		std::vector<std::string> friends = utils::split(preferences::get("friends"));
		std::vector<std::string> ignores = utils::split(preferences::get("ignores"));
		if (u_elem.name == preferences::login()) {
			u_elem.relation = ME;
		} else if (std::find(ignores.begin(), ignores.end(), u_elem.name) != ignores.end()) {
			u_elem.relation = IGNORED;
		} else if (std::find(friends.begin(), friends.end(), u_elem.name) != friends.end()) {
			u_elem.relation = FRIEND;
		} else {
			u_elem.relation = NEUTRAL;
		}
		u_list.push_back(u_elem);
	}

	if (preferences::sort_list()) {
		u_list.sort(std::greater<user_info>());
	}

	// can't use the bold tag here until the menu code
	// calculates a correct ellipsis for it
	const std::string lobby_color_tag   = "";
	const std::string ingame_color_tag  = "#";
	const std::string selgame_color_tag = "<0,191,255>";

	std::string const imgpre = IMAGE_PREFIX + std::string("misc/status-");
	std::vector<std::string> user_strings;
	std::vector<std::string> menu_strings;

	std::list<user_info>::const_iterator u_itor = u_list.begin();
	while (u_itor != u_list.end()) {
		const std::string game_str = (u_itor->state == LOBBY) ? "" : " (" + u_itor->location + ")";
		std::string img_str = "";
		std::string color_str = "";
		switch (u_itor->state) {
			case LOBBY:    color_str = lobby_color_tag;   break;
			case GAME:     color_str = ingame_color_tag;  break;
			case SEL_GAME: color_str = selgame_color_tag; break;
		}
		if (preferences::iconize_list()) {
			switch (u_itor->relation) {
				case NEUTRAL: img_str = imgpre + "neutral.png" + COLUMN_SEPARATOR; break;
				case IGNORED: img_str = imgpre + "ignore.png"  + COLUMN_SEPARATOR; break;
				case FRIEND:  img_str = imgpre + "friend.png"  + COLUMN_SEPARATOR; break;
				case ME:      img_str = imgpre + "self.png"    + COLUMN_SEPARATOR; break;
			}
		}
		user_strings.push_back(u_itor->name);
		menu_strings.push_back(img_str + color_str + u_itor->name + game_str + HELP_STRING_SEPARATOR + u_itor->name + game_str);
		u_itor++;
	}

	set_user_list(user_strings, silent);
	set_user_menu_items(menu_strings);
}

void ui::set_selected_game(const std::string game_id)
{
	// reposition the player list to show the players in the selected game
	if (preferences::sort_list() && (selected_game_ != game_id)) {
		users_menu_.move_selection(0);
	}
	selected_game_ = game_id;
}

void ui::set_user_menu_items(const std::vector<std::string>& list)
{
	users_menu_.set_items(list,true,true);
}

void ui::set_user_list(const std::vector<std::string>& list, bool silent)
{
	if(!silent) {
		if(list.size() < user_list_.size()) {
			sound::play_UI_sound(game_config::sounds::user_leave);
		} else if(list.size() > user_list_.size()) {
			sound::play_UI_sound(game_config::sounds::user_arrive);
		}
	}

	user_list_ = list;
}

void ui::append_to_title(const std::string& text) {
	title_.set_text(title_.get_text() + text);
}

const gui::label& ui::title() const
{
	return title_;
}


}// namespace mp
