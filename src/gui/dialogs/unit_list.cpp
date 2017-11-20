/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/unit_list.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "display.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "game_board.hpp"
#include "resources.hpp"
#include "units/map.hpp"
#include "units/ptr.hpp"
#include "units/unit.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(unit_list)

unit_list::unit_list(unit_ptr_vector& unit_list, map_location& scroll_to)
	: unit_list_(unit_list)
	, scroll_to_(scroll_to)
{
}

static std::string format_level_string(const int level)
{
	std::string lvl = std::to_string(level);

	if(level < 1) {
		return "<span color='#969696'>" + lvl + "</span>";
	} else if(level == 1) {
		return lvl;
	} else if(level == 2) {
		return "<b>" + lvl + "</b>";
	} else { // level must be > 2
		return"<b><span color='#ffffff'>" + lvl + "</span></b>";
	}

}

static std::string format_if_leader(unit_const_ptr u, const std::string str)
{
	return (*u).can_recruit() ? "<span color='#cdad00'>" + str + "</span>" : str;
}

static std::string format_movement_string(unit_const_ptr u)
{
	const int moves_left = (*u).movement_left();
	const int moves_max  = (*u).total_movement();

	std::string color = "#00ff00";

	if(moves_left == 0) {
		color = "#ff0000";
	} else if(moves_left < moves_max) {
		color = "#ffff00";
	}

	return formatter() << "<span color='" << color << "'>" << moves_left << "/" << moves_max << "</span>";
}

void unit_list::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "units_list", false);

	connect_signal_notify_modified(list, std::bind(&unit_list::list_item_clicked, this, std::ref(window)));

	list.clear();

	window.keyboard_capture(&list);

	for(const unit_const_ptr& unit : unit_list_) {
		std::map<std::string, string_map> row_data;
		string_map column;

		column["use_markup"] = "true";

		column["label"] = format_if_leader(unit, unit->type_name());
		row_data.emplace("unit_type", column);

		const std::string& name = !unit->name().empty() ? format_if_leader(unit, unit->name().str()) : font::unicode_en_dash;
		column["label"] = name;
		row_data.emplace("unit_name", column);

		column["label"] = format_movement_string(unit);
		row_data.emplace("unit_moves", column);

		std::stringstream hp_str;
		hp_str << font::span_color(unit->hp_color()) << unit->hitpoints() << "/" << unit->max_hitpoints() << "</span>";

		column["label"] = hp_str.str();
		row_data.emplace("unit_hp", column);

		column["label"] = format_level_string(unit->level());
		row_data.emplace("unit_level", column);

		std::stringstream exp_str;
		exp_str << font::span_color(unit->xp_color()) << unit->experience() << "/"
		        << (unit->can_advance() ? std::to_string(unit->max_experience()) : font::unicode_en_dash) << "</span>";

		column["label"] = exp_str.str();
		row_data.emplace("unit_experience", column);

		column["label"] = utils::join(unit->trait_names(), ", ");
		row_data.emplace("unit_traits", column);

		grid* row_grid = &list.add_row(row_data);

		// NOTE: this needs to be done *after* the row is added
		// TODO: show custom statuses
		if(!unit->get_state(unit::STATE_PETRIFIED)) {
			find_widget<image>(row_grid, "unit_status_petrified", false).set_visible(widget::visibility::invisible);
		}

		if(!unit->get_state(unit::STATE_POISONED)) {
			find_widget<image>(row_grid, "unit_status_poisoned", false).set_visible(widget::visibility::invisible);
		}

		if(!unit->get_state(unit::STATE_SLOWED)) {
			find_widget<image>(row_grid, "unit_status_slowed", false).set_visible(widget::visibility::invisible);
		}

		if(!unit->invisible(unit->get_location(), *resources::gameboard, false)) {
			find_widget<image>(row_grid, "unit_status_invisible", false).set_visible(widget::visibility::invisible);
		}
	}

	list.register_sorting_option(0, [this](const int i) { return unit_list_[i]->type_name().str(); });
	list.register_sorting_option(1, [this](const int i) { return unit_list_[i]->name().str(); });
	list.register_sorting_option(2, [this](const int i) { return unit_list_[i]->movement_left(); });
	list.register_sorting_option(3, [this](const int i) { return unit_list_[i]->hitpoints(); });
	list.register_sorting_option(4, [this](const int i) { return unit_list_[i]->level(); });
	list.register_sorting_option(5, [this](const int i) { return unit_list_[i]->experience(); });
	list.register_sorting_option(6, [this](const int i) {
		return !unit_list_[i]->trait_names().empty() ? unit_list_[i]->trait_names().front().str() : ""; });

	list_item_clicked(window);
}

void unit_list::list_item_clicked(window& window)
{
	const int selected_row
		= find_widget<listbox>(&window, "units_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<unit_preview_pane>(&window, "unit_details", false)
		.set_displayed_unit(*unit_list_[selected_row].get());
}

void unit_list::post_show(window& window)
{
	if(get_retval() == window::OK) {
		const int selected_row = find_widget<listbox>(&window, "units_list", false).get_selected_row();

		scroll_to_ = unit_list_[selected_row]->get_location();
	}
}

void show_unit_list(display& gui)
{
	unit_ptr_vector unit_list;
	map_location scroll_to;

	const unit_map& units = gui.get_units();
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->side() != gui.viewing_side()) {
			continue;
		}

		unit_list.push_back(i.get_shared_ptr());
	}

	if(unit_list::execute(unit_list, scroll_to)) {
		gui.scroll_to_tile(scroll_to, display::WARP);
		gui.select_hex(scroll_to);
	}
}

} // namespace dialogs
} // namespace gui2
