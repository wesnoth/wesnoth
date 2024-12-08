/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "gui/widgets/listbox.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "display.hpp"
#include "formatter.hpp"
#include "units/map.hpp"
#include "units/ptr.hpp"
#include "units/unit.hpp"
#include "serialization/markup.hpp"

#include <functional>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2::dialogs
{

REGISTER_DIALOG(unit_list)

unit_list::unit_list(std::vector<unit_const_ptr>& unit_list, map_location& scroll_to)
	: modal_dialog(window_id())
	, unit_list_(unit_list)
	, scroll_to_(scroll_to)
{
}

static std::string format_level_string(const int level)
{
	if(level < 1) {
		return markup::span_color("#969696", level);
	} else if(level == 1) {
		return std::to_string(level);
	} else if(level == 2) {
		return markup::bold(level);
	} else { // level must be > 2
		return markup::span_color("#ffffff", markup::bold(level));
	}

}

static std::string format_if_leader(const unit_const_ptr& u, const std::string& str)
{
	return u->can_recruit() ? markup::span_color("#cdad00", str) : str;
}

static std::string format_movement_string(const unit_const_ptr& u)
{
	const int moves_left = u->movement_left();
	const int moves_max  = u->total_movement();

	std::string color = "#00ff00";
	if(moves_left == 0) {
		color = "#ff0000";
	} else if(moves_left < moves_max) {
		color = "#ffff00";
	}

	return markup::span_color(color, moves_left, "/", moves_max);
}

void unit_list::pre_show()
{
	listbox& list = find_widget<listbox>("units_list");

	connect_signal_notify_modified(list, std::bind(&unit_list::list_item_clicked, this));

	list.clear();

	keyboard_capture(&list);

	for(const unit_const_ptr& unit : unit_list_) {
		widget_data row_data;
		widget_item column;

		column["use_markup"] = "true";

		column["label"] = format_if_leader(unit, unit->type_name());
		row_data.emplace("unit_type", column);

		const std::string& name = !unit->name().empty() ? format_if_leader(unit, unit->name().str()) : font::unicode_en_dash;
		column["label"] = name;
		row_data.emplace("unit_name", column);

		column["label"] = format_movement_string(unit);
		row_data.emplace("unit_moves", column);

		std::stringstream hp_str;
		hp_str << markup::span_color(unit->hp_color(), unit->hitpoints(), "/", unit->max_hitpoints());

		column["label"] = hp_str.str();
		row_data.emplace("unit_hp", column);

		column["label"] = format_level_string(unit->level());
		row_data.emplace("unit_level", column);

		if(unit->can_advance()) {
			column["label"] = markup::span_color(unit->xp_color(), unit->experience(), "/", unit->max_experience());
		} else {
			column["label"] = markup::span_color(unit->xp_color(), font::unicode_en_dash);
		}
		row_data.emplace("unit_experience", column);

		column["label"] = utils::join(unit->trait_names(), ", ");
		row_data.emplace("unit_traits", column);

		grid& row_grid = list.add_row(row_data);

		// NOTE: this needs to be done *after* the row is added
		// TODO: show custom statuses
		if(!unit->get_state(unit::STATE_PETRIFIED)) {
			row_grid.find_widget<image>("unit_status_petrified").set_visible(widget::visibility::invisible);
		}

		if(!unit->get_state(unit::STATE_POISONED)) {
			row_grid.find_widget<image>("unit_status_poisoned").set_visible(widget::visibility::invisible);
		}

		if(!unit->get_state(unit::STATE_SLOWED)) {
			row_grid.find_widget<image>("unit_status_slowed").set_visible(widget::visibility::invisible);
		}

		if(!unit->invisible(unit->get_location(), false)) {
			row_grid.find_widget<image>("unit_status_invisible").set_visible(widget::visibility::invisible);
		}
	}

	list.set_sorters(
		[this](const std::size_t i) { return unit_list_[i]->type_name(); },
		[this](const std::size_t i) { return unit_list_[i]->name(); },
		[this](const std::size_t i) { return unit_list_[i]->movement_left(); },
		[this](const std::size_t i) { return unit_list_[i]->hitpoints(); },
		[this](const std::size_t i) {
			const unit& u = *unit_list_[i];
			return std::tuple(u.level(), -static_cast<int>(u.experience_to_advance()));
		},
		[this](const std::size_t i) { return unit_list_[i]->experience(); },
		[this](const std::size_t i) {
			return !unit_list_[i]->trait_names().empty() ? unit_list_[i]->trait_names().front() : t_string(); }
	);

	list_item_clicked();
}

void unit_list::list_item_clicked()
{
	const int selected_row
		= find_widget<listbox>("units_list").get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<unit_preview_pane>("unit_details")
		.set_displayed_unit(*unit_list_[selected_row].get());
}

void unit_list::post_show()
{
	if(get_retval() == retval::OK) {
		const int selected_row = find_widget<listbox>("units_list").get_selected_row();

		scroll_to_ = unit_list_[selected_row]->get_location();
	}
}

void show_unit_list(display& gui)
{
	std::vector<unit_const_ptr> unit_list;
	map_location scroll_to;

	const unit_map& units = gui.context().units();
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->side() != gui.viewing_team().side()) {
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
