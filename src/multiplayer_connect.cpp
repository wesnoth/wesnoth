/* $Id$ */
/*
   Copyright (C) 2007
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file multiplayer_connect.cpp 
//! Prepare to join a multiplayer-game.

#include "global.hpp"

#include "dialogs.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "multiplayer_connect.hpp"
#include "game_preferences.hpp"
#include "statistics.hpp"
#include "show_dialog.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "ai.hpp"
#include "random.hpp"

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
	combo_ai_algorithm_(parent.disp(), std::vector<std::string>()),
	combo_faction_(parent.disp(), parent.player_factions_),
	combo_leader_(parent.disp(), std::vector<std::string>()),
	combo_gender_(parent.disp(), std::vector<std::string>()),
	combo_team_(parent.disp(), parent.player_teams_),
	combo_colour_(parent.disp(), parent.player_colours_),
	slider_gold_(parent.video()),
	slider_income_(parent.video()),
	label_gold_(parent.video(), "100", font::SIZE_SMALL, font::LOBBY_COLOUR),
	label_income_(parent.video(), _("Normal"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	allow_player_(utils::string_bool(cfg_["allow_player"], true)),
	enabled_(!parent_->params_.saved_game), changed_(false),
	llm_(parent.era_sides_, &parent.game_data_, enabled_ ? &combo_leader_ : NULL, enabled_ ? &combo_gender_ : NULL)
{
	if(allow_player_ && enabled_) {
		controller_ = parent_->default_controller_;
	} else {
		size_t i = CNTR_NETWORK;
		// If player isn't allowed, network controller doesn't make sense
		if (!allow_player_)
			++i;
		for(; i != CNTR_LAST; ++i) {
			if(cfg_["controller"] == controller_names[i]) {
				controller_ = static_cast<mp::controller>(i);
				break;
			}
		}
	}

	slider_gold_.set_min(20);
	slider_gold_.set_max(800);
	slider_gold_.set_increment(25);
	slider_gold_.set_value(lexical_cast_default<int>(cfg_["gold"], 100));
	slider_gold_.set_measurements(80, 16);

	slider_income_.set_min(-2);
	slider_income_.set_max(18);
	slider_income_.set_increment(1);
	slider_income_.set_value(lexical_cast_default<int>(cfg_["income"], 0));
	slider_income_.set_measurements(50, 16);

	combo_faction_.enable(enabled_);
	combo_leader_.enable(enabled_);
	combo_gender_.enable(enabled_);
	combo_team_.enable(enabled_);
	combo_colour_.enable(enabled_);
	slider_gold_.hide(!enabled_);
	slider_income_.hide(!enabled_);
	label_gold_.hide(!enabled_);
	label_income_.hide(!enabled_);

	id_ = ""; // Id is reset, and not imported from loading savegames
	save_id_ = cfg_["save_id"];
	faction_ = lexical_cast_default<int>(cfg_["faction"], 0);
	std::vector<std::string>::const_iterator itor = std::find(parent_->team_names_.begin(), 
													parent_->team_names_.end(), cfg_["team_name"]);
	if(itor == parent_->team_names_.end()) {
		wassert(!parent_->team_names_.empty());
		team_ = 0;
	} else {
		team_ = itor - parent_->team_names_.begin();
	}
	colour_ = index_;
	if(!cfg_["colour"].empty()) {
		colour_ = game_config::color_info(cfg_["colour"]).index() - 1;
	}
	gold_ = lexical_cast_default<int>(cfg_["gold"], 100);
	income_ = lexical_cast_default<int>(cfg_["income"], 0);
	config *ai = cfg_.child("ai");
	if (ai)
        ai_algorithm_ = lexical_cast_default<std::string>((*ai)["ai_algorithm"], "default");
    else
        ai_algorithm_ = "default";
	init_ai_algorithm_combo();

	// "Faction name" hack
	if (!enabled_) {
		faction_ = 0;
		std::vector<std::string> pseudo_factions;
		pseudo_factions.push_back(cfg_["name"]);
		combo_faction_.set_items(pseudo_factions);
		combo_faction_.set_selected(0);

		// Hack: if there is a unit which can recruit, use it as a leader. 
		// Necessary to display leader information when loading saves.
		config::const_child_itors side_units = cfg.child_range("unit");
		std::string leader_type;
		for(;side_units.first != side_units.second; ++side_units.first) {
			if(utils::string_bool((**side_units.first)["canrecruit"], false)) {
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
		std::vector<std::string> gender_name_pseudolist;
		gender_ = cfg_["gender"];
		if (!gender_.empty()) {
			if(leader_type.empty() || parent_->game_data_.unit_types.find(leader_type) == parent_->game_data_.unit_types.end()) {
					gender_name_pseudolist.push_back("-");
			} else {
				if (gender_ == "female")
					gender_name_pseudolist.push_back( _("Female ♀") );
				else if (gender_ == "male")
					gender_name_pseudolist.push_back( _("Male ♂") );
				else if (gender_ == "random")
					gender_name_pseudolist.push_back( _("gender^Random") );
				else gender_name_pseudolist.push_back("?");
			}
		} else {
			gender_name_pseudolist.push_back("-");
			std::cerr << "CKJSADHFCKSAJFKDJFLDSKJ";
		}
		combo_gender_.set_items(gender_name_pseudolist);
		combo_gender_.set_selected(0);
	} else if(parent_->params_.use_map_settings) {
		// gold, income, team, and colour are only suggestions unless explicitly locked
		if(utils::string_bool(cfg_["gold_lock"], false)) {
			slider_gold_.enable(false);
			label_gold_.enable(false);
		}
		if(utils::string_bool(cfg_["income_lock"], false)) {
			slider_income_.enable(false);
			label_income_.enable(false);
		}
		if(utils::string_bool(cfg_["team_lock"], false)) {
			combo_team_.enable(false);
		}
		if(utils::string_bool(cfg_["colour_lock"], false)) {
			combo_colour_.enable(false);
		}

		// Set the leader and gender
		leader_ = cfg_["type"];
		gender_ = cfg_["gender"];
		if(!leader_.empty()) {
			combo_leader_.enable(false);
			combo_gender_.enable(false);
			llm_.set_leader_combo(NULL);
			llm_.set_gender_combo(NULL);
			std::vector<std::string> leader_name_pseudolist;
			game_data::unit_type_map::const_iterator leader_name = parent_->game_data_.unit_types.find(leader_);
			if(leader_name == parent_->game_data_.unit_types.end()) {
				leader_name_pseudolist.push_back("?");
			} else {
				leader_name_pseudolist.push_back(leader_name->second.language_name());
			}
			combo_leader_.set_items(leader_name_pseudolist);
			combo_leader_.set_selected(0);
			std::vector<std::string> gender_name_pseudolist;
			if (!gender_.empty()) {
				if(leader_name == parent_->game_data_.unit_types.end()) {
					gender_name_pseudolist.push_back("?");
				} else {
					if (gender_ == "female")
						gender_name_pseudolist.push_back( _("Female ♀") );
					else if (gender_ == "male")
						gender_name_pseudolist.push_back( _("Male ♂") );
					else if (gender_ == "random")
						gender_name_pseudolist.push_back( _("gender^Random") );
					else gender_name_pseudolist.push_back("?");
				}
			} else gender_name_pseudolist.push_back("?");
			combo_gender_.set_items(gender_name_pseudolist);
			combo_gender_.set_selected(0);
		}

		// Try to pick a faction for the sake of appearance 
		// and for filling in the blanks
		if(faction_ == 0) {
			std::vector<std::string> find;
			std::string search_field;
			if(!cfg_["faction"].empty()) {
				// Choose based on faction
				find.push_back(cfg_["faction"]);
				search_field = "id";
			} else if(!cfg_["recruit"].empty()) {
				// Choose based on recruit
				find = utils::split(cfg_["recruit"]);
				search_field = "recruit";
			} else if(!leader_.empty()) {
				// Choose based on leader
				find.push_back(leader_);
				search_field = "leader";
			}

			std::vector<std::string>::const_iterator search = find.begin();
			while(search != find.end()) {
				int faction_index = 0;
				std::vector<config*>::const_iterator faction = parent.era_sides_.begin();
				while(faction_ == 0 && faction != parent.era_sides_.end()) {
					const config& side = (**faction);
					std::vector<std::string> recruit;
					recruit = utils::split(side[search_field]);
					for(itor = recruit.begin(); itor != recruit.end(); ++itor) {
						if(*itor == *search) {
							faction_ = faction_index;
							llm_.update_leader_list(faction_);
							llm_.update_gender_list(llm_.get_leader());
							combo_faction_.enable(false);
						}
					}
					++faction;
					faction_index++;
				}
				// Exit outmost loop if we've found a faction
				if(!combo_faction_.enabled()) {
					break;
				}
				++search;
			}
		} else {
			combo_faction_.enable(false);
		}
	}

	update_ui();
}

connect::side::side(const side& a) :
	parent_(a.parent_), cfg_(a.cfg_),
	index_(a.index_), id_(a.id_),  save_id_(a.save_id_),
	controller_(a.controller_),
	faction_(a.faction_), team_(a.team_), colour_(a.colour_),
	gold_(a.gold_), income_(a.income_), leader_(a.leader_), /* taken_(a.taken_), */
	gender_(a.gender_),
	ai_algorithm_(a.ai_algorithm_),
	player_number_(a.player_number_), combo_controller_(a.combo_controller_),
	orig_controller_(a.orig_controller_),
	combo_ai_algorithm_(a.combo_ai_algorithm_),
	combo_faction_(a.combo_faction_), combo_leader_(a.combo_leader_), combo_gender_(a.combo_gender_),
	combo_team_(a.combo_team_), combo_colour_(a.combo_colour_),
	slider_gold_(a.slider_gold_), slider_income_(a.slider_income_),
	label_gold_(a.label_gold_), label_income_(a.label_income_),
	allow_player_(a.allow_player_), enabled_(a.enabled_),
	changed_(a.changed_), llm_(a.llm_)
{
	llm_.set_leader_combo((enabled_ && leader_.empty()) ? &combo_leader_ : NULL);
	llm_.set_gender_combo((enabled_ && leader_.empty()) ? &combo_gender_ : NULL);
	// FIXME: this is an ugly hack to force updating the gender list when the side
	// widget is initialized. Need an optimal way. -- shadowmaster
	llm_.update_gender_list(llm_.get_leader());
}

void connect::side::add_widgets_to_scrollpane(gui::scrollpane& pane, int pos)
{
	pane.add_widget(&player_number_,     0, 5 + pos);
	pane.add_widget(&combo_controller_, 20, 5 + pos);
	pane.add_widget(&orig_controller_,  20 + (combo_controller_.width() - orig_controller_.width()) / 2,
									    35 + pos + (combo_leader_.height() - orig_controller_.height()) / 2);
	pane.add_widget(&combo_ai_algorithm_, 20, 35 + pos);
	pane.add_widget(&combo_faction_, 135, 5 + pos);
	pane.add_widget(&combo_leader_,  135, 35 + pos);
	pane.add_widget(&combo_gender_,  250, 35 + pos);
	pane.add_widget(&combo_team_,    250, 5 + pos);
	pane.add_widget(&combo_colour_,  365, 5 + pos);
	pane.add_widget(&slider_gold_,   475, 5 + pos);
	pane.add_widget(&label_gold_,    475, 35 + pos);
	pane.add_widget(&slider_income_, 475 + slider_gold_.width(),  5 + pos);
	pane.add_widget(&label_income_,  475 + slider_gold_.width(), 35 + pos);
}

void connect::side::process_event()
{
	if(combo_controller_.changed() && combo_controller_.selected() >= 0) {
		if (combo_controller_.selected() == CNTR_LAST) {
			update_controller_ui();
		} else if (combo_controller_.selected() < CNTR_LAST) {
			// If the current side corresponds to an existing user,
			// we must kick it!

			// Update controller first, or else kick will reset it.
			controller_ = mp::controller(combo_controller_.selected());

			// Don't kick an empty player or the game creator
			if(!id_.empty()) {
				if (id_ != preferences::login()) {
					parent_->kick_player(id_);
				}
				id_ = "";
			}
			changed_ = true;
		} else {
			size_t user = combo_controller_.selected() - CNTR_LAST - 1;

			// If the selected user already was attributed to
			// another side, find its side, and switch users.
			//! todo Let one player get several sides like it is
			//! already possible once the game started.
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
		update_ai_algorithm_combo();
	}

	if(!enabled_)
		return;

	if (combo_faction_.changed() && combo_faction_.selected() >= 0) {
		faction_ = combo_faction_.selected();
		llm_.update_leader_list(faction_);
		llm_.update_gender_list(llm_.get_leader());
		changed_ = true;
	}
	if (combo_ai_algorithm_.changed() && combo_ai_algorithm_.selected() >= 0) {
		ai_algorithm_ = parent_->ai_algorithms_[combo_ai_algorithm_.selected()];
		changed_ = true;
	}
	if (combo_leader_.changed() && combo_leader_.selected() >= 0) {
		llm_.update_gender_list(llm_.get_leader());
		changed_ = true;
	}
	if (combo_gender_.changed() && combo_gender_.selected() >= 0) {
		llm_.set_leader_combo(&combo_leader_);
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
	if (slider_income_.value() != income_) {

		income_ = slider_income_.value();
		std::stringstream buf;
		if(income_ < 0) {
			buf << _("(") << income_ << _(")");
		} else if(income_ > 0) {
			buf << _("+") << income_;
		} else {
			buf << _("Normal");
		}
		label_income_.set_text(buf.str());
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
	return allow_player_ && controller_ == CNTR_NETWORK && id_.empty();
}

bool connect::side::allow_player() const
{
	return allow_player_;
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

    update_ai_algorithm_combo();
}

void connect::side::hide_ai_algorithm_combo(bool invis)
{
	if(!invis)
	{
		if(controller_ == CNTR_COMPUTER)
		{
			// Computer selected, show AI combo
			orig_controller_.hide(true);
			combo_ai_algorithm_.hide(false);
		} else {
			// Computer de-selected, hide AI combo
			combo_ai_algorithm_.hide(true);
			orig_controller_.hide(false);
		}
	} else {
		combo_ai_algorithm_.hide(true);
	}
}

void connect::side::init_ai_algorithm_combo()
{
	wassert(parent_->ai_algorithms_.empty() == false);

	int sel = 0;
	std::vector<std::string> ais = parent_->ai_algorithms_;
	for (unsigned int i = 0; i < ais.size(); i++) {
		if (ais[i] == ai_algorithm_) {
			sel = i;
		}
		if (ais[i] == "default") {
			ais[i] = _("Default AI");
		}
	}
	combo_ai_algorithm_.set_items(ais);
	combo_ai_algorithm_.set_selected(sel);
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
	slider_income_.set_value(income_);
	std::stringstream buf;
	if(income_ < 0) {
		buf << _("(") << income_ << _(")");
	} else if(income_ > 0) {
		buf << _("+") << income_;
	} else {
		buf << _("Normal");
	}
	label_income_.set_text(buf.str());
}

config connect::side::get_config() const
{
	config res(cfg_);

	// If the user is allowed to change type, faction, leader etc, 
	// then import their new values in the config.
	if(enabled_ && !parent_->era_sides_.empty()) {
		// Merge the faction data to res
		res.append(*(parent_->era_sides_[faction_]));
	}
	if(cfg_["side"].empty() || cfg_["side"] != lexical_cast<std::string>(index_ + 1)) {
		res["side"] = lexical_cast<std::string>(index_ + 1);
	}
	res["controller"] = controller_names[controller_];
	res["description"] = id_;
	res["current_player"] = id_;

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
			{
				config *ai = res.child("ai");
				if (!ai) ai = &res.add_child("ai");
				#ifdef HAVE_PYTHON
				if (ai_algorithm_.substr(ai_algorithm_.length() - 3) == ".py") {
					(*ai)["ai_algorithm"] = "python_ai";
					(*ai)["python_script"] = ai_algorithm_;
				}
				else
				#endif
				{
					if (ai_algorithm_ != "default")
						(*ai)["ai_algorithm"] = ai_algorithm_;
				}
			}
			description = N_("Computer player");
			break;
		case CNTR_EMPTY:
			description = N_("(Empty slot)");
			res["no_leader"] = "yes";
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
		if (gender_.empty()) {
			std::string dummy = llm_.get_gender();
			if (!dummy.empty() && dummy != "null" && dummy != "?")
				res["gender"] = dummy;
		} else {
			// If no genders could be resolved, let the unit engine use
			// the default gender
			if (gender_ != "null")
				res["gender"] = gender_;
		}
		// res["team"] = lexical_cast<std::string>(team_);
		res["team_name"] = parent_->team_names_[team_];
		res["user_team_name"] = parent_->user_team_names_[team_];
		res["colour"] = lexical_cast<std::string>(colour_ + 1);
		res["gold"] = lexical_cast<std::string>(gold_);
		res["income"] = lexical_cast<std::string>(income_);

		if(!parent_->params_.use_map_settings || res["fog"].empty() || (res["fog"] != "yes" && res["fog"] != "no")) {
			res["fog"] = parent_->params_.fog_game ? "yes" : "no";
		}

		if(!parent_->params_.use_map_settings || res["shroud"].empty() || (res["shroud"] != "yes" && res["shroud"] != "no")) {
			res["shroud"] = parent_->params_.shroud_game ? "yes" : "no";
		}


		if(!parent_->params_.use_map_settings || res["mp_countdown"].empty() || (res["mp_countdown"] != "yes" && res["mp_countdown"] != "no")) {
			res["mp_countdown"] = parent_->params_.mp_countdown ? "yes" : "no";;
		}

		if(!parent_->params_.use_map_settings || res["mp_countdown_init_time"].empty()) {
			res["mp_countdown_init_time"] = lexical_cast<std::string>(parent_->params_.mp_countdown_init_time);
		}
		if(!parent_->params_.use_map_settings || res["mp_countdown_turn_bonus"].empty()) {
			res["mp_countdown_turn_bonus"] = lexical_cast<std::string>(parent_->params_.mp_countdown_turn_bonus);
		}
		if(!parent_->params_.use_map_settings || res["mp_countdown_reservoir_time"].empty()) {
			res["mp_countdown_reservoir_time"] = lexical_cast<std::string>(parent_->params_.mp_countdown_reservoir_time);
		}
		if(!parent_->params_.use_map_settings || res["mp_countdown_action_bonus"].empty()) {
			res["mp_countdown_action_bonus"] = lexical_cast<std::string>(parent_->params_.mp_countdown_action_bonus);
		}

		res["share_maps"] = parent_->params_.share_maps ? "yes" : "no";
		res["share_view"] =  parent_->params_.share_view ? "yes" : "no";
		if(!parent_->params_.use_map_settings || res["village_gold"].empty())
			res["village_gold"] = lexical_cast<std::string>(parent_->params_.village_gold);

		res["allow_changes"] = "yes";
	} else {
		res["allow_changes"] = "no";
	}

	if(parent_->params_.use_map_settings && enabled_) {
		config trimmed = cfg_;
		trimmed["side"] = "";
		trimmed["controller"] = "";
		trimmed["description"] = "";
		trimmed["team_name"] = "";
		trimmed["user_team_name"] = "";
		trimmed["colour"] = "";
		trimmed["gold"] = "";
		trimmed["income"] = "";
		trimmed["allow_changes"] = "";
		if(controller_ != CNTR_COMPUTER) {
			// Only override names for computer controlled players
			trimmed["user_description"] = "";
		}
		trimmed.prune();
		res.merge_with(trimmed);
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

	if(enabled_ && !parent_->era_sides_.empty()) {
		if(combo_faction_.enabled()) {
			faction_ = lexical_cast_default<int>(data["faction"], 0);
			if(faction_ > int(parent_->era_sides_.size()))
				faction_ = 0;
			llm_.update_leader_list(faction_);
			llm_.update_gender_list(llm_.get_leader());
		}
		if(combo_leader_.enabled()) {
			llm_.set_leader(data["leader"]);
			// FIXME: not optimal, but this hack is necessary to do after updating
			// the leader selection. Otherwise, gender gets always forced to "male".
			llm_.update_gender_list(llm_.get_leader());
		}
		if (combo_gender_.enabled()) {
			llm_.set_gender(data["gender"]);
		}
	}

	update_ui();
}

void connect::side::reset(mp::controller controller)
{
	id_ = "";
	controller_ = controller;

	if(enabled_ && !parent_->era_sides_.empty()) {
		if(combo_faction_.enabled())
			faction_ = 0;
		if(combo_leader_.enabled())
			llm_.update_leader_list(0);
		if (combo_gender_.enabled())
			llm_.update_gender_list(llm_.get_leader());
	}

	update_ui();
}

void connect::side::resolve_random()
{
	if(!enabled_ || parent_->era_sides_.empty())
		return;

	if(utils::string_bool((*parent_->era_sides_[faction_])["random_faction"], false)) {

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
	bool solved_random_leader = false;

	if (llm_.get_leader() == "random") {
		// Choose a random leader type, and force gender to be random
		llm_.set_gender("random");
		const config& fact = *parent_->era_sides_[faction_];
		std::vector<std::string> types = utils::split(fact["random_leader"]);
		if (!types.empty()) {
			const int lchoice = rand() % types.size();
			leader_ = types[lchoice];
		} else {
			// If random_leader= doesn't exists, we use leader=
			types = utils::split(fact["leader"]);
			if (!types.empty()) {
				const int lchoice = rand() % types.size();
				leader_ = types[lchoice];
			} else {
				utils::string_map i18n_symbols;
				i18n_symbols["faction"] = fact["name"];
				throw config::error(vgettext("Unable to find a leader type for faction $faction", i18n_symbols));
			}
		}
		solved_random_leader = true;
	}
	// Resolve random genders "very much" like standard unit code
	if (llm_.get_gender() == "random" || solved_random_leader) {
		game_data::unit_type_map::const_iterator ut = parent_->game_data_.unit_types.find(leader_.empty() ? llm_.get_leader() : leader_);

		if (ut != parent_->game_data_.unit_types.end()) {
			const std::vector<unit_race::GENDER> glist = ut->second.genders();
			if (!glist.empty()) {
				const int gchoice = get_random() % glist.size();
				// Pick up a gender, using the random 'gchoice' index
				unit_race::GENDER sgender = glist[gchoice];
				switch (sgender)
				{
					case unit_race::FEMALE:
						gender_ = "female";
						break;
					case unit_race::MALE:
						gender_ = "male";
						break;
					default:
						gender_ = "null";
				}
			} else gender_ = "null";
		} else {
			ERR_CF << "cannot obtain genders for invalid leader '" << (leader_.empty() ? llm_.get_leader() : leader_) << "'.\n";
			gender_ = "null";
		}
	}
}

connect::connect(game_display& disp, const config& game_config, const game_data& data,
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
	team_title_label_(video(), _("Team/Gender"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	colour_title_label_(video(), _("Color"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	gold_title_label_(video(), _("Gold"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	income_title_label_(video(), _("Income"), font::SIZE_SMALL, font::LOBBY_COLOUR),

	launch_(video(), _("I'm Ready")),
	cancel_(video(), _("Cancel"))
{
	load_game();

	if(get_result() == QUIT)
		return;
	lists_init();
	if(sides_.empty()) {
		throw config::error(_("The scenario is invalid because it has no sides."));
	}
	// Send Initial information
	config response;
	config& create_game = response.add_child("create_game");
	create_game["name"] = params.name;
/*
	// The number of human-controlled sides is important to know 
	// to let the server decide how many players can join this game
	int human_sides = 0;
	config::child_list cfg_sides = current_config()->get_children("side");
	for (config::child_list::const_iterator side = cfg_sides.begin(); side != cfg_sides.end(); side++){
		if ((**side)["controller"] == "human"){
			human_sides++;
		}
	}
	create_game["human_sides"] = lexical_cast<std::string>(human_sides);*/
	network::send_data(response);

	// Adds the current user as default user.
	users_.push_back(connected_user(preferences::login(), CNTR_LOCAL, 0));
	update_user_combos();
	// Take the first available side or available side with id == login
	int side_choice = -1;
	for(side_list::const_iterator s = sides_.begin(); s != sides_.end(); ++s) {
		if (s->allow_player()) {
			if (side_choice == -1)
				side_choice = s - sides_.begin();
			if(s->get_save_id() == preferences::login()) {
				side_choice = s - sides_.begin();
				break;
			}
		}
	}

	if (side_choice != -1)
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

	if (launch_.pressed()) {
		if (!sides_available())
			set_result(mp::ui::PLAY);
	}

	// If something has changed in the level config, 
	// send it to the network:
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
	// Resolves the "random faction", "random gender" and "random message"
	for (side_list::iterator itor = sides_.begin(); itor != sides_.end();
			++itor) {

		itor->resolve_random();
	}
	// Make other clients not show the results of resolve_random().
	config lock;
	lock.add_child("stop_updates");
	network::send_data(lock);
	update_and_send_diff(true);

	// Build the gamestate object after updating the level
	level_to_gamestate(level_, state_, params_.saved_game);

	network::send_data(config("start_game"));
}

void connect::hide_children(bool hide)
{
	ui::hide_children(hide);

	waiting_label_.hide(hide);
	// Hiding the scrollpane automatically hides its contents
	scroll_pane_.hide(hide);
	for (side_list::iterator itor = sides_.begin(); itor != sides_.end(); ++itor) {
		itor->hide_ai_algorithm_combo(hide);
	}
	faction_title_label_.hide(hide);
	team_title_label_.hide(hide);
	colour_title_label_.hide(hide);
	if (!params_.saved_game) {
		gold_title_label_.hide(hide);
		income_title_label_.hide(hide);
	}

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
			sides_[side_drop].reset(sides_[side_drop].get_controller());
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
		if(name.empty()) {
			config response;
			response.values["failed"] = "yes";
			network::send_data(response,sock);
			ERR_CF << "ERROR: No username provided with the side.\n";
			return;
		}

		connected_user_list::iterator player = find_player(name);
		if(player != users_.end()) {
			if(find_player_side(name) != -1) {
				config response;
				response.values["failed"] = "yes";
				response["message"] = "The nick '" + name + "' is already in use.";
				network::send_data(response,sock);
				return;
			} else {
				users_.erase(player);
				config observer_quit;
				observer_quit.add_child("observer_quit").values["name"] = name;
				network::send_data(observer_quit);
				update_user_combos();
			}
		}

		// Assigns this user to a side
		if(side_taken >= 0 && side_taken < int(sides_.size())) {
			if(!sides_[side_taken].available()) {
				// This side is already taken.
				// Try to reassing the player to a different position.
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
					config kick;
					kick["username"] = data["name"];
					config res;
					res.add_child("kick", kick);
					network::send_data(res);
					update_user_combos();
					update_and_send_diff();
					ERR_CF << "ERROR: Couldn't assign a side to '" << name << "'\n";
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

	const config* change_faction = data.child("change_faction");
	if(change_faction != NULL) {
		int side_taken = find_player_side(change_faction->get_attribute("name"));
		if(side_taken != -1) {
			sides_[side_taken].import_network_user(*change_faction);
			update_playerlist_state();
			update_and_send_diff();
		}
	}

	if(data.child("observer") != NULL) {
		const t_string& observer_name = data.child("observer")->get_attribute("name");
		if(!observer_name.empty()) {
			connected_user_list::iterator player = find_player(observer_name);
			if(player == users_.end()) {
				users_.push_back(connected_user(observer_name, CNTR_NETWORK, sock));
				update_user_combos();
				update_playerlist_state();
				update_and_send_diff();
			}
		}
	}
	if(data.child("observer_quit") != NULL) {
		const t_string& observer_name = data.child("observer_quit")->get_attribute("name");
		if(!observer_name.empty()) {
			connected_user_list::iterator player = find_player(observer_name);
			if(player != users_.end() && find_player_side(observer_name) == -1) {
				users_.erase(player);
				update_user_combos();
				update_playerlist_state();
				update_and_send_diff();
			}
		}
	}
}

void connect::process_network_error(network::error& error)
{
	// If the problem isn't related to any specific connection,
	// it's a general error and we should just re-throw the error.
	// Likewise if we are not a server, we cannot afford any connection
	// to go down, so also re-throw the error.
	if(!error.socket || !network::is_server()) {
		error.disconnect();
		throw network::error(error.message);
	}

	bool changes = false;

	// A socket has disconnected. Remove it, and resets its side
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

	// Now disconnect the socket
	error.disconnect();

	// If there have been changes to the positions taken,
	// then notify other players
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

	network::send_data(config("join_game"));

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

	// Buttons
	right_button->set_location(right  - right_button->width(),
	                           bottom - right_button->height());
	left_button->set_location(right  - right_button->width() - left_button->width() - gui::ButtonHPadding,
	                          bottom - left_button->height());

	waiting_label_.set_location(left + 8, bottom-left_button->height() + 4);
	type_title_label_.set_location(left+30, top+35);
	faction_title_label_.set_location((left+145), top+35);
	team_title_label_.set_location((left+260), top+35);
	colour_title_label_.set_location((left+375), top+35);
	gold_title_label_.set_location((left+493), top+35);
	income_title_label_.set_location((left+560), top+35);

	SDL_Rect scroll_pane_rect;
	scroll_pane_rect.x = ca.x;
	scroll_pane_rect.y = ca.y + 50;
	scroll_pane_rect.w = ca.w;
	scroll_pane_rect.h = launch_.location().y - scroll_pane_rect.y - gui::ButtonVPadding;

	scroll_pane_.set_location(scroll_pane_rect);
}

void connect::lists_init()
{
	// Options
	player_types_.push_back(_("Network Player"));
	player_types_.push_back(_("Local Player"));
	player_types_.push_back(_("Computer Player"));
	player_types_.push_back(_("Empty"));

	for(std::vector<config*>::const_iterator faction = era_sides_.begin(); 
													   faction != era_sides_.end(); 
													   ++faction) {
		player_factions_.push_back((**faction)["name"]);
	}

	// AI algorithms
    ai_algorithms_ = get_available_ais();

	// Factions
	const config::child_itors sides = current_config()->child_range("side");

	// Teams
	config::child_iterator sd;
	if(params_.use_map_settings) {
		for(sd = sides.first; sd != sides.second; ++sd) {
			const int side_num = sd - sides.first + 1;
			t_string& team_name = (**sd)["team_name"];
			t_string& user_team_name = (**sd)["user_team_name"];

			if(team_name.empty())
				team_name = lexical_cast<std::string>(side_num);

			if(user_team_name.empty())
			{
				user_team_name = team_name;
			}

			std::vector<std::string>::const_iterator itor = std::find(team_names_.begin(), 
															team_names_.end(), team_name);
			if(itor == team_names_.end()) {
				team_names_.push_back(team_name);
				user_team_names_.push_back(user_team_name);
				player_teams_.push_back(user_team_name.str());
			}
		}
	} else {
		std::vector<std::string> map_team_names;
		for(sd = sides.first; sd != sides.second; ++sd) {
			const std::string side_num = lexical_cast<std::string>(sd - sides.first + 1);
			t_string& team_name = (**sd)["team_name"];

			if(team_name.empty())
				team_name = side_num;

			std::vector<std::string>::const_iterator itor = std::find(map_team_names.begin(), 
															map_team_names.end(), team_name);
			if(itor == map_team_names.end()) {
				map_team_names.push_back(team_name);
				team_name = lexical_cast<std::string>(map_team_names.size());
			} else {
				team_name = lexical_cast<std::string>(itor - map_team_names.begin() + 1);
			}

			team_names_.push_back(side_num);
			user_team_names_.push_back(team_prefix_ + side_num);
			player_teams_.push_back(team_prefix_ + side_num);
		}
	}

	// Colours
	for(int i = 0; i < gamemap::MAX_PLAYERS; ++i) {
		player_colours_.push_back(get_colour_string(i));
	}

	// Populates "sides_" from the level configuration
	sides_.reserve(sides.second - sides.first);
	int index = 0;
	for(sd = sides.first; sd != sides.second; ++sd, ++index) {
		sides_.push_back(side(*this, **sd, index));
	}
	int offset=0;
	// This function must be called after the sides_ vector is fully populated.
	for(side_list::iterator s = sides_.begin(); s != sides_.end(); ++s) {
		const int side_num = s - sides_.begin();
		const int spos = 60 * (side_num-offset);
		if(!utils::string_bool(s->get_config()["allow_player"], true)) {
			offset++;
			continue;
		}

		s->add_widgets_to_scrollpane(scroll_pane_, spos);
	}
}

//! Called by the constructor to initialize the game from a create::parameters structure.
void connect::load_game()
{
	if(params_.saved_game) {
		bool show_replay = false;
		const std::string game = dialogs::load_game_dialog(disp(), 
								 game_config(), game_data_, &show_replay);
		if(game.empty()) {
			set_result(QUIT);
			return;
		}

		std::string error_log;
		{
			cursor::setter cur(cursor::WAIT);
			::load_game(game_data_, game, state_, &error_log);
		}
		if(!error_log.empty()) {
			gui::show_error_message(disp(),
					_("The file you have tried to load is corrupt: '") +
					error_log);
			set_result(QUIT);
			return;
		}

		if(state_.campaign_type != "multiplayer") {
			/* GCC-3.3 needs a temp var otherwise compilation fails */
			gui::message_dialog dlg(disp(), "", _("This is not a multiplayer save"));
			dlg.show();
			set_result(QUIT);
			return;
		}

		if(state_.version != game_config::version) {
			// Do not load if too old, but if either the savegame or 
			// the current game has the version 'test' allow loading.
			if(state_.version < game_config::min_savegame_version &&
					game_config::test_version.full != state_.version &&
					game_config::test_version.full != game_config::version) {

				/* GCC-3.3 needs a temp var otherwise compilation fails */
				gui::message_dialog dlg2(disp(), "", _("This save is from a version too old to be loaded."));
				dlg2.show();
				set_result(QUIT);
				return;
			}

			const int res = gui::dialog(disp(), "",
					_("This save is from a different version of the game. Do you want to try to load it?"),
					gui::YES_NO).show();
			if(res == 1) {
				set_result(QUIT);
				return;
			}
		}

		level_["savegame"] = "yes";
		level_["map_data"] = state_.snapshot["map_data"];
		level_["id"] = state_.snapshot["id"];
		level_["name"] = state_.snapshot["name"];
		// Probably not needed.
		level_["turn"] = state_.snapshot["turn_at"];
		level_["turn_at"] = state_.snapshot["turn_at"];
		level_["turns"] = state_.snapshot["turns"];
		level_["mp_use_map_settings"] = state_.snapshot["mp_use_map_settings"];
		level_["mp_village_gold"] = state_.snapshot["mp_village_gold"];
		level_["mp_fog"] = state_.snapshot["mp_fog"];
		level_["mp_shroud"] = state_.snapshot["mp_shroud"];
		level_["mp_countdown"] = state_.snapshot["mp_countdown"];
		level_["mp_countdown_init_time"] = state_.snapshot["mp_countdown_init_time"];
		level_["mp_countdown_turn_bonus"] = state_.snapshot["mp_countdown_turn_bonus"];
		level_["mp_countdown_action_bonus"] = state_.snapshot["mp_countdown_action_bonus"];
		level_["experience_modifier"] = state_.snapshot["experience_modifier"];
		level_["observer"] = state_.snapshot["observer"];
		level_.add_child("snapshot") = state_.snapshot;

		// Adds the replay data, and the replay start, to the level, 
		// so clients can receive it.
		level_.add_child("replay") = state_.replay_data;
		if(!state_.starting_pos.empty())
			level_.add_child("replay_start") = state_.starting_pos;
		level_.add_child("statistics") = statistics::write_stats();

	} else {
		level_["savegame"] = "no";
		level_ = params_.scenario_data;

		level_["hash"] = level_.hash();
		level_["turns"] = lexical_cast_default<std::string>(params_.num_turns, "20");
		level_["mp_countdown"] = params_.mp_countdown ? "yes" : "no";
		level_["mp_countdown_init_time"] = lexical_cast_default<std::string>(params_.mp_countdown_init_time, "270");
		level_["mp_countdown_turn_bonus"] = lexical_cast_default<std::string>(params_.mp_countdown_turn_bonus, "35");
		level_["mp_countdown_reservoir_time"] = lexical_cast_default<std::string>(params_.mp_countdown_reservoir_time, "330");
		level_["mp_countdown_action_bonus"] = lexical_cast_default<std::string>(params_.mp_countdown_action_bonus, "13");

		if (params_.random_start_time)
		{
			if (!gamestatus::is_start_ToD(level_["random_start_time"]))
			{
				level_["random_start_time"] = "yes";
			}
		}
		else
		{
			level_["random_start_time"] = "no";
		}

		level_["scenario"] = params_.name;
		level_["experience_modifier"] = lexical_cast<std::string>(params_.xp_modifier);
	}

	const std::string& era = params_.era;

	// Initialize the list of sides available for the current era.
	const config* const era_cfg = game_config().find_child("era", "id", era);
	if(era_cfg == NULL) {
		utils::string_map i18n_symbols;
		i18n_symbols["era"] = era;
		throw config::error(vgettext("Cannot find era $era", i18n_symbols));
	}
	era_sides_ = era_cfg->get_children("multiplayer_side");
	level_.add_child("era", *era_cfg);

	gold_title_label_.hide(params_.saved_game);
	income_title_label_.hide(params_.saved_game);

	level_["mp_village_gold"] = lexical_cast<std::string>(params_.village_gold);
	level_["mp_use_map_settings"] = params_.use_map_settings ? "yes" : "no";
	level_["mp_fog"] = params_.fog_game ? "yes" : "no";
	level_["mp_shroud"] = params_.shroud_game ? "yes" : "no";

	// This will force connecting clients to be using the same version number as us.
	level_["version"] = game_config::version;

	level_["observer"] = params_.allow_observers ? "yes" : "no";

	if(level_["objectives"].empty()) {
		level_["objectives"] = t_string(N_("Victory:\n\
@Defeat enemy leader(s)"), "wesnoth");
	}
}

config* connect::current_config(){
	config* cfg_level = NULL;

	if (level_.child("snapshot") != NULL){
		// Savegame
		cfg_level = level_.child("snapshot");
	}
	else{
		// Fresh game, no snapshot available
		cfg_level = &level_;
	}

	return cfg_level;
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

void connect::update_and_send_diff(bool update_time_of_day)
{
	config old_level = level_;
	update_level();

	if (update_time_of_day)
	{
		// Set random start ToD
		gamestatus game_status(level_,atoi(level_["turns"].c_str()),&state_);

	}

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

	// If the "gamelist_" variable has users, use it. 
	// Else, extracts the user list from the actual player list.
	if (gamelist().child("user") != NULL) {
		ui::gamelist_updated(silent);
	} else {
		// Updates the player list
		std::vector<std::string> playerlist;
		for(connected_user_list::const_iterator itor = users_.begin(); 
													   itor != users_.end(); 
													   ++itor) {
			playerlist.push_back(itor->name);
		}
		set_user_list(playerlist, silent);
		set_user_menu_items(playerlist);
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
	for (side_list::iterator itor = sides_.begin(); 
									itor != sides_.end(); ++itor) {
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

	// If we are the server, kick the user ourselves; 
	// else, ask the server to do so.
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

} // end namespace mp

