/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "game_board.hpp"
#include "gettext.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"
#include "utils/functional.hpp"

#include <boost/dynamic_bitset.hpp>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2
{
namespace dialogs
{

// Index 2 is by-level
static listbox::order_pair sort_last    {-1, listbox::SORT_NONE};
static listbox::order_pair sort_default { 2, listbox::SORT_DESCENDING};

REGISTER_DIALOG(unit_recall)

unit_recall::unit_recall(recalls_ptr_vector& recall_list, team& team)
	: recall_list_(recall_list)
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

	LOG_DP << "size: " << units.size() << "\n";

	size_t idx = 0;
	for(const unit_const_ptr& u_ptr : units) {
		LOG_DP << "\tunit[" << (idx++) << "]: " << u_ptr->id() << " name = '" << u_ptr->name() << "'\n";
	}
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

bool unit_recall::unit_recall_default_compare(const unit_const_ptr first, const unit_const_ptr second)
{
	if (first->level() > second->level()) return true;
	if (first->level() < second->level()) return false;
	if (first->experience_to_advance() < second->experience_to_advance()) return true;
	return false;
}

void unit_recall::pre_show(window& window)
{
	label& title = find_widget<label>(&window, "title", true);
	title.set_label(title.get_label() + get_title_suffix(team_.side()));

	text_box* filter
			= find_widget<text_box>(&window, "filter_box", false, true);

	filter->set_text_changed_callback(
			std::bind(&unit_recall::filter_text_changed, this, _1, _2));

	listbox& list = find_widget<listbox>(&window, "recall_list", false);

	connect_signal_notify_modified(list, std::bind(&unit_recall::list_item_clicked, this, std::ref(window)));

	list.clear();

	window.keyboard_capture(filter);
	window.add_to_keyboard_chain(&list);

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "dismiss", false),
		std::bind(&unit_recall::dismiss_unit, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "show_help", false),
		std::bind(&unit_recall::show_help, this));

	for(const unit_const_ptr& unit : recall_list_) {
		std::map<std::string, string_map> row_data;
		string_map column;

		std::string mods = unit->image_mods();

		if(unit->can_recruit()) {
			mods += "~BLIT(" + unit::leader_crown() + ")";
		}

		for(const std::string& overlay : unit->overlays()) {
			mods += "~BLIT(" + overlay + ")";
		}

		column["use_markup"] = "true";

		column["label"] = unit->absolute_image() + mods;
		row_data.emplace("unit_image", column);

		column["label"] = unit->type_name();
		row_data.emplace("unit_type", column);

		column["label"] = format_cost_string(unit->recall_cost(), team_.recall_cost());
		row_data.emplace("unit_recall_cost", column);

		const std::string& name = !unit->name().empty() ? unit->name().str() : font::unicode_en_dash;
		column["label"] = name;
		row_data.emplace("unit_name", column);

		column["label"] = format_level_string(unit->level());
		row_data.emplace("unit_level", column);

		std::stringstream exp_str;
		exp_str << font::span_color(unit->xp_color()) << unit->experience() << "/"
		        << (unit->can_advance() ? std::to_string(unit->max_experience()) : font::unicode_en_dash) << "</span>";

		column["label"] = exp_str.str();
		row_data.emplace("unit_experience", column);

		// Since the table widgets use heavy formatting, we save a bare copy
		// of certain options to filter on.
		std::string filter_text = unit->type_name() + " " + name + " " + std::to_string(unit->level());

		std::string traits;
		for(const std::string& trait : unit->trait_names()) {
			traits += (traits.empty() ? "" : "\n") + trait;
			filter_text += " " + trait;
		}

		column["label"] = !traits.empty() ? traits : font::unicode_en_dash;
		row_data.emplace("unit_traits", column);

		list.add_row(row_data);
		filter_options_.push_back(filter_text);
	}

	list.register_sorting_option(0, [this](const int i) { return recall_list_[i]->type_name().str(); });
	list.register_sorting_option(1, [this](const int i) { return recall_list_[i]->name().str(); });
	list.set_column_order(2, {{
			[this](int lhs, int rhs) { return unit_recall_default_compare(recall_list_[rhs], recall_list_[lhs]); },
			[this](int lhs, int rhs) { return unit_recall_default_compare(recall_list_[lhs], recall_list_[rhs]); }
	}});
	list.register_sorting_option(3, [this](const int i) { return recall_list_[i]->experience(); });
	list.register_sorting_option(4, [this](const int i) {
		return !recall_list_[i]->trait_names().empty() ? recall_list_[i]->trait_names().front().str() : "";
	});

	list.set_active_sorting_option(sort_last.first >= 0 ? sort_last	: sort_default);

	list_item_clicked(window);
}

void unit_recall::dismiss_unit(window& window)
{
	LOG_DP << "Recall list units:\n"; dump_recall_list_to_console(recall_list_);

	listbox& list = find_widget<listbox>(&window, "recall_list", false);
	const int index = list.get_selected_row();

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
	list_item_clicked(window);

	// Remove the entry from the filter list
	filter_options_.erase(filter_options_.begin() + index);
	assert(filter_options_.size() == list.get_item_count());

	LOG_DP << "Dismissing a unit, side = " << u.side() << ", id = '" << u.id() << "'\n";
	LOG_DP << "That side's recall list:\n";
	dump_recall_list_to_console(team_.recall_list());

	// Find the unit in the recall list.
	unit_ptr dismissed_unit = team_.recall_list().find_if_matches_id(u.id());
	assert(dismissed_unit);

	// Record the dismissal, then delete the unit.
	synced_context::run_and_throw("disband", replay_helper::get_disband(dismissed_unit->id()));

	// Close the dialog if all units are dismissed
	if(list.get_item_count() == 0) {
		window.set_retval(retval::CANCEL);
	}
}

void unit_recall::show_help()
{
	help::show_help("recruit_and_recall");
}

void unit_recall::list_item_clicked(window& window)
{
	const int selected_row
		= find_widget<listbox>(&window, "recall_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<unit_preview_pane>(&window, "unit_details", false)
		.set_displayed_unit(*recall_list_[selected_row].get());
}

void unit_recall::post_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "recall_list", false);
	sort_last = list.get_active_sorting_option();

	if(get_retval() == retval::OK) {
		selected_index_ = list.get_selected_row();
	}
}

void unit_recall::filter_text_changed(text_box_base* textbox, const std::string& text)
{
	window& window = *textbox->get_window();

	listbox& list = find_widget<listbox>(&window, "recall_list", false);

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
				found = std::search(filter_options_[i].begin(),
							filter_options_[i].end(),
							word.begin(),
							word.end(),
							chars_equal_insensitive)
						!= filter_options_[i].end();

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);
}

} // namespace dialogs
} // namespace gui2
