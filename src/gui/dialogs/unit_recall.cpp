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

#include "gui/dialogs/unit_recall.hpp"

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
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "marked-up_text.hpp"
#include "help/help.hpp"
#include "gettext.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2
{

REGISTER_DIALOG(unit_recall)

tunit_recall::tunit_recall(recalls_ptr_vector& recall_list, team& team)
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
	} else if(level > 2 ) {
		return"<b><span color='#ffffff'>" + lvl + "</span></b>";
	}

	return lvl;
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

template<typename Fnc>
void tunit_recall::init_sorting_option(generator_sort_array& order_funcs, Fnc filter_on)
{
	order_funcs[0] = [this, filter_on](unsigned i1, unsigned i2) {
		return filter_on((*recall_list_)[i1]) < filter_on((*recall_list_)[i2]);
	};

	order_funcs[1] = [this, filter_on](unsigned i1, unsigned i2) {
		return filter_on((*recall_list_)[i1]) > filter_on((*recall_list_)[i2]);
	};
}

static std::string get_title_suffix(int side_num)
{
	if(!resources::teams || !resources::units) {
		return "";
	}

	unit_map& units = *resources::units;

	int controlled_recruiters = 0;
	for(const auto& team : *resources::teams) {
		if(team.is_local_human() && !team.recruits().empty() && units.find_leader(team.side()) !=units.end()) {
			++controlled_recruiters;
		}
	}

	std::stringstream msg;
	if(controlled_recruiters >= 2) {
		unit_map::const_iterator leader = resources::units->find_leader(side_num);
		if(leader != resources::units->end() && !leader->name().empty()) {
			msg << " (" << leader->name(); msg << ")";
		}
	}

	return msg.str();
}

void tunit_recall::pre_show(twindow& window)
{
	tlabel& title = find_widget<tlabel>(&window, "title", true);
	title.set_label(title.label() + get_title_suffix(team_.side()));

	ttext_box* filter
			= find_widget<ttext_box>(&window, "filter_box", false, true);

	filter->set_text_changed_callback(
			std::bind(&tunit_recall::filter_text_changed, this, _1, _2));

	tlistbox& list = find_widget<tlistbox>(&window, "recall_list", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*list,
			std::bind(&tunit_recall::list_item_clicked,
				*this, std::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tunit_recall, &tunit_recall::list_item_clicked>);
#endif

	list.clear();

	window.add_to_keyboard_chain(filter);
	window.add_to_keyboard_chain(&list);

	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "dismiss", false),
		std::bind(&tunit_recall::dismiss_unit, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "show_help", false),
		std::bind(&tunit_recall::show_help, this, std::ref(window)));

	for(const unit_const_ptr& unit : *recall_list_) {
		std::map<std::string, string_map> row_data;
		string_map column;

		std::string mods
			= "~RC(" + unit->team_color() + ">" + team::get_side_color_index(unit->side()) + ")";

		if(unit->can_recruit()) {
			mods += "~BLIT(" + unit::leader_crown() + ")";
		}

		for(const std::string& overlay : unit->overlays()) {
			mods += "~BLIT(" + overlay + ")";
		}

		column["label"] = unit->absolute_image() + mods;
		row_data.insert(std::make_pair("unit_image", column));

		column["label"] = unit->type_name();
		row_data.insert(std::make_pair("unit_type", column));

		column["label"] = format_cost_string(unit->recall_cost(), team_.recall_cost());
		column["use_markup"] = "true";
		row_data.insert(std::make_pair("unit_recall_cost", column));

		const std::string& name = !unit->name().empty() ? unit->name().str() : utils::unicode_en_dash;
		column["label"] = name;
		row_data.insert(std::make_pair("unit_name", column));

		column["label"] = format_level_string(unit->level());
		row_data.insert(std::make_pair("unit_level", column));

		std::stringstream exp_str;
		exp_str << font::span_color(unit->xp_color()) << unit->experience() << "/"
		        << (unit->can_advance() ? std::to_string(unit->max_experience()) : utils::unicode_en_dash) << "</span>";

		column["label"] = exp_str.str();
		row_data.insert(std::make_pair("unit_experience", column));

		// Since the table widgets use heavy formatting, we save a bare copy
		// of certain options to filter on.
		std::string filter_text = unit->type_name() + " " + name + " " + std::to_string(unit->level());

		std::string traits;
		for(const std::string& trait : unit->trait_names()) {
			traits += (traits.empty() ? "" : "\n") + trait;
			filter_text += " " + trait;
		}

		column["label"] = !traits.empty() ? traits : utils::unicode_en_dash;
		row_data.insert(std::make_pair("unit_traits", column));

		list.add_row(row_data);
		filter_options_.push_back(filter_text);
	}

	generator_sort_array order_funcs;

	init_sorting_option(order_funcs, [](unit_const_ptr u) { return u.get()->type_name().str(); });
	list.set_column_order(0, order_funcs);

	init_sorting_option(order_funcs, [](unit_const_ptr u) { return u.get()->name().str(); });
	list.set_column_order(1, order_funcs);

	init_sorting_option(order_funcs, [](unit_const_ptr u) { return u.get()->level(); });
	list.set_column_order(2, order_funcs);

	init_sorting_option(order_funcs, [](unit_const_ptr u) { return u.get()->experience(); });
	list.set_column_order(3, order_funcs);

	init_sorting_option(order_funcs, [](unit_const_ptr u) { 
		return !u.get()->trait_names().empty() ? u.get()->trait_names().front().str() : "";
	});

	list.set_column_order(4, order_funcs);

	list_item_clicked(window);
}

void tunit_recall::dismiss_unit(twindow& window)
{
	LOG_DP << "Recall list units:\n"; dump_recall_list_to_console(*recall_list_);

	tlistbox& list = find_widget<tlistbox>(&window, "recall_list", false);
	const int index = list.get_selected_row();

	const unit& u = *recall_list_->at(index);

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
		const int res = gui2::show_message(window.video(), _("Dismiss Unit"), message.str(), gui2::tmessage::yes_no_buttons);

		if(res != gui2::twindow::OK) {
			return;
		}
	}

	*recall_list_->erase(recall_list_->begin() + index);

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
		window.set_retval(twindow::CANCEL);
	}
}

void tunit_recall::show_help(twindow& window)
{
	help::show_help(window.video(), "recruit_and_recall");
}

void tunit_recall::list_item_clicked(twindow& window)
{
	const int selected_row
		= find_widget<tlistbox>(&window, "recall_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<tunit_preview_pane>(&window, "unit_details", false)
		.set_displayed_unit(recall_list_->at(selected_row).get());
}

void tunit_recall::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		selected_index_ = find_widget<tlistbox>(&window, "recall_list", false)
			.get_selected_row();
	}
}

void tunit_recall::filter_text_changed(ttext_* textbox, const std::string& text)
{
	twindow& window = *textbox->get_window();

	tlistbox& list = find_widget<tlistbox>(&window, "recall_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return;
	last_words_ = words;

	std::vector<bool> show_items(list.get_item_count(), true);

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

}
