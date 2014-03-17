/*
   Copyright (C) 2007 - 2014
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Prepare to join a multiplayer-game.
 */
#include "multiplayer_connect.hpp"

#include "ai/configuration.hpp"
#include "dialogs.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "wml_separators.hpp"
#include "sound.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_mp_connect("mp/connect");
#define DBG_MP LOG_STREAM(debug, log_mp_connect)
#define LOG_MP LOG_STREAM(info, log_mp_connect)

namespace mp {

std::vector<std::string> controller_options_names(
	const std::vector<controller_option>& controller_options)
{
	std::vector<std::string> names;
	BOOST_FOREACH(const controller_option& option, controller_options) {
		names.push_back(option.second);
	}

	return names;
}

connect::side::side(connect& parent, side_engine_ptr engine) :
	parent_(&parent),
	engine_(engine),
	gold_lock_(engine_->cfg()["gold_lock"].to_bool(
		parent_->force_lock_settings())),
	income_lock_(engine_->cfg()["income_lock"].to_bool(
		parent_->force_lock_settings())),
	team_lock_(engine_->cfg()["team_lock"].to_bool(
		parent_->force_lock_settings())),
	color_lock_(engine_->cfg()["color_lock"].to_bool(
		parent_->force_lock_settings())),
	changed_(false),
	label_player_number_(parent.video(), str_cast(engine_->index() + 1),
		font::SIZE_LARGE, font::LOBBY_COLOR),
	label_original_controller_(parent.video(), engine_->current_player(),
		font::SIZE_SMALL),
	label_gold_(parent.video(), "100", font::SIZE_SMALL, font::LOBBY_COLOR),
	label_income_(parent.video(), _("Normal"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	combo_controller_(new gui::combo_drag(parent.disp(),
		std::vector<std::string>(), parent.combo_control_group_)),
	combo_ai_algorithm_(parent.disp(), std::vector<std::string>()),
	combo_faction_(parent.disp(), std::vector<std::string>()),
	label_leader_name_(parent.video(), engine_->cfg()["name"], font::SIZE_SMALL),
	combo_leader_(parent.disp(), std::vector<std::string>()),
	combo_gender_(parent.disp(), std::vector<std::string>()),
	combo_team_(parent.disp(), engine_->player_teams()),
	combo_color_(parent.disp(), parent.player_colors_),
	slider_gold_(parent.video()),
	slider_income_(parent.video())
{
	DBG_MP << "initializing side" << std::endl;

	slider_gold_.set_min(20);
	slider_gold_.set_max(800);
	slider_gold_.set_increment(25);
	slider_gold_.set_value(engine_->cfg()["gold"].to_int(100));
	slider_gold_.set_measurements(80, 16);

	slider_income_.set_min(-2);
	slider_income_.set_max(18);
	slider_income_.set_increment(1);
	slider_income_.set_value(engine_->cfg()["income"]);
	slider_income_.set_measurements(50, 16);

	combo_faction_.enable(!parent_->params().saved_game);
	combo_leader_.enable(!parent_->params().saved_game);
	combo_gender_.enable(!parent_->params().saved_game);
	combo_team_.enable(!parent_->params().saved_game);
	combo_color_.enable(!parent_->params().saved_game);
	slider_gold_.hide(parent_->params().saved_game);
	slider_income_.hide(parent_->params().saved_game);
	label_gold_.hide(parent_->params().saved_game);
	label_income_.hide(parent_->params().saved_game);

	if (parent_->params().use_map_settings) {
		// Gold, income, team, and color are only suggestions.
		// (Unless explicitly locked).
		slider_gold_.enable(!gold_lock_);
		label_gold_.enable(!gold_lock_);
		slider_income_.enable(!income_lock_);
		label_income_.enable(!income_lock_);
		combo_team_.enable(!team_lock_);
		combo_color_.enable(!color_lock_);
	}

	update_ui();
}

connect::side::side(const side& a) :
	parent_(a.parent_),
	engine_(a.engine_),
	gold_lock_(a.gold_lock_),
	income_lock_(a.income_lock_),
	team_lock_(a.team_lock_),
	color_lock_(a.color_lock_),
	changed_(a.changed_),
	label_player_number_(a.label_player_number_),
	label_original_controller_(a.label_original_controller_),
	label_gold_(a.label_gold_),
	label_income_(a.label_income_),
	combo_controller_(a.combo_controller_),
	combo_ai_algorithm_(a.combo_ai_algorithm_),
	combo_faction_(a.combo_faction_),
	label_leader_name_(a.label_leader_name_),
	combo_leader_(a.combo_leader_),
	combo_gender_(a.combo_gender_),
	combo_team_(a.combo_team_),
	combo_color_(a.combo_color_),
	slider_gold_(a.slider_gold_),
	slider_income_(a.slider_income_)
{
}

connect::side::~side()
{
}

void connect::side::process_event()
{
	int drop_target;
	if ((drop_target = combo_controller_->get_drop_target()) > -1) {
		if (engine_->swap_sides_on_drop_target(drop_target)) {
			changed_ = true;
			parent_->sides_[drop_target].changed_ = true;

			update_ui();
			parent_->sides_[drop_target].update_ui();
		}
	} else if (combo_controller_->changed() &&
		combo_controller_->selected() >= 0) {

		changed_ = engine_->controller_changed(combo_controller_->selected());
		update_controller_ui();
	}
	if (combo_controller_->hidden()) {
		combo_controller_->hide(false);
	}

	if (parent_->params().saved_game) {
		return;
	}

	if (combo_color_.changed() && combo_color_.selected() >= 0) {
		engine_->set_color(combo_color_.selected());
		update_faction_combo();
		engine_->flg().reset_leader_combo(combo_leader_);
		engine_->flg().reset_gender_combo(combo_gender_);
		changed_ = true;
	}
	if (combo_faction_.changed() && combo_faction_.selected() >= 0) {
		engine_->flg().set_current_faction(combo_faction_.selected());
		engine_->flg().reset_leader_combo(combo_leader_);
		engine_->flg().reset_gender_combo(combo_gender_);
		changed_ = true;
	}
	if (combo_ai_algorithm_.changed() && combo_ai_algorithm_.selected() >= 0) {
		engine_->set_ai_algorithm(
			parent_->ai_algorithms_[combo_ai_algorithm_.selected()]->id);
		changed_ = true;
	}
	if (combo_leader_.changed() && combo_leader_.selected() >= 0) {
		engine_->flg().set_current_leader(combo_leader_.selected());
		engine_->flg().reset_gender_combo(combo_gender_);
		changed_ = true;
	}
	if (combo_gender_.changed() && combo_gender_.selected() >= 0) {
		engine_->flg().set_current_gender(combo_gender_.selected());
		changed_ = true;
	}
	if (combo_team_.changed() && combo_team_.selected() >= 0) {
		engine_->set_team(combo_team_.selected());
		changed_ = true;
	}
	if (slider_gold_.value() != engine_->gold()) {
		engine_->set_gold(slider_gold_.value());
		label_gold_.set_text(str_cast(engine_->gold()));
		changed_ = true;
	}
	if (slider_income_.value() != engine_->income()) {
		engine_->set_income(slider_income_.value());
		std::stringstream buf;
		if (engine_->income() < 0) {
			buf << _("(") << engine_->income() << _(")");
		} else if (engine_->income() > 0) {
			buf << _("+") << engine_->income();
		} else {
			buf << _("Normal");
		}
		label_income_.set_text(buf.str());
		changed_ = true;
	}
}

bool connect::side::changed()
{
	if (changed_) {
		changed_ = false;
		return true;
	}

	return false;
}

void connect::side::update_ui()
{
	update_faction_combo();
	engine_->flg().reset_leader_combo(combo_leader_);
	engine_->flg().reset_gender_combo(combo_gender_);

	update_controller_ui();

	combo_team_.set_selected(engine_->team());
	combo_color_.set_selected(engine_->color());
	slider_gold_.set_value(engine_->gold());
	label_gold_.set_text(str_cast(engine_->gold()));
	slider_income_.set_value(engine_->income());
	std::stringstream buf;
	if (engine_->income() < 0) {
		buf << _("(") << engine_->income() << _(")");
	} else if (engine_->income() > 0) {
		buf << _("+") << engine_->income();
	} else {
		buf << _("Normal");
	}
	label_income_.set_text(buf.str());
}

void connect::side::add_widgets_to_scrollpane(gui::scrollpane& pane, int pos)
{
	// Offset value to align labels in y-axis.
	int label_y_offset = (combo_leader_.height() - label_leader_name_.height()) / 2;

	pane.add_widget(&label_player_number_, 0, 5 + pos);

	pane.add_widget(combo_controller_.get(), 20, 5 + pos);
	pane.add_widget(&label_original_controller_, 20 +
		(combo_controller_->width() - label_original_controller_.width()) / 2,
		35 + pos + label_y_offset);
	pane.add_widget(&combo_ai_algorithm_, 20, 35 + pos);

	pane.add_widget(&combo_faction_, 135, 5 + pos);
	pane.add_widget(&label_leader_name_, 135 +
		(combo_faction_.width() - label_leader_name_.width()) / 2,
		35 + pos + label_y_offset);

	pane.add_widget(&combo_leader_, 250, 5 + pos);
	pane.add_widget(&combo_gender_, 250, 35 + pos);

	pane.add_widget(&combo_team_, 365, 5 + pos);
	pane.add_widget(&combo_color_, 365, 35 + pos);

	pane.add_widget(&slider_gold_, 475, 5 + pos);
	pane.add_widget(&label_gold_, 475 + 5, 35 + pos + label_y_offset);

	pane.add_widget(&slider_income_, 475 + slider_gold_.width(), 5 + pos);
	pane.add_widget(&label_income_, 475 + 5 + slider_gold_.width(),
		35 + pos + label_y_offset);
}

void connect::side::update_faction_combo()
{
	std::vector<std::string> factions;
	BOOST_FOREACH(const config* faction, engine_->flg().choosable_factions()) {
		const std::string& name = (*faction)["name"];
		const std::string& icon = (*faction)["image"];
		if (!icon.empty()) {
			std::string rgb = (*faction)["flag_rgb"];
			if (rgb.empty())
				rgb = "magenta";

			factions.push_back(IMAGE_PREFIX + icon + "~RC(" + rgb + ">" +
				lexical_cast<std::string>(engine_->color() + 1) + ")" +
				COLUMN_SEPARATOR + name);
		} else {
			factions.push_back(name);
		}
	}

	combo_faction_.enable(factions.size() > 1 && !parent_->params().saved_game);

	combo_faction_.set_items(factions);
	combo_faction_.set_selected(engine_->flg().current_faction_index());
}

void connect::side::update_controller_ui()
{
	// Update controllers combo.
	combo_controller_->set_items(controller_options_names(
		engine_->controller_options()));
	combo_controller_->set_selected(engine_->current_controller_index());
	combo_controller_->enable(engine_->controller_options().size() > 1);

	// Update AI algorithm combo.
	assert(!parent_->ai_algorithms_.empty());
	int sel = 0;
	int i = 0;
	std::vector<std::string> ais;
	BOOST_FOREACH(const ai::description* desc,  parent_->ai_algorithms_){
		ais.push_back(desc->text);
		if (desc->id == engine_->ai_algorithm()) {
			sel = i;
		}
		i++;
	}
	combo_ai_algorithm_.set_items(ais);
	combo_ai_algorithm_.set_selected(sel);
	// Ensures that the visually selected AI
	// is the one that will be loaded.
	engine_->set_ai_algorithm(parent_->ai_algorithms_[sel]->id);

	// Adjust the visibility of AI algorithm combo
	// and original controller label.
	if (!parent_->hidden()) {
		if (engine_->controller() == CNTR_COMPUTER) {
			// Computer selected, show AI combo.
			combo_ai_algorithm_.hide(false);

			label_original_controller_.hide(true);
		} else {
			// Computer de-selected, hide AI combo.
			combo_ai_algorithm_.hide(true);

			label_original_controller_.hide(false);
			label_original_controller_.set_text(engine_->current_player());
		}
	} else {
		combo_ai_algorithm_.hide(true);
	}
}

connect::connect(game_display& disp, const std::string& game_name,
	const config& game_config, chat& c, config& gamelist,
	connect_engine& engine) :
	mp::ui(disp, _("Game Lobby: ") + game_name, game_config, c, gamelist),
	player_colors_(),
	ai_algorithms_(),
	sides_(),
	engine_(engine),
	waiting_label_(video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	type_title_label_(video(), _("Player/Type"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	faction_name_title_label_(video(), _("Faction/Name"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	leader_gender_title_label_(video(), _("Leader/Gender"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	team_color_title_label_(video(), _("Team/Color"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	gold_title_label_(video(), _("Gold"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	income_title_label_(video(), _("Income"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	scroll_pane_(video()),
	launch_(video(), _("I’m Ready")),
	cancel_(video(), engine_.first_scenario() ? _("Cancel") : _("Quit")),
	combo_control_group_(new gui::drop_group_manager())
{
	DBG_MP << "setting up connect dialog" << std::endl;

	// Load game exception occured.
	if (engine_.level().empty()) {
		set_result(CREATE);
		return;
	}

	if (get_result() == QUIT || get_result() == CREATE) {
		return;
	}
	if (engine_.level()["id"].empty()) {
		throw config::error(_("The scenario is invalid because it has no id."));
	}

	ai_algorithms_ = ai::configuration::get_available_ais();

	// Colors.
	for(int i = 0; i < gamemap::MAX_PLAYERS; ++i) {
		player_colors_.push_back(get_color_string(i));
	}

	// Sides.
	BOOST_FOREACH(side_engine_ptr s, engine_.side_engines()) {
		sides_.push_back(side(*this, s));
	}
	if (sides_.empty() && !game_config::debug) {
		throw config::error(
			_("The scenario is invalid because it has no sides."));
	}

	// Add side widgets to scroll pane.
	int side_pos_y_offset = 0;
	BOOST_FOREACH(side& s, sides_) {
		if (!s.engine()->allow_player() && !game_config::debug) {
			continue;
		}

		s.add_widgets_to_scrollpane(scroll_pane_, side_pos_y_offset);
		side_pos_y_offset += 60;
	}

	append_to_title(" — " + engine_.level()["name"].t_str());
	gold_title_label_.hide(params().saved_game);
	income_title_label_.hide(params().saved_game);

	update_playerlist_state(true);
}

connect::~connect()
{
}

void connect::process_event()
{
	bool changed = false;

	BOOST_FOREACH(side& s, sides_) {
		s.process_event();
		if (s.changed()) {
			changed = true;
		}
	}

	if (cancel_.pressed()) {
		if (network::nconnections() > 0) {
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg, 0);
		}

		set_result(QUIT);
		return;
	}

	if (launch_.pressed()) {
		if (engine_.can_start_game()) {
			set_result(mp::ui::PLAY);
		}
	}

	// If something has changed in the level config,
	// send it to the network.
	if (changed) {
		update_playerlist_state();
		engine_.update_and_send_diff();
	}
}

void connect::hide_children(bool hide)
{
	DBG_MP << (hide ? "hiding" : "showing" ) <<
		" children widgets" << std::endl;

	ui::hide_children(hide);

	waiting_label_.hide(hide);
	scroll_pane_.hide(hide); // Scroll pane contents are automatically hidden.

	faction_name_title_label_.hide(hide);
	leader_gender_title_label_.hide(hide);
	team_color_title_label_.hide(hide);

	if (!params().saved_game) {
		gold_title_label_.hide(hide);
		income_title_label_.hide(hide);
	}

	launch_.hide(hide);
	cancel_.hide(hide);
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

	// Buttons.
	right_button->set_location(right - right_button->width(),
		bottom - right_button->height());
	left_button->set_location(right - right_button->width() -
		left_button->width()- gui::ButtonHPadding, bottom -
		left_button->height());

	type_title_label_.set_location(left + 30, top + 35);
	faction_name_title_label_.set_location((left + 145), top + 35);
	leader_gender_title_label_.set_location((left + 260), top + 35);
	team_color_title_label_.set_location((left + 375), top + 35);
	gold_title_label_.set_location((left + 493), top + 35);
	income_title_label_.set_location((left + 560), top + 35);

	waiting_label_.set_location(left + gui::ButtonHPadding,
		bottom - left_button->height() + 4);

	SDL_Rect scroll_pane_rect;
	scroll_pane_rect.x = ca.x;
	scroll_pane_rect.y = ca.y + 50;
	scroll_pane_rect.w = ca.w;
	scroll_pane_rect.h = launch_.location().y - scroll_pane_rect.y -
		gui::ButtonVPadding;

	scroll_pane_.set_location(scroll_pane_rect);
}

void connect::process_network_data(const config& data,
	const network::connection sock)
{
	ui::process_network_data(data, sock);
	std::pair<bool, bool> result = engine_.process_network_data(data, sock);

	if (result.first) {
		set_result(QUIT);
	}

	BOOST_FOREACH(side& s, sides_) {
		s.update_ui();
	}

	update_playerlist_state(result.second);
}

void connect::process_network_error(network::error& error)
{
	engine_.process_network_error(error);
}

void connect::process_network_connection(const network::connection sock)
{
	ui::process_network_connection(sock);
	engine_.process_network_connection(sock);
}

bool connect::accept_connections()
{
	return engine_.sides_available();
}

void connect::update_playerlist_state(bool silent)
{
	DBG_MP << "updating player list state" << std::endl;

	waiting_label_.set_text(engine_.can_start_game() ? ""
			: engine_.sides_available()
				? _("Waiting for players to join...")
				: _("Waiting for players to choose factions..."));
	launch_.enable(engine_.can_start_game());

	// If the "gamelist_" variable has users, use it.
	// Else, extracts the user list from the actual player list.
	if (gamelist().child("user")) {
		ui::gamelist_updated(silent);
	} else {
		// Updates the player list
		std::vector<std::string> playerlist;
		BOOST_FOREACH(const std::string& user, engine_.connected_users()) {
			playerlist.push_back(user);
		}
		set_user_list(playerlist, silent);
		set_user_menu_items(playerlist);
	}
}

} // end namespace mp

