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
#include "formatter.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "units/map.hpp"
#include "units/ptr.hpp"
#include "units/unit.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2
{

REGISTER_DIALOG(unit_list)

tunit_list::tunit_list(const display& gui)
	: unit_list_{std::make_shared<std::vector<unit_const_ptr> >()}
{
	const unit_map& units = gui.get_units();
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->side() != gui.viewing_side()) {
			continue;
		}

		unit_list_->push_back(i.get_shared_ptr());
	}

	//for(const unit_map& u : gui.get_units()) {
	//	unit_list_->push_back(u.get_shared_ptr());
	//}
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

template<typename Fcn>
void tunit_list::init_sorting_option(generator_sort_array& order_funcs, Fcn filter_on)
{
	order_funcs[0] = [this, filter_on](unsigned i1, unsigned i2) {
		return filter_on((*unit_list_)[i1]) < filter_on((*unit_list_)[i2]);
	};

	order_funcs[1] = [this, filter_on](unsigned i1, unsigned i2) {
		return filter_on((*unit_list_)[i1]) > filter_on((*unit_list_)[i2]);
	};
}

void tunit_list::pre_show(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "units_list", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*list,
			std::bind(&tunit_list::list_item_clicked,
				*this, std::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tunit_list, &tunit_list::list_item_clicked>);
#endif

	list.clear();

	window.keyboard_capture(&list);

	if(!unit_list_) return;

	for(const unit_const_ptr& unit : *unit_list_) {
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

		column["label"] = format_if_leader(unit, unit->type_name());
		row_data.insert(std::make_pair("unit_type", column));

		const std::string& name = !unit->name().empty() ? format_if_leader(unit, unit->name().str()) : utils::unicode_en_dash;
		column["label"] = name;
		row_data.insert(std::make_pair("unit_name", column));

		column["label"] = format_movement_string(unit);
		row_data.insert(std::make_pair("unit_moves", column));

		std::stringstream hp_str;
		hp_str << font::span_color(unit->hp_color()) << unit->hitpoints() << "/" << unit->max_hitpoints() << "</span>";

		column["label"] = hp_str.str();
		row_data.insert(std::make_pair("unit_hp", column));

		column["label"] = format_level_string(unit->level());
		row_data.insert(std::make_pair("unit_level", column));

		std::stringstream exp_str;
		exp_str << font::span_color(unit->xp_color()) << unit->experience() << "/"
		        << (unit->can_advance() ? std::to_string(unit->max_experience()) : utils::unicode_en_dash) << "</span>";

		column["label"] = exp_str.str();
		row_data.insert(std::make_pair("unit_experience", column));

		column["label"] = utils::join(unit->trait_names(), ", ");
		row_data.insert(std::make_pair("unit_traits", column));

		list.add_row(row_data);
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

void tunit_list::list_item_clicked(twindow& window)
{
	const int selected_row
		= find_widget<tlistbox>(&window, "units_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<tunit_preview_pane>(&window, "unit_details", false)
		.set_displayed_unit(unit_list_->at(selected_row).get());
}

void tunit_list::post_show(twindow& window)
{
	//if(get_retval() == twindow::OK) {
	//	selected_index_ = find_widget<tlistbox>(&window, "units_list", false)
	//		.get_selected_row();
	//}
}

}
