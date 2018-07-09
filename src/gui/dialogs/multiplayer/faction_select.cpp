/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "gui/auxiliary/old_markup.hpp"
#include "gui/core/log.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "formatter.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "preferences/game.hpp"	// for encountered_units
#include "units/types.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(faction_select)

faction_select::faction_select(ng::flg_manager& flg_manager, const std::string& color, const int side)
	: flg_manager_(flg_manager)
	, tc_color_(color)
	, side_(side)
	, last_faction_(flg_manager.current_faction_index())
	, last_leader_(flg_manager.current_leader_index())
	, last_gender_(flg_manager.current_gender_index())
{
}

void faction_select::pre_show(window& window)
{
	find_widget<label>(&window, "starting_pos", false).set_label(std::to_string(side_));

	//
	// Set up gender radio buttons
	//
	toggle_button& gender_rand = find_widget<toggle_button>(&window, "gender_random", false);
	toggle_button& gender_male = find_widget<toggle_button>(&window, "gender_male", false);
	toggle_button& gender_female = find_widget<toggle_button>(&window, "gender_female", false);

	gender_toggle_.add_member(&gender_rand,   "random");
	gender_toggle_.add_member(&gender_male,   unit_race::s_male);
	gender_toggle_.add_member(&gender_female, unit_race::s_female);

	gender_toggle_.set_member_states("random");

	gender_toggle_.set_callback_on_value_change(
		std::bind(&faction_select::on_gender_select, this, std::ref(window)));

	//
	// Set up leader menu button
	//
	connect_signal_notify_modified(find_widget<menu_button>(&window, "leader_menu", false),
		std::bind(&faction_select::on_leader_select, this, std::ref(window)));

	// Leader's profile button
	find_widget<button>(&window, "type_profile", false).connect_click_handler(
		std::bind(&faction_select::profile_button_callback, this, std::ref(window)));

	//
	// Set up faction list
	//
	listbox& list = find_widget<listbox>(&window, "faction_list", false);

	window.keyboard_capture(&list);

	connect_signal_notify_modified(list,
		std::bind(&faction_select::on_faction_select, this, std::ref(window)));

	for(const config *s : flg_manager_.choosable_factions()) {
		const config& side = *s;

		std::map<std::string, string_map> data;
		string_map item;

		const std::string name = side["name"].str();
		// flag_rgb here is unrelated to any handling in the unit class
		const std::string flag_rgb = !side["flag_rgb"].empty() ? side["flag_rgb"].str() : "magenta";

		// Handle legacy DescriptionWML format.
		if(name.find_first_of("=") != std::string::npos) {
			gui2::legacy_menu_item parsed(name, "Use separate name= and image= keys. Multiple text columns are no longer supported.");

			if(!side.has_attribute("image")) {
				item["label"] = (formatter() << parsed.icon() << "~RC(" << flag_rgb << ">" << tc_color_ << ")").str();
				data.emplace("faction_image", item);
			}

			item["label"] = parsed.label();
			if(!parsed.description().empty()) {
				item["label"] += " " + parsed.description();
			}
			data.emplace("faction_name", item);
		} else {
			item["label"] = (formatter() << side["image"] << "~RC(" << flag_rgb << ">" << tc_color_ << ")").str();
			data.emplace("faction_image", item);

			item["label"] = name;
			data.emplace("faction_name", item);
		}

		list.add_row(data);
	}

	list.select_row(flg_manager_.current_faction_index());

	on_faction_select(window);
}

void faction_select::on_faction_select(window& window)
{
	const int selected_row = find_widget<listbox>(&window, "faction_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	// Since set_current_faction overrides the current leader, save a copy of the previous leader index so the
	// leader dropdown can be set to the appropriate initial selection.
	const int previous_leader_selection = flg_manager_.current_leader_index();

	flg_manager_.set_current_faction(selected_row);

	std::vector<config> leaders;

	for(const std::string& leader : flg_manager_.choosable_leaders()) {
		const unit_type* unit = unit_types.find(leader);

		if(unit) {
			const std::string icon = formatter() << unit->image() << "~RC(" << unit->flag_rgb() << ">" << tc_color_ << ")";
			leaders.emplace_back(config {"label", unit->type_name(), "icon", icon});
		} else if(leader == "random") {
			leaders.emplace_back(config {"label", _("Random"), "icon", ng::random_enemy_picture});
		} else if(leader == "null") {
			leaders.emplace_back(config {"label", font::unicode_em_dash});
		} else {
			leaders.emplace_back(config {"label", "?"});
		}
	}

	menu_button& leader_dropdown = find_widget<menu_button>(&window, "leader_menu", false);

	leader_dropdown.set_values(leaders, std::min<int>(leaders.size() - 1, previous_leader_selection));
	leader_dropdown.set_active(leaders.size() > 1 && !flg_manager_.is_saved_game());

	on_leader_select(window);

	// Print recruits
	const std::vector<std::string> recruit_list = utils::split(flg_manager_.current_faction()["recruit"]);
	std::vector<t_string> recruit_names;

	for(const auto& recruit : recruit_list) {
		if(const unit_type* rt = unit_types.find(recruit)) {
			recruit_names.push_back(font::unicode_bullet + " " + rt->type_name());
		}
	}

	std::sort(recruit_names.begin(), recruit_names.end(), [](const std::string& s1, const std::string& s2) {
		return translation::compare(s1, s2) < 0;
	});

	find_widget<styled_widget>(&window, "recruits", false).set_label(utils::join(recruit_names, "\n"));
}

void faction_select::on_leader_select(window& window)
{
	flg_manager_.set_current_leader(find_widget<menu_button>(&window, "leader_menu", false).get_value());

	// TODO: should we decouple this from the flg manager and instead just check the unit type directly?
	// If that's done so, we'd need to come up with a different check for Random availability.
	gender_toggle_.set_members_enabled([this](const std::string& gender)->bool {
		const std::vector<std::string>& genders = flg_manager_.choosable_genders();
		return std::find(genders.begin(), genders.end(), gender) != genders.end();
	});

	update_leader_image(window);

	// Disable the profile button if leader_type is dash or "Random"
	button& profile_button = find_widget<button>(&window, "type_profile", false);
	const std::string& leader_type = find_widget<menu_button>(&window, "leader_menu", false).get_value_string();
	profile_button.set_active(unit_types.find(leader_type) != nullptr);
}

void faction_select::profile_button_callback(window& window)
{
	const std::string& leader_type = find_widget<menu_button>(&window, "leader_menu", false).get_value_string();
	const unit_type* ut = unit_types.find(leader_type);
	if(ut != nullptr) {
		preferences::encountered_units().insert(ut->id());
		help::help_manager help_manager(&game_config_manager::get()->game_config());
		help::show_unit_description(*ut);
	}
}

void faction_select::on_gender_select(window& window)
{
	flg_manager_.set_current_gender(gender_toggle_.get_active_member_value());

	update_leader_image(window);
}

void faction_select::update_leader_image(window& window)
{
	std::string leader_image = ng::random_enemy_picture;

	if(const unit_type* ut = unit_types.find(flg_manager_.current_leader())) {
		const unit_type& utg = ut->get_gender_unit_type(flg_manager_.current_gender());
		leader_image = formatter() << utg.image() << "~RC(" << utg.flag_rgb() << ">" << tc_color_ << ")" << "~SCALE_INTO_SHARP(144,144)";
	}

	find_widget<image>(&window, "leader_image", false).set_label(leader_image);
}

void faction_select::post_show(window& /*window*/)
{
	//
	// If we're canceling, restore the previous selections. It might be worth looking
	// into only making selections at all here in post_show, but that would require a
	// refactor of the flg_manager class.
	//
	// Also, note it's important to set these in the order of faction -> leader -> gender
	// or the saved indices will be invalid!
	//
	// -- vultraz, 2018-06-16
	//
	if(get_retval() != retval::OK) {
		flg_manager_.set_current_faction(last_faction_);
		flg_manager_.set_current_leader(last_leader_);
		flg_manager_.set_current_gender(last_gender_);
	}
}

} // namespace dialogs
} // namespace gui2
