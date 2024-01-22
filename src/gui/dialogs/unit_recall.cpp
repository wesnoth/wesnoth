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

#include "gui/dialogs/unit_recall.hpp"

#include "font/text_formatting.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "game_board.hpp"
#include "gettext.hpp"
#include "replay_helper.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"
#include <functional>
#include "whiteboard/manager.hpp"

#include <boost/dynamic_bitset.hpp>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2::dialogs
{

// Index 2 is by-level
static listbox::order_pair sort_last    {-1, sort_order::type::none};
static listbox::order_pair sort_default { 2, sort_order::type::descending};

REGISTER_DIALOG(unit_recall)

unit_recall::unit_recall(std::vector<unit_const_ptr>& recall_list, team& team)
	: modal_dialog(window_id())
	, recall_list_(recall_list)
	, team_(team)
	, selected_index_()
	, filter_options_()
	, last_words_()
{
}

template<typename T>
static void dump_recall_list_to_console(const T& units)
{
	log_scope2(log_display, "dump_recall_list_to_console()")

	LOG_DP << "size: " << units.size();

	std::size_t idx = 0;
	for(const auto& u_ptr : units) {
		LOG_DP << "\tunit[" << (idx++) << "]: " << u_ptr->id() << " name = '" << u_ptr->name() << "'";
	}
}

static const color_t inactive_row_color(0x96, 0x96, 0x96);

static const inline std::string maybe_inactive(const std::string& str, bool active)
{
	if(active)
		return str;
	else
		return font::span_color(inactive_row_color, str);
}

static std::string format_level_string(const int level, bool recallable)
{
	std::string lvl = std::to_string(level);

	if(!recallable) {
		// Same logic as when recallable, but always in inactive_row_color.
		if(level < 2) {
			return font::span_color(inactive_row_color, lvl);
		} else {
			return font::span_color(inactive_row_color, "<b>" + lvl + "</b>");
		}
	} else if(level < 1) {
		return font::span_color(inactive_row_color, lvl);
	} else if(level == 1) {
		return lvl;
	} else if(level == 2) {
		return "<b>" + lvl + "</b>";
	} else {
		return"<b><span color='#ffffff'>" + lvl + "</span></b>";
	}
}

static std::string format_cost_string(int unit_recall_cost, const int team_recall_cost)
{
	std::stringstream str;

	if(unit_recall_cost < 0) {
		unit_recall_cost = team_recall_cost;
	}

	if(unit_recall_cost > team_recall_cost) {
		str << "<span color='#ff0000'>" << unit_recall_cost << "</span>";
	} else if(unit_recall_cost == team_recall_cost) {
		str << unit_recall_cost;
	} else if(unit_recall_cost < team_recall_cost) {
		str << "<span color='#00ff00'>" << unit_recall_cost << "</span>";
	}

	return str.str();
}

static std::string get_title_suffix(int side_num)
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
			msg << " (" << leader->name(); msg << ")";
		}
	}

	return msg.str();
}

void unit_recall::pre_show(window& window)
{
	label& title = find_widget<label>(&window, "title", true);
	title.set_label(title.get_label() + get_title_suffix(team_.side()));

	text_box* filter
			= find_widget<text_box>(&window, "filter_box", false, true);

	filter->set_text_changed_callback(
			std::bind(&unit_recall::filter_text_changed, this, std::placeholders::_2));

	listbox& list = find_widget<listbox>(&window, "recall_list", false);

	connect_signal_notify_modified(list, std::bind(&unit_recall::list_item_clicked, this));

	list.clear();

	window.keyboard_capture(filter);
	window.add_to_keyboard_chain(&list);

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "rename", false),
		std::bind(&unit_recall::rename_unit, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "dismiss", false),
		std::bind(&unit_recall::dismiss_unit, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "show_help", false),
		std::bind(&unit_recall::show_help, this));

	for(const unit_const_ptr& unit : recall_list_) {
		widget_data row_data;
		widget_item column;

		std::string mods = unit->image_mods();

		int wb_gold = 0;
		if(resources::controller) {
			if(const std::shared_ptr<wb::manager>& whiteb = resources::controller->get_whiteboard()) {
				wb::future_map future; // So gold takes into account planned spending
				wb_gold = whiteb->get_spent_gold_for(team_.side());
			}
		}

		// Note: Our callers apply [filter_recall], but leave it to us
		// to apply cost-based filtering.
		const int recall_cost = (unit->recall_cost() > -1 ? unit->recall_cost() : team_.recall_cost());
		const bool recallable = (recall_cost <= team_.gold() - wb_gold);

		if(unit->can_recruit()) {
			mods += "~BLIT(" + unit::leader_crown() + ")";
		}

		for(const std::string& overlay : unit->overlays()) {
			mods += "~BLIT(" + overlay + ")";
		}

		if(!recallable) {
			mods += "~GS()";

			// Just set the tooltip on every single element in this row.
			if(wb_gold > 0)
				column["tooltip"] = _("This unit cannot be recalled because you will not have enough gold at this point in your plan.");
			else
				column["tooltip"] = _("This unit cannot be recalled because you do not have enough gold.");
		}

		column["use_markup"] = "true";

		column["label"] = unit->absolute_image() + mods;
		row_data.emplace("unit_image", column);

		column["label"] = maybe_inactive(unit->type_name(), recallable);
		row_data.emplace("unit_type", column);

		// gold_icon is handled below

		column["label"] =
			recallable
			? format_cost_string(unit->recall_cost(), team_.recall_cost())
			: maybe_inactive(std::to_string(recall_cost), recallable);
		row_data.emplace("unit_recall_cost", column);

		const std::string& name = !unit->name().empty() ? unit->name().str() : font::unicode_en_dash;
		column["label"] = maybe_inactive(name, recallable);
		row_data.emplace("unit_name", column);

		column["label"] = format_level_string(unit->level(), recallable);
		row_data.emplace("unit_level", column);

		std::stringstream exp_str;
		if(unit->can_advance()) {
			exp_str << unit->experience() << "/" << unit->max_experience();
		} else {
			exp_str << font::unicode_en_dash;
		}

		column["label"] = font::span_color(recallable ? unit->xp_color() : inactive_row_color, exp_str.str());
		row_data.emplace("unit_experience", column);

		// Since the table widgets use heavy formatting, we save a bare copy
		// of certain options to filter on.
		std::string filter_text = unit->type_name() + " " + name + " " + std::to_string(unit->level());

		if(recallable) {
			// This is to allow filtering for recallable units by typing "vvv" in the search box.
			// That's intended to be easy to type and unlikely to match unit or type names.
			//
			// TODO: document this. (Also, implement a "Hide non-recallable units" checkbox.)
			filter_text += " " + std::string("vvv");
		}

		std::string traits;
		for(const std::string& trait : unit->trait_names()) {
			traits += (traits.empty() ? "" : "\n") + trait;
			filter_text += " " + trait;
		}

		column["label"] = maybe_inactive(
					!traits.empty() ? traits : font::unicode_en_dash,
					recallable);
		row_data.emplace("unit_traits", column);

		filter_options_.push_back(filter_text);
		grid& grid = list.add_row(row_data);
		if(!recallable) {
			image *gold_icon = dynamic_cast<image*>(grid.find("gold_icon", false));
			assert(gold_icon);
			gold_icon->set_image(gold_icon->get_image() + "~GS()");
		}
	}

	list.register_translatable_sorting_option(0, [this](const int i) { return recall_list_[i]->type_name().str(); });
	list.register_translatable_sorting_option(1, [this](const int i) { return recall_list_[i]->name().str(); });
	list.register_sorting_option(2, [this](const int i) {
		const unit& u = *recall_list_[i];
		return std::tuple(u.level(), -static_cast<int>(u.experience_to_advance()));
	});
	list.register_sorting_option(3, [this](const int i) { return recall_list_[i]->experience(); });
	list.register_translatable_sorting_option(4, [this](const int i) {
		return !recall_list_[i]->trait_names().empty() ? recall_list_[i]->trait_names().front().str() : "";
	});

	list.set_active_sorting_option(sort_last.first >= 0 ? sort_last	: sort_default, true);

	list_item_clicked();
}

void unit_recall::rename_unit()
{
	listbox& list = find_widget<listbox>(get_window(), "recall_list", false);

	const int index = list.get_selected_row();
	if (index == -1) {
		return;
	}

	unit& selected_unit = const_cast<unit&>(*recall_list_[index].get());

	std::string name = selected_unit.name();
	const std::string dialog_title(_("Rename Unit"));
	const std::string dialog_label(_("Name:"));

	if(gui2::dialogs::edit_text::execute(dialog_title, dialog_label, name)) {
		selected_unit.rename(name);

		find_widget<label>(list.get_row_grid(index), "unit_name", false).set_label(name);

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

void unit_recall::dismiss_unit()
{
	LOG_DP << "Recall list units:"; dump_recall_list_to_console(recall_list_);

	listbox& list = find_widget<listbox>(get_window(), "recall_list", false);
	const int index = list.get_selected_row();
	if (index == -1) {
		return;
	}

	const unit& u = *recall_list_[index].get();

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

	recall_list_.erase(recall_list_.begin() + index);

	// Remove the entry from the dialog list
	list.remove_row(index);
	list_item_clicked();

	// Remove the entry from the filter list
	filter_options_.erase(filter_options_.begin() + index);
	assert(filter_options_.size() == list.get_item_count());

	LOG_DP << "Dismissing a unit, side = " << u.side() << ", id = '" << u.id() << "'";
	LOG_DP << "That side's recall list:";
	dump_recall_list_to_console(team_.recall_list());

	// Find the unit in the recall list.
	unit_ptr dismissed_unit = team_.recall_list().find_if_matches_id(u.id());
	assert(dismissed_unit);

	// Record the dismissal, then delete the unit.
	synced_context::run_and_throw("disband", replay_helper::get_disband(dismissed_unit->id()));

	// Close the dialog if all units are dismissed
	if(list.get_item_count() == 0) {
		set_retval(retval::CANCEL);
	}
}

void unit_recall::show_help()
{
	help::show_help("recruit_and_recall");
}

void unit_recall::list_item_clicked()
{
	const int selected_row
		= find_widget<listbox>(get_window(), "recall_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	const unit& selected_unit = *recall_list_[selected_row].get();

	find_widget<unit_preview_pane>(get_window(), "unit_details", false)
		.set_displayed_unit(selected_unit);

	find_widget<button>(get_window(), "rename", false).set_active(!selected_unit.unrenamable());
}

void unit_recall::post_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "recall_list", false);
	sort_last = list.get_active_sorting_option();

	if(get_retval() == retval::OK) {
		selected_index_ = list.get_selected_row();
	}
}

void unit_recall::filter_text_changed(const std::string& text)
{
	listbox& list = find_widget<listbox>(get_window(), "recall_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return;
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
	find_widget<button>(get_window(), "rename", false).set_active(any_shown);
	find_widget<button>(get_window(), "dismiss", false).set_active(any_shown);
}

} // namespace dialogs
