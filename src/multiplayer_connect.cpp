/* $Id$ */
/*
   Copyright (C) 2007 - 2011
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file multiplayer_connect.cpp
 * Prepare to join a multiplayer-game.
 */

#include "global.hpp"


#include "ai/configuration.hpp"
#include "dialogs.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "multiplayer_connect.hpp"
#include "savegame.hpp"
#include "statistics.hpp"
#include "unit_id.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"
#include "tod_manager.hpp"

#include <boost/bind.hpp>

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

namespace {
	const char* controller_names[] = {
		"network",
		"human",
		"ai",
		"null",
		"reserved"
	};
}

namespace mp {

connect::side::side(connect& parent, const config& cfg, int index) :
	parent_(&parent),
	cfg_(cfg),
	index_(index),
	id_(cfg_.get_attribute("id")),
	player_id_(cfg_.get_attribute("player_id")),
	save_id_(cfg_.get_attribute("save_id")),
	current_player_(cfg_.get_attribute("current_player")),
	controller_(CNTR_NETWORK),
	faction_(lexical_cast_default<int>(cfg_.get_attribute("faction"), 0)),
	team_(0),
	colour_(index),
	gold_(lexical_cast_default<int>(cfg_.get_attribute("gold"), 100)),
	income_(lexical_cast_default<int>(cfg_.get_attribute("income"), 0)),
	leader_(),
	gender_(),
	ai_algorithm_(),
	ready_for_start_(false),
	gold_lock_(utils::string_bool(cfg_.get_attribute("gold_lock"), false)),
	income_lock_(utils::string_bool(cfg_.get_attribute("income_lock"), false)),
	team_lock_(utils::string_bool(cfg_.get_attribute("team_lock"), false)),
	colour_lock_(utils::string_bool(cfg_.get_attribute("colour_lock"), false)),
	player_number_(parent.video(), lexical_cast_default<std::string>(index+1, ""),
	               font::SIZE_LARGE, font::LOBBY_COLOUR),
	combo_controller_(new gui::combo_drag(parent.disp(), parent.player_types_, parent.combo_control_group_)),
	orig_controller_(parent.video(), current_player_, font::SIZE_SMALL),
	combo_ai_algorithm_(parent.disp(), std::vector<std::string>()),
	combo_faction_(parent.disp(), std::vector<std::string>()),
	combo_leader_(parent.disp(), std::vector<std::string>()),
	combo_gender_(parent.disp(), std::vector<std::string>()),
	combo_team_(parent.disp(), parent.player_teams_),
	combo_colour_(parent.disp(), parent.player_colours_),
	slider_gold_(parent.video()),
	slider_income_(parent.video()),
	label_gold_(parent.video(), "100", font::SIZE_SMALL, font::LOBBY_COLOUR),
	label_income_(parent.video(), _("Normal"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	allow_player_(utils::string_bool(cfg_.get_attribute("allow_player"), true)),
	allow_changes_(utils::string_bool(cfg_.get_attribute("allow_changes"), true)),
	enabled_(!parent_->params_.saved_game), changed_(false),
	llm_(parent.era_sides_, enabled_ ? &combo_leader_ : NULL, enabled_ ? &combo_gender_ : NULL)
{
	// convert ai controllers
	if (cfg_["controller"] == "human_ai"
			|| cfg_["controller"] == "network_ai")
		cfg_["controller"] = "ai";
	if(allow_player_ && enabled_) {
		controller_ = parent_->default_controller_;
	} else {
		size_t i = CNTR_NETWORK;
		if(!allow_player_)
		{
			if(cfg_["controller"] == "null") {
				controller_ = CNTR_EMPTY;
			} else {
				cfg_["controller"] = controller_names[CNTR_COMPUTER];
				controller_ = static_cast<mp::controller>(CNTR_COMPUTER);
			}
		} else {
			if (cfg_["controller"] == "network"
					|| cfg_["controller"] == "human")
			{
				cfg_["controller"] = "reserved";

			}
			for(; i != CNTR_LAST; ++i) {
				if(cfg_["controller"] == controller_names[i]) {
					controller_ = static_cast<mp::controller>(i);
					break;
				}
			}
		}
	}

	slider_gold_.set_min(20);
	slider_gold_.set_max(800);
	slider_gold_.set_increment(25);
	slider_gold_.set_value(lexical_cast_default<int>(cfg_.get_attribute("gold"), 100));
	slider_gold_.set_measurements(80, 16);

	slider_income_.set_min(-2);
	slider_income_.set_max(18);
	slider_income_.set_increment(1);
	slider_income_.set_value(lexical_cast_default<int>(cfg_.get_attribute("income"), 0));
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

	std::vector<std::string>::const_iterator itor =
			std::find(parent_->team_names_.begin(), parent_->team_names_.end(),
			cfg_.get_attribute("team_name"));
	if(itor == parent_->team_names_.end()) {
		assert(!parent_->team_names_.empty());
		team_ = 0;
	} else {
		team_ = itor - parent_->team_names_.begin();
	}
	if(!cfg_.get_attribute("colour").empty()) {
		colour_ = game_config::color_info(cfg_["colour"]).index() - 1;
	}
	llm_.set_colour(colour_);

	update_faction_combo();

	if (const config &ai = cfg_.child("ai"))
		ai_algorithm_ = lexical_cast_default<std::string>(ai["ai_algorithm"], "default");
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
		std::string leader_type;
		BOOST_FOREACH (const config &side_unit, cfg.child_range("unit"))
		{
			if (utils::string_bool(side_unit["canrecruit"], false)) {
				leader_type = side_unit["type"];
				gender_ = side_unit["gender"];
				break;
			}
		}
		std::vector<std::string> leader_name_pseudolist;
		if(leader_type.empty()) {
			leader_name_pseudolist.push_back("-");
		} else {
			const unit_type *leader_name = unit_types.find(leader_type);
			if (!leader_name) {
				leader_name_pseudolist.push_back("-");
			} else {
				leader_name_pseudolist.push_back(leader_name->get_gender_unit_type(gender_).type_name());
			}
		}
		combo_leader_.set_items(leader_name_pseudolist);
		combo_leader_.set_selected(0);
		std::vector<std::string> gender_name_pseudolist;

		if (!gender_.empty()) {
			if (leader_type.empty() || !unit_types.find(leader_type)) {
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
		}
		combo_gender_.set_items(gender_name_pseudolist);
		combo_gender_.set_selected(0);
	} else if(parent_->params_.use_map_settings) {
		// gold, income, team, and colour are only suggestions unless explicitly locked
		slider_gold_.enable(!gold_lock_);
		label_gold_.enable(!gold_lock_);
		slider_income_.enable(!income_lock_);
		label_income_.enable(!income_lock_);
		combo_team_.enable(!team_lock_);
		combo_colour_.enable(!colour_lock_);

		// Set the leader and gender
		leader_ = cfg_.get_attribute("type");
		gender_ = cfg_.get_attribute("gender");
		if(!leader_.empty()) {
			combo_leader_.enable(false);
			combo_gender_.enable(false);
			llm_.set_leader_combo(NULL);
			llm_.set_gender_combo(NULL);
			std::vector<std::string> leader_name_pseudolist;
			const unit_type *leader_name = unit_types.find(leader_);
			if (!leader_name) {
				leader_name_pseudolist.push_back("?");
			} else {
				leader_name_pseudolist.push_back(leader_name->type_name());
			}
			combo_leader_.set_items(leader_name_pseudolist);
			combo_leader_.set_selected(0);
			std::vector<std::string> gender_name_pseudolist;
			if (!gender_.empty()) {
				if (!leader_name) {
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
			if(!cfg_.get_attribute("faction").empty()) {
				// Choose based on faction
				find.push_back(cfg_["faction"]);
				search_field = "id";
			} else if(utils::string_bool(cfg["faction_from_recruit"]) && !cfg_.get_attribute("recruit").empty()) {
				// Choose based on recruit
				find = utils::split(cfg_["recruit"]);
				search_field = "recruit";
			} else if(!leader_.empty()) {
				// Choose based on leader
				find.push_back(leader_);
				search_field = "leader";
			}
			// Pick the first faction with the greater amount of data matching the criteria
			int faction_index = 0;
			int best_score = 0;
			std::vector<const config*>::const_iterator faction = parent.era_sides_.begin();
			while(faction != parent.era_sides_.end()) {
				int faction_score = 0;
				const config& side = (**faction);
				std::vector<std::string> recruit;
				recruit = utils::split(side[search_field]);
				std::vector<std::string>::const_iterator search = find.begin();
				while(search != find.end()) {
					for(itor = recruit.begin(); itor != recruit.end(); ++itor) {
						if(*itor == *search) {
							faction_score++;
							break;
						}
					}
					++search;
				}
				if(faction_score > best_score) {
					best_score = faction_score;
					faction_ = faction_index;
				}
				++faction;
				faction_index++;
			}
			if (faction_) {
				llm_.update_leader_list(faction_);
				llm_.update_gender_list(llm_.get_leader());
				combo_faction_.enable(false);
			}
		} else {
			combo_faction_.enable(false);
		}
	}

	update_ui();
}

connect::side::side(const side& a) :
	parent_(a.parent_), cfg_(a.cfg_),
	index_(a.index_), id_(a.id_), player_id_(a.player_id_),  save_id_(a.save_id_),
	current_player_(a.current_player_),
	controller_(a.controller_),
	faction_(a.faction_), team_(a.team_), colour_(a.colour_),
	gold_(a.gold_), income_(a.income_), leader_(a.leader_),
	gender_(a.gender_),
	ai_algorithm_(a.ai_algorithm_),
	ready_for_start_(a.ready_for_start_),
	gold_lock_(a.gold_lock_),
	income_lock_(a.income_lock_),
	team_lock_(a.team_lock_),
	colour_lock_(a.colour_lock_),
	player_number_(a.player_number_), combo_controller_(a.combo_controller_),
	orig_controller_(a.orig_controller_),
	combo_ai_algorithm_(a.combo_ai_algorithm_),
	combo_faction_(a.combo_faction_), combo_leader_(a.combo_leader_), combo_gender_(a.combo_gender_),
	combo_team_(a.combo_team_), combo_colour_(a.combo_colour_),
	slider_gold_(a.slider_gold_), slider_income_(a.slider_income_),
	label_gold_(a.label_gold_), label_income_(a.label_income_),
	allow_player_(a.allow_player_), allow_changes_(a.allow_changes_),
	enabled_(a.enabled_), changed_(a.changed_), llm_(a.llm_)
{
	llm_.set_colour(colour_);
	llm_.set_leader_combo((enabled_ && leader_.empty()) ? &combo_leader_ : NULL);
	llm_.set_gender_combo((enabled_ && leader_.empty()) ? &combo_gender_ : NULL);
	// FIXME: this is an ugly hack to force updating the gender list when the side
	// widget is initialized. Need an optimal way. -- shadowmaster
	llm_.update_gender_list(llm_.get_leader());
}

void connect::side::add_widgets_to_scrollpane(gui::scrollpane& pane, int pos)
{
	pane.add_widget(&player_number_,     0, 5 + pos);
	pane.add_widget(combo_controller_.get(), 20, 5 + pos);
	pane.add_widget(&orig_controller_,  20 + (combo_controller_->width() - orig_controller_.width()) / 2,
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
	int drop_target;
	if ( ( drop_target = combo_controller_->get_drop_target() )> -1)
	{
		const std::string target_id = parent_->sides_[drop_target].get_player_id();
		const mp::controller target_controller = parent_->sides_[drop_target].get_controller();
		const std::string target_ai = parent_->sides_[drop_target].ai_algorithm_;

		parent_->sides_[drop_target].ai_algorithm_ = ai_algorithm_;
		if (player_id_.empty())
		{
			parent_->sides_[drop_target].set_controller(controller_);
		} else {
			parent_->sides_[drop_target].set_player_id(player_id_);
		}

		ai_algorithm_ = target_ai;
		if (target_id.empty())
		{
			set_controller(target_controller);
		} else {
			set_player_id(target_id);
		}
		changed_ = true;
		parent_->sides_[drop_target].changed_ = true;
		init_ai_algorithm_combo();
		update_ui();
		parent_->sides_[drop_target].init_ai_algorithm_combo();
		parent_->sides_[drop_target].update_ui();
	}
	else if(combo_controller_->changed() && combo_controller_->selected() >= 0) {
		const int cntr_last = (save_id_.empty() ? CNTR_LAST-1 : CNTR_LAST) - (parent_->local_only_ ? 1 : 0);
		if (combo_controller_->selected() == cntr_last) {
			update_controller_ui();
		} else if (combo_controller_->selected() < cntr_last) {
			// Correct entry number if CNTR_NETWORK is not allowed for combo_controller_
			controller_ = mp::controller(combo_controller_->selected() + (parent_->local_only_ ? 1 : 0));
			player_id_ = "";
			ready_for_start_ = false;
			changed_ = true;
		} else {
			// give user second side
			size_t user = combo_controller_->selected() - cntr_last - 1;

			const std::string new_id = parent_->users_[user].name;
			if (new_id != player_id_) {
				player_id_ = new_id;
				controller_ = parent_->users_[user].controller;
				ready_for_start_ = true;
				changed_ = true;
			}
		}
		update_ai_algorithm_combo();
	}

	if (combo_controller_->hidden())
		combo_controller_->hide(false);
	if(!enabled_)
		return;

	if (combo_colour_.changed() && combo_colour_.selected() >= 0) {
		colour_ = combo_colour_.selected();
		llm_.set_colour(colour_);
		update_faction_combo();
		llm_.set_leader_combo(&combo_leader_);
		llm_.set_gender_combo(&combo_gender_);
		changed_ = true;
	}
	if (combo_faction_.changed() && combo_faction_.selected() >= 0) {
		faction_ = combo_faction_.selected();
		llm_.update_leader_list(faction_);
		llm_.update_gender_list(llm_.get_leader());
		changed_ = true;
	}
	if (combo_ai_algorithm_.changed() && combo_ai_algorithm_.selected() >= 0) {
		ai_algorithm_ = parent_->ai_algorithms_[combo_ai_algorithm_.selected()]->id;
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

bool connect::side::ready_for_start() const
{
	//sides without players are always ready
	if (!allow_player_)
		return true;

	//the host and the AI are always ready
	if ((controller_ == mp::CNTR_COMPUTER) ||
		(controller_ == mp::CNTR_EMPTY) ||
		(controller_ == mp::CNTR_LOCAL))
		return true;

	return ready_for_start_;
}

bool connect::side::available(const std::string& name) const
{
	if (name.empty())
	{
		return allow_player_
			&& ((controller_ == CNTR_NETWORK && player_id_.empty())
					|| controller_ == CNTR_RESERVED);
	}
	return allow_player_
		&& ((controller_ == CNTR_NETWORK && player_id_.empty())
			|| (controller_ == CNTR_RESERVED && current_player_ == name));
}

bool connect::side::allow_player() const
{
	return allow_player_;
}

void connect::side::update_controller_ui()
{
	if (player_id_.empty()) {
		combo_controller_->set_selected(controller_ - (parent_->local_only_ ? 1 : 0));
	} else {
		connected_user_list::iterator player = parent_->find_player(player_id_);

		if (player != parent_->users_.end()) {
			const int no_reserve = save_id_.empty()?-1:0;
			combo_controller_->set_selected(CNTR_LAST + no_reserve + 1 + (player - parent_->users_.begin()) - (parent_->local_only_ ? 1 : 0));
		} else {
			assert(parent_->local_only_ != true);
			combo_controller_->set_selected(CNTR_NETWORK);
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
	assert(parent_->ai_algorithms_.empty() == false);

	int sel = 0;
	std::vector<ai::description*> &ais_list = parent_->ai_algorithms_;
	std::vector<std::string> ais;
	int i = 0;
	BOOST_FOREACH (const ai::description *desc,  ais_list){
		ais.push_back(desc->text);
		if (desc->id==ai_algorithm_){
			sel = i;
		}
		i++;
	}
	combo_ai_algorithm_.set_items(ais);
	combo_ai_algorithm_.set_selected(sel);
}

void connect::side::update_faction_combo()
{
	std::vector<std::string> factions;
	BOOST_FOREACH (const config *faction, parent_->era_sides_)
	{
		const std::string& name = (*faction)["name"];
		const std::string& icon = (*faction)["image"];
		if (!icon.empty()) {
			std::string rgb = (*faction)["flag_rgb"];
			if (rgb.empty())
				rgb = "magenta";

			factions.push_back(IMAGE_PREFIX + icon + "~RC(" + rgb + ">" + lexical_cast<std::string>(colour_+1) + ")" + COLUMN_SEPARATOR + name);
		} else {
			factions.push_back(name);
		}
	}
	combo_faction_.set_items(factions);
	combo_faction_.set_selected(faction_);
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
	config res;

	// If the user is allowed to change type, faction, leader etc,
	// then import their new values in the config.
	if(enabled_ && !parent_->era_sides_.empty()) {
		// Merge the faction data to res
		res.append(*(parent_->era_sides_[faction_]));
		res["faction_name"] = res["name"];
	}
	res.append(cfg_);
	if (cfg_.get_attribute("side").empty()
			|| cfg_["side"] != lexical_cast<std::string>(index_ + 1))
	{
		res["side"] = lexical_cast<std::string>(index_ + 1);
	}
	res["controller"] = controller_names[controller_];
	res["current_player"] = player_id_.empty() ? current_player_ : player_id_;
	res["id"] = id_;

	if (player_id_.empty()) {
		std::string description;
		switch(controller_) {
		case CNTR_NETWORK:
			description = N_("(Vacant slot)");
			break;
		case CNTR_LOCAL:
			if(enabled_ && cfg_.get_attribute("save_id").empty()) {
				res["save_id"] = preferences::login() + res["side"].str();
			}
			res["player_id"] = preferences::login() + res["side"].str();
			res["current_player"] = preferences::login();
			description = N_("Anonymous local player");
			break;
		case CNTR_COMPUTER:
			if(enabled_ && cfg_.get_attribute("save_id").empty()) {
				res["save_id"] = "ai" + res["side"].str();
			}
			{
				utils::string_map symbols;
				if (allow_player_) {
					const config &ai_cfg = ai::configuration::get_ai_config_for(ai_algorithm_);
					res.add_child("ai",ai_cfg);
					symbols["playername"] = ai_cfg["description"];
				} else { // do not import default ai cfg here - all is set by scenario config
					symbols["playername"] = _("Computer Player");
				}
				symbols["side"] = res["side"].str();
				description = vgettext("$playername $side",symbols);
			}
			break;
		case CNTR_EMPTY:
			description = N_("(Empty slot)");
			res["no_leader"] = "yes";
			break;
		case CNTR_RESERVED:
			{
				utils::string_map symbols;
				symbols["playername"] = current_player_;
				description = vgettext("(Reserved for $playername)",symbols);
			}
			break;
		case CNTR_LAST:
		default:
			description = N_("(empty)");
			assert(false);
			break;
		}
		res["user_description"] = t_string(description, "wesnoth");
	} else {
		res["player_id"] = player_id_ + res["side"];
		if(enabled_ && cfg_.get_attribute("save_id").empty()) {
			res["save_id"] = player_id_ + res["side"].str();
		}

		res["user_description"] = player_id_;
	}

	res["name"] = res["user_description"];
	res["allow_changes"] = (enabled_ && allow_changes_) ? "yes" : "no";

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
		res["allow_player"] = allow_player_ ? "yes" : "no";
		res["colour"] = lexical_cast<std::string>(colour_ + 1);
		res["gold"] = lexical_cast<std::string>(gold_);
		res["income"] = lexical_cast<std::string>(income_);

		if(!parent_->params_.use_map_settings || res["fog"].empty() || (res["fog"] != "yes" && res["fog"] != "no")) {
			res["fog"] = parent_->params_.fog_game ? "yes" : "no";
		}

		if(!parent_->params_.use_map_settings || res["shroud"].empty() || (res["shroud"] != "yes" && res["shroud"] != "no")) {
			res["shroud"] = parent_->params_.shroud_game ? "yes" : "no";
		}

		res["share_maps"] = parent_->params_.share_maps ? "yes" : "no";
		res["share_view"] =  parent_->params_.share_view ? "yes" : "no";
		if(!parent_->params_.use_map_settings || res["village_gold"].empty())
			res["village_gold"] = lexical_cast<std::string>(parent_->params_.village_gold);
	}

	if(parent_->params_.use_map_settings && enabled_) {
		config trimmed = cfg_;
		trimmed["side"] = "";
		trimmed["controller"] = "";
		trimmed["id"] = "";
		trimmed["team_name"] = "";
		trimmed["user_team_name"] = "";
		trimmed["colour"] = "";
		trimmed["gold"] = "";
		trimmed["income"] = "";
		trimmed["allow_changes"] = "";
/*
		trimmed["allow_player"] = "";
		trimmed["colour_lock"] = "";
		trimmed["faction"] = "";
		trimmed["gender"] = "";
		trimmed["gold_lock"] = "";
		trimmed["income_lock"] = "";
		trimmed["recruit"] = "";
		trimmed["team_lock"] = "";
*/
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
	player_id_ = "";

	update_ui();
}

mp::controller connect::side::get_controller() const
{
	return controller_;
}

void connect::side::update_user_list()
{
	bool name_present = false;

	typedef std::vector<std::string> name_list;

	name_list list = parent_->player_types_;
	if (!save_id_.empty())
		list.push_back(_("Reserved"));
	list.push_back(_("--give--"));

	connected_user_list::const_iterator itor;
	for (itor = parent_->users_.begin(); itor != parent_->users_.end();
			++itor) {
		list.push_back(itor->name);
		if (itor->name == player_id_)
			name_present = true;
	}

	if (name_present == false) {
		player_id_ = "";
	}

	combo_controller_->set_items(list);
	update_controller_ui();
}

const std::string& connect::side::get_player_id() const
{
	return player_id_;
}

void connect::side::set_player_id(const std::string& player_id)
{
	connected_user_list::iterator i = parent_->find_player(player_id);
	if (i != parent_->users_.end()) {
		player_id_ = player_id;
		controller_ = i->controller;
	}
	update_ui();
}

void connect::side::set_ready_for_start(bool ready_for_start)
{
	ready_for_start_ = ready_for_start;
}

const std::string& connect::side::get_save_id() const
{
	return save_id_;
}

void connect::side::import_network_user(const config& data)
{
	if (controller_ == CNTR_RESERVED)
		set_ready_for_start(true);

	player_id_ = data["name"];
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
	player_id_ = "";
	controller_ = controller;

	if ((controller == mp::CNTR_NETWORK) || (controller == mp::CNTR_RESERVED))
		ready_for_start_ = false;

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

	if(utils::string_bool(
			(*parent_->era_sides_[faction_]).get_attribute("random_faction"),
			false))
	{
		std::vector<std::string> faction_choices, faction_excepts;
		faction_choices = utils::split((*parent_->era_sides_[faction_]).get_attribute("choices"));
		if(faction_choices.size() == 1 && faction_choices.front() == "") {
			faction_choices.clear();
		}
		faction_excepts = utils::split((*parent_->era_sides_[faction_]).get_attribute("except"));
		if(faction_excepts.size() == 1 && faction_excepts.front() == "") {
			faction_excepts.clear();
		}

		// Builds the list of sides eligible for choice (nonrandom factions)
		std::vector<int> nonrandom_sides;
		int num = -1;
		BOOST_FOREACH (const config *i, parent_->era_sides_)
		{
			++num;
			if ((*i)["random_faction"] != "yes") {
				const std::string& faction_id = (*i)["id"];
				if (
					!faction_choices.empty() &&
					std::find(faction_choices.begin(),faction_choices.end(),faction_id) == faction_choices.end()
				)
					continue;
				if (
					!faction_excepts.empty() &&
					std::find(faction_excepts.begin(),faction_excepts.end(),faction_id) != faction_excepts.end()
				)
					continue;
				nonrandom_sides.push_back(num);
			}
		}

		if (nonrandom_sides.size() == 0) {
			throw config::error(_("Only random sides in the current era."));
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
		const unit_type *ut = unit_types.find(leader_.empty() ? llm_.get_leader() : leader_);

		if (ut) {
			const std::vector<unit_race::GENDER> glist = ut->genders();
			if (!glist.empty()) {
				const int gchoice = rand() % glist.size();
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

connect::connect(game_display& disp, const config& game_config,
		chat& c, config& gamelist, const mp_game_settings& params, const int num_turns,
		mp::controller default_controller, bool local_players_only) :
	mp::ui(disp, _("Game Lobby: ") + params.name, game_config, c, gamelist),
	local_only_(local_players_only),
	level_(),
	state_(),
	params_(params),
	num_turns_(num_turns),
	era_sides_(),
	player_types_(),
	player_factions_(),
	player_teams_(),
	player_colours_(),
	ai_algorithms_(),
	team_names_(),
	user_team_names_(),
	team_prefix_(std::string(_("Team")) + " "),
	sides_(),
	users_(),
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
	cancel_(video(), _("Cancel")),
	combo_control_group_(new gui::drop_group_manager())
{
	load_game();

	if(get_result() == QUIT
		|| get_result() == CREATE)
		return;
	if (level_["id"].empty()) {
		throw config::error(_("The scenario is invalid because it has no id."));
	}
	lists_init();
	if(sides_.empty()) {
		throw config::error(_("The scenario is invalid because it has no sides."));
	}
	// Send Initial information
	config response;
	config& create_game = response.add_child("create_game");
	create_game["name"] = params.name;
	if(params.password.empty() == false) {
		create_game["password"] = params.password;
	}
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
	network::send_data(response, 0, true);

	// Adds the current user as default user.
	users_.push_back(connected_user(preferences::login(), CNTR_LOCAL, 0));
	update_user_combos();
	// Take the first available side or available side with id == login
	int side_choice = -1;
	for(side_list::const_iterator s = sides_.begin(); s != sides_.end(); ++s) {
		if (s->allow_player()) {
			if (side_choice == -1)
				side_choice = s - sides_.begin();
			if(s->get_current_player() == preferences::login()) {
				sides_[s - sides_.begin()].set_player_id(preferences::login());
				side_choice = gamemap::MAX_PLAYERS;
			}
		}
	}

	if (side_choice != -1
			&& side_choice != gamemap::MAX_PLAYERS)
	{
		if (sides_[side_choice].get_player_id() == "")
			sides_[side_choice].set_player_id(preferences::login());
	}

	// Updates the "level_" variable, now that sides are loaded
	update_level();
	update_playerlist_state(true);

	// If we are connected, send data to the connected host
	network::send_data(level_, 0, true);
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
			network::send_data(cfg, 0, true);
		}

		set_result(QUIT);
		return;
	}

	if (launch_.pressed()) {
		if (can_start_game())
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
	network::send_data(lock, 0, true);
	update_and_send_diff(true);

	// Build the gamestate object after updating the level

	level_to_gamestate(level_, state_);

	network::send_data(config("start_game"), 0, true);
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

void connect::process_network_data(const config& data, const network::connection sock)
{
	ui::process_network_data(data, sock);

	if(data.child("leave_game")) {
		set_result(QUIT);
		return;
	}

	if (!data["side_drop"].empty()) {
		const int side_drop = lexical_cast_default<int>(data["side_drop"], 0) - 1;
		if(side_drop >= 0 && side_drop < int(sides_.size())) {
			connected_user_list::iterator player = find_player(sides_[side_drop].get_player_id());
			sides_[side_drop].reset(sides_[side_drop].get_controller());
			if (player != users_.end()) {
				users_.erase(player);
				update_user_combos();
			}
			update_and_send_diff();
			update_playerlist_state(true);
			return;
		}
	}

	if (!data["side"].empty()) {
		int side_taken = lexical_cast_default<int>(data["side"], 0) - 1;

		// Checks if the connecting user has a valid and unique name.
		const std::string name = data["name"];
		if(name.empty()) {
			config response;
			response["failed"] = "yes";
			network::send_data(response, sock, true);
			ERR_CF << "ERROR: No username provided with the side.\n";
			return;
		}

		connected_user_list::iterator player = find_player(name);
		if(player != users_.end()) {
			/**
			 * @todo Seems like a needless limitation to only allow one side
			 * per player.
			 */
			if(find_player_side(name) != -1) {
				config response;
				response["failed"] = "yes";
				response["message"] = "The nick '" + name + "' is already in use.";
				network::send_data(response, sock, true);
				return;
			} else {
				users_.erase(player);
				config observer_quit;
				observer_quit.add_child("observer_quit")["name"] = name;
				network::send_data(observer_quit, 0, true);
				update_user_combos();
			}
		}

		// Assigns this user to a side
		if(side_taken >= 0 && side_taken < int(sides_.size())) {
			if(!sides_[side_taken].available(name)) {
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
					response["failed"] = "yes";
					network::send_data(response, sock, true);
					config kick;
					kick["username"] = data["name"];
					config res;
					res.add_child("kick", kick);
					network::send_data(res, 0, true);
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

			// Go thought and check if more sides are reserved
			// For this player
			std::for_each(sides_.begin(), sides_.end(), boost::bind(&connect::take_reserved_side, this,_1, data));
			update_playerlist_state(false);
			update_and_send_diff();

			LOG_NW << "sent player data\n";

		} else {
			ERR_CF << "tried to take illegal side: " << side_taken << '\n';
			config response;
			response["failed"] = "yes";
			network::send_data(response, sock, true);
		}
	}

	if (const config &change_faction = data.child("change_faction")) {
		int side_taken = find_player_side(change_faction["name"]);
		if(side_taken != -1) {
			sides_[side_taken].import_network_user(change_faction);
			sides_[side_taken].set_ready_for_start(true);
			update_playerlist_state();
			update_and_send_diff();
		}
	}

	if (const config &c = data.child("observer"))
	{
		const t_string &observer_name = c["name"];
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
	if (const config &c = data.child("observer_quit"))
	{
		const t_string &observer_name = c["name"];
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

void connect::take_reserved_side(connect::side& side, const config& data)
{
	if (side.available(data["name"])
			&& side.get_current_player() == data["name"])
	{
		side.import_network_user(data);
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

	network::send_data(config("join_game"), 0, true);

	network::send_data(level_, sock, true);
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
	if(!local_only_) {
		player_types_.push_back(_("Network Player"));
	}
	player_types_.push_back(_("Local Player"));
	player_types_.push_back(_("Computer Player"));
	player_types_.push_back(_("Empty"));

	BOOST_FOREACH (const config *faction, era_sides_) {
		player_factions_.push_back((*faction)["name"]);
	}

	// AI algorithms
	ai_algorithms_ = ai::configuration::get_available_ais();

	// Factions
	config::child_itors sides = current_config()->child_range("side");

	// Teams
	if(params_.use_map_settings) {
		int side_num = 1;
		BOOST_FOREACH (config &side, sides)
		{
			t_string &team_name = side["team_name"];
			t_string &user_team_name = side["user_team_name"];

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
				user_team_names_.push_back(user_team_name.to_serialized());
				player_teams_.push_back(user_team_name.str());
			}
			++side_num;
		}
	} else {
		std::vector<std::string> map_team_names;
		int _side_num = 1;
		BOOST_FOREACH (config &side, sides)
		{
			const std::string side_num = lexical_cast<std::string>(_side_num);
			t_string &team_name = side["team_name"];

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
			++_side_num;
		}
	}

	// Colours
	for(int i = 0; i < gamemap::MAX_PLAYERS; ++i) {
		player_colours_.push_back(get_colour_string(i));
	}

	// Populates "sides_" from the level configuration
	int index = 0;
	BOOST_FOREACH (const config &s, sides) {
		sides_.push_back(side(*this, s, index++));
	}
	int offset=0;
	// This function must be called after the sides_ vector is fully populated.
	for(side_list::iterator s = sides_.begin(); s != sides_.end(); ++s) {
		const int side_num = s - sides_.begin();
		const int spos = 60 * (side_num-offset);
		if(!s->allow_player()) {
			offset++;
			continue;
		}

		s->add_widgets_to_scrollpane(scroll_pane_, spos);
	}
}

void connect::load_game()
{
	if(params_.saved_game) {
		try{
			savegame::loadgame load(disp(), game_config(), state_);
			load.load_multiplayer_game();
			load.fill_mplevel_config(level_);
		}
		catch (load_game_cancelled_exception){
			set_result(CREATE);
			return;
		}
	} else {
		level_.clear();
		params_.saved_game = false;
		params_.mp_scenario = params_.scenario_data["id"];
		level_.merge_with(params_.scenario_data);
		level_["turns"] = lexical_cast<std::string>(num_turns_);
		level_.add_child("multiplayer", params_.to_config());

		params_.hash = level_.hash();
		level_["next_underlying_unit_id"] = lexical_cast<std::string>(0);
		n_unit::id_manager::instance().clear();

		if (params_.random_start_time)
		{
			if (!tod_manager::is_start_ToD(level_["random_start_time"]))
			{
				level_["random_start_time"] = "yes";
			}
		}
		else
		{
			level_["random_start_time"] = "no";
		}

		level_["experience_modifier"] = lexical_cast<std::string>(params_.xp_modifier);
		level_["random_seed"] = lexical_cast<std::string>(state_.rng().get_random_seed());
	}

	// Add the map name to the title.
	append_to_title(" - " + level_["name"]);


	std::string era = params_.mp_era;
	if (params_.saved_game) {
		if (const config &c = level_.child("snapshot").child("era"))
			era = c["id"];
	}

	// Initialize the list of sides available for the current era.
	const config &era_cfg = game_config().find_child("era", "id", era);
	if (!era_cfg) {
		if (!params_.saved_game)
		{
			utils::string_map i18n_symbols;
			i18n_symbols["era"] = era;
			throw config::error(vgettext("Cannot find era $era", i18n_symbols));
		}
		// FIXME: @todo We should tell user about missing era but still load game
		WRN_CF << "Missing era in MP load game " << era << "\n";
	}
	else
	{
		era_sides_.clear();
		BOOST_FOREACH (const config &e, era_cfg.child_range("multiplayer_side")) {
			era_sides_.push_back(&e);
		}
		level_.add_child("era", era_cfg);
	}

	gold_title_label_.hide(params_.saved_game);
	income_title_label_.hide(params_.saved_game);

	// This will force connecting clients to be using the same version number as us.
	level_["version"] = game_config::version;

	level_["observer"] = params_.allow_observers ? "yes" : "no";

	if(level_["objectives"].empty()) {
		level_["objectives"] = "<big>" + t_string(N_("Victory:"), "wesnoth") +
			"</big>\n<span foreground=\"#00ff00\">" +
			t_string(N_("Defeat enemy leader(s)"), "wesnoth") + "</span>";
	}
}

config* connect::current_config(){
	config* cfg_level = NULL;

	//It might make sense to invent a mechanism of some sort to check whether a config node contains information
	//that you can load from(side information, specifically)
	config &snapshot = level_.child("snapshot");
	if (snapshot && snapshot.child("side")) {
		// Savegame
		cfg_level = &snapshot;
	} else if (!level_.child("side")) {
		// Start-of-scenario save, the info has to be taken from the starting_pos
		cfg_level = &state_.starting_pos;
	} else {
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
		tod_manager tod_mng(level_,atoi(level_["turns"].c_str()),&state_);
	}

	config diff = level_.get_diff(old_level);
	if (!diff.empty()) {
		config scenario_diff;
		scenario_diff.add_child("scenario_diff", diff);
		network::send_data(scenario_diff, 0, true);
	}
}

bool connect::sides_ready() const
{
	for(side_list::const_iterator itor = sides_.begin(); itor != sides_.end(); ++itor) {
		if (!itor->ready_for_start()) {
			return false;
		}
	}
	return true;
}

bool connect::sides_available() const
{
	for(side_list::const_iterator itor = sides_.begin(); itor != sides_.end(); ++itor) {
		if (itor->available())
			return true;
	}
	return false;
}

bool connect::can_start_game() const
{
	if(!sides_ready()) {
		return false;
	}

	/*
	 * If at least one human player is slotted with a player/ai we're allowed
	 * to start. Before used a more advanced test but it seems people are
	 * creative in what is used in multiplayer [1] so use a simpler test now.
	 * [1] http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=568029
	 */
	BOOST_FOREACH(const side& s, sides_) {
		if(s.get_controller() != CNTR_EMPTY) {
			if(s.allow_player()) {
				return true;
			}
		}
	}

	return false;
}

void connect::update_playerlist_state(bool silent)
{
	/**
	 * @todo Better message for 1.9.
	 *
	 * If !sides_available() && !can_start() we could show a better message,
	 * but that needs to wait until the stringfreeze is lifted.
	 */
	waiting_label_.set_text(!can_start_game()
			? _("Waiting for players to join...")
			: "");
	launch_.enable(can_start_game());

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
		if (itor->get_player_id() == id)
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

} // end namespace mp

