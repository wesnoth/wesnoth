/* $Id$ */
/*
   Copyright (C)
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "dialogs.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "multiplayer_connect.hpp"
#include "preferences.hpp"
#include "statistics.hpp"
#include "show_dialog.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"

#define LOG_NW LOG_STREAM(info, network)
#define ERR_NW LOG_STREAM(err, network)
#define LOG_CF LOG_STREAM(info, config)
#define ERR_CF LOG_STREAM(err, config)

namespace {
	const char* controller_names[] = {
		"network",
		"human",
		"ai",
		"null"
	};
}

namespace mp {

connect::side::side(connect& parent, const config& cfg, int index) :
	parent_(&parent),

	cfg_(cfg),

	index_(index),

	player_number_(parent.video(), lexical_cast_default<std::string>(index+1, ""),
	               font::SIZE_LARGE, font::LOBBY_COLOUR),
	combo_controller_(parent.disp(), parent.player_types_),
	orig_controller_(parent.video(), cfg["description"], font::SIZE_SMALL),
	combo_faction_(parent.disp(), parent.player_factions_),
	combo_leader_(parent.disp(), std::vector<std::string>()),
	combo_team_(parent.disp(), parent.player_teams_),
	combo_colour_(parent.disp(), parent.player_colours_),
	slider_gold_(parent.video()),
	label_gold_(parent.video(), "100", font::SIZE_SMALL, font::LOBBY_COLOUR),
	enabled_(!parent_->params_.saved_game),
	changed_(false),
	llm_(parent.era_sides_, &parent.game_data_, enabled_ ? &combo_leader_ : NULL)
{
	if(enabled_) {
		controller_ = parent_->default_controller_;
	} else {
		for(size_t i = CNTR_NETWORK; i != CNTR_LAST; ++i) {
			if(cfg_["controller"] == controller_names[i]) {
				controller_ = (mp::controller)i;
				break;
			}
		}
	}

	slider_gold_.set_min(20);
	slider_gold_.set_max(1000);
	slider_gold_.set_increment(25);
	slider_gold_.set_value(lexical_cast_default<int>(cfg_["gold"], 100));
	slider_gold_.set_measurements(100, 16);

	combo_faction_.enable(enabled_);
	combo_leader_.enable(enabled_);
	combo_team_.enable(enabled_);
	combo_colour_.enable(enabled_);
	slider_gold_.hide(!enabled_);

	id_ = ""; // Id is reset, and not imported from loading savegames
	save_id_ = cfg_["save_id"];
	faction_ = lexical_cast_default<int>(cfg_["faction"], 0);
	std::vector<std::string>::const_iterator itor = std::find(parent_->team_names_.begin(), parent_->team_names_.end(), cfg_["team_name"]);
	if(itor == parent_->team_names_.end()) {
		wassert(!parent_->team_names_.empty());
		team_ = 0;
	} else {
		team_ = itor - parent_->team_names_.begin();
	}
	colour_ = lexical_cast_default<int>(cfg_["colour"], index_ + 1) - 1;
	gold_ = lexical_cast_default<int>(cfg_["gold"], 100);

	// "Faction name" hack
	if (!enabled_) {
		faction_ = 0;
		std::vector<std::string> pseudo_factions;
		pseudo_factions.push_back(cfg_["name"]);
		combo_faction_.set_items(pseudo_factions);
		combo_faction_.set_selected(0);

		// Hack: if there is a unit which can recruit, use it as a
		// leader. Necessary to display leader information when loading
		// saves.
		config::const_child_itors side_units = cfg.child_range("unit");
		std::string leader_type;
		for(;side_units.first != side_units.second; ++side_units.first) {
			if((**side_units.first)["canrecruit"] == "1") {
				leader_type = (**side_units.first)["type"];
				break;
			}
		}
		std::vector<std::string> leader_name_pseudolist;
		if(leader_type.empty()) {
			leader_name_pseudolist.push_back("-");
		} else {
			game_data::unit_type_map::const_iterator leader_name =
				parent_->game_data_.unit_types.find(leader_type);
			if(leader_name == parent_->game_data_.unit_types.end()) {
				leader_name_pseudolist.push_back("-");
			} else {
				leader_name_pseudolist.push_back(leader_name->second.language_name());
			}
		}
		combo_leader_.set_items(leader_name_pseudolist);
		combo_leader_.set_selected(0);
	}

	update_ui();
}

connect::side::side(const side& a) :
	parent_(a.parent_), cfg_(a.cfg_),
	index_(a.index_), id_(a.id_),  save_id_(a.save_id_),
	controller_(a.controller_),
	faction_(a.faction_), team_(a.team_), colour_(a.colour_),
	gold_(a.gold_), leader_(a.leader_), /* taken_(a.taken_), */
	player_number_(a.player_number_), combo_controller_(a.combo_controller_),
	orig_controller_(a.orig_controller_),
	combo_faction_(a.combo_faction_), combo_leader_(a.combo_leader_),
	combo_team_(a.combo_team_), combo_colour_(a.combo_colour_),
	slider_gold_(a.slider_gold_), label_gold_(a.label_gold_),
	enabled_(a.enabled_), changed_(a.changed_), llm_(a.llm_)
{
	llm_.set_combo(enabled_ ? &combo_leader_ : NULL);
}

void connect::side::add_widgets_to_scrollpane(gui::scrollpane& pane, int pos)
{
	pane.add_widget(&player_number_, 0, 5 + pos);
	pane.add_widget(&combo_controller_, 20, 5 + pos);
	pane.add_widget(&orig_controller_, 20 + (combo_controller_.width() - orig_controller_.width()) / 2, 35 + pos + (combo_leader_.height() - orig_controller_.height()) / 2);
	pane.add_widget(&combo_faction_, 135, 5 + pos);
	pane.add_widget(&combo_leader_, 135, 35 + pos);
	pane.add_widget(&combo_team_, 250, 5 + pos);
	pane.add_widget(&combo_colour_, 365, 5 + pos);
	pane.add_widget(&slider_gold_, 480, 5 + pos);
	pane.add_widget(&label_gold_, 490 + slider_gold_.width(), 8 + pos);
}

void connect::side::process_event()
{
	if(combo_controller_.changed() && combo_controller_.selected() >= 0) {
		if (combo_controller_.selected() == CNTR_LAST) {
			update_controller_ui();
		} else if (combo_controller_.selected() < CNTR_LAST) {
			// If the current side corresponds to an existing user,
			// we must kick it!

			if(id_ == preferences::login()) {
				update_controller_ui(); // Cannot kick game creator
			} else {
				// Update controller first, or else kick will reset it.
				controller_ = mp::controller(combo_controller_.selected());
				if(!id_.empty()) {
					parent_->kick_player(id_);
				}
				id_ = "";
				changed_ = true;
			}
		} else {
			size_t user = combo_controller_.selected() - CNTR_LAST - 1;

			// If the selected user already was attributed to
			// another side, find its side, and switch users.
			const std::string new_id = parent_->users_[user].name;
			if (new_id != id_) {
				int old_side = parent_->find_player_side(new_id);
				if (old_side != -1) {
					if (id_.empty()) {
						parent_->sides_[old_side].set_controller(controller_);
					} else {
						parent_->sides_[old_side].set_id(id_);
					}
				}
				id_ = new_id;
				controller_ = parent_->users_[user].controller;
				changed_ = true;
			}
		}
	}

	if(!enabled_)
		return;

	if (combo_faction_.changed() && combo_faction_.selected() >= 0) {
		faction_ = combo_faction_.selected();
		llm_.update_leader_list(faction_);
		changed_ = true;
	}
	if (combo_leader_.changed() && combo_leader_.selected() >= 0) {
		changed_ = true;
	}
	if (combo_team_.changed() && combo_team_.selected() >= 0) {
		team_ = combo_team_.selected();
		changed_ = true;
	}
	if (combo_colour_.changed() && combo_colour_.selected() >= 0) {
		colour_ = combo_colour_.selected();
		changed_ = true;
	}
	if (slider_gold_.value() != gold_) {
		gold_ = slider_gold_.value();
		label_gold_.set_text(lexical_cast_default<std::string>(gold_, "0"));
		changed_ = true;
	}
}

bool connect::side::changed()
{
	bool res = changed_;
	changed_ = false;
	return res;
}

bool connect::side::available() const
{
	return controller_ == CNTR_NETWORK && id_.empty();
}

void connect::side::update_controller_ui()
{
	if (id_.empty()) {
		combo_controller_.set_selected(controller_);
	} else {
		connected_user_list::iterator player = parent_->find_player(id_);

		if (player != parent_->users_.end()) {
			combo_controller_.set_selected(CNTR_LAST + 1 + (player - parent_->users_.begin()));
		} else {
			combo_controller_.set_selected(CNTR_NETWORK);
		}
	}

}

void connect::side::update_ui()
{
	update_controller_ui();

	if (combo_faction_.selected() != faction_ && combo_faction_.selected() >= 0) {
		combo_faction_.set_selected(faction_);
	}

	combo_team_.set_selected(team_);
	combo_colour_.set_selected(colour_);
	slider_gold_.set_value(gold_);
	label_gold_.set_text(lexical_cast_default<std::string>(gold_, "0"));
}

config connect::side::get_config() const
{
	config res = cfg_;

	// If the user is allowed to change type, faction, leader etc, then
	// import their new values in the config.
	if(enabled_ && !parent_->era_sides_.empty()) {
		// Merge the faction data to res
		res.append(*(parent_->era_sides_[faction_]));
	}
	if(cfg_["side"].empty() || cfg_["side"] != lexical_cast<std::string>(index_ + 1)) {
		res["side"] = lexical_cast<std::string>(index_ + 1);
	}
	res["controller"] = controller_names[controller_];
	res["description"] = id_;

	if (id_.empty()) {
		char const *description;
		switch(controller_) {
		case CNTR_NETWORK:
			description = N_("(Vacant slot)");
			break;
		case CNTR_LOCAL:
			if(enabled_ && cfg_["save_id"].empty()) {
				res["save_id"] = "local" + res["side"].str();
			}
			description = N_("Anonymous local player");
			break;
		case CNTR_COMPUTER:
			if(enabled_ && cfg_["save_id"].empty()) {
				res["save_id"] = "ai" + res["side"].str();
			}
			description = N_("Computer player");
			break;
		case CNTR_EMPTY:
			description = N_("(Empty slot)");
			break;
		default:
			wassert(false);
			break;
		}
		res["user_description"] = t_string(description, "wesnoth");
	} else {
		if(enabled_ && cfg_["save_id"].empty()) {
			res["save_id"] = id_;
		}

		res["user_description"] = id_;
	}

	if(enabled_) {
		if (leader_.empty()) {
			res["type"] = llm_.get_leader();
		} else {
			res["type"] = leader_;
		}
		// res["team"] = lexical_cast<std::string>(team_);
		res["team_name"] = parent_->team_names_[team_];
		res["colour"] = lexical_cast<std::string>(colour_ + 1);
		res["gold"] = lexical_cast<std::string>(gold_);

		res["fog"] = parent_->params_.fog_game ? "yes" : "no";
		res["shroud"] = parent_->params_.shroud_game ? "yes" : "no";
		res["share_maps"] = parent_->params_.share_maps ? "yes" : "no";
		res["share_view"] =  parent_->params_.share_view ? "yes" : "no";
		res["village_gold"] = lexical_cast<std::string>(parent_->params_.village_gold);

		res["allow_changes"] = "yes";
	} else {
		res["allow_changes"] = "no";
	}

	return res;
}

void connect::side::set_controller(mp::controller controller)
{
	controller_ = controller;
	id_ = "";

	update_ui();
}

mp::controller connect::side::get_controller() const
{
	return controller_;
}

void connect::side::update_user_list()
{
	bool name_present = false;

	std::vector<std::string> list = parent_->player_types_;
	list.push_back("----");

	connected_user_list::const_iterator itor;
	for (itor = parent_->users_.begin(); itor != parent_->users_.end();
			++itor) {
		list.push_back(itor->name);
		if (itor->name == id_)
			name_present = true;
	}

	if (name_present == false) {
		id_ = "";
	}

	combo_controller_.set_items(list);
	update_controller_ui();
}

const std::string& connect::side::get_id() const
{
	return id_;
}

void connect::side::set_id(const std::string& id)
{
	connected_user_list::iterator i = parent_->find_player(id);
	if (i != parent_->users_.end()) {
		id_ = id;
		controller_ = i->controller;
	}
	update_ui();
}

const std::string& connect::side::get_save_id() const
{
	return save_id_;
}

void connect::side::import_network_user(const config& data)
{
	id_ = data["name"];
	controller_ = CNTR_NETWORK;

	if (enabled_ && !parent_->era_sides_.empty()) {
		faction_ = lexical_cast_default<int>(data["faction"], 0);
		if (faction_ > int(parent_->era_sides_.size()))
			faction_ = 0;
		llm_.update_leader_list(faction_);
		llm_.set_leader(data["leader"]);
	}

	update_ui();
}

void connect::side::reset(mp::controller controller)
{
	id_ = "";
	controller_ = controller;

	if(enabled_ && !parent_->era_sides_.empty()) {
		faction_ = 0;
		llm_.update_leader_list(0);
	}

	update_ui();
}

void connect::side::resolve_random()
{
	if(!enabled_ || parent_->era_sides_.empty())
		return;

	if((*parent_->era_sides_[faction_])["random_faction"] == "yes") {

		// Builds the list of sides which aren't random
		std::vector<int> nonrandom_sides;
		for(config::child_iterator itor = parent_->era_sides_.begin();
				itor != parent_->era_sides_.end(); ++itor) {
			if((**itor)["random_faction"] != "yes") {
				nonrandom_sides.push_back(itor - parent_->era_sides_.begin());
			}
		}

		if (nonrandom_sides.size() == 0) {
			throw config::error(_("No non-random sides in the current era"));
		}

		faction_ = nonrandom_sides[rand() % nonrandom_sides.size()];
	}

	if (llm_.get_leader() == "random") {
		// Choose a random leader type.
		const config& fact = *parent_->era_sides_[faction_];
		std::vector<std::string> types = utils::split(fact["leader"]);
		if (!types.empty()) {
			const int lchoice = rand() % types.size();
			leader_ = types[lchoice];
		} else {
			utils::string_map i18n_symbols;
			i18n_symbols["faction"] = fact["name"];
			throw config::error(vgettext("Unable to find a leader type for faction $faction", i18n_symbols));
		}
	}
}

connect::connect(display& disp, const config& game_config, const game_data& data,
		chat& c, config& gamelist, const create::parameters& params,
		mp::controller default_controller) :
	mp::ui(disp, _("Game Lobby"), game_config, c, gamelist),

	game_data_(data),
	level_(),
	params_(params),

	team_prefix_(std::string(_("Team")) + " "),

	waiting_label_(video(), "", font::SIZE_SMALL, font::LOBBY_COLOUR),
	message_full_(false),
	default_controller_(default_controller),

	scroll_pane_(video()),
	type_title_label_(video(), _("Player/Type"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	faction_title_label_(video(), _("Faction"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	team_title_label_(video(), _("Team"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	colour_title_label_(video(), _("Color"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	gold_title_label_(video(), _("Gold"), font::SIZE_SMALL, font::LOBBY_COLOUR),

	ai_(video(), _("Computer vs Computer")),
	launch_(video(), _("I'm Ready")),
	cancel_(video(), _("Cancel"))
{
	// Send Initial information
	config response;
	config& create_game = response.add_child("create_game");
	create_game["name"] = params.name;
	network::send_data(response);

	load_game();
	if(get_result() == QUIT)
		return;
	lists_init();

	// Adds the current user as default user.
	users_.push_back(connected_user(preferences::login(), CNTR_LOCAL, 0));
	update_user_combos();
	if(sides_.empty()) {
		throw config::error(_("The scenario is invalid because it has no sides."));
	}

	int side_choice = 0;
	for(side_list::const_iterator s = sides_.begin(); s != sides_.end(); ++s) {
		if(s->get_save_id() == preferences::login()) {
			side_choice = s - sides_.begin();
		}
	}
	sides_[side_choice].set_id(preferences::login());

	update_playerlist_state();

	// Updates the "level_" variable, now that sides are loaded
	update_level();
	gamelist_updated(true);

	// If we are connected, send data to the connected host
	network::send_data(level_);
}

void connect::process_event()
{
	bool changed = false;

	for(size_t n = 0; n != sides_.size(); ++n) {
		sides_[n].process_event();
		if (sides_[n].changed())
			changed = true;
	}

	if (cancel_.pressed()) {
		if(network::nconnections() > 0) {
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg);
		}

		set_result(QUIT);
		return;
	}

	if (ai_.pressed()) {
		for(size_t m = 0; m != sides_.size(); ++m) {
			if(sides_[m].get_id() != preferences::login())
				kick_player(sides_[m].get_id());
			sides_[m].set_controller(CNTR_COMPUTER);
		}
		changed = true;
	}

	if (launch_.pressed()) {
		if (!sides_available())
			set_result(mp::ui::PLAY);
	}

	// If something has changed in the level config, send it to the
	// network:
	if (changed) {
		update_playerlist_state();
		update_and_send_diff();
	}
}

const game_state& connect::get_state()
{
	return state_;
}

void connect::start_game()
{
	// Resolves the "random faction" and "random message"
	for (side_list::iterator itor = sides_.begin(); itor != sides_.end();
			++itor) {

		itor->resolve_random();
	}

	config lock;
	lock.add_child("stop_updates");
	network::send_data(lock);

	// Re-sends the whole level
	update_and_send_diff();

	// Build the gamestate object after updating the level
	level_to_gamestate(level_, state_);

	config cfg;
	cfg.add_child("start_game");
	network::send_data(cfg);
}

void connect::hide_children(bool hide)
{
	ui::hide_children(hide);

	waiting_label_.hide(hide);
	// Hiding the scrollpane automatically hides its contents
	scroll_pane_.hide(hide);
	type_title_label_.hide(hide);
	faction_title_label_.hide(hide);
	team_title_label_.hide(hide);
	colour_title_label_.hide(hide);
	gold_title_label_.hide(hide);

	ai_.hide(hide);
	launch_.hide(hide);
	cancel_.hide(hide);
}

void connect::gamelist_updated(bool silent)
{
	update_playerlist_state(silent);
}

void connect::process_network_data(const config& data, const network::connection sock)
{
	ui::process_network_data(data, sock);

	if (!data["side_drop"].empty()) {
		const int side_drop = lexical_cast_default<int>(data["side_drop"], 0) - 1;
		if(side_drop >= 0 && side_drop < int(sides_.size())) {
			connected_user_list::iterator player = find_player(sides_[side_drop].get_id());
			sides_[side_drop].reset(default_controller_);
			if (player != users_.end()) {
				users_.erase(player);
				update_user_combos();
			}
			update_and_send_diff();
			return;
		}
	}

	if (!data["side"].empty()) {
		int side_taken = lexical_cast_default<int>(data["side"], 0) - 1;

		// Checks if the connecting user has a valid and unique name.
		const std::string name = data["name"];
		if (name.empty() || find_player(name) != users_.end()) {
			config response;
			response.values["failed"] = "yes";
			network::send_data(response,sock);
			return;
		}


		// Assigns this user to a side
		if(side_taken >= 0 && side_taken < int(sides_.size())) {
			if(!sides_[side_taken].available()) {
				// This side is already taken. Try to reassing the player to a
				// different position
				side_list::const_iterator itor;
				side_taken = 0;
				for (itor = sides_.begin(); itor != sides_.end();
						++itor, ++side_taken) {
					if(itor->available())
						break;
				}

				if(itor == sides_.end()) {
					config response;
					response.values["failed"] = "yes";
					network::send_data(response, sock);
					return;
				}
			}

			LOG_CF << "client has taken a valid position\n";

			// Adds the name to the list
			users_.push_back(connected_user(name, CNTR_NETWORK, sock));
			update_user_combos();

			// sides_[side_taken].set_connection(sock);
			sides_[side_taken].import_network_user(data);
			update_playerlist_state();
			update_and_send_diff();

			LOG_NW << "sent player data\n";

		} else {
			ERR_CF << "tried to take illegal side: " << side_taken << '\n';
			config response;
			response.values["failed"] = "yes";
			network::send_data(response, sock);
		}
	}
}

void connect::process_network_error(network::error& error)
{
	//if the problem isn't related to any specific connection,
	//it's a general error and we should just re-throw the error
	//likewise if we are not a server, we cannot afford any connection
	//to go down, so also re-throw the error
	if(!error.socket || !network::is_server()) {
		error.disconnect();
		throw network::error(error.message);
	}

	bool changes = false;

	//a socket has disconnected. Remove it, and resets its side
	connected_user_list::iterator user;
	for(user = users_.begin(); user != users_.end(); ++user) {
		if(user->connection == error.socket) {
			changes = true;

			int i = find_player_side(user->name);
			if (i != -1)
				sides_[i].reset(default_controller_);

			break;
		}
	}
	if(user != users_.end()) {
		users_.erase(user);
		update_user_combos();
	}

	//now disconnect the socket
	error.disconnect();

	//if there have been changes to the positions taken,
	//then notify other players
	if(changes) {
		update_and_send_diff();
		update_playerlist_state();
	}
}

bool connect::accept_connections()
{
	if (sides_available())
		return true;
	return false;
}

void connect::process_network_connection(const network::connection sock)
{
	ui::process_network_connection(sock);

	config cfg;
	cfg.add_child("join_game");
	network::send_data(cfg);

	network::send_data(level_, sock);
}

void connect::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);

	SDL_Rect ca = client_area();

	gui::button* left_button = &launch_;
	gui::button* right_button = &cancel_;
#ifdef OK_BUTTON_ON_RIGHT
	std::swap(left_button,right_button);
#endif
	size_t left = ca.x;
	size_t right = ca.x + ca.w;
	size_t top = ca.y;
	size_t bottom = ca.y + ca.h;

	//Buttons
	right_button->set_location(right - right_button->width(),
	                           bottom - right_button->height());
	left_button->set_location(right - right_button->width() - left_button->width() - gui::ButtonHPadding,
	                          bottom - left_button->height());

	ai_.set_location(left, bottom-left_button->height());
	waiting_label_.set_location(ai_.location().x + ai_.location().w + 8,
	                            bottom-left_button->height() + 4);

	type_title_label_.set_location(left+30, top+35);
	faction_title_label_.set_location((left+145), top+35);
	team_title_label_.set_location((left+260), top+35);
	colour_title_label_.set_location((left+375), top+35);
	gold_title_label_.set_location((left+493), top+35);

	SDL_Rect scroll_pane_rect;
	scroll_pane_rect.x = ca.x;
	scroll_pane_rect.y = ca.y + 50;
	scroll_pane_rect.w = ca.w;
	scroll_pane_rect.h = launch_.location().y - scroll_pane_rect.y - gui::ButtonVPadding;

	scroll_pane_.set_location(scroll_pane_rect);
}

void connect::lists_init()
{
	//Options
	player_types_.push_back(_("Network Player"));
	player_types_.push_back(_("Local Player"));
	player_types_.push_back(_("Computer Player"));
	player_types_.push_back(_("Empty"));

	for(std::vector<config*>::const_iterator faction = era_sides_.begin(); faction != era_sides_.end(); ++faction) {
		player_factions_.push_back((**faction)["name"]);
	}

	//Factions
	const config::child_itors sides = level_.child_range("side");

	//Teams
	config::child_iterator sd;
	for(sd = sides.first; sd != sides.second; ++sd) {
		const int side_num = sd - sides.first + 1;
		t_string& team_name = (**sd)["team_name"];
		if(team_name.empty()) {
			team_name = lexical_cast<std::string>(side_num);
		}
		std::vector<std::string>::const_iterator itor = std::find(team_names_.begin(), team_names_.end(), team_name);
		if(itor == team_names_.end()) {
			team_names_.push_back(team_name);
			player_teams_.push_back(team_prefix_ + team_name.str());
		}
	}

	// Colours
	for(int i = 0; i < 9; ++i) {
		player_colours_.push_back(get_colour_string(i));
	}

	// Populates "sides_" from the level configuration
	sides_.reserve(sides.second - sides.first);
	int index = 0;
	for(sd = sides.first; sd != sides.second; ++sd, ++index) {
		sides_.push_back(side(*this, **sd, index));
	}
	// This function must be called after the sides_ vector is fully populated.
	for(side_list::iterator s = sides_.begin(); s != sides_.end(); ++s) {
		const int side_num = s - sides_.begin();
		const int spos = 60 * side_num;

		s->add_widgets_to_scrollpane(scroll_pane_, spos);
	}
}

// Called by the constructor to initialize the game from a create::parameters structure.
void connect::load_game()
{
	if(params_.saved_game) {
		bool show_replay = false;
		const std::string game = dialogs::load_game_dialog(disp(), game_config(), game_data_, &show_replay);
		if(game.empty()) {
			set_result(QUIT);
			return;
		}

		std::string error_log;
		::load_game(game_data_, game, state_, &error_log);
		if(!error_log.empty()) {
			gui::show_error_message(disp(),
					_("The file you have tried to load is corrupt: '") +
					error_log);
			set_result(QUIT);
			return;
		}

		if(state_.campaign_type != "multiplayer") {
			gui::show_dialog(disp(), NULL, "", _("This is not a multiplayer save"),
					gui::OK_ONLY);
			set_result(QUIT);
			return;
		}

		if(state_.version != game_config::version) {
			const int res = gui::show_dialog(disp(), NULL, "",
					_("This save is from a different version of the game. Do you want to try to load it?"),
					gui::YES_NO);
			if(res == 1) {
				set_result(QUIT);
				return;
			}
		}

		level_ = state_.snapshot;

		// Adds the replay data, and the replay start, to the level, so
		// clients can receive it.

		//TO-DO: this should be done only if the player has selected for the replay to be viewed
		if(false) {
			level_.add_child("replay") = state_.replay_data;
			if(!state_.starting_pos.empty())
				level_.add_child("replay_start") = state_.starting_pos;
		}

		level_.add_child("statistics") = statistics::write_stats();

	} else {
		level_ = params_.scenario_data;
		level_["turns"] = lexical_cast_default<std::string>(params_.num_turns, "20");

		const std::string& era = params_.era;

		// Initialize the list of sides available for the current era.
		const config* const era_cfg = game_config().find_child("era", "id", era);
		if(era_cfg == NULL) {
			utils::string_map i18n_symbols;
			i18n_symbols["era"] = era;
			throw config::error(vgettext("Cannot find era $era", i18n_symbols));
		}
		era_sides_ = era_cfg->get_children("multiplayer_side");
		level_["scenario"] = params_.name;
		level_.add_child("era", *era_cfg);
		level_["experience_modifier"] = lexical_cast<std::string>(params_.xp_modifier);
	}

	//this will force connecting clients to be using the same version number as us.
	level_["version"] = game_config::version;

	level_["observers"] = params_.allow_observers ? "yes" : "no";

	if(level_["objectives"].empty()) {
		level_["objectives"] = t_string(N_("Victory\n\
@Defeat enemy leader(s)"), "wesnoth");
	}
}

void connect::update_level()
{
	// Import all sides into the level
	level_.clear_children("side");
	for(side_list::const_iterator itor = sides_.begin(); itor != sides_.end();
			++itor) {

		level_.add_child("side", itor->get_config());
	}
}

void connect::update_and_send_diff()
{
	config old_level = level_;
	update_level();
	config diff;
	diff.add_child("scenario_diff",level_.get_diff(old_level));
	network::send_data(diff);
}

bool connect::sides_available()
{
	for(side_list::const_iterator itor = sides_.begin(); itor != sides_.end(); ++itor) {
		if (itor->available())
			return true;
	}
	return false;
}

void connect::update_playerlist_state(bool silent)
{
	waiting_label_.set_text(sides_available() ? _("Waiting for players to join...") : "");
	launch_.enable(!sides_available());

	// If the "gamelist_" variable has users, use it. Else, extracts the
	// user list from the actual player list.
	if (gamelist().child("user") != NULL) {
		ui::gamelist_updated(silent);
	} else {
		// Updates the player list
		std::vector<std::string> playerlist;
		for(connected_user_list::const_iterator itor = users_.begin(); itor != users_.end();
				++itor) {
			playerlist.push_back(itor->name);
		}
		set_user_list(playerlist, silent);
	}
}

connect::connected_user_list::iterator connect::find_player(const std::string& id)
{
	connected_user_list::iterator itor;
	for (itor = users_.begin(); itor != users_.end(); ++itor) {
		if (itor->name == id)
			break;
	}
	return itor;
}

int connect::find_player_side(const std::string& id) const
{
	side_list::const_iterator itor;
	for (itor = sides_.begin(); itor != sides_.end(); ++itor) {
		if (itor->get_id() == id)
			break;
	}

	if (itor == sides_.end())
		return -1;

	return itor - sides_.begin();
}

void connect::update_user_combos()
{
	for (side_list::iterator itor = sides_.begin(); itor != sides_.end(); ++itor) {
		itor->update_user_list();
	}
}

void connect::kick_player(const std::string& name)
{
	connected_user_list::iterator player = find_player(name);
	if(player == users_.end())
		return;

	if(player->controller != CNTR_NETWORK)
		return;

	// If we are the server, kick the user ourselves; else, ask the server
	// to do so.
	if(network::is_server()) {
		network::disconnect(player->connection);
	} else {
		config kick;
		kick["username"] = name;
		config res;
		res.add_child("kick", kick);
		network::send_data(res);
	}

	int side = find_player_side(name);
	if (side != -1) {
		wassert(size_t(side) < sides_.size());
		sides_[side].reset(sides_[side].get_controller());
	}
	users_.erase(player);
	update_user_combos();
}

}


