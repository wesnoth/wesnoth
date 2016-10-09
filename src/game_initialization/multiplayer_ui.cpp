/*
   Copyright (C) 2005 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "construct_dialog.hpp"
#include "video.hpp"
#include "formatter.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/multiplayer/mp_cmd_wrapper.hpp"
#include "gui/dialogs/loadscreen.hpp"
#include "lobby_preferences.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "multiplayer.hpp"
#include "multiplayer_ui.hpp"
#include "multiplayer_lobby.hpp" //needed for dynamic cast when implementing the lobby_sounds preference
#include "mp_ui_alerts.hpp"
#include "wml_separators.hpp"
#include "formula/string_utils.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "team.hpp"
#include "sdl/utils.hpp"
#include "sdl/rect.hpp"
#include "wesnothd_connection.hpp"

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

static lg::log_domain log_mp("mp/main");
#define DBG_MP LOG_STREAM(debug, log_mp)

namespace {
	/** The maximum number of messages in the chat history. */
	const size_t max_messages = 256;

	class user_menu_style : public gui::menu::imgsel_style {
	public:
		user_menu_style() : gui::menu::imgsel_style("dialogs/selection", false,
										   0x000000, 0x4a4440, 0x999999,
										   0.0, 0.2, 0.2),
										   item_size_(sdl::empty_rect)
		{}
		virtual void init();
		virtual SDL_Rect item_size(const std::string& /*item*/) const { return item_size_; }
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

std::string get_color_string(int id)
{
	std::string prefix = team::get_side_highlight(id);
	std::map<std::string, t_string>::iterator name = game_config::team_rgb_name.find(std::to_string(id + 1));
	if(name != game_config::team_rgb_name.end()){
		return prefix + name->second;
	}else{
		return prefix + _("Invalid Color");
	}
}

std::string get_color_string(const std::string& id)
{
	std::map<std::string, color_range>::iterator i_color = game_config::team_rgb_range.find(id);
	std::map<std::string, t_string>::iterator i_name = game_config::team_rgb_name.find(id);
	bool has_color = i_color != game_config::team_rgb_range.end();
	bool has_name= i_name != game_config::team_rgb_name.end();

	return rgb2highlight(has_color ? i_color->second.mid() : 0x00FF0000) + (has_name ? std::string(i_name->second) : _("Invalid Color"));
}

// TODO: should probably move this somewhere more general
std::string get_color_string_pango(const std::string& id)
{
	const auto color = game_config::team_rgb_colors.find(id);
	const auto name  = game_config::team_rgb_name.find(id);

	const bool has_color = color != game_config::team_rgb_colors.end();
	const bool has_name  = name  != game_config::team_rgb_name.end();

	if(has_color && has_name) {
		return formatter() << "<span color='" << rgb2highlight_pango(color->second[0]) << "'>" << name->second << "</span>";
	}

	return _("Invalid Color");
}

chat::chat() :
	message_history_(),
	last_update_()
{
}

void chat::add_message(const time_t& time, const std::string& user,
		const std::string& message)
{
	message_history_.push_back(msg(time, user, message));

	while (message_history_.size() > max_messages) {
		message_history_.pop_front();

		if (last_update_ > 0)
			last_update_--;
	}
}

void chat::init_textbox(gui::textbox& textbox)
{
	for(msg_hist::const_iterator itor = message_history_.begin();
			itor != message_history_.end(); ++itor) {
		textbox.append_text(format_message(*itor), true, color_message(*itor));
	}

	last_update_ = message_history_.size();
}

void chat::update_textbox(gui::textbox& textbox)
{
	//DBG_MP << "update_textbox...\n";
	for(msg_hist::const_iterator itor = message_history_.begin() + last_update_;
			itor != message_history_.end(); ++itor) {
		textbox.append_text(format_message(*itor), true, color_message(*itor));
	}
	//DBG_MP << "update_textbox end\n";

	last_update_ = message_history_.size();
}

void chat::clear_history()
{
	message_history_.clear();
	last_update_ = 0;
}

std::string chat::format_message(const msg& message)
{
	std::string msg_text = message.message;
	if(message.user == "server"
	|| message.user.substr(0,29) == "whisper: server message from ") {
		std::string::const_iterator after_markup =
			font::parse_markup(message.message.begin(), message.message.end(), nullptr, nullptr, nullptr);

		msg_text = std::string(after_markup,message.message.end());
	}
	if(message.message.substr(0,3) == "/me") {
		return preferences::get_chat_timestamp(message.time) + "<" + message.user
				+ msg_text.substr(3) + ">\n";
	} else {
		return preferences::get_chat_timestamp(message.time) + "<" + message.user
				+ "> " + msg_text + "\n";
	}
}

SDL_Color chat::color_message(const msg& message) {
	SDL_Color c = font::NORMAL_COLOR;
	// Normal users are not allowed to color their messages
	if(message.user == "server"
	|| message.user.substr(0,29) == "whisper: server message from ") {
		font::parse_markup(message.message.begin(), message.message.end(), nullptr, &c, nullptr);
	// Highlight private messages too
	} else if(message.user.substr(0,8) == "whisper:") {
	    c = font::LABEL_COLOR;
	}
	return c;
}

ui::ui(CVideo& video, twesnothd_connection* wesnothd_connection, const std::string& title, const config& cfg, chat& c, config& gamelist) :
	gui::widget(video),
	video_(video),
	wesnothd_connection_(wesnothd_connection),
	initialized_(false),
	gamelist_initialized_(false),

	game_config_(cfg),
	chat_(c),
	gamelist_(gamelist),

	title_(video_, title, font::SIZE_LARGE, font::TITLE_COLOR),
	entry_textbox_(video_, 100),
	chat_textbox_(video_, 100, "", false),
	users_menu_(video_, std::vector<std::string>(), false, -1, -1, nullptr, &umenu_style),

	user_list_(),
	selected_game_(""),
	selected_user_(""),
	selected_user_changed_(false),

	result_(CONTINUE),
	gamelist_refresh_(false),
	lobby_clock_(0),
	whisper_warnings_(),
	plugins_context_(nullptr)
{
	const SDL_Rect area = sdl::create_rect(0
			, 0
			, video.getx()
			, video.gety());
	users_menu_.set_numeric_keypress_selection(false);
	set_location(area);
}

void ui::process_network()
{
	config data;
	if(receive_from_server(data)) {
		process_network_data(data);
	}

	//apply diffs at a set interval
	if(gamelist_refresh_ && SDL_GetTicks() - lobby_clock_ > game_config::lobby_refresh)
	{
		const cursor::setter cursor_setter(cursor::WAIT);
		gamelist_updated(false);
		gamelist_refresh_ = false;
		lobby_clock_ = SDL_GetTicks();
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

	surface background(image::get_image("misc/lobby.png"));
	background = scale_surface(background, video().getx(), video().gety());
	if(background == nullptr)
		return;
	sdl_blit(background, nullptr, video().getSurface(), nullptr);
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
		chat_textbox_.set_edit_target(&entry_textbox_);
		initialized_ = true;
	}
	hide_children(false);
}

void ui::process_event()
{
}

void ui::handle_event(const SDL_Event& event)
{
	if (gui2::tloadscreen::displaying()) {
		return;
	}
	gui::widget::handle_event(event);

	if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
		SDL_Rect new_location;
		new_location.x = 0;
		new_location.y = 0;
		new_location.w = event.window.data1;
		new_location.h = event.window.data2;
		set_location(new_location);
	}

	if(event.type == SDL_KEYDOWN) {
		handle_key_event(event.key);
	}
	if(users_menu_.double_clicked()) {
		std::string usr_text = user_list_[users_menu_.selection()];
		Uint32 show_time = SDL_GetTicks();

		// Hack: for some reason the help string stays visible for ever
		/** @todo find out why the help string stays visible and fix it */
		video().clear_all_help_strings();

		gui2::tmp_cmd_wrapper dlg(_("Selected user: ") + usr_text);
		dlg.show(video());

		std::stringstream msg;
		switch(dlg.get_retval()) {
			case -1:
				if(!dlg.message().empty()) msg << "/msg " << usr_text << ' ' << dlg.message();
				break;
			case 1:
				msg << "/friend " << usr_text;
				break;
			case 2:
				msg << "/ignore " << usr_text;
				break;
			case 3:
				msg << "/remove " << usr_text;
				break;
			case 4:
				msg << "/query status " << usr_text;
				break;
			case 5:
				msg << "/query kick " << usr_text;
				if(!dlg.reason().empty()) msg << ' ' << dlg.reason();
				break;
			case 6:
				msg << "/query kban " << usr_text;
				if(!dlg.time().empty()) msg << ' ' << dlg.time();
				if(!dlg.reason().empty()) msg << ' ' << dlg.reason();
		}

		chat_handler::do_speak(msg.str());

		if(show_time + 60000 < SDL_GetTicks()) {
			//if the dialog has been open for a long time, refresh the lobby
			config request;
			request.add_child("refresh_lobby");
			send_to_server(request);
		}
	}
	if(users_menu_.selection() > 0 // -1 indicates an invalid selection
			&& selected_user_ != user_list_[users_menu_.selection()]) {
		selected_user_ = user_list_[users_menu_.selection()];
		selected_user_changed_ = true;
	}
}

void ui::add_chat_message(const time_t& time, const std::string& speaker, int /*side*/, const std::string& message, events::chat_handler::MESSAGE_TYPE /*type*/)
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

	add_chat_message(time(nullptr), preferences::login(),0, message);	//local echo
	send_to_server(data);
}

void ui::handle_key_event(const SDL_KeyboardEvent& event)
{
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
			std::string completion_list = utils::join(matches, " ");
			chat_.add_message(time(nullptr), "", completion_list);
			chat_.update_textbox(chat_textbox_);
		}
		entry_textbox_.set_text(text);
	}
}

void ui::process_message(const config& msg, const bool whisper) {
	const std::string& sender = msg["sender"];
	const std::string& message = msg["message"];
	std::string room = msg["room"];
	if (!preferences::parse_should_show_lobby_join(sender, message)) return;
	if (preferences::is_ignored(sender)) return;

	// Warn about people trying to whisper a player with the
	// whisper_friends_only option enabled.
	if (whisper &&
		preferences::whisper_friends_only() &&
		sender != "server" &&
		sender.find(' ') == std::string::npos && // "server message from foo"
		!preferences::is_friend(sender))
	{
		LOG_NW << "Accepting whispers from friends only, ignored whisper from " << sender << '\n';

		typedef std::map<std::string, time_t> timetable;
		timetable::const_iterator i = whisper_warnings_.find(sender);

		time_t last_warning = 0;
		const time_t cur_time = time(nullptr);
		static const time_t warning_duration = 5 * 60;

		if (i != whisper_warnings_.end()) {
			last_warning = i->second;
		}

		//
		// Don't warn if it's been less than warning_duration seconds since
		// the last warning. Also, make sure the clock isn't running backwards,
		// warn anyway if it is.
		//
		// We don't need to hande the case where preferences change between
		// whispers because the lobby instance gets recreated along with the
		// table after closing the preferences dialog.
		//
		if (last_warning && last_warning < cur_time && cur_time - last_warning < warning_duration) {
			return;
		}

		utils::string_map symbols;
		symbols["sender"] = sender;

		chat_.add_message(cur_time,
						  "server",
						  VGETTEXT("$sender is messaging you, and you accept whispers from friends only.", symbols));
		chat_.update_textbox(chat_textbox_);

		whisper_warnings_[sender] = cur_time;

		return;
	}

	preferences::parse_admin_authentication(sender, message);

	bool is_lobby = dynamic_cast<mp::lobby*>(this) != nullptr;

	if (whisper || utils::word_match(message, preferences::login())) {
		mp_ui_alerts::private_message(is_lobby, sender, message);
	} else if (preferences::is_friend(sender)) {
		mp_ui_alerts::friend_message(is_lobby, sender, message);
	} else if (sender == "server") {
		mp_ui_alerts::server_message(is_lobby, sender, message);
	} else {
		mp_ui_alerts::public_message(is_lobby, sender, message);
	}

	std::string prefix;

	if(whisper) {
		utils::string_map symbols;
		symbols["sender"] = msg["sender"].str();
		prefix = VGETTEXT("whisper: $sender", symbols);
	}
	else {
		prefix = msg["sender"].str();
	}

	if (!room.empty()) room = room + ": ";

	chat_.add_message(time(nullptr), room + prefix, msg["message"]);
	chat_.update_textbox(chat_textbox_);

	config temp = msg;
	temp["whisper"] = whisper;
	plugins_manager::get()->notify_event("chat", temp); //notify plugins of the network message
}

void ui::process_network_data(const config& data)
{
	if (const config &error = data.child("error")) {
		throw wesnothd_error(error["message"]);
	} else if (const config &message = data.child("message")) {
		process_message(message);
	} else if (const config &whisper = data.child("whisper")) {
		process_message(whisper, true);
	} else if(data.child("gamelist")) {
		const cursor::setter cursor_setter(cursor::WAIT);
		gamelist_initialized_ = true;
		gamelist_ = data;
		gamelist_updated(false);
		gamelist_refresh_ = false;
		lobby_clock_ = SDL_GetTicks();
	} else if (const config &gamelist_diff = data.child("gamelist_diff")) {
		if(gamelist_initialized_) {
			try {
				gamelist_.apply_diff(gamelist_diff);
			} catch(config::error& e) {
				ERR_CF << "Error while applying the gamelist diff: '"
					<< e.message << "' Getting a new gamelist.\n";
				send_to_server(config("refresh_lobby"));
			}
			gamelist_refresh_ = true;
		}
	} else if (const config &room_join = data.child("room_join")) {
		if (room_join["player"] == preferences::login()) {
			chat_.add_message(time(nullptr), "server",
				"You have joined the room '" + room_join["room"].str() + "'");
		} else {
			chat_.add_message(time(nullptr), "server",
				room_join["player"].str() + " has joined the room '" + room_join["room"].str() + "'");
		}
		chat_.update_textbox(chat_textbox_);
	} else if (const config &room_part = data.child("room_part")) {
		if (room_part["player"] == preferences::login()) {
			chat_.add_message(time(nullptr), "server",
				"You have left the room '" + room_part["room"].str() + "'");
		} else {
			chat_.add_message(time(nullptr), "server",
				room_part["player"].str() + " has left the room '" + room_part["room"].str() + "'");
		}
		chat_.update_textbox(chat_textbox_);
	} else if (const config &room_query_response = data.child("room_query_response")) {
		if (const config &ms = room_query_response.child("members")) {
			std::stringstream ss;
			ss << "Room " << room_query_response["room"].str() << " members: ";
			for (const config& m : ms.child_range("member")) {
				ss << m["name"] << " ";
			}
			chat_.add_message(time(nullptr), "server", ss.str());
			chat_.update_textbox(chat_textbox_);
		}
		if (const config &rooms = room_query_response.child("rooms")) {
			std::stringstream ss;
			ss << "Rooms: ";
			for (const config& room : rooms.child_range("room")) {
				ss << room["name"].str() << "(" << room["size"].str() << ") ";
			}
			chat_.add_message(time(nullptr), "server", ss.str());
			chat_.update_textbox(chat_textbox_);
		}
	}
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
	umenu_style.set_width(xscale(159));
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

bool ui::user_info::operator> (const user_info& b) const
{
	//FIXME: to cmpare names, use translation::compare from gettext.hpp
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
	std::list<user_info> u_list;

	for (const config &user : gamelist_.child_range("user"))
	{
		user_info u_elem;
		u_elem.name = user["name"].str();
		u_elem.state = user["available"].to_bool(true) ? LOBBY : GAME;
		u_elem.registered = user["registered"].to_bool();
		u_elem.game_id = user["game_id"].str();
		u_elem.location = user["location"].str();
		if (!u_elem.game_id.empty() && u_elem.game_id == selected_game_) {
			u_elem.state = SEL_GAME;
		}
		if (u_elem.name == preferences::login()) {
			u_elem.relation = ME;
		} else if (preferences::is_ignored(u_elem.name)) {
			u_elem.relation = IGNORED;
		} else if (preferences::is_friend(u_elem.name)) {
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

	// for now I just disregard the above till I know something better,
	// it works for me anyways
	const std::string registered_user_tag = "~";

	std::string const imgpre = IMAGE_PREFIX + std::string("misc/status-");
	std::vector<std::string> user_strings;
	std::vector<std::string> menu_strings;

	std::list<user_info>::const_iterator u_itor = u_list.begin();
	while (u_itor != u_list.end()) {
		const std::string name_str = u_itor->name +
				((u_itor->state == LOBBY) ? "" : " (" + u_itor->location + ")");
		std::string img_str = "";
		std::string color_str = "";
		std::string reg_str = "";
		switch (u_itor->state) {
			case LOBBY:    color_str = lobby_color_tag;   break;
			case GAME:     color_str = ingame_color_tag;  break;
			case SEL_GAME: color_str = selgame_color_tag; break;
		}
		if (preferences::iconize_list()) {
			switch (u_itor->relation) {
				case NEUTRAL: img_str = imgpre + "neutral.png" + IMG_TEXT_SEPARATOR; break;
				case IGNORED: img_str = imgpre + "ignore.png"  + IMG_TEXT_SEPARATOR; break;
				case FRIEND:  img_str = imgpre + "friend.png"  + IMG_TEXT_SEPARATOR; break;
				case ME:      img_str = imgpre + "self.png"    + IMG_TEXT_SEPARATOR; break;
			}
		}
		reg_str = u_itor->registered ? registered_user_tag : "";
		user_strings.push_back(u_itor->name);
		menu_strings.push_back(img_str + reg_str + color_str + name_str + HELP_STRING_SEPARATOR + name_str);
		++u_itor;
	}

	set_user_list(user_strings, silent);
	set_user_menu_items(menu_strings);
}

void ui::set_selected_game(const std::string& game_id)
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

	// Try to keep selected player
	std::vector<std::string>::const_iterator i =
			std::find(user_list_.begin(), user_list_.end(), selected_user_);
	if(i != user_list_.end()) {
		users_menu_.move_selection_keeping_viewport(i - user_list_.begin());
	}
}

void ui::set_user_list(const std::vector<std::string>& list, bool silent)
{
	if(!silent) {
		bool is_lobby = dynamic_cast<mp::lobby*>(this) != nullptr;

		if(list.size() < user_list_.size()) {
			mp_ui_alerts::player_leaves(is_lobby);
		} else if(list.size() > user_list_.size()) {
			mp_ui_alerts::player_joins(is_lobby);
		}
	}

	user_list_ = list;
}

std::string ui::get_selected_user_game()
{
	const config &u = gamelist_.find_child("user", "name", selected_user_);
	if (u) return u["game_id"];
	return std::string();
}

void ui::append_to_title(const std::string& text) {
	title_.set_text(title_.get_text() + text);
}

const gui::label& ui::title() const
{
	return title_;
}

plugins_context * ui::get_plugins_context()
{
	return plugins_context_.get();
}

void ui::send_to_server(const config& cfg)
{
	if (wesnothd_connection_) {
		wesnothd_connection_->send_data(cfg);
	}
}

bool ui::receive_from_server(config& cfg)
{
	return wesnothd_connection_ && wesnothd_connection_->receive_data(cfg);
}

}// namespace mp
