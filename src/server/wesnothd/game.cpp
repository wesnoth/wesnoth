/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "server/wesnothd/game.hpp"

#include "filesystem.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "serialization/chrono.hpp"
#include "server/wesnothd/player_network.hpp"
#include "server/wesnothd/server.hpp"
#include "utils/math.hpp"

#include <iomanip>
#include <sstream>

#include <boost/coroutine/exceptions.hpp>

static lg::log_domain log_server("server");
#define ERR_GAME LOG_STREAM(err, log_server)
#define WRN_GAME LOG_STREAM(warn, log_server)
#define LOG_GAME LOG_STREAM(info, log_server)
#define DBG_GAME LOG_STREAM(debug, log_server)

static lg::log_domain log_config("config");
#define WRN_CONFIG LOG_STREAM(warn, log_config)

namespace
{
void split_conv_impl(std::vector<int>& res, const simple_wml::string_span& span)
{
	if(!span.empty()) {
		res.push_back(span.to_int());
	}
}

template<typename TResult, typename TConvert>
std::vector<TResult> split(const simple_wml::string_span& val, TConvert conv, const char c = ',')
{
	std::vector<TResult> res;

	simple_wml::string_span::const_iterator i1 = val.begin();
	simple_wml::string_span::const_iterator i2 = i1;

	while(i2 != val.end()) {
		if(*i2 == c) {
			conv(res, simple_wml::string_span(i1, i2));
			++i2;
			i1 = i2;
		} else {
			++i2;
		}
	}

	conv(res, simple_wml::string_span(i1, i2));
	return res;
}
}

namespace wesnothd
{

int game::id_num = 1;
int game::db_id_num = 1;

game::game(wesnothd::server& server, player_connections& player_connections,
		player_iterator host,
		const std::string& name,
		bool save_replays,
		const std::string& replay_save_path)
	: server(server)
	, player_connections_(player_connections)
	, id_(id_num++)
	, db_id_(db_id_num++)
	, name_(name)
	, password_()
	, owner_(host)
	, players_()
	, observers_()
	, muted_observers_()
	, sides_()
	, side_controllers_()
	, nsides_(0)
	, started_(false)
	, level_()
	, history_()
	, chat_history_()
	, description_(nullptr)
	, description_updated_(false)
	, current_turn_(0)
	, current_side_index_(0)
	, next_side_index_(0)
	, num_turns_(0)
	, all_observers_muted_(false)
	, bans_()
	, name_bans_()
	, players_not_advanced_()
	, termination_()
	, save_replays_(save_replays)
	, replay_save_path_(replay_save_path)
	, rng_()
	, last_choice_request_id_(-1) /* or maybe 0 ? it shouldn't matter*/
{
	players_.push_back(owner_);

	// Mark the host as unavailable in the lobby.
	owner_->info().mark_available(id_, name_);
	owner_->info().set_status(player::PLAYING);
}

game::~game()
{
	try {
		save_replay();

		for(player_iterator user_ptr : all_game_users()) {
			remove_player(user_ptr, false, true);
		}

		clear_history();
	} catch(const boost::coroutines::detail::forced_unwind&) {
		ERR_GAME << "Caught forced_unwind in game destructor!";
	} catch(...) {
		ERR_GAME << "Caught other exception in game destructor: " << utils::get_unknown_exception_type();
	}
}

/** returns const so that operator [] won't create empty keys if not existent */
static const simple_wml::node& get_multiplayer(const simple_wml::node& root)
{
	if(const simple_wml::node* multiplayer = root.child("multiplayer")) {
		return *multiplayer;
	} else {
		ERR_GAME << "no [multiplayer] found. Returning root";
		return root;
	}
}

bool game::allow_observers() const
{
	return get_multiplayer(level_.root())["observer"].to_bool(true);
}

bool game::is_observer(player_iterator player) const
{
	return std::find(observers_.begin(), observers_.end(), player) != observers_.end();
}

bool game::is_muted_observer(player_iterator player) const
{
	if(!is_observer(player)) {
		return false;
	}

	if(all_observers_muted_) {
		return true;
	}

	return std::find(muted_observers_.begin(), muted_observers_.end(), player) != muted_observers_.end();
}

bool game::is_player(player_iterator player) const
{
	return std::find(players_.begin(), players_.end(), player) != players_.end();
}

std::string game::username(player_iterator iter) const
{
	return iter->info().name();
}

std::string game::list_users(const user_vector& users) const
{
	std::string list;

	for(auto user : users) {
		if(!list.empty()) {
			list += ", ";
		}

		list += user->info().name();
	}

	return list;
}

void game::perform_controller_tweaks()
{
	const simple_wml::node::child_list& sides = get_sides_list();

	DBG_GAME << "****\n Performing controller tweaks. sides = ";
	DBG_GAME << debug_sides_info();
	DBG_GAME << "****";

	update_side_data(); // Necessary to read the level_ and get sides_, etc. updated to match

	for(unsigned side_index = 0; side_index < sides.size(); ++side_index) {
		simple_wml::node& side = *sides[side_index];

		if(side["controller"] != side_controller::none) {
			if(!sides_[side_index]) {
				sides_[side_index] = owner_;
				std::stringstream msg;
				msg << "Side " << side_index + 1
					<< " had no controller during controller tweaks! The host was assigned control.";

				LOG_GAME << msg.str() << " (game id: " << id_ << ", " << db_id_ << ")";
				send_and_record_server_message(msg.str());
			}

			std::string user_name = username(*sides_[side_index]);

			// Issue change_controller command, transferring this side to its owner with proper name and controller.
			// Ensures that what the server now thinks is true is effected on all of the clients.
			//
			// In the server controller tweaks, we want to avoid sending controller change messages to the host.
			// Doing this has the negative consequence that all of the AI side names are given the owners name.
			// Therefore, if the side belongs to the host, we pass player_left = true, otherwise player_left = false.
			change_controller(side_index, *sides_[side_index], user_name, sides_[side_index] == owner_);

			// next line change controller types found in level_ to be what is appropriate for an observer at game
			// start.
			side.set_attr("is_local", "no");

			if(!sides_[side_index]) {
				std::stringstream msg;
				msg << "Side " << side_index + 1 << " had no controller AFTER controller tweaks! Ruh Roh!";
				LOG_GAME << msg.str() << " (game id: " << id_ << ", " << db_id_ << ")";
			}
		}
	}

	// This is the last time that update_side_data will actually run, as now the game will start and
	// started_ will be true.
	update_side_data();

	// TODO: Does it matter that the server is telling the host to change a bunch of sides?
	// According to playturn.cpp, the host should ignore all such messages. Still might be better
	// not to send them at all, although not if it complicates the server code.
}

void game::start_game(player_iterator starter)
{
	const simple_wml::node::child_list& sides = get_sides_list();
	DBG_GAME << "****\n Starting game. sides = ";
	DBG_GAME << debug_sides_info();
	DBG_GAME << "****";

	// If the game was already started we're actually advancing.
	const bool advance = started_;
	started_ = true;
	// Prevent inserting empty keys when reading.
	const simple_wml::node& multiplayer = get_multiplayer(level_.root());

	const bool save = multiplayer["savegame"].to_bool();
	LOG_GAME
		<< starter->client_ip() << "\t" << starter->name() << "\t"
		<< (advance ? "advanced" : "started") << (save ? " reloaded" : "") << " game:\t\"" << name_ << "\" (" << id_
		<< ", " << db_id_ << ") with: " << list_users(players_)
		<< ". Settings: map: " << multiplayer["mp_scenario"]
		<< "\tera: "       << multiplayer["mp_era"]
		<< "\tXP: "        << multiplayer["experience_modifier"]
		<< "\tGPV: "       << multiplayer["mp_village_gold"]
		<< "\tfog: "       << multiplayer["mp_fog"]
		<< "\tshroud: "    << multiplayer["mp_shroud"]
		<< "\tobservers: " << multiplayer["observer"]
		<< "\tshuffle: "   << multiplayer["shuffle_sides"]
		<< "\ttimer: "     << multiplayer["mp_countdown"]
		<< (multiplayer["mp_countdown"].to_bool()
			? "\treservoir time: "   + multiplayer["mp_countdown_reservoir_time"].to_string()
				+ "\tinit time: "    + multiplayer["mp_countdown_init_time"].to_string()
				+ "\taction bonus: " + multiplayer["mp_countdown_action_bonus"].to_string()
				+ "\tturn bonus: "   + multiplayer["mp_countdown_turn_bonus"].to_string()
			: "");


	for(unsigned side_index = 0; side_index < sides.size(); ++side_index) {
		simple_wml::node& side = *sides[side_index];

		if(side["controller"] != side_controller::none) {
			if(side_index >= sides_.size()) {
				continue;
			}

			if(!sides_[side_index]) {
				std::stringstream msg;
				msg << "Side " << side_index + 1
				    << " has no controller but should! The host needs to assign control for the game to proceed past "
					   "that side's turn.";

				LOG_GAME << msg.str() << " (game id: " << id_ << ", " << db_id_ << ")";
				send_and_record_server_message(msg.str());
			}
		}
	}

	DBG_GAME << "Number of sides: " << nsides_;
	int turn = 1;
	int side = 0;
	int next_side = 0;

	// Savegames have a snapshot that tells us which side starts.
	if(const simple_wml::node* snapshot = level_.root().child("snapshot")) {
		turn = lexical_cast_default<int>((*snapshot)["turn_at"], 1);
		side = lexical_cast_default<int>((*snapshot)["playing_team"], 0);
		if((*snapshot)["init_side_done"].to_bool(false)) {
			next_side = -1;
		} else {
			next_side = side;
		}
		LOG_GAME << "Reload from turn: " << turn << ". Current side is: " << side + 1 << ". Next side is: " << next_side + 1;
	}

	current_turn_ = turn;
	current_side_index_ = side;
	next_side_index_ = next_side;

	num_turns_ = lexical_cast_default<int>((*starting_pos(level_.root()))["turns"], -1);

	update_turn_data();
	clear_history();
	clear_chat_history();

	// Send [observer] tags for all observers that are already in the game.
	send_observerjoins();
}

bool game::send_taken_side(simple_wml::document& cfg, const simple_wml::node* side) const
{
	const std::size_t side_index = (*side)["side"].to_int() - 1;

	// Negative values are casted (int -> std::size_t) to very high values to this check will fail for them too.
	if(side_index >= sides_.size()) {
		return false;
	}

	if(sides_[side_index]) {
		return false;
	}

	// We expect that the host will really use our proposed side number. (He could do different...)
	cfg.root().set_attr_dup("side", (*side)["side"]);

	// Tell the host which side the new player should take.
	server.send_to_player(owner_, cfg);
	return true;
}

bool game::take_side(player_iterator user)
{
	DBG_GAME << "take_side...";

	if(started_) {
		return false;
	}

	simple_wml::document cfg;
	cfg.root().set_attr_dup("name", user->name().c_str());

	// FIXME: The client code (multiplayer.wait.cpp) the host code (connect_engine.cpp) and the server code
	// (this file) has this code to figure out a fitting side for new players, this is clearly too much.
	// Check if we can figure out a fitting side.
	const simple_wml::node::child_list& sides = get_sides_list();

	for(const simple_wml::node* side : sides) {
		if(((*side)["controller"] == side_controller::human || (*side)["controller"] == side_controller::reserved)
				&& (*side)["current_player"] == user->name().c_str()) {

			if(send_taken_side(cfg, side)) {
				return true;
			}
		}
	}

	// If there was no fitting side just take the first available.
	for(const simple_wml::node* side : sides) {
		if((*side)["controller"] == side_controller::human) {
			if(send_taken_side(cfg, side)) {
				return true;
			}
		}
	}

	DBG_GAME << "take_side: there are no more sides available";

	// If we get here we couldn't find a side to take
	return false;
}

void game::reset_sides()
{
	side_controllers_.clear();
	sides_.clear();

	nsides_ = get_sides_list().size();

	side_controllers_.resize(nsides_);
	sides_.resize(nsides_);
}

void game::update_side_data()
{
	// Added by iceiceice: since level_ will now reflect how an observer views the replay start
	// position and not the current position, the sides_, side_controllers_, players_ info should
	// not be updated from the level_ after the game has started. Controller changes are now stored
	// in the history, so an observer that joins will get up to date that way.
	if(started_) {
		return;
	}

	DBG_GAME << "update_side_data...";
	DBG_GAME << debug_player_info();

	// Remember everyone that is in the game.
	const user_vector users = all_game_users();

	players_.clear();
	observers_.clear();

	reset_sides();

	const simple_wml::node::child_list& level_sides = get_sides_list();

	// For each user:
	// * Find the username.
	// * Find the side this username corresponds to.
	for(auto iter : users) {

		bool side_found = false;
		for(unsigned side_index = 0; side_index < level_sides.size(); ++side_index) {
			const simple_wml::node* side = level_sides[side_index];

			if(side_index >= sides_.size() || sides_[side_index]) {
				continue;
			}

			const simple_wml::string_span& player_id = (*side)["player_id"];
			const simple_wml::string_span& controller = (*side)["controller"];

			auto type = side_controller::get_enum(controller.to_string());
			if(player_id == iter->info().name().c_str()) {
				// We found invalid [side] data. Some message would be cool.
				if(controller != side_controller::human && controller != side_controller::ai) {
					continue;
				}

				if(type) {
					side_controllers_[side_index] = *type;
				}
				sides_[side_index] = iter;
				side_found = true;
			} else if(iter == owner_ && controller == side_controller::none) {
				// the *user == owner_ check has no effect,
				// it's just an optimisation so that we only do this once.
				if(type) {
					side_controllers_[side_index] = *type;
				}
			}
		}

		if(side_found) {
			players_.push_back(iter);
			iter->info().set_status(player::PLAYING);
		} else {
			observers_.push_back(iter);
			iter->info().set_status(player::OBSERVING);
		}
	}

	DBG_GAME << debug_player_info();
}

void game::transfer_side_control(player_iterator player, const simple_wml::node& cfg)
{
	DBG_GAME << "transfer_side_control...";

	if(!is_player(player) && player != owner_) {
		send_server_message("You cannot change controllers: not a player.", player);
		return;
	}

	// Check the side number.
	const unsigned int side_num = cfg["side"].to_int();
	if(side_num < 1 || side_num > sides_.size()) {
		std::ostringstream msg;
		msg << "The side number has to be between 1 and " << sides_.size() << ".";
		send_server_message(msg.str(), player);
		return;
	}

	if(side_num > get_sides_list().size()) {
		send_server_message("Invalid side number.", player);
		return;
	}

	const simple_wml::string_span& newplayer_name = cfg["player"];
	auto old_player = sides_[side_num - 1];
	const std::string& controller_type = cfg["to"].to_string();

	const std::string old_player_name = old_player ? username(*old_player) : "null";

	// Not supported anymore.
	if(newplayer_name.empty()) {
		std::stringstream msg;
		msg << "Received invalid [change_controller] with no player= attribute specified";
		DBG_GAME << msg.str();
		send_server_message(msg.str(), player);
		return;
	}

	// Check if the sender actually owns the side he gives away or is the host.
	if(!(player == old_player || player == owner_)) {
		std::stringstream msg;
		msg << "You can't give away side " << side_num << ". It's controlled by '" << old_player_name << "' not you.";
		DBG_GAME << msg.str();
		send_server_message(msg.str(), player);
		return;
	}

	// find the player that is passed control
	auto newplayer { find_user(newplayer_name) };

	// Is he in this game?
	if(!newplayer || !is_member(*newplayer)) {
		send_server_message(newplayer_name.to_string() + " is not in this game", player);
		return;
	}

	if(newplayer == old_player) {
		// if the player is unchanged and the controller type (human or ai) is also unchanged then nothing to do
		// else only need to change the controller type rather than the player who controls the side
		// :droid provides a valid controller_type; :control provides nothing since it's only tranferring control between players regardless of type
		auto type = side_controller::get_enum(controller_type);
		if(!type || type == side_controllers_[side_num - 1]) {
			std::stringstream msg;
			msg << "Side " << side_num << " is already controlled by " << newplayer_name << ".";
			send_server_message(msg.str(), player);
			return;
		} else {
			side_controllers_[side_num - 1] = *side_controller::get_enum(controller_type);
			change_controller_type(side_num - 1, *newplayer, (*newplayer)->info().name());
			return;
		}
	}

	sides_[side_num - 1].reset();

	// If the old player lost his last side, make him an observer.
	if(old_player && std::find(sides_.begin(), sides_.end(), old_player) == sides_.end() && is_player(*old_player)) {
		observers_.push_back(*old_player);

		(*old_player)->info().set_status(player::OBSERVING);
		utils::erase(players_, old_player);

		// Tell others that the player becomes an observer.
		send_and_record_server_message(old_player_name + " becomes an observer.");

		// Update the client side observer list for everyone except old player.
		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", old_player_name.c_str());
		send_data(observer_join, *old_player);
	}

	change_controller(side_num - 1, *newplayer, (*newplayer)->info().name(), false);

	// If we gave the new side to an observer add him to players_.
	if(is_observer(*newplayer)) {
		players_.push_back(*newplayer);
		(*newplayer)->info().set_status(player::PLAYING);
		utils::erase(observers_, newplayer);
		// Send everyone but the new player the observer_quit message.
		send_observerquit(*newplayer);
	}
}

void game::change_controller(
		const std::size_t side_index, player_iterator player, const std::string& player_name, const bool player_left)
{
	DBG_GAME << __func__ << "...";

	const std::string& side = lexical_cast_default<std::string, std::size_t>(side_index + 1);
	sides_[side_index] = player;

	if(player_left && side_controllers_[side_index] == side_controller::type::ai) {
		// Automatic AI side transfer.
	} else {
		if(started_) {
			send_and_record_server_message(player_name + " takes control of side " + side + ".");
		}
	}

	auto response = change_controller_type(side_index, player, player_name);

	if(started_) {
		// the purpose of these records is so that observers, replay viewers, etc get controller updates correctly
		record_data(response->clone());
	}

	// Tell the new player that he controls this side now.
	// Just don't send it when the player left the game. (The host gets the
	// side_drop already.)
	if(!player_left) {
		response->root().child("change_controller")->set_attr("is_local", "yes");
		server.send_to_player(player, *response.get());
	}
}

std::unique_ptr<simple_wml::document> game::change_controller_type(const std::size_t side_index, player_iterator player, const std::string& player_name)
{
	const std::string& side = std::to_string(side_index + 1);
	simple_wml::document response;
	simple_wml::node& change = response.root().add_child("change_controller");

	change.set_attr_dup("side", side.c_str());
	change.set_attr_dup("player", player_name.c_str());

	change.set_attr_dup("controller", side_controller::get_string(side_controllers_[side_index]).c_str());
	change.set_attr("is_local", "no");

	send_data(response, player);
	return response.clone();
}

void game::notify_new_host()
{
	const std::string owner_name = username(owner_);
	simple_wml::document cfg;
	cfg.root().add_child("host_transfer");

	std::string message = owner_name + " has been chosen as the new host.";
	server.send_to_player(owner_, cfg);
	send_and_record_server_message(message);
}

void game::describe_slots()
{
	if(started_ || description_ == nullptr) {
		return;
	}

	int available_slots = 0;
	int num_sides = get_sides_list().size();
	int i = 0;

	for(const simple_wml::node* side : get_sides_list()) {
		if(((*side)["allow_player"].to_bool(true) == false) || (*side)["controller"] == side_controller::none) {
			num_sides--;
		} else if(!sides_[i]) {
			++available_slots;
		}

		++i;
	}

	simple_wml::node& slots_cfg = description_for_writing()->child_or_add("slot_data");

	slots_cfg.set_attr_int("vacant", available_slots);
	slots_cfg.set_attr_int("max", num_sides);
}

bool game::player_is_banned(player_iterator player, const std::string& name) const
{
	auto ban = std::find(bans_.begin(), bans_.end(), player->client_ip());
	auto name_ban = std::find(name_bans_.begin(), name_bans_.end(), name);

	return ban != bans_.end() || name_ban != name_bans_.end();
}

void game::mute_all_observers()
{
	all_observers_muted_ = !all_observers_muted_;
	if(all_observers_muted_) {
		send_and_record_server_message("All observers have been muted.");
	} else {
		send_and_record_server_message("Muting of all observers has been removed.");
	}
}

void game::send_muted_observers(player_iterator user) const
{
	if(all_observers_muted_) {
		send_server_message("All observers are muted.", user);
		return;
	}

	std::string muted_nicks = list_users(muted_observers_);

	send_server_message("Muted observers: " + muted_nicks, user);
}

void game::mute_observer(const simple_wml::node& mute, player_iterator muter)
{
	if(muter != owner_) {
		send_server_message("You cannot mute: not the game host.", muter);
		return;
	}

	const simple_wml::string_span& username = mute["username"];
	if(username.empty()) {
		send_muted_observers(muter);
		return;
	}

	auto user { find_user(username) };

	/*
	 * @todo FIXME: Maybe rather save muted nicks as a set of strings and also allow muting of usernames not in the game.
	 */
	if(!user || !is_observer(*user)) {
		send_server_message("Observer '" + username.to_string() + "' not found.", muter);
		return;
	}

	// Prevent muting ourselves.
	if(user == muter) {
		send_server_message("Don't mute yourself, silly.", muter);
		return;
	}

	if(is_muted_observer(*user)) {
		send_server_message(username.to_string() + " is already muted.", muter);
		return;
	}

	LOG_GAME << muter->client_ip() << "\t" << game::username(muter) << " muted: " << username << " ("
	         << (*user)->client_ip() << ")\tin game:\t\"" << name_ << "\" (" << id_ << ", " << db_id_ << ")";

	muted_observers_.push_back(*user);
	send_and_record_server_message(username.to_string() + " has been muted.");
}

void game::unmute_observer(const simple_wml::node& unmute, player_iterator unmuter)
{
	if(unmuter != owner_) {
		send_server_message("You cannot unmute: not the game host.", unmuter);
		return;
	}

	const simple_wml::string_span& username = unmute["username"];
	if(username.empty()) {
		muted_observers_.clear();
		send_and_record_server_message("Everyone has been unmuted.");
		return;
	}

	auto user { find_user(username) };
	if(!user || !is_observer(*user)) {
		send_server_message("Observer '" + username.to_string() + "' not found.", unmuter);
		return;
	}

	if(!is_muted_observer(*user)) {
		send_server_message(username.to_string() + " is not muted.", unmuter);
		return;
	}

	LOG_GAME << unmuter->client_ip() << "\t" << game::username(unmuter) << " unmuted: " << username << " ("
	         << (*user)->client_ip() << ")\tin game:\t\"" << name_ << "\" (" << id_ << ", " << db_id_ << ")";

	utils::erase(muted_observers_, user);
	send_and_record_server_message(username.to_string() + " has been unmuted.");
}

void game::send_leave_game(player_iterator user) const
{
	static simple_wml::document leave_game("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	server.send_to_player(user, leave_game);
}

utils::optional<player_iterator> game::kick_member(const simple_wml::node& kick, player_iterator kicker)
{
	if(kicker != owner_) {
		send_server_message("You cannot kick: not the game host", kicker);
		return {};
	}

	const simple_wml::string_span& username = kick["username"];
	auto user { find_user(username) };

	if(!user || !is_member(*user)) {
		send_server_message("'" + username.to_string() + "' is not a member of this game.", kicker);
		return {};
	} else if(user == kicker) {
		send_server_message("Don't kick yourself, silly.", kicker);
		return {};
	} else if((*user)->info().is_moderator()) {
		send_server_message("You're not allowed to kick a moderator.", kicker);
		return {};
	}

	LOG_GAME << kicker->client_ip() << "\t" << game::username(kicker) << "\tkicked: " << username << " ("
	         << (*user)->client_ip() << ")\tfrom game:\t\"" << name_ << "\" (" << id_ << ", " << db_id_ << ")";

	send_and_record_server_message(username.to_string() + " has been kicked.");

	// Tell the user to leave the game.
	send_leave_game(*user);
	remove_player(*user);
	return user;
}

utils::optional<player_iterator> game::ban_user(const simple_wml::node& ban, player_iterator banner)
{
	if(banner != owner_) {
		send_server_message("You cannot ban: not the game host", banner);
		return {};
	}

	const simple_wml::string_span& username = ban["username"];
	auto user { find_user(username) };

	if(!user) {
		send_server_message("User '" + username.to_string() + "' not found.", banner);
		return {};
	} else if(user == banner) {
		send_server_message("Don't ban yourself, silly.", banner);
		return {};
	} else if(player_is_banned(*user, username.to_string())) {
		send_server_message("'" + username.to_string() + "' is already banned.", banner);
		return {};
	} else if((*user)->info().is_moderator()) {
		send_server_message("You're not allowed to ban a moderator.", banner);
		return {};
	}

	LOG_GAME << banner->client_ip() << "\t" << game::username(banner) << "\tbanned: " << username << " ("
	         << (*user)->client_ip() << ")\tfrom game:\t\"" << name_ << "\" (" << id_ << ", " << db_id_ << ")";

	bans_.push_back((*user)->client_ip());
	name_bans_.push_back(username.to_string());
	send_and_record_server_message(username.to_string() + " has been banned.");

	if(is_member(*user)) {
		// tell the user to leave the game.
		send_leave_game(*user);
		remove_player(*user);
		return user;
	}

	// Don't return the user if he wasn't in this game.
	return {};
}

void game::unban_user(const simple_wml::node& unban, player_iterator unbanner)
{
	if(unbanner != owner_) {
		send_server_message("You cannot unban: not the game host.", unbanner);
		return;
	}

	const simple_wml::string_span& username = unban["username"];
	auto user { find_user(username) };

	if(!user) {
		send_server_message("User '" + username.to_string() + "' not found.", unbanner);
		return;
	}

	if(!player_is_banned(*user, username.to_string())) {
		send_server_message("'" + username.to_string() + "' is not banned.", unbanner);
		return;
	}

	LOG_GAME
		<< unbanner->client_ip() << "\t" << unbanner->info().name()
		<< "\tunbanned: " << username << " (" << (*user)->client_ip() << ")\tfrom game:\t\"" << name_ << "\" ("
		<< id_ << ", " << db_id_ << ")";

	utils::erase(bans_, (*user)->client_ip());
	utils::erase(name_bans_, username.to_string());
	send_and_record_server_message(username.to_string() + " has been unbanned.");
}

void game::process_message(simple_wml::document& data, player_iterator user)
{
	simple_wml::node* const message = data.root().child("message");
	assert(message);
	message->set_attr_dup("sender", user->info().name().c_str());
	const simple_wml::string_span& msg = (*message)["message"];
	chat_message::truncate_message(msg, *message);
	// Save chat as history to be sent to newly joining players
	chat_history_.push_back(data.clone());
	send_data(data, user);
}

bool game::is_legal_command(const simple_wml::node& command, player_iterator user)
{
	const bool is_player = this->is_player(user);
	const bool is_host = user == owner_;
	const bool is_current = is_current_player(user);

	if(command.has_attr("from_side")) {
		const std::size_t from_side_index = command["from_side"].to_int() - 1;

		// Someone pretends to be the server...
		if(command["from_side"] == "server") {
			return false;
		}

		if(get_side_player(from_side_index) != user) {
			return false;
		}
	}

	if(command.child("init_side")) {
		// If side init was already done, the rhs returns nullopt so this also return fale in this case.
		return get_side_player(get_next_side_index()) == user;
	}

	if(is_current) {
		return true;
	}

	// Only single commands allowed.
	// NOTE: some non-dependent commands like move,attack.. might contain a [checkup] tag after their first data.
	// But those packages are only sent by the currently active player which we check above.
	if(!command.one_child()) {
		return false;
	}

	// Chatting is never an illegal command.
	if(command.child("speak")) {
		return true;
	}
	if(command.child("surrender")) {
		const simple_wml::string_span& sn = command.child("surrender")->attr("side_number");
		if(sn.is_null()) {
			return false;
		}

		std::size_t side_number = sn.to_int();
		if(get_side_player(side_number) != user) {
			return false;
		} else {
			return true;
		}
	}

	// AKA it's generated by get_user_input for example [global_variable]
	if(is_player && command.has_attr("dependent") && command.has_attr("from_side")) {
		return true;
	}

	if((is_player || is_host) && (
		command.child("label") ||
		command.child("clear_labels") ||
		command.child("rename") ||
		command.child("countdown_update")
	)) {
		return true;
	}

	return false;
}

bool game::process_turn(simple_wml::document& data, player_iterator user)
{
	// DBG_GAME << "processing commands: '" << cfg << "'";
	if(!started_) {
		return false;
	}

	simple_wml::node* const turn = data.root().child("turn");
	bool turn_ended = false;

	// Any private 'speak' commands must be repackaged separate
	// to other commands, and re-sent, since they should only go
	// to some clients.
	bool repackage = false;
	int index = 0;
	std::vector<int> marked;

	const simple_wml::node::child_list& commands = turn->children("command");

	for(simple_wml::node* command : commands) {
		DBG_GAME << "game " << id_ << ", " << db_id_ << " received [" << (*command).first_child() << "] from player '" << username(user)
				 << "'(" << ") during turn " << current_side_index_ + 1 << "," << current_turn_;
		if(!is_legal_command(*command, user)) {
			LOG_GAME << "ILLEGAL COMMAND in game: " << id_ << ", " << db_id_ << " (((" << simple_wml::node_to_string(*command)
					 << ")))";

			std::stringstream msg;
			msg << "Removing illegal command '" << (*command).first_child().to_string() << "' from: " << username(user)
				<< ". Current player is: " << (current_player() ? username(*current_player()) : "<none>") << " (" << current_side_index_ + 1 << "/" << nsides_
				<< ").";
			LOG_GAME << msg.str() << " (game id: " << id_ << ", " << db_id_ << ")";
			send_and_record_server_message(msg.str());

			marked.push_back(index - marked.size());
		} else if((*command).child("speak")) {
			simple_wml::node& speak = *(*command).child("speak");
			if(!speak["to_sides"].empty() || is_muted_observer(user)) {
				DBG_GAME << "repackaging...";
				repackage = true;
			}

			const simple_wml::string_span& msg = speak["message"];
			chat_message::truncate_message(msg, speak);

			// Force the description to be correct,
			// to prevent spoofing of messages.
			speak.set_attr_dup("id", user->info().name().c_str());

			// Also check the side for players.
			if(is_player(user)) {
				const std::size_t side_index = speak["side"].to_int() - 1;

				if(side_index >= sides_.size() || sides_[side_index] != user) {
					if(user == current_player()) {
						speak.set_attr_dup("side", lexical_cast_default<std::string>(current_side() + 1).c_str());
					} else {
						const auto s = std::find(sides_.begin(), sides_.end(), user);
						speak.set_attr_dup("side", lexical_cast_default<std::string>(s - sides_.begin() + 1).c_str());
					}
				}
			}
		} else if (command->child("surrender")) {
			std::size_t side_index = 0;

			for(auto s : sides_) {
				if(s == user) {
					break;
				}
				++side_index;
			}

			if(side_index < sides_.size()) {
				simple_wml::document cfg;
				std::string playername;
				cfg.root().set_attr_dup("side", std::to_string(side_index + 1).c_str());

				// figure out who gets the surrendered side
				if(owner_ == user) {
					auto new_side_index = (side_index + 1) % sides_.size();
					auto new_owner = sides_[new_side_index];
					while(!new_owner) {
						new_side_index = (new_side_index + 1) % sides_.size();
						if(new_side_index == side_index) {
							ERR_GAME << "Ran out of sides to surrender to.";
							return false;
						}
						new_owner = sides_[new_side_index];
					}
					playername = username(*new_owner);
				} else {
					playername = username(owner_);
				}

				cfg.root().set_attr_dup("player", playername.c_str());
				transfer_side_control(user, cfg.root());
			}
			send_and_record_server_message(username(user) + " has surrendered.");
		} else if(is_current_player(user) && (*command).child("end_turn")) {
			simple_wml::node& endturn = *(*command).child("end_turn");
			end_turn(endturn["next_player_number"].to_int());
		} else if(command->child("init_side")) {
			//["side_number"]
			init_turn();
		}

		++index;
	}

	for(const int j : marked) {
		turn->remove_child("command", j);
	}

	if(turn->no_children()) {
		return false;
	}

	if(!repackage) {
		record_data(data.clone());
		send_data(data, user);
		return turn_ended;
	}

	for(simple_wml::node* command : commands) {
		simple_wml::node* const speak = (*command).child("speak");
		if(speak == nullptr) {
			auto mdata = std::make_unique<simple_wml::document>();
			simple_wml::node& mturn = mdata->root().add_child("turn");
			(*command).copy_into(mturn.add_child("command"));
			send_data(*mdata, user);
			record_data(std::move(mdata));
			continue;
		}

		const simple_wml::string_span& to_sides = (*speak)["to_sides"];

		// Anyone can send to the observer team.
		if(is_muted_observer(user) && to_sides != game_config::observer_team_name.c_str()) {
			send_server_message("You have been muted, others can't see your message!", user);
			continue;
		}

		auto message = std::make_unique<simple_wml::document>();
		simple_wml::node& message_turn = message->root().add_child("turn");
		simple_wml::node& message_turn_command = message_turn.add_child("command");
		message_turn_command.set_attr("undo", "no");
		speak->copy_into(message_turn_command.add_child("speak"));

		if(to_sides.empty()) {
			send_data(*message, user);
			record_data(std::move(message));
		} else if(to_sides == game_config::observer_team_name) {
			send_to_players(*message, observers_, user);
			record_data(std::move(message));
		} else {
			send_data_sides(*message, to_sides, user);
		}
	}

	return turn_ended;
}

void game::handle_random_choice()
{
	uint32_t seed = rng_.get_next_random();

	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex << seed;

	auto mdata = std::make_unique<simple_wml::document>();
	simple_wml::node& turn = mdata->root().add_child("turn");
	simple_wml::node& command = turn.add_child("command");
	simple_wml::node& random_seed = command.add_child("random_seed");

	random_seed.set_attr_dup("new_seed", stream.str().c_str());
	random_seed.set_attr_int("request_id", last_choice_request_id_);

	command.set_attr("from_side", "server");
	command.set_attr("dependent", "yes");

	send_data(*mdata, {});
	record_data(std::move(mdata));
}

void game::handle_add_side_wml()
{
	++nsides_;
	side_controllers_.push_back(side_controller::type::none);
	sides_.emplace_back();
}

void game::handle_controller_choice(const simple_wml::node& req)
{
	const std::size_t side_index = req["side"].to_int() - 1;
	auto new_controller = side_controller::get_enum(req["new_controller"].to_string());
	auto old_controller = side_controller::get_enum(req["old_controller"].to_string());

	if(!new_controller) {
		send_and_record_server_message(
			"Could not handle [request_choice] [change_controller] with invalid controller '" + req["new_controller"].to_string() + "'");
		return;
	}

	if(!old_controller) {
		send_and_record_server_message(
			"Could not handle [request_choice] [change_controller] with invalid controller '" + req["old_controller"].to_string() + "'");
		return;
	}

	if(old_controller != this->side_controllers_[side_index]) {
		send_and_record_server_message(
			"Found unexpected old_controller= '" + side_controller::get_string(*old_controller) + "' in [request_choice] [change_controller]");
	}

	if(side_index >= sides_.size()) {
		send_and_record_server_message(
			"Could not handle [request_choice] [change_controller] with invalid side '" + req["side"].to_string() + "'");
		return;
	}

	const bool was_null = this->side_controllers_[side_index] == side_controller::type::none;
	const bool becomes_null = new_controller == side_controller::type::none;

	if(was_null) {
		assert(!sides_[side_index]);
		sides_[side_index] = current_player();
	}

	if(becomes_null) {
		sides_[side_index].reset();
	}

	side_controllers_[side_index] = *new_controller;

	auto mdata = std::make_unique<simple_wml::document>();
	simple_wml::node& turn = mdata->root().add_child("turn");
	simple_wml::node& command = turn.add_child("command");
	simple_wml::node& change_controller_wml = command.add_child("change_controller_wml");

	change_controller_wml.set_attr_dup("controller", side_controller::get_string(*new_controller).c_str());
	change_controller_wml.set_attr("is_local", "yes");
	change_controller_wml.set_attr_int("request_id", last_choice_request_id_);

	command.set_attr("from_side", "server");
	command.set_attr("dependent", "yes");

	if(sides_[side_index]) {
		server.send_to_player((*sides_[side_index]), *mdata);
	}

	change_controller_wml.set_attr("is_local", "no");

	send_data(*mdata, sides_[side_index]);
	record_data(std::move(mdata));
}

void game::handle_choice(const simple_wml::node& data, player_iterator user)
{

	if(!started_) {
		return;
	}

	// note, that during end turn events, it's side=1 for the server but side= side_count() on the clients.

	// Otherwise we allow observers to cause OOS for the playing clients by sending
	// server choice requests based on incompatible local changes. To solve this we block
	// server choice requests from observers.
	if(user != owner_ && !is_player(user)) {
		return;
	}

	// since we reset the last_choice_request_id_ when a new scenario is loaded,
	// the code would otherwise wrongly accept these requests from client in old
	// scenarios. which would result on oos.
	if(players_not_advanced_.find(&*user) != players_not_advanced_.end()) {
		return;
	}

	int request_id = lexical_cast_default<int>(data["request_id"], -10);
	if(request_id <= last_choice_request_id_) {
		// We gave already an anwer to this request.
		return;
	}

	DBG_GAME << "answering choice request " << request_id << " by player "
			 << user->info().name();
	last_choice_request_id_ = request_id;

	if(data.child("random_seed")) {
		handle_random_choice();
	} else if(const simple_wml::node* ccw = data.child("change_controller_wml")) {
		handle_controller_choice(*ccw);
	} else if(data.child("add_side_wml")) {
		handle_add_side_wml();
	} else {
		send_and_record_server_message("Found unknown server choice request: [" + data.first_child().to_string() + "]");
	}
}

void game::process_whiteboard(simple_wml::document& data, player_iterator user)
{
	if(!started_ || !is_player(user)) {
		return;
	}

	const simple_wml::node& wb_node = *data.child("whiteboard");

	// Ensure "side" attribute match with user
	const simple_wml::string_span& to_sides = wb_node["to_sides"];
	std::size_t const side_index = wb_node["side"].to_int() - 1;

	if(side_index >= sides_.size() || sides_[side_index] != user) {
		std::ostringstream msg;
		msg << "Ignoring illegal whiteboard data, sent from user '" << user->info().name()
		    << "' which had an invalid side '" << side_index + 1 << "' specified" << std::endl;

		const std::string& msg_str = msg.str();

		LOG_GAME << msg_str;
		send_and_record_server_message(msg_str);
		return;
	}

	send_data_sides(data, to_sides, user);
}

void game::process_change_turns_wml(simple_wml::document& data, player_iterator user)
{
	if(!started_ || !is_player(user)) {
		return;
	}

	const simple_wml::node& ctw_node = *data.child("change_turns_wml");
	const int current_turn = ctw_node["current"].to_int();
	const int num_turns = ctw_node["max"].to_int();
	if(num_turns > 10000 || current_turn > 10000) {
		// ignore this to prevent errors related to integer overflow.
		return;
	}

	current_turn_ = current_turn;
	num_turns_ = num_turns;

	assert(static_cast<int>(this->current_turn()) == current_turn);

	simple_wml::node& turns_cfg = description_for_writing()->child_or_add("turn_data");

	ctw_node.copy_into(turns_cfg);

	// Don't send or store this change, all players should have gotten it by wml.
}

void game::end_turn(int new_side)
{
	if(new_side > 0) {
		next_side_index_ = new_side - 1;
	} else {
		next_side_index_ = current_side_index_ + 1;
	}
}

void game::init_turn()
{
	int new_side = get_next_side_index();
	if(new_side > current_side_index_) {
		++current_turn_;
	}

	current_side_index_ = new_side;
	next_side_index_ = -1;

	update_turn_data();

}

void game::update_turn_data()
{
	if(description_ == nullptr) {
		return;
	}

	simple_wml::node& turns_cfg = description_for_writing()->child_or_add("turn_data");

	turns_cfg.set_attr_int("current", current_turn());
	turns_cfg.set_attr_int("max", num_turns_);
}

bool game::add_player(player_iterator player, bool observer)
{
	if(is_member(player)) {
		ERR_GAME << "ERROR: Player is already in this game.";
		return false;
	}

	auto user = player;

	DBG_GAME << debug_player_info();

	bool became_observer = false;
	if(!started_ && !observer && take_side(user)) {
		DBG_GAME << "adding player...";
		players_.push_back(player);

		user->info().set_status(player::PLAYING);

		send_and_record_server_message(user->info().name() + " has joined the game.", player);
	} else if(!allow_observers() && !user->info().is_moderator()) {
		return false;
	} else {
		if(!observer) {
			became_observer = true;
			observer = true;
		}

		DBG_GAME << "adding observer...";
		observers_.push_back(player);
		if(!allow_observers()) {
			send_and_record_server_message(
				user->info().name() + " is now observing the game.", player);
		}

		simple_wml::document observer_join;
		observer_join.root()
			.add_child("observer")
			.set_attr_dup("name", user->info().name().c_str());

		// Send observer join to everyone except the new observer.
		send_data(observer_join, player);
	}

	LOG_GAME
		<< player->client_ip() << "\t" << user->info().name() << "\tjoined game:\t\""
		<< name_ << "\" (" << id_ << ", " << db_id_ << ")" << (observer ? " as an observer" : "") << ".";

	user->info().mark_available(id_, name_);
	user->info().set_status((observer) ? player::OBSERVING : player::PLAYING);
	DBG_GAME << debug_player_info();

	// Send the user the game data.
	server.send_to_player(player, level_);

	if(started_) {
		// Tell this player that the game has started
		static simple_wml::document start_game_doc("[start_game]\n[/start_game]\n", simple_wml::INIT_COMPRESSED);
		server.send_to_player(player, start_game_doc);

		// Send observer join of all the observers in the game to the new player
		// only once the game started. The client forgets about it anyway otherwise.
		send_observerjoins(player);

		// Send the player the history of the game to-date.
		send_history(player);
	} else {
		send_user_list();
		// Send the game chat log, regardless if observer or not
		send_chat_history(player);
	}

	const std::string clones = has_same_ip(player);
	if(!clones.empty()) {
		send_and_record_server_message(
			user->info().name() + " has the same IP as: " + clones);
	}

	if(became_observer) {
		// in case someone took the last slot right before this player
		send_server_message("You are an observer.", player);
	}

	return true;
}

bool game::remove_player(player_iterator player, const bool disconnect, const bool destruct)
{
	if(!is_member(player)) {
		ERR_GAME << "ERROR: User is not in this game.";
		return false;
	}

	DBG_GAME << debug_player_info();
	DBG_GAME << "removing player...";

	const bool host = (player == owner_);
	const bool observer = is_observer(player);

	utils::erase(players_, player);
	utils::erase(observers_, player);
	players_not_advanced_.erase(&*player);

	const bool game_ended = players_.empty() || (host && !started_);

	auto user = player;

	LOG_GAME
		<< user->client_ip()
		<< "\t" << user->info().name()
		<< ((game_ended && !(observer && destruct)) ? (started_ ? "\tended" : "\taborted") : "\thas left")
		<< " game:\t\"" << name_ << "\" (" << id_ << ", " << db_id_ << ")"
		<< (game_ended && started_ && !(observer && destruct)
			? " at turn: " + lexical_cast_default<std::string, std::size_t>(current_turn())
				+ " with reason: '" + termination_reason() + "'"
			: "")
		<< (observer ? " as an observer" : "") << (disconnect ? " and disconnected" : "") << ".";

	if(game_ended && started_ && !(observer && destruct)) {
		send_server_message_to_all(user->info().name() + " ended the game.", player);
	}

	if(game_ended || destruct) {
		owner_ = player_connections_.end();
		return game_ended;
	}

	// Don't mark_available() since the player got already removed from the
	// games_and_users_list_.
	if(!disconnect) {
		user->info().mark_available();
	}

	if(observer) {
		send_observerquit(user);
	} else {
		send_and_record_server_message(user->info().name()
			+ (disconnect ? " has disconnected." : " has left the game."), player);
	}

	// If the player was host choose a new one.
	if(host) {
		owner_ = players_.front();
		notify_new_host();
	}

	bool ai_transfer = false;

	// Look for all sides the player controlled and drop them.
	// (Give them to the host.
	for(unsigned side_index = 0; side_index < sides_.size(); ++side_index) {
		auto side = sides_[side_index];

		if(side != player) {
			continue;
		}

		if(side_controllers_[side_index] == side_controller::type::ai) {
			ai_transfer = true;
		}

		change_controller(side_index, owner_, username(owner_));

		// Check whether the host is actually a player and make him one if not.
		if(!is_player(owner_)) {
			DBG_GAME << "making the owner a player...";
			owner_->info().set_status(player::PLAYING);
			utils::erase(observers_, owner_);
			players_.push_back(owner_);
			send_observerquit(owner_);
		}

		// send the host a notification of removal of this side
		const std::string side_drop = lexical_cast_default<std::string, std::size_t>(side_index + 1);

		simple_wml::document drop;
		auto& node_side_drop = drop.root().add_child("side_drop");

		node_side_drop.set_attr_dup("side_num", side_drop.c_str());
		node_side_drop.set_attr_dup("controller", side_controller::get_string(side_controllers_[side_index]).c_str());

		DBG_GAME << "*** sending side drop: \n" << drop.output();

		server.send_to_player(owner_, drop);
	}

	if(ai_transfer) {
		send_and_record_server_message("AI sides transferred to host.");
	}

	DBG_GAME << debug_player_info();

	send_user_list(player);
	return false;
}

void game::send_user_list(utils::optional<player_iterator> exclude)
{
	// If the game hasn't started yet, then send all players a list of the users in the game.
	if(started_ /*|| description_ == nullptr*/) {
		return;
	}

	simple_wml::document cfg;
	simple_wml::node& list = cfg.root();

	for(auto pl : all_game_users()) {
		simple_wml::node& user = list.add_child("user");

		// Don't need to duplicate pl->info().name().c_str() because the
		// document will be destroyed by the end of the function
		user.set_attr_dup("name", pl->info().name().c_str());
		user.set_attr("host", is_owner(pl) ? "yes" : "no");
		user.set_attr("observer", is_observer(pl) ? "yes" : "no");
	}

	send_data(cfg, exclude);
}

void game::new_scenario(player_iterator sender)
{
	assert(sender == owner_);
	players_not_advanced_.clear();
	for(auto user_ptr : all_game_users()) {
		if(user_ptr != sender) {
			players_not_advanced_.insert(&*user_ptr);
		}
	}
	started_ = false;
}

void game::load_next_scenario(player_iterator user)
{
	send_server_message_to_all(user->info().name() + " advances to the next scenario", user);

	simple_wml::document cfg_scenario;
	simple_wml::node& next_scen = cfg_scenario.root().add_child("next_scenario");
	level_.root().copy_into(next_scen);
	next_scen.set_attr("started", started_ ? "yes" : "no");

	DBG_GAME << "****\n loading next scenario for a client. sides info = ";
	DBG_GAME << debug_sides_info();
	DBG_GAME << "****";

	//
	// Change the controller to match that client.
	//
	// FIXME: This breaks scenario transitions with mp connect screen shown.
	//
	// FIXME: This causes bugs, esp if controller have changed since the
	//        beginning of the next scenario
	//
	//        There are currently 2 possible ideas to fix this issue:
	//
	//          1) When the scenario starts, we store the controllers at that
	//             point and use that data when a client loads the the next
	//             scenario (here)
	//
	//          2) When a client loads the next scenario we send it the
	//             observers' starting point (meaning we don't change sides
	//             here), and then we send that side an automatic controller
	//             change later.
	//
	simple_wml::document doc_controllers;
	simple_wml::node& cfg_controllers = doc_controllers.root().add_child("controllers");

	for(const auto& side_user : sides_) {
		simple_wml::node& cfg_controller = cfg_controllers.add_child("controller");
		cfg_controller.set_attr("is_local", side_user == user ? "yes" : "no");
	}

	server.send_to_player(user, cfg_scenario);
	server.send_to_player(user, doc_controllers);

	players_not_advanced_.erase(&*user);

	// Send the player the history of the game to-date.
	send_history(user);

	// Send observer join of all the observers in the game to the user.
	send_observerjoins(user);
}

template<typename Container>
void game::send_to_players(simple_wml::document& data, const Container& players, utils::optional<player_iterator> exclude)
{
	for(const auto& player : players) {
		if(player != exclude) {
			server.send_to_player(player, data);
		}
	}
}

void game::send_data(simple_wml::document& data, utils::optional<player_iterator> exclude)
{
	send_to_players(data, all_game_users(), exclude);
}

void game::send_data_sides(simple_wml::document& data,
		const simple_wml::string_span& sides,
		utils::optional<player_iterator> exclude)
{
	std::vector<int> sides_vec = ::split<int>(sides, ::split_conv_impl);

	DBG_GAME << __func__ << "...";

	decltype(players_) filtered_players;

	std::copy_if(players_.begin(), players_.end(), std::back_inserter(filtered_players),
		[this, &sides_vec](player_iterator user) { return controls_side(sides_vec, user); });

	send_to_players(data, filtered_players, exclude);
}

bool game::controls_side(const std::vector<int>& sides, player_iterator player) const
{
	for(int side : sides) {
		std::size_t side_index = side - 1;

		if(side_index < sides_.size() && sides_[side_index] == player) {
			return true;
		}
	}

	return false;
}

std::string game::has_same_ip(player_iterator user) const
{
	const user_vector users = all_game_users();
	const std::string ip = user->client_ip();

	std::string clones;
	for(auto u : users) {
		if(ip == u->client_ip() && user != u) {
			clones += (clones.empty() ? "" : ", ") + u->info().name();
		}
	}

	return clones;
}

void game::send_observerjoins(utils::optional<player_iterator> player)
{
	for(auto ob : observers_) {
		if(ob == player) {
			continue;
		}

		simple_wml::document cfg;
		cfg.root().add_child("observer").set_attr_dup("name", ob->info().name().c_str());

		if(!player) {
			// Send to everyone except the observer in question.
			send_data(cfg, ob);
		} else {
			// Send to the (new) user.
			server.send_to_player(*player, cfg);
		}
	}
}

void game::send_observerquit(player_iterator observer)
{
	simple_wml::document observer_quit;

	// Don't need to dup the attribute because this document is short-lived.
	observer_quit.root()
		.add_child("observer_quit")
		.set_attr_dup("name", observer->info().name().c_str());

	send_data(observer_quit, observer);
}

void game::send_history(player_iterator player) const
{
	if(history_.empty()) {
		return;
	}

	// we make a new document based on converting to plain text and
	// concatenating the buffers.
	// TODO: Work out how to concentate buffers without decompressing.
	std::string buf;
	for(auto& h : history_) {
		buf += h->output();
	}

	try {
		auto doc = std::make_unique<simple_wml::document>(buf.c_str(), simple_wml::INIT_STATIC);
		doc->compress();

		server.send_to_player(player, *doc);

		history_.clear();
		history_.push_back(std::move(doc));
	} catch(const simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message;
	}
}

void game::send_chat_history(player_iterator player) const
{
	if(chat_history_.empty()) {
		return;
	}
	for(auto& h : chat_history_) {
		server.send_to_player(player, *h);
	}
}


static bool is_invalid_filename_char(char c)
{
	return !(isalnum(c) ||
		(c == '_') ||
		(c == '-') ||
		(c == '.') ||
		(c == '(') ||
		(c == ')') ||
		(c == '#') ||
		(c == ',') ||
		(c == '!') ||
		(c == '^') ||
		(c == '+') ||
		(c == '=') ||
		(c == '@') ||
		(c == '%') ||
		(c == '\'')
	);
}

std::string game::get_replay_filename()
{
	std::stringstream name;
	name << (*starting_pos(level_.root()))["name"] << " Turn " << current_turn() << " (" << db_id_ << ").bz2";
	std::string filename(name.str());
	std::replace(filename.begin(), filename.end(), ' ', '_');
	utils::erase_if(filename, is_invalid_filename_char);
	return filename;
}

void game::save_replay()
{
	if(!save_replays_ || !started_ || history_.empty()) {
		return;
	}

	std::string replay_commands;
	for(const auto& h : history_) {
		const simple_wml::node::child_list& turn_list = h->root().children("turn");

		for(const simple_wml::node* turn : turn_list) {
			replay_commands += simple_wml::node_to_string(*turn);
		}
	}

	history_.clear();

	std::stringstream replay_data;
	try {
		// level_.set_attr_dup("label", name.str().c_str());

		// Used by replays.wesnoth.org as of December 2017. No client usecases.
		level_.set_attr_dup("mp_game_title", name_.c_str());

		const bool has_old_replay = level_.child("replay") != nullptr;

		// If there is already a replay in the level_, which means this is a reloaded game,
		// then we don't need to add the [start] in the replay.
		replay_data
			<< level_.output()
			// This can result in having 2 [replay] at toplevel since level_ can contain one already. But the
			// client can handle this (simply merges them).
			<< "[replay]\n"
			// The [start] is generated at the clients and not sent over the network so we add it here.
			// It usually contains some checkup data that is used to check whether the calculated results
			// match the ones calculated in the replay. But that's not necessary
			<< (has_old_replay ? "" : "\t[command]\n\t\t[start]\n\t\t[/start]\n\t[/command]\n")
			<< replay_commands << "[/replay]\n";

		std::string replay_data_str = replay_data.str();
		simple_wml::document replay(replay_data_str.c_str(), simple_wml::INIT_STATIC);

		std::string filename = get_replay_filename();
		DBG_GAME << "saving replay: " << filename;

		filesystem::scoped_ostream os(filesystem::ostream_file(replay_save_path_ + filename));
		(*os) << replay.output_compressed(true);

		if(!os->good()) {
			ERR_GAME << "Could not save replay! (" << filename << ")";
		}
	} catch(const simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message;
	}
}

void game::record_data(std::unique_ptr<simple_wml::document> data)
{
	data->compress();
	history_.push_back(std::move(data));
}

void game::clear_history()
{
	history_.clear();
}

void game::clear_chat_history()
{
	chat_history_.clear();
}

void game::set_description(simple_wml::node* desc)
{
	description_ = desc;
	if(!password_.empty()) {
		description_->set_attr("password", "yes");
	}
}

void game::set_termination_reason(const std::string& reason)
{
	/*	if (reason == "out of sync") {
			simple_wml::string_span era;
			if (level_.child("era")) {
				era = level_.child("era")->attr("id");
			}
			termination_ = "out of sync - " + era.to_string();
		}*/
	if(termination_.empty()) {
		termination_ = reason;
	}
}

const user_vector game::all_game_users() const
{
	user_vector res;

	res.insert(res.end(), players_.begin(), players_.end());
	res.insert(res.end(), observers_.begin(), observers_.end());

	return res;
}

std::string game::debug_player_info() const
{
	std::stringstream result;
	result << "game id: " << id_ << ", " << db_id_ << "\n";

	for(auto user : players_) {
		result << "player: " << user->info().name().c_str() << "\n";
	}

	for(auto user : observers_) {
		result << "observer: " << user->info().name().c_str() << "\n";
	}

	return result.str();
}

std::string game::debug_sides_info() const
{
	std::stringstream result;
	result << "game id: " << id_ << ", " << db_id_ << "\n";
	const simple_wml::node::child_list& sides = get_sides_list();

	result << "\t\t level, server\n";

	for(const simple_wml::node* s : sides) {
		result
			<< "side " << (*s)["side"].to_int()
			<< " :\t" << (*s)["controller"].to_string()
			<< "\t, " << side_controller::get_string(side_controllers_[(*s)["side"].to_int() - 1])
			<< "\t( " << (*s)["current_player"].to_string() << " )\n";
	}

	return result.str();
}

utils::optional<player_iterator> game::find_user(const simple_wml::string_span& name)
{
	auto player { player_connections_.get<name_t>().find(name.to_string()) };
	if(player != player_connections_.get<name_t>().end()) {
		return player_connections_.project<0>(player);
	} else {
		return {};
	}
}

void game::send_and_record_server_message(const char* message, utils::optional<player_iterator> exclude)
{
	auto doc = std::make_unique<simple_wml::document>();
	send_server_message(message, {}, doc.get());
	send_data(*doc, exclude);

	if(started_) {
		record_data(std::move(doc));
	}
}

void game::send_server_message_to_all(const char* message, utils::optional<player_iterator> exclude)
{
	simple_wml::document doc;
	send_server_message(message, {}, &doc);
	send_data(doc, exclude);
}

void game::send_server_message(const char* message, utils::optional<player_iterator> player, simple_wml::document* docptr) const
{
	simple_wml::document docbuf;
	if(docptr == nullptr) {
		docptr = &docbuf;
	}

	simple_wml::document& doc = *docptr;

	if(started_) {
		simple_wml::node& cmd = doc.root().add_child("turn");
		simple_wml::node& cfg = cmd.add_child("command");
		cfg.set_attr("undo", "no");
		simple_wml::node& msg = cfg.add_child("speak");

		msg.set_attr("id", "server");
		msg.set_attr_dup("message", message);
		std::stringstream ss;
		ss << chrono::serialize_timestamp(std::chrono::system_clock::now());
		msg.set_attr_dup("time", ss.str().c_str());
	} else {
		simple_wml::node& msg = doc.root().add_child("message");

		msg.set_attr("sender", "server");
		msg.set_attr_dup("message", message);
	}

	if(player) {
		server.send_to_player(*player, doc);
	}
}

bool game::is_reload() const
{
	const simple_wml::node& multiplayer = get_multiplayer(level_.root());
	return multiplayer.has_attr("savegame") && multiplayer["savegame"].to_bool();
}

int game::get_next_side_index() const
{
	return get_next_nonempty(next_side_index_);
}

int game::get_next_nonempty(int side_index) const
{
	if(side_index == -1) {
		return -1;
	}
	if(nsides_ == 0) {
		return 0;
	}
	for(int i = 0; i < nsides_; ++i) {
		int res = modulo(side_index + i, nsides_, 0);
		if(side_controllers_[res] != side_controller::type::none) {
			return res;
		}
	}
	return -1;
}

} // namespace wesnothd
