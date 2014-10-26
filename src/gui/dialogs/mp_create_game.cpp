/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/mp_create_game.hpp"

#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/integer_selector.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "../../settings.hpp"
#include "utils/foreach.tpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include <boost/bind.hpp>
#endif

namespace gui2
{

REGISTER_DIALOG(mp_create_game)

tmp_create_game::tmp_create_game(const config& cfg)
	: cfg_(cfg)
	, scenario_(NULL)
	, use_map_settings_(register_bool(
			  "use_map_settings",
			  true,
			  preferences::use_map_settings,
			  preferences::set_use_map_settings,
			  dialog_callback<tmp_create_game,
							  &tmp_create_game::update_map_settings>))
	, fog_(register_bool("fog", true, preferences::fog, preferences::set_fog))
	, shroud_(register_bool(
			  "shroud", true, preferences::shroud, preferences::set_shroud))
	, start_time_(register_bool("random_start_time",
								true,
								preferences::random_start_time,
								preferences::set_random_start_time))
	, turns_(register_integer(
			  "turn_count", true, preferences::turns, preferences::set_turns))
	, gold_(register_integer("village_gold",
							 true,
							 preferences::village_gold,
							 preferences::set_village_gold))
	, support_(register_integer("village_support",
								false,
								preferences::village_support,
								preferences::set_village_support))
	, experience_(register_integer("experience_modifier",
								   true,
								   preferences::xp_modifier,
								   preferences::set_xp_modifier))
{
}

void tmp_create_game::pre_show(CVideo& /*video*/, twindow& window)
{
	find_widget<tminimap>(&window, "minimap", false).set_config(&cfg_);

	tlistbox& list = find_widget<tlistbox>(&window, "map_list", false);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(list,
								   boost::bind(&tmp_create_game::update_map,
											   *this,
											   boost::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tmp_create_game, &tmp_create_game::update_map>);
#endif

	// Load option (might turn it into a button later).
	string_map item;
	item.insert(std::make_pair("label", _("Load Game")));
	item.insert(std::make_pair("tooltip", _("Load Game...")));
	list.add_row(item);

	// User maps
/*	FIXME implement user maps
	std::vector<std::string> maps;
	filesystem::get_files_in_dir(filesystem::get_user_data_dir() + "/editor/maps", &maps, NULL, filesystem::FILE_NAME_ONLY);

	FOREACH(const AUTO& map, maps) {
		std::map<std::string, t_string> item;
		item.insert(std::make_pair("label", map));
		list->add_row(item);
	}
*/

	// Standard maps
	int i = 0;
	FOREACH(const AUTO & map, cfg_.child_range("multiplayer"))
	{
		if(map["allow_new_game"].to_bool(true)) {
			string_map item;
			item.insert(std::make_pair("label", map["name"].str()));
			item.insert(std::make_pair("tooltip", map["name"].str()));
			list.add_row(item);

			// This hack is needed since the next item is too wide to fit.
			// and the scrollbar can't truncate text yet.
			if(++i == 46) {
				break;
			}
		}
	}

	update_map_settings(window);
}

void tmp_create_game::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		find_widget<tlistbox>(&window, "map_list", false);
	}
}

void tmp_create_game::update_map(twindow& window)
{
	tminimap& minimap = find_widget<tminimap>(&window, "minimap", false);

	const int index = find_widget<tlistbox>(&window, "map_list", false)
							  .get_selected_row() - 1;

	if(index >= 0) {
		config::const_child_itors children = cfg_.child_range("multiplayer");
		std::advance(children.first, index);
		scenario_ = &*children.first;
		minimap.set_map_data((*scenario_)["map_data"]);
	} else {
		minimap.set_map_data("");
		scenario_ = NULL;
	}

	update_map_settings(window);
}

void tmp_create_game::update_map_settings(twindow& window)
{
	const bool use_map_settings = use_map_settings_->get_widget_value(window);

	fog_->widget_set_enabled(window, !use_map_settings, false);
	shroud_->widget_set_enabled(window, !use_map_settings, false);
	start_time_->widget_set_enabled(window, !use_map_settings, false);

	turns_->widget_set_enabled(window, !use_map_settings, false);
	gold_->widget_set_enabled(window, !use_map_settings, false);
	support_->widget_set_enabled(window, !use_map_settings, false);
	experience_->widget_set_enabled(window, !use_map_settings, false);

	if(use_map_settings) {
		if(scenario_) {
			const config& side = scenario_->child("side");

			fog_->set_widget_value(window, side["fog"].to_bool(true));
			shroud_->set_widget_value(window, side["shroud"].to_bool(false));
			start_time_->set_widget_value(
					window, (*scenario_)["random_start_time"].to_bool(true));

			turns_->set_widget_value(
					window, ::settings::get_turns((*scenario_)["turns"]));
			gold_->set_widget_value(
					window, ::settings::get_village_gold(side["village_gold"]));
			support_->set_widget_value(
					window,
					::settings::get_village_support(side["village_support"]));
			experience_->set_widget_value(
					window,
					::settings::get_xp_modifier(
							(*scenario_)["experience_modifier"]));
		}
		// No scenario selected just leave the state unchanged for now.

	} else {

		// Fixme we should store the value and reuse it later...
		fog_->set_widget_value(window, preferences::fog());
		shroud_->set_widget_value(window, preferences::shroud());
		start_time_->set_widget_value(window, preferences::random_start_time());

		turns_->set_widget_value(window, preferences::turns());
		gold_->set_widget_value(window, preferences::village_gold());
		support_->set_widget_value(window, preferences::village_support());
		experience_->set_widget_value(window, preferences::xp_modifier());
	}
}

} // namespace gui2
