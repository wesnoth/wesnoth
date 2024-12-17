/*
	Copyright (C) 2009 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/unit_create.hpp"

#include "gui/core/log.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "units/types.hpp"
#include "utils/ci_searcher.hpp"

#include <functional>

static std::string last_chosen_type_id = "";
static std::string last_variation = "";
static unit_race::GENDER last_gender = unit_race::MALE;

namespace gui2::dialogs
{

REGISTER_DIALOG(unit_create)

unit_create::unit_create()
	: modal_dialog(window_id())
	, gender_(last_gender)
	, choice_(last_chosen_type_id)
	, variation_(last_variation)
{
}

void unit_create::pre_show()
{
	toggle_button& male_toggle
			= find_widget<toggle_button>("male_toggle");
	toggle_button& female_toggle
			= find_widget<toggle_button>("female_toggle");

	gender_toggle.add_member(&male_toggle, unit_race::MALE);
	gender_toggle.add_member(&female_toggle, unit_race::FEMALE);

	gender_toggle.set_member_states(last_gender);

	gender_toggle.set_callback_on_value_change(
		std::bind(&unit_create::gender_toggle_callback, this, std::placeholders::_2));

	menu_button& var_box = find_widget<menu_button>("variation_box");

	connect_signal_notify_modified(var_box, std::bind(&unit_create::variation_menu_callback, this));

	listbox& list = find_widget<listbox>("unit_type_list");

	text_box* filter
			= find_widget<text_box>("filter_box", false, true);

	filter->set_text_changed_callback(
			std::bind(&unit_create::filter_text_changed, this, std::placeholders::_2));

	keyboard_capture(filter);
	add_to_keyboard_chain(&list);

	connect_signal_notify_modified(list, std::bind(&unit_create::list_item_clicked, this));

	list.clear();

	for(const auto& i : unit_types.types())
	{
		// Make sure this unit type is built with the data we need.
		unit_types.build_unit_type(i.second, unit_type::FULL);

		units_.push_back(&i.second);

		widget_data row_data;
		widget_item column;

		column["label"] = units_.back()->race()->plural_name();
		row_data.emplace("race", column);

		column["label"] = units_.back()->type_name();
		if(units_.back()->type_name().str() != units_.back()->id()) {
			column["label"] += " (" + units_.back()->id() + ")";
		}
		row_data.emplace("unit_type", column);

		list.add_row(row_data);

		// Select the previous choice, if any.
		if(!choice_.empty() && choice_ == i.first) {
			list.select_last_row();
		}
	}

	if(units_.empty()) {
		ERR_GUI_G << "no unit types found for unit create dialog; not good"
				  << std::endl;
	}

	list.set_sorters(
		[this](const std::size_t i) { return units_[i]->race()->plural_name(); },
		[this](const std::size_t i) { return units_[i]->type_name(); }
	);

	// Select the first entry on sort if no previous selection was provided.
	list.set_active_sorter("sort_0", sort_order::type::ascending, choice_.empty());

	list_item_clicked();
}

void unit_create::post_show()
{
	listbox& list = find_widget<listbox>("unit_type_list");

	choice_ = "";

	if(get_retval() != retval::OK) {
		return;
	}

	const int selected_row = list.get_selected_row();
	if(selected_row < 0) {
		return;
	} else if(static_cast<std::size_t>(selected_row) >= units_.size()) {
		// FIXME: maybe assert?
		ERR_GUI_G << "unit create dialog has more list items than known unit "
		             "types; not good";
		return;
	}

	last_chosen_type_id = choice_ = units_[selected_row]->id();
	last_gender = gender_;
	last_variation = variation_;
}

void unit_create::update_displayed_type()
{
	const int selected_row
		= find_widget<listbox>("unit_type_list").get_selected_row();

	if(selected_row == -1) {
		return;
	}

	const unit_type* ut = &units_[selected_row]->get_gender_unit_type(gender_);

	if(!variation_.empty()) {
		// This effectively translates to `ut = ut` if somehow variation_ does
		// not refer to a variation that the unit type supports.
		ut = &ut->get_variation(variation_);
	}

	find_widget<unit_preview_pane>("unit_details").set_display_data(*ut);
}

void unit_create::list_item_clicked()
{
	const int selected_row
		= find_widget<listbox>("unit_type_list").get_selected_row();

	if(selected_row == -1) {
		return;
	}

	update_displayed_type();

	gender_toggle.set_members_enabled([&](const unit_race::GENDER& gender)->bool {
		return units_[selected_row]->has_gender_variation(gender);
	});

	menu_button& var_box = find_widget<menu_button>("variation_box");
	std::vector<config> var_box_values;
	var_box_values.emplace_back("label", _("unit_variation^Default Variation"), "variation_id", "");

	const auto& ut = *units_[selected_row];
	const auto& uvars = ut.variation_types();

	var_box.set_active(!uvars.empty());

	unsigned n = 0, selection = 0;

	for(const auto& pair : uvars) {
		++n;

		const std::string& uv_id = pair.first;
		const unit_type& uv = pair.second;

		std::string uv_label;
		if(!uv.variation_name().empty()) {
			uv_label = uv.variation_name() + " (" + uv_id + ")";
		} else if(!uv.type_name().empty() && uv.type_name() != ut.type_name()) {
			uv_label = uv.type_name() + " (" + uv_id + ")";
		} else {
			uv_label = uv_id;
		}

		var_box_values.emplace_back("label", uv_label, "variation_id", uv_id);

		if(uv_id == variation_) {
			selection = n;
		}
	}

	// If we didn't find the variation selection again then the new selected
	// unit type doesn't have that variation id.
	if(!selection) {
		variation_.clear();
	}

	var_box.set_values(var_box_values, selection);
}

void unit_create::filter_text_changed(const std::string& text)
{
	find_widget<listbox>("unit_type_list")
		.filter_rows_by([this, match = translation::make_ci_matcher(text)](std::size_t row) {
			return match(
				units_[row]->type_name(),
				units_[row]->race()->plural_name(),
				units_[row]->id()
			);
		});
}

void unit_create::gender_toggle_callback(const unit_race::GENDER val)
{
	gender_ = val;

	update_displayed_type();
}

void unit_create::variation_menu_callback()
{
	menu_button& var_box = find_widget<menu_button>("variation_box");
	variation_ = var_box.get_value_config()["variation_id"].str();

	update_displayed_type();
}

} // namespace dialogs
