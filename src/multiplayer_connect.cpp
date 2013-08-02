/*
   Copyright (C) 2007 - 2013
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

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_connect("mp/connect");
#define DBG_MP LOG_STREAM(debug, log_mp_connect)
#define LOG_MP LOG_STREAM(info, log_mp_connect)

namespace mp {

connect::side::side(connect& parent, side_engine_ptr engine) :
	parent_(&parent),
	engine_(engine),
	gold_lock_(engine_->cfg()["gold_lock"].to_bool()),
	income_lock_(engine_->cfg()["income_lock"].to_bool()),
	team_lock_(engine_->cfg()["team_lock"].to_bool()),
	color_lock_(engine_->cfg()["color_lock"].to_bool()),
	changed_(false),
	label_player_number_(parent.video(), str_cast(engine_->index() + 1),
		font::SIZE_LARGE, font::LOBBY_COLOR),
	label_original_controller_(parent.video(), engine_->current_player(),
		font::SIZE_SMALL),
	label_gold_(parent.video(), "100", font::SIZE_SMALL, font::LOBBY_COLOR),
	label_income_(parent.video(), _("Normal"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	combo_controller_(new gui::combo_drag(parent.disp(), parent.player_types_,
		parent.combo_control_group_)),
	combo_ai_algorithm_(parent.disp(), std::vector<std::string>()),
	combo_faction_(parent.disp(), std::vector<std::string>()),
	combo_leader_(parent.disp(), std::vector<std::string>()),
	combo_gender_(parent.disp(), std::vector<std::string>()),
	combo_team_(parent.disp(), parent.player_teams_),
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

	combo_faction_.enable(!parent_->params_.saved_game);
	combo_leader_.enable(!parent_->params_.saved_game);
	combo_gender_.enable(!parent_->params_.saved_game);
	combo_team_.enable(!parent_->params_.saved_game);
	combo_color_.enable(!parent_->params_.saved_game);
	slider_gold_.hide(parent_->params_.saved_game);
	slider_income_.hide(parent_->params_.saved_game);
	label_gold_.hide(parent_->params_.saved_game);
	label_income_.hide(parent_->params_.saved_game);

	init_ai_algorithm_combo();

	if (parent_->params_.use_map_settings) {
		// Gold, income, team, and color are only suggestions.
		// (Unless explicitly locked).
		slider_gold_.enable(!gold_lock_);
		label_gold_.enable(!gold_lock_);
		slider_income_.enable(!income_lock_);
		label_income_.enable(!income_lock_);
		combo_team_.enable(!team_lock_);
		combo_color_.enable(!color_lock_);
	}

	if (parent_->params_.use_map_settings || parent_->params_.saved_game) {
		// Explicitly assign a faction, if possible.
		int faction_index = 0;
		if (engine_->choosable_factions().size() > 1) {
			faction_index = find_suitable_faction(engine_->choosable_factions(),
				engine_->cfg());
			if (faction_index < 0) {
				faction_index = 0;
			}
		} else {
			combo_faction_.enable(false);
		}

		engine_->set_current_faction(
			engine_->choosable_factions()[faction_index]);
	}

	update_faction_combo();
	update_leader_combo();
	update_gender_combo();
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
		engine_->swap_sides_on_drop_target(drop_target);

		changed_ = true;
		parent_->sides_[drop_target].changed_ = true;

		init_ai_algorithm_combo();
		parent_->sides_[drop_target].init_ai_algorithm_combo();

		update_ui();
		parent_->sides_[drop_target].update_ui();
	} else if (combo_controller_->changed() &&
		combo_controller_->selected() >= 0) {

		const int cntr_last =
			(engine_->save_id().empty() ? CNTR_LAST-1 :	CNTR_LAST) -
			(parent_->local_only_ ? 1 : 0);
		if (combo_controller_->selected() == cntr_last) {
			update_controller_ui();
		} else if (combo_controller_->selected() < cntr_last) {
			// Correct entry number if CNTR_NETWORK
			// is not allowed for combo_controller_.
			engine_->
				set_mp_controller(mp::controller(combo_controller_->selected() +
					(parent_->local_only_ ? 1 : 0)));
			engine_->set_player_id("");
			engine_->set_ready_for_start(false);
			changed_ = true;
		} else {
			// Give user a second side.
			size_t user = combo_controller_->selected() - cntr_last - 1;

			const std::string new_id = parent_->engine_.users()[user].name;
			if (new_id != engine_->player_id()) {
				engine_->set_player_id(new_id);
				engine_->set_mp_controller(
					parent_->engine_.users()[user].controller);
				engine_->set_ready_for_start(true);
				changed_ = true;
			}
		}
		update_ai_algorithm_combo();
	}

	if (combo_controller_->hidden()) {
		combo_controller_->hide(false);
	}
	if (parent_->params_.saved_game) {
		return;
	}

	if (combo_color_.changed() && combo_color_.selected() >= 0) {
		engine_->set_color(combo_color_.selected());
		update_faction_combo();
		update_leader_combo();
		update_gender_combo();
		changed_ = true;
	}
	if (combo_faction_.changed() && combo_faction_.selected() >= 0) {
		engine_->set_current_faction(engine_->
			choosable_factions()[combo_faction_.selected()]);
		update_leader_combo();
		update_gender_combo();
		changed_ = true;
	}
	if (combo_ai_algorithm_.changed() && combo_ai_algorithm_.selected() >= 0) {
		engine_->set_ai_algorithm(
			parent_->ai_algorithms_[combo_ai_algorithm_.selected()]->id);
		changed_ = true;
	}
	if (combo_leader_.changed() && combo_leader_.selected() >= 0) {
		engine_->set_current_leader(engine_->
			choosable_leaders()[combo_leader_.selected()]);
		update_gender_combo();
		changed_ = true;
	}
	if (combo_gender_.changed() && combo_gender_.selected() >= 0) {
		engine_->set_current_gender(engine_->
			choosable_genders()[combo_gender_.selected()]);
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

void connect::side::update_user_list(
	const std::vector<std::string>& name_list) {

	combo_controller_->set_items(name_list);
	update_controller_ui();
}

void connect::side::import_network_user(const config& data)
{
	engine_->import_network_user(data);

	update_faction_combo();
	update_leader_combo();
	update_gender_combo();

	update_ui();
}

void connect::side::reset(mp::controller controller)
{
	engine_->reset(controller);

	update_leader_combo();
	update_gender_combo();
	update_ui();
}

void connect::side::hide_ai_algorithm_combo(bool invisible)
{
	if (!invisible) {
		if (engine_->mp_controller() == CNTR_COMPUTER) {
			// Computer selected, show AI combo.
			label_original_controller_.hide(true);
			combo_ai_algorithm_.hide(false);
		} else {
			// Computer de-selected, hide AI combo.
			combo_ai_algorithm_.hide(true);
			label_original_controller_.hide(false);
		}
	} else {
		combo_ai_algorithm_.hide(true);
	}
}

void connect::side::add_widgets_to_scrollpane(gui::scrollpane& pane, int pos)
{
	pane.add_widget(&label_player_number_, 0, 5 + pos);
	pane.add_widget(combo_controller_.get(), 20, 5 + pos);
	pane.add_widget(&label_original_controller_, 20 +
		(combo_controller_->width() - label_original_controller_.width()) / 2,
		35 + pos + (combo_leader_.height() -
		label_original_controller_.height()) / 2);
	pane.add_widget(&combo_ai_algorithm_, 20, 35 + pos);
	pane.add_widget(&combo_faction_, 135, 5 + pos);
	pane.add_widget(&combo_leader_, 135, 35 + pos);
	pane.add_widget(&combo_gender_, 250, 35 + pos);
	pane.add_widget(&combo_team_, 250, 5 + pos);
	pane.add_widget(&combo_color_, 365, 5 + pos);
	pane.add_widget(&slider_gold_, 475, 5 + pos);
	pane.add_widget(&label_gold_, 475, 35 + pos);
	pane.add_widget(&slider_income_, 475 + slider_gold_.width(), 5 + pos);
	pane.add_widget(&label_income_,  475 + slider_gold_.width(), 35 + pos);
}

void connect::side::init_ai_algorithm_combo()
{
	assert(parent_->ai_algorithms_.empty() == false);

	int sel = 0;
	std::vector<ai::description*> &ais_list = parent_->ai_algorithms_;
	std::vector<std::string> ais;
	int i = 0;
	BOOST_FOREACH(const ai::description *desc,  ais_list){
		ais.push_back(desc->text);
		if (desc->id == engine_->ai_algorithm()){
			sel = i;
		}
		i++;
	}
	combo_ai_algorithm_.set_items(ais);
	combo_ai_algorithm_.set_selected(sel);
	if (!ais_list.empty()) {
		// Ensures that the visually selected AI
		// is the one that will be loaded.
		engine_->set_ai_algorithm(ais_list[sel]->id);
	}
}

void connect::side::update_faction_combo()
{
	std::vector<std::string> factions;
	BOOST_FOREACH(const config* faction, engine_->choosable_factions()) {
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

	combo_faction_.set_items(factions);
	combo_faction_.set_selected(engine_->current_faction_index());
}

void connect::side::update_leader_combo()
{
	std::vector<std::string> leaders;
	BOOST_FOREACH(const std::string& leader, engine_->choosable_leaders()) {
		const unit_type* unit = unit_types.find(leader);
		if (unit) {
			leaders.push_back(IMAGE_PREFIX + unit->image() +
				get_RC_suffix(unit->flag_rgb()) + COLUMN_SEPARATOR +
				unit->type_name());
		} else if (leader == "random") {
			leaders.push_back(IMAGE_PREFIX + random_enemy_picture +
				COLUMN_SEPARATOR + _("Random"));
		} else if (leader == "null") {
			leaders.push_back(utils::unicode_em_dash);
		} else {
			leaders.push_back("?");
		}
	}

	combo_leader_.enable(leaders.size() > 1 && !parent_->params_.saved_game);

	combo_leader_.set_items(leaders);
	combo_leader_.set_selected(engine_->current_leader_index());
}

void connect::side::update_gender_combo()
{
	const unit_type* unit = unit_types.find(engine_->current_leader());

	std::vector<std::string> genders;
	BOOST_FOREACH(const std::string& gender, engine_->choosable_genders()) {
		if (gender == unit_race::s_female || gender == unit_race::s_male) {
			if (unit) {
				const unit_type& gender_unit =
					unit->get_gender_unit_type(gender);

				std::string gender_name = (gender == unit_race::s_female) ?
					_("Female ♀") : _("Male ♂");
				genders.push_back(IMAGE_PREFIX + gender_unit.image() +
					get_RC_suffix(gender_unit.flag_rgb()) + COLUMN_SEPARATOR +
					gender_name);
			}
		} else if (gender == "random") {
			genders.push_back(IMAGE_PREFIX + random_enemy_picture +
				COLUMN_SEPARATOR + _("Random"));
		} else if (gender == "null") {
			genders.push_back(utils::unicode_em_dash);
		} else {
			genders.push_back("?");
		}
	}

	combo_gender_.enable(genders.size() > 1 && !parent_->params_.saved_game);

	combo_gender_.set_items(genders);
	combo_gender_.set_selected(engine_->current_gender_index());
}

void connect::side::update_controller_ui()
{
	if (engine_->player_id().empty()) {
		combo_controller_->set_selected(
			engine_->mp_controller() - (parent_->local_only_ ? 1 : 0));
	} else {
		connected_user_list::iterator player =
			parent_->engine_.find_player_by_id(engine_->player_id());

		if (player != parent_->engine_.users().end()) {
			const int no_reserve = engine_->save_id().empty() ? - 1 : 0;
			combo_controller_->set_selected(
				CNTR_LAST + no_reserve + 1 +
				(player - parent_->engine_.users().begin()) -
				(parent_->local_only_ ? 1 : 0));
		} else {
			assert(parent_->local_only_ != true);
			combo_controller_->set_selected(CNTR_NETWORK);
		}
	}

    update_ai_algorithm_combo();
}

void connect::side::update_ui()
{
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

#ifdef LOW_MEM
std::string connect::side::get_RC_suffix(
	const std::string& /*unit_color*/) const {

	return "";
}
#else
std::string connect::side::get_RC_suffix(
	const std::string& unit_color) const {

	return "~RC(" + unit_color + ">" +
		lexical_cast<std::string>(engine_->color() + 1) + ")";
}
#endif

connect::connect(game_display& disp, const config& game_config,
	chat& c, config& gamelist, const mp_game_settings& params,
	mp::controller default_controller, bool local_players_only) :
	mp::ui(disp, _("Game Lobby: ") + params.name, game_config, c, gamelist),
	local_only_(local_players_only),
	params_(params),
	player_types_(),
	player_teams_(),
	player_colors_(),
	ai_algorithms_(),
	sides_(),
	engine_(disp, default_controller, params),
	waiting_label_(video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	type_title_label_(video(), _("Player/Type"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	faction_title_label_(video(), _("Faction"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	team_title_label_(video(), _("Team/Gender"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	color_title_label_(video(), _("Color"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	gold_title_label_(video(), _("Gold"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	income_title_label_(video(), _("Income"), font::SIZE_SMALL,
		font::LOBBY_COLOR),
	scroll_pane_(video()),
	launch_(video(), _("I’m Ready")),
	cancel_(video(), _("Cancel")),
	//add_local_player_(video(), _("Add named local player")),
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

	lists_init();
	if (sides_.empty()) {
		throw config::error(
			_("The scenario is invalid because it has no sides."));
	}

	// Send Initial information
	config response;
	config& create_game = response.add_child("create_game");
	create_game["name"] = params.name;
	if (params.password.empty() == false) {
		response["password"] = params.password;
	}
	network::send_data(response, 0);

	update_user_combos();

	engine_.assign_side_for_host();

	append_to_title(" — " + engine_.level()["name"].t_str());
	gold_title_label_.hide(params_.saved_game);
	income_title_label_.hide(params_.saved_game);

	engine_.update_level();
	update_playerlist_state(true);

	// If we are connected, send data to the connected host.
	network::send_data(engine_.level(), 0);
}

connect::~connect()
{
}

void connect::process_event()
{
	bool changed = false;

	// TODO: fix this feature or remove the code.
	// If the Add Local Player button is pressed, display corresponding
	// dialog box. Dialog box is shown again if an already existing
	// player name is entered. If the name is valid, add a new user with
	// that name to the list of connected users, and refresh the UI.
	/*if (add_local_player_.pressed()) {
		bool already_exists = false;
		do {
			already_exists = false;
			gui::dialog d(disp(), _("Enter a name for the new player"), "",
				gui::OK_CANCEL);
			d.set_textbox(_("Name: "));
			d.show();
			if (d.result() != gui::CLOSE_DIALOG && !d.textbox_text().empty()) {
				for(connected_user_list::iterator it = engine_.users().begin();
					it != engine_.users().end(); ++it) {

					if ((*it).name == d.textbox_text() ) already_exists = true;
				}
				if (!already_exists) {
					engine_.users().push_back(connected_user(d.textbox_text(),
						CNTR_LOCAL, 0));
					update_playerlist_state();
					update_user_combos();
				}
			}
		} while (already_exists);
	}*/

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

	BOOST_FOREACH(side& s, sides_) {
		s.hide_ai_algorithm_combo(hide);
	}

	faction_title_label_.hide(hide);
	team_title_label_.hide(hide);
	color_title_label_.hide(hide);

	if (!params_.saved_game) {
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
	faction_title_label_.set_location((left + 145), top + 35);
	team_title_label_.set_location((left + 260), top + 35);
	color_title_label_.set_location((left + 375), top + 35);
	gold_title_label_.set_location((left + 493), top + 35);
	income_title_label_.set_location((left + 560), top + 35);

	/*add_local_player_.set_help_string(("Feature currently disabled."));
	add_local_player_.set_active(false);
	add_local_player_.hide(true);
	add_local_player_.set_location(left, bottom - add_local_player_.height());*/
	waiting_label_.set_location(left + gui::ButtonHPadding /*+
		add_local_player_.width()*/, bottom - left_button->height() + 4);

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

	if (data.child("leave_game")) {
		set_result(QUIT);
		return;
	}

	if (!data["side_drop"].empty()) {
		unsigned side_drop = data["side_drop"].to_int() - 1;
		if (side_drop < sides_.size()) {
			connected_user_list::iterator player = engine_.find_player_by_id(
				sides_[side_drop].engine()->player_id());
			sides_[side_drop].reset(sides_[side_drop].engine()->
				mp_controller());
			if (player != engine_.users().end()) {
				engine_.users().erase(player);
				update_user_combos();
			}
			engine_.update_and_send_diff();
			update_playerlist_state(true);
			return;
		}
	}

	if (!data["side"].empty()) {
		unsigned side_taken = data["side"].to_int() - 1;

		// Checks if the connecting user has a valid and unique name.
		const std::string name = data["name"];
		if (name.empty()) {
			config response;
			response["failed"] = true;
			network::send_data(response, sock);
			ERR_CF << "ERROR: No username provided with the side.\n";
			return;
		}

		connected_user_list::iterator player = engine_.find_player_by_id(name);
		if (player != engine_.users().end()) {
			 // TODO: Seems like a needless limitation
			 // to only allow one side per player.
			if (engine_.find_player_side_index_by_id(name) != -1) {
				config response;
				response["failed"] = true;
				response["message"] = "The nickname '" + name +
					"' is already in use.";
				network::send_data(response, sock);
				return;
			} else {
				engine_.users().erase(player);
				config observer_quit;
				observer_quit.add_child("observer_quit")["name"] = name;
				network::send_data(observer_quit, 0);
				update_user_combos();
			}
		}

		// Assigns this user to a side.
		if (side_taken < sides_.size()) {
			if (!sides_[side_taken].engine()->available(name)) {
				// This side is already taken.
				// Try to reassing the player to a different position.
				side_taken = 0;
				BOOST_FOREACH(side& s, sides_) {
					if (s.engine()->available()) {
						break;
					}

					side_taken++;
				}

				if (side_taken >= sides_.size()) {
					config response;
					response["failed"] = true;
					network::send_data(response, sock);
					config kick;
					kick["username"] = data["name"];
					config res;
					res.add_child("kick", kick);
					network::send_data(res, 0);
					update_user_combos();
					engine_.update_and_send_diff();
					ERR_CF << "ERROR: Couldn't assign a side to '" <<
						name << "'\n";
					return;
				}
			}

			LOG_CF << "client has taken a valid position\n";

			// Adds the name to the list.
			engine_.users().push_back(connected_user(name, CNTR_NETWORK, sock));
			update_user_combos();

			sides_[side_taken].import_network_user(data);

			// Go thought and check if more sides are reserved
			// for this player.
			std::for_each(sides_.begin(), sides_.end(),
				boost::bind(&connect::take_reserved_side, this, _1, data));
			update_playerlist_state(false);
			engine_.update_and_send_diff();

			LOG_NW << "sent player data\n";

		} else {
			ERR_CF << "tried to take illegal side: " << side_taken << "\n";
			config response;
			response["failed"] = true;
			network::send_data(response, sock);
		}
	}

	if (const config &change_faction = data.child("change_faction")) {
		int side_taken =
			engine_.find_player_side_index_by_id(change_faction["name"]);
		if (side_taken != -1) {
			sides_[side_taken].import_network_user(change_faction);
			sides_[side_taken].engine()->set_ready_for_start(true);
			update_playerlist_state();
			engine_.update_and_send_diff();
		}
	}

	if (const config &c = data.child("observer")) {
		const t_string &observer_name = c["name"];
		if (!observer_name.empty()) {
			connected_user_list::iterator player =
				engine_.find_player_by_id(observer_name);
			if (player == engine_.users().end()) {
				engine_.users().push_back(connected_user(observer_name,
					CNTR_NETWORK, sock));
				update_user_combos();
				update_playerlist_state();
				engine_.update_and_send_diff();
			}
		}
	}
	if (const config &c = data.child("observer_quit")) {
		const t_string &observer_name = c["name"];
		if (!observer_name.empty()) {
			connected_user_list::iterator player =
				engine_.find_player_by_id(observer_name);
			if (player != engine_.users().end() &&
				engine_.find_player_side_index_by_id(observer_name) == -1) {

				engine_.users().erase(player);
				update_user_combos();
				update_playerlist_state();
				engine_.update_and_send_diff();
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
	if (!error.socket || !network::is_server()) {
		error.disconnect();
		throw network::error(error.message);
	}

	bool changes = false;

	// A socket has disconnected. Remove it, and resets its side.
	connected_user_list::iterator user;
	for(user = engine_.users().begin(); user != engine_.users().end(); ++user) {
		if (user->connection == error.socket) {
			changes = true;

			int i = engine_.find_player_side_index_by_id(user->name);
			if (i != -1) {
				sides_[i].reset(engine_.mp_controller());
			}

			break;
		}
	}
	if (user != engine_.users().end()) {
		engine_.users().erase(user);
		update_user_combos();
	}

	// Now disconnect the socket.
	error.disconnect();

	// If there have been changes to the positions taken,
	// then notify other players.
	if (changes) {
		engine_.update_and_send_diff();
		update_playerlist_state();
	}
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

void connect::lists_init()
{
	// Options.
	if (!local_only_) {
		player_types_.push_back(_("Network Player"));
	}
	player_types_.push_back(_("Local Player"));
	player_types_.push_back(_("Computer Player"));
	player_types_.push_back(_("Empty"));

	// AI algorithms.
	const config &era = engine_.level().child("era");
	ai::configuration::add_era_ai_from_config(era);
	ai::configuration::add_mod_ai_from_config(
		engine_.level().child_range("modification"));
	ai_algorithms_ = ai::configuration::get_available_ais();

	// Factions.
	config::child_itors sides = engine_.current_config()->child_range("side");

	// Teams.
	if (params_.use_map_settings) {
		int side_count = 1;
		BOOST_FOREACH(config &side, sides) {
			config::attribute_value &team_name = side["team_name"];
			config::attribute_value &user_team_name = side["user_team_name"];

			if (team_name.empty()) {
				team_name = side_count;
			}

			if (user_team_name.empty()) {
				user_team_name = team_name;
			}

			bool found = false;
			BOOST_FOREACH(const std::string& name, engine_.team_names()) {
				if (name == team_name) {
					found = true;
					break;
				}
			}

			if (!found) {
				engine_.team_names().push_back(team_name);
				engine_.user_team_names().push_back(
					user_team_name.t_str().to_serialized());
				if (side["allow_player"].to_bool(true)) {
					player_teams_.push_back(user_team_name.str());
				}
			}
			++side_count;
		}
	} else {
		std::vector<std::string> map_team_names;
		int side_count = 1;
		BOOST_FOREACH(config &side, sides) {
			const std::string side_num = lexical_cast<std::string>(side_count);
			config::attribute_value &team_name = side["team_name"];

			if (team_name.empty()) {
				team_name = side_num;
			}

			unsigned team_index = 0;
			BOOST_FOREACH(const std::string& name, map_team_names) {
				if (name == team_name) {
					break;
				}

				team_index++;
			}

			if (team_index >= map_team_names.size()) {
				map_team_names.push_back(team_name);
				team_name = lexical_cast<std::string>(map_team_names.size());
			} else {
				team_name = lexical_cast<std::string>(team_index + 1);
			}

			const std::string team_prefix(std::string(_("Team")) + " ");

			engine_.team_names().push_back(side_num);
			engine_.user_team_names().push_back(team_prefix + side_num);
			if (side["allow_player"].to_bool(true)) {
				player_teams_.push_back(team_prefix + side_num);
			}
			++side_count;
		}
	}

	// Colors.
	for(int i = 0; i < gamemap::MAX_PLAYERS; ++i) {
		player_colors_.push_back(get_color_string(i));
	}

	// Populates "sides_" from the level configuration
	int index = 0;
	BOOST_FOREACH(const config &s, sides) {
		side_engine_ptr new_side_engine(new side_engine(s, engine_, index));
		engine_.add_side_engine(new_side_engine);

		sides_.push_back(side(*this, new_side_engine));
		
		index++;
	}

	// Add side widgets to scroll pane.
	int side_pos_y_offset = 0;
	BOOST_FOREACH(side& s, sides_) {
		if (!s.engine()->allow_player()) {
			continue;
		}

		s.add_widgets_to_scrollpane(scroll_pane_, side_pos_y_offset);
		side_pos_y_offset += 60;
	}
}

void connect::take_reserved_side(connect::side& side, const config& data)
{
	if (side.engine()->available(data["name"])
			&& side.engine()->current_player() == data["name"]) {
		side.import_network_user(data);
	}
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
		BOOST_FOREACH(const connected_user& user, engine_.users()) {
			playerlist.push_back(user.name);
		}
		set_user_list(playerlist, silent);
		set_user_menu_items(playerlist);
	}
}

void connect::update_user_combos()
{
	BOOST_FOREACH(side& s, sides_) {
		bool name_present = false;

		typedef std::vector<std::string> name_list;

		name_list list = player_types_;
		if (!s.engine()->save_id().empty()) {
			list.push_back(_("Reserved"));
		}
		list.push_back(_("--give--"));

		BOOST_FOREACH(const connected_user& user, engine_.users()) {
			list.push_back(user.name);
			if (user.name == s.engine()->player_id()) {
				name_present = true;
			}
		}

		if (name_present == false) {
			s.engine()->set_player_id("");
		}

		s.update_user_list(list);
	}
}

} // end namespace mp

