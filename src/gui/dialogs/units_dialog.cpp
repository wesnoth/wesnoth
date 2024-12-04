/*
	Copyright (C) 2024
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

#include "gui/dialogs/units_dialog.hpp"

#include "formatter.hpp"
#include "game_board.hpp"
#include "gettext.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "log.hpp"
#include "replay_helper.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "serialization/markup.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/helper.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"
#include "units/types.hpp"
#include "whiteboard/manager.hpp"

#include <functional>
#include <string>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2::dialogs
{

namespace
{
// Index 2 is by-level
listbox::order_pair sort_last    {-1, sort_order::type::none};
listbox::order_pair sort_default { 2, sort_order::type::descending};
}

REGISTER_DIALOG(units_dialog)

units_dialog::units_dialog()
	: modal_dialog(window_id())
	, unit_type_list_()
	, unit_list_()
	, team_(nullptr)
	, selected_index_(-1)
	, row_num_()
	, ok_label_(_("OK"))
	, show_variation_grid_(false)
	, show_gender_grid_(false)
	, show_dismiss_(false)
	, show_rename_(false)
	, gender_(unit_race::GENDER::MALE)
	, variation_()
	, filter_options_()
	, last_words_()
{
	// FIXME number of columns shouldn't be hardcoded.
	// should be calculated from grid instead
	visible_headers_.resize(8);
	visible_headers_.set(); // headers are shown by default
}

namespace {

template<typename T>
void dump_recall_list_to_console(const T& units)
{
	log_scope2(log_display, "dump_recall_list_to_console()")

	LOG_DP << "size: " << units.size();

	std::size_t idx = 0;
	for(const auto& u_ptr : units) {
		LOG_DP << "\tunit[" << (idx++) << "]: " << u_ptr->id() << " name = '" << u_ptr->name() << "'";
	}
}

std::string get_title_suffix(int side_num)
{
	if(!resources::gameboard) {
		return "";
	}

	unit_map& units = resources::gameboard->units();

	int controlled_recruiters = 0;
	for(const auto& team : resources::gameboard->teams()) {
		if(team.is_local_human() && !team.recruits().empty() && units.find_leader(team.side()) !=units.end()) {
			++controlled_recruiters;
		}
	}

	std::stringstream msg;
	if(controlled_recruiters >= 2) {
		unit_map::const_iterator leader = resources::gameboard->units().find_leader(side_num);
		if(leader != resources::gameboard->units().end() && !leader->name().empty()) {
			msg << " (" << leader->name() << ")";
		}
	}

	return msg.str();
}
}

void units_dialog::pre_show()
{
	label& title = find_widget<label>("title", true);
	if (team_) {
		title.set_label(title.get_label() + get_title_suffix(team_->side()));
	}

	text_box& filter = find_widget<text_box>("filter_box");
	connect_signal_notify_modified(filter, std::bind(&units_dialog::filter_text_changed, this));

	listbox& list = find_widget<listbox>("recall_list");
	connect_signal_notify_modified(list, std::bind(&units_dialog::list_item_clicked, this));

	connect_signal_mouse_left_click(
		find_widget<button>("show_help"),
		std::bind(&units_dialog::show_help, this));

	list.clear();

	keyboard_capture(&filter);
	add_to_keyboard_chain(&list);

	if (show_gender_grid_) {
		toggle_button& male_toggle = find_widget<toggle_button>("male_toggle");
		toggle_button& female_toggle = find_widget<toggle_button>("female_toggle");

		gender_toggle_.add_member(&male_toggle, unit_race::MALE);
		gender_toggle_.add_member(&female_toggle, unit_race::FEMALE);
		gender_toggle_.set_member_states(unit_race::MALE);
		gender_toggle_.set_callback_on_value_change(
			std::bind(&units_dialog::gender_toggle_callback, this, std::placeholders::_2));
	}

	if (show_variation_grid_) {
		menu_button& var_box = find_widget<menu_button>("variation_box");
		connect_signal_notify_modified(var_box, std::bind(&units_dialog::variation_menu_callback, this));
	}

	if (show_rename_) {
		connect_signal_mouse_left_click(
		find_widget<button>("rename"),
		std::bind(&units_dialog::rename_unit, this));
	}

	if (show_dismiss_ && team_) {
		connect_signal_mouse_left_click(
			find_widget<button>("dismiss"),
			std::bind(&units_dialog::dismiss_unit, this));
	}

	show_list(list);

	find_widget<label>("title").set_label(title_);
	find_widget<button>("ok").set_label(ok_label_);
	find_widget<button>("dismiss").set_visible(
		show_dismiss_ ? widget::visibility::visible : widget::visibility::invisible);
	find_widget<button>("rename").set_visible(
		show_rename_ ? widget::visibility::visible : widget::visibility::invisible);
	find_widget<grid>("variation_gender_grid").set_visible(
		show_gender_grid_ ? widget::visibility::visible : widget::visibility::invisible);

	for (size_t i = 0; i < visible_headers_.size(); i++) {
		find_widget<toggle_button>("sort_" + std::to_string(i))
			.set_visible(visible_headers_[i]
				? widget::visibility::visible
				: widget::visibility::invisible);
	}
	list_item_clicked();
}

void units_dialog::show_list(listbox& list)
{
	if (unit_type_list_.empty() && unit_list_.empty()) {
		return;
	}

	for(size_t i = 0; i < row_num_; i++) {
		widget_data row_data;
		widget_item column;
		formatter filter_fmt;
		// generate tooltip for ith row
		if (tooltip_gen_) {
			column["tooltip"] = tooltip_gen_(i);
		}

		for (const auto& [id, gen] : column_generators_) {
			column["use_markup"] = "true";
			// generate label for ith row and column with 'id'
			column["label"] = gen(i);
			if (id != "unit_image") {
				filter_fmt << column["label"];
			}
			row_data.emplace(id, column);
		}

		filter_options_.push_back(filter_fmt.str());
		list.add_row(row_data);
	}

	list.set_active_sorting_option(sort_last.first >= 0 ? sort_last : sort_default, true);
}

void units_dialog::rename_unit()
{
	listbox& list = find_widget<listbox>("recall_list");

	const int index = list.get_selected_row();
	if (index == -1) {
		return;
	}

	unit& selected_unit = const_cast<unit&>(*unit_list_[index].get());

	std::string name = selected_unit.name();
	const std::string dialog_title(_("Rename Unit"));
	const std::string dialog_label(_("Name:"));

	if(gui2::dialogs::edit_text::execute(dialog_title, dialog_label, name)) {
		selected_unit.rename(name);

		list.get_row_grid(index)->find_widget<label>("unit_name").set_label(name);

		filter_options_.erase(filter_options_.begin() + index);
		std::ostringstream filter_text;
		filter_text << selected_unit.type_name() << " " << name << " " << std::to_string(selected_unit.level());
		for(const std::string& trait : selected_unit.trait_names()) {
			filter_text << " " << trait;
		}
		filter_options_.insert(filter_options_.begin() + index, filter_text.str());

		list_item_clicked();
		get_window()->invalidate_layout();
	}
}

void units_dialog::dismiss_unit()
{
	if (team_ == nullptr) {
		LOG_DP << "No team specificed, can't dismiss.";
		return;
	}

	LOG_DP << "Recall list units:"; dump_recall_list_to_console(unit_list_);

	listbox& list = find_widget<listbox>("recall_list");
	const int index = list.get_selected_row();
	if (index == -1) {
		return;
	}

	const unit& u = *unit_list_[index].get();

	// If the unit is of level > 1, or is close to advancing, we warn the player about it
	std::stringstream message;
	if(u.loyal()) {
		message << _("This unit is loyal and requires no upkeep.") << " " << (u.gender() == unit_race::MALE
		         ? _("Do you really want to dismiss him?")
		         : _("Do you really want to dismiss her?"));

	} else if(u.level() > 1) {
		message << _("This unit is an experienced one, having advanced levels.") << " " << (u.gender() == unit_race::MALE
		         ? _("Do you really want to dismiss him?")
		         : _("Do you really want to dismiss her?"));

	} else if(u.experience() > u.max_experience()/2) {
		message << _("This unit is close to advancing a level.") << " " << (u.gender() == unit_race::MALE
		         ? _("Do you really want to dismiss him?")
		         : _("Do you really want to dismiss her?"));
	}

	if(!message.str().empty()) {
		const int res = gui2::show_message(_("Dismiss Unit"), message.str(), message::yes_no_buttons);
		if(res != gui2::retval::OK) {
			return;
		}
	}

	unit_list_.erase(unit_list_.begin() + index);

	// Remove the entry from the dialog list
	list.remove_row(index);
	list_item_clicked();

	// Remove the entry from the filter list
	filter_options_.erase(filter_options_.begin() + index);
	assert(filter_options_.size() == list.get_item_count());

	LOG_DP << "Dismissing a unit, side = " << u.side() << ", id = '" << u.id() << "'";
	LOG_DP << "That side's recall list:";
	dump_recall_list_to_console(team_->recall_list());


	// Find the unit in the recall list.
	unit_ptr dismissed_unit = team_->recall_list().find_if_matches_id(u.id());
	assert(dismissed_unit);

	// Record the dismissal, then delete the unit.
	synced_context::run_and_throw("disband", replay_helper::get_disband(dismissed_unit->id()));

	// Close the dialog if all units are dismissed
	if(list.get_item_count() == 0) {
		set_retval(retval::CANCEL);
	}
}

void units_dialog::show_help() const
{
	if (!topic_id_.empty()) {
		help::show_help(topic_id_);
	}
}

void units_dialog::list_item_clicked()
{
	const int selected_row
		= find_widget<listbox>("recall_list").get_selected_row();

	if (selected_row == -1) {
		return;
	}

	auto& unit_preview = find_widget<unit_preview_pane>("unit_details");

	if (!unit_list_.empty()) {
		const unit& selected_unit = *unit_list_[selected_row].get();
		unit_preview.set_displayed_unit(selected_unit);
		find_widget<button>("rename").set_active(!selected_unit.unrenamable());
	} else if (!unit_type_list_.empty()) {
		if (show_gender_grid_ || show_variation_grid_) {
			update_gender_and_variations(unit_preview, selected_row);
		} else {
			unit_preview.set_displayed_type(*unit_type_list_[selected_row]);
		}
	}
	// TODO handle the case when both lists are non-empty?
}

void units_dialog::update_gender_and_variations(unit_preview_pane& preview, int selected_row)
{
	const unit_type* ut = &unit_type_list_[selected_row]->get_gender_unit_type(gender_);
	if(!variation_.empty()) {
		// This effectively translates to `ut = ut` if somehow variation_ does
		// not refer to a variation that the unit type supports.
		ut = &ut->get_variation(variation_);
	}
	preview.set_displayed_type(*ut);

	gender_toggle_.set_members_enabled([&](const unit_race::GENDER& gender)->bool {
		return unit_type_list_[selected_row]->has_gender_variation(gender);
	});

	menu_button& var_box = find_widget<menu_button>("variation_box");
	std::vector<config> var_box_values;
	var_box_values.emplace_back("label", _("unit_variation^Default Variation"), "variation_id", "");

	const auto& uvars = ut->variation_types();

	var_box.set_active(!uvars.empty());

	unsigned n = 0, selection = 0;

	for(const auto& pair : uvars) {
		++n;

		const std::string& uv_id = pair.first;
		const unit_type& uv = pair.second;

		std::string uv_label;
		if(!uv.variation_name().empty()) {
			uv_label = uv.variation_name() + " (" + uv_id + ")";
		} else if(!uv.type_name().empty() && uv.type_name() != ut->type_name()) {
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

void units_dialog::post_show()
{
	listbox& list = find_widget<listbox>("recall_list");
	sort_last = list.get_active_sorting_option();

	if(get_retval() == retval::OK) {
		selected_index_ = list.get_selected_row();
	}
}

void units_dialog::filter_text_changed()
{
	const std::string& text = find_widget<text_box>("filter_box").get_value();
	listbox& list = find_widget<listbox>("recall_list");

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_) {
		return;
	}
	last_words_ = words;

	boost::dynamic_bitset<> show_items;
	show_items.resize(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			bool found = false;

			for(const auto & word : words) {
				found = translation::ci_search(filter_options_[i], word);

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);

	// Disable rename and dismiss buttons if no units are shown
	const bool any_shown = list.any_rows_shown();
	find_widget<button>("rename").set_active(any_shown);
	find_widget<button>("dismiss").set_active(any_shown);
}

void units_dialog::gender_toggle_callback(const unit_race::GENDER val)
{
	gender_ = val;

	const int selected_row = find_widget<listbox>("recall_list").get_selected_row();
	if(selected_row == -1) {
		return;
	}

	auto& unit_preview = find_widget<unit_preview_pane>("unit_details");
	const unit_type* ut = &unit_type_list_[selected_row]->get_gender_unit_type(gender_);
	unit_preview.set_displayed_type(*ut);
}

void units_dialog::variation_menu_callback()
{
	menu_button& var_box = find_widget<menu_button>("variation_box");
	variation_ = var_box.get_value_config()["variation_id"].str();

	const int selected_row = find_widget<listbox>("recall_list").get_selected_row();
	if(selected_row == -1) {
		return;
	}

	if(!variation_.empty()) {
		auto& unit_preview = find_widget<unit_preview_pane>("unit_details");
		unit_preview.set_displayed_type(unit_type_list_[selected_row]->get_variation(variation_));
	}
}

} // namespace dialogs
