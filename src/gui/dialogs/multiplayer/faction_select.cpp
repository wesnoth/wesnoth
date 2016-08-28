/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/multiplayer/faction_select.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/dialogs/helper.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "config_assign.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "units/types.hpp"

#include "utils/functional.hpp"

namespace gui2
{

REGISTER_DIALOG(faction_select)

tfaction_select::tfaction_select(ng::flg_manager& flg_manager, const std::string& color, const int side)
	: flg_manager_(flg_manager)
	, tc_color_(color)
	, side_(side)
{
}

void tfaction_select::pre_show(twindow& window)
{
	window.set_escape_disabled(true);

	find_widget<tlabel>(&window, "starting_pos", false).set_label(std::to_string(side_));

	//
	// Set up gender radio buttons
	//
	ttoggle_button& gender_rand = find_widget<ttoggle_button>(&window, "gender_random", false);
	ttoggle_button& gender_male = find_widget<ttoggle_button>(&window, "gender_male", false);
	ttoggle_button& gender_female = find_widget<ttoggle_button>(&window, "gender_female", false);

	gender_toggle_.add_member(&gender_rand,   "random");
	gender_toggle_.add_member(&gender_male,   unit_race::s_male);
	gender_toggle_.add_member(&gender_female, unit_race::s_female);

	gender_toggle_.set_member_states("random");

	// TODO: consolidate when adding a all-member callback setter to tgroup
	gender_rand.set_callback_state_change(
			dialog_callback<tfaction_select, &tfaction_select::on_gender_select>);

	gender_male.set_callback_state_change(
			dialog_callback<tfaction_select, &tfaction_select::on_gender_select>);

	gender_female.set_callback_state_change(
			dialog_callback<tfaction_select, &tfaction_select::on_gender_select>);

	//
	// Set up leader menu button
	//
	find_widget<tmenu_button>(&window, "leader_menu", false).connect_click_handler(
		std::bind(&tfaction_select::on_leader_select, this, std::ref(window)));

	//
	// Set up faction list
	//
	tlistbox& list = find_widget<tlistbox>(&window, "faction_list", false);

	window.keyboard_capture(&list);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*list,
	   std::bind(&tfaction_select::on_faction_select,
			*this, std::ref(window)));
#else
	list.set_callback_value_change(
		dialog_callback<tfaction_select, &tfaction_select::on_faction_select>);
#endif

	for(const config *s : flg_manager_.choosable_factions()) {
		const config& side = *s;

		std::map<std::string, string_map> data;
		string_map item;

		// TODO: don't hardcode magenta?
		item["label"] = (formatter() << side["image"] << "~RC(magenta>" << tc_color_ << ")").str();
		data.emplace("faction_image", item);

		item["label"] = side["name"];
		data.emplace("faction_name", item);

		list.add_row(data);
	}

	on_faction_select(window);
}

void tfaction_select::on_faction_select(twindow& window)
{
	const int selected_row = find_widget<tlistbox>(&window, "faction_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	flg_manager_.set_current_faction(selected_row);

	std::vector<config> leaders;

	for(const std::string& leader : flg_manager_.choosable_leaders()) {
		const unit_type* unit = unit_types.find(leader);

		if(unit) {
			const std::string icon = formatter() << unit->image() << "~RC(" << unit->flag_rgb() << ">" << tc_color_ << ")";
			leaders.push_back(config_of("label", unit->type_name())("icon", icon));
		} else if(leader == "random") {
			leaders.push_back(config_of("label", _("Random"))("icon", "units/random-dice.png"));
		} else if(leader == "null") {
			leaders.push_back(config_of("label", utils::unicode_em_dash));
		} else {
			leaders.push_back(config_of("label", "?"));
		}
	}

	// TODO: this breaks ordering within the FLG manager
	//std::sort(leaders.begin(), leaders.end(), [](const config& cfg1, const config& cfg2) {
	//	return cfg1["label"].str() < cfg2["label"].str();
	//});

	tmenu_button& leader_dropdown = find_widget<tmenu_button>(&window, "leader_menu", false);

	leader_dropdown.set_values(leaders, flg_manager_.current_leader_index());
	leader_dropdown.set_active(leaders.size() > 1 && !flg_manager_.is_saved_game());

	on_leader_select(window);

	// Print recruits
	const std::vector<std::string> recruit_list = utils::split(flg_manager_.current_faction()["recruit"]);
	std::vector<t_string> recruit_names;

	for(const auto& recruit : recruit_list) {
		if(const unit_type* rt = unit_types.find(recruit)) {
			recruit_names.push_back("• " + rt->type_name());
		}
	}

	find_widget<tcontrol>(&window, "recruits", false).set_label(utils::join(recruit_names, "\n"));
}

void tfaction_select::on_leader_select(twindow& window)
{
	flg_manager_.set_current_leader(find_widget<tmenu_button>(&window, "leader_menu", false).get_value());

	auto gender_available = [this](const std::string gender)->bool {
		const std::vector<std::string>& genders = flg_manager_.choosable_genders();
		return std::find(genders.begin(), genders.end(), gender) != genders.end();
	};

	// TODO: should we decouple this from the dlg manager and instead just check the unit type directly?
	find_widget<ttoggle_button>(&window, "gender_male", false).set_active(gender_available(unit_race::s_male));
	find_widget<ttoggle_button>(&window, "gender_female", false).set_active(gender_available(unit_race::s_female));

	update_leader_image(window);
}

void tfaction_select::on_gender_select(twindow& window)
{
	flg_manager_.set_current_gender(gender_toggle_.get_active_member_value());

	update_leader_image(window);
}

void tfaction_select::update_leader_image(twindow& window)
{
	std::string leader_image = "units/random-dice.png";

	if(const unit_type* ut = unit_types.find(flg_manager_.current_leader())) {
		const unit_type& utg = ut->get_gender_unit_type(flg_manager_.current_gender());
		leader_image = formatter() << utg.image() << "~RC(" << utg.flag_rgb() << ">" << tc_color_ << ")" << "~SCALE_INTO_SHARP(144,144)";
	}

	find_widget<timage>(&window, "leader_image", false).set_label(leader_image);
}

}
