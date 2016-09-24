/*
   Copyright (C) 2008 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/multiplayer/mp_staging.hpp"

#include "config_assign.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/faction_select.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/chatbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/status_label_helper.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/text_box.hpp"
#include "game_config.hpp"
#include "savegame.hpp"
#include "settings.hpp"
#include "units/types.hpp"
#include "formatter.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "utils/functional.hpp"
#endif

#include <boost/algorithm/string.hpp>

namespace gui2
{

// FIXME: doesn't show properly...
//static const std::string male = "&#9794;";
//static const std::string female = "&#9792;";

static const std::string male = "(M)";
static const std::string female = "(F)";

REGISTER_DIALOG(mp_staging)

tmp_staging::tmp_staging(const config& /*cfg*/, ng::connect_engine& connect_engine)
	: connect_engine_(connect_engine)
	, ai_algorithms_(ai::configuration::get_available_ais())
{
	//ai_algorithms_ = ai::configuration::get_available_ais();
}

void tmp_staging::pre_show(twindow& window)
{
	window.set_enter_disabled(true);

	//
	// Set title
	//
	tlabel& title = find_widget<tlabel>(&window, "title", false);
	title.set_label((formatter() << title.label() << " " << utils::unicode_em_dash << " " << connect_engine_.scenario()["name"].t_str()).str());

	//
	// Set up sides list
	//
	tlistbox& list = find_widget<tlistbox>(&window, "side_list", false);

	window.keyboard_capture(&list);

	for(const auto& side_ptr : connect_engine_.side_engines()) {
		// Shorthand variable
		ng::side_engine& side = *side_ptr.get();

		if(!side.allow_player() && !game_config::debug) {
			continue;
		}

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = std::to_string(side.index() + 1);
		data.emplace("side_number", item);

		// TODO: don't hardcode meganta?
		item["label"] = "units/unknown-unit.png~RC(magenta>" + std::to_string(side.color() + 1) + ")";
		data.emplace("leader_image", item);

		item["label"] = "icons/icon-random.png";
		data.emplace("leader_gender", item);

		tgrid& row_grid = list.add_row(data);

		update_leader_display(side, row_grid);

		// Status variables
		const bool fls = connect_engine_.force_lock_settings();
		const bool ums = connect_engine_.params().use_map_settings;

		const bool lock_gold   = side.cfg()["gold_lock"].to_bool(fls);
		const bool lock_income = side.cfg()["income_lock"].to_bool(fls);
		const bool lock_team   = side.cfg()["team_lock"].to_bool(fls);
		const bool lock_color  = side.cfg()["color_lock"].to_bool(fls);

		const bool saved_game = connect_engine_.params().saved_game;

		//
		// AI Algorithm
		//
		assert(!ai_algorithms_.empty());

		int selection = 0;

		// We use an index-based loop in order to get the index of the selected option
		std::vector<config> ai_options;
		for(unsigned i = 0; i < ai_algorithms_.size(); i++) {
			ai_options.push_back(config_of("label", ai_algorithms_[i]->text));

			if(ai_algorithms_[i]->id == side.ai_algorithm()) {
				selection = i;
			}
		}

		tmenu_button& ai_selection = find_widget<tmenu_button>(&row_grid, "ai_controller", false);

		ai_selection.set_values(ai_options, selection);
		ai_selection.connect_click_handler(std::bind(&tmp_staging::on_ai_select, this, std::ref(side), std::ref(ai_selection)));

		on_ai_select(side, ai_selection);

		//
		// Controller
		//
		std::vector<config> controller_names;
		for(const auto& controller : side.controller_options()) {
			controller_names.push_back(config_of("label", controller.second));
		}

		tmenu_button& controller_selection = find_widget<tmenu_button>(&row_grid, "controller", false);

		controller_selection.set_values(controller_names, side.current_controller_index());
		controller_selection.set_active(side.controller_options().size() > 1);
		controller_selection.connect_click_handler(std::bind(&tmp_staging::on_controller_select, this, std::ref(side), std::ref(row_grid)));

		on_controller_select(side, row_grid);

		//
		// Leader controls
		//
		connect_signal_mouse_left_click(
			find_widget<tbutton>(&row_grid, "select_leader", false),
			std::bind(&tmp_staging::select_leader_callback, this, std::ref(window), std::ref(side), std::ref(row_grid)));

		//
		// Team
		//
		std::vector<config> team_names;
		for(const auto& team : side.player_teams()) {
			team_names.push_back(config_of("label", team));
		}

		tmenu_button& team_selection = find_widget<tmenu_button>(&row_grid, "side_team", false);

		team_selection.set_values(team_names, side.team());
		team_selection.set_active(!saved_game);

		//
		// Colors
		//
		std::vector<config> color_options;
		for(const auto& color : side.get_colors()) {
			// BIG FAT TODO: get rid of the hardcoded GUI1 formatting and do something about this hideous string manipulation
			const std::string c = color.substr(color.find_first_of(">") + 1);
			std::string cid = c;
			cid[0] = std::tolower(cid[0]);

			color_options.push_back(config_of
				("label", c)
				("icon", (formatter() << "misc/status.png~RC(magenta>" << cid << ")").str())
			);
		}

		tmenu_button& color_selection = find_widget<tmenu_button>(&row_grid, "side_color", false);

		color_selection.set_values(color_options, side.color());
		color_selection.set_active(!saved_game);

		//
		// Gold and Income
		//
		tslider& slider_gold = find_widget<tslider>(&row_grid, "side_gold_slider", false);
		slider_gold.set_value(side.cfg()["gold"].to_int(100));

		connect_signal_notify_modified(slider_gold, std::bind([&]() { side.set_gold(slider_gold.get_value()); }));

		tslider& slider_income = find_widget<tslider>(&row_grid, "side_income_slider", false);
		slider_income.set_value(side.cfg()["income"]);

		connect_signal_notify_modified(slider_income, std::bind([&]() { side.set_income(slider_income.get_value()); }));

		// TODO: hide header, or maybe display the saved values
		if(saved_game) {
			slider_gold.set_visible(twidget::tvisible::invisible);
			slider_income.set_visible(twidget::tvisible::invisible);
		}

		//
		// Gold, income, team, and color are only suggestions unless explicitly locked
		//
		if(ums) {
			team_selection.set_active(!lock_team);
			color_selection.set_active(!lock_color);

			slider_gold.set_active(!lock_gold);
			slider_income.set_active(!lock_income);
		}
	}

	//
	// Initialize chatbox and game rooms
	//

	// TODO: seems the chatbox requires lobby info to be set first
	/*tchatbox& chat = find_widget<tchatbox>(&window, "chat", false);

	chat.room_window_open("this game", true);
	chat.active_window_changed();*/

	//
	// Set up the Lua plugin context
	//
	plugins_context_.reset(new plugins_context("Multiplayer Staging"));

	plugins_context_->set_callback("launch", [&window](const config&) { window.set_retval(twindow::OK); }, false);
	plugins_context_->set_callback("quit",   [&window](const config&) { window.set_retval(twindow::CANCEL); }, false);
	plugins_context_->set_callback("chat",   [this, &window](const config&) { return; /* TODO*/ }, false);
}

void tmp_staging::sync_changes()
{
	// TODO: should this call somehow be integrated into the connect engine setters?
	connect_engine_.update_and_send_diff();
}

void tmp_staging::on_controller_select(ng::side_engine& side, tgrid& row_grid)
{
	tmenu_button& ai_selection         = find_widget<tmenu_button>(&row_grid, "ai_controller", false);
	tmenu_button& controller_selection = find_widget<tmenu_button>(&row_grid, "controller", false);

	side.set_controller(side.controller_options()[controller_selection.get_value()].first);

	ai_selection.set_visible(side.controller() == ng::CNTR_COMPUTER ? twidget::tvisible::visible : twidget::tvisible::hidden);

	sync_changes();
}

void tmp_staging::on_ai_select(ng::side_engine& side, tmenu_button& ai_menu)
{
	side.set_ai_algorithm(ai_algorithms_[ai_menu.get_value()]->id);

	sync_changes();
}

void tmp_staging::select_leader_callback(twindow& window, ng::side_engine& side, tgrid& row_grid)
{
	gui2::tfaction_select dlg(side.flg(), std::to_string(side.color() + 1), side.index() + 1);
	dlg.show(window.video());

	if(dlg.get_retval() == twindow::OK) {
		update_leader_display(side, row_grid);

		sync_changes();
	}
}

void tmp_staging::update_leader_display(ng::side_engine& side, tgrid& row_grid)
{
	const std::string current_faction = (*side.flg().choosable_factions()[side.flg().current_faction_index()])["name"];

	// BIG FAT TODO: get rid of this shitty "null" string value in the FLG manager
	const std::string current_leader = side.flg().current_leader() != "null" ? side.flg().current_leader() : utils::unicode_em_dash;
	const std::string current_gender = side.flg().current_gender() != "null" ? side.flg().current_gender() : utils::unicode_em_dash;

	// Sprite
	std::string new_image;

	if(side.flg().is_random_faction() || current_leader == "random") {
		new_image = "units/random-dice.png";
	} else {
		const unit_type& type = unit_types.find(current_leader)->get_gender_unit_type(current_gender);

		new_image = formatter() << type.image() << "~RC(magenta>" << side.color() + 1 << ")";
	}

	find_widget<timage>(&row_grid, "leader_image", false).set_label(new_image);

	// Faction and leader
	find_widget<tlabel>(&row_grid, "leader_type", false).set_label(current_leader);
	find_widget<tlabel>(&row_grid, "leader_faction", false).set_label(current_faction);

	// Gender
	if(current_gender != utils::unicode_em_dash) {
		const std::string gender_icon = formatter() << "icons/icon-" << current_gender << ".png";
		find_widget<timage>(&row_grid, "leader_gender", false).set_label(gender_icon);
	}
}

void tmp_staging::post_show(twindow& window)
{
	if(window.get_retval() == twindow::OK) {
		connect_engine_.start_game();
	}
}

} // namespace gui2
