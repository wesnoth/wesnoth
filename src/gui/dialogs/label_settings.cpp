/*
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

#include "label_settings.hpp"

#include <vector>
#include <boost/bind.hpp>
#include "gettext.hpp"
#include "game_display.hpp"
#include "map_label.hpp"
#include "resources.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/control.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/label.hpp"

namespace gui2 {
REGISTER_DIALOG(label_settings);

tlabel_settings::tlabel_settings(display_context& dc) : viewer(dc) {
	const std::vector<std::string>& all_categories = resources::screen->labels().all_categories();
	const std::vector<std::string>& hidden_categories = viewer.hidden_label_categories();
	
	for(size_t i = 0; i < all_categories.size(); i++) {
		all_labels[all_categories[i]] = true;
		if(all_categories[i].substr(0,4) == "cat:")
			labels_display[all_categories[i]] = all_categories[i].substr(4);
		else if(all_categories[i] == "team")
			labels_display[all_categories[i]] = _("Team Labels");
		// TODO: Translatable names for categories?
	}
	for(size_t i = 0; i < hidden_categories.size(); i++) {
		all_labels[hidden_categories[i]] = false;
	}
	for(size_t i = 0; i < dc.teams().size(); i++) {
		labels_display["side:" + str_cast(i + 1)] = dc.teams()[i].name();
	}
}

void tlabel_settings::pre_show(CVideo& /*video*/, twindow& window) {
	std::map<std::string, string_map> list_data;
	tlistbox& cats_listbox = find_widget<tlistbox>(&window, "label_types", false);
	FOREACH(const AUTO & label_entry, all_labels) {
		const std::string& category = label_entry.first;
		const bool& visible = label_entry.second;
		
		std::string name = labels_display[category];
		if(category.substr(0,5) == "side:") {
			int team = lexical_cast<int>(category.substr(5)) - 1;
			Uint32 which_color = game_config::tc_info(viewer.teams()[team].color())[0];
			std::ostringstream sout;
			sout << "<span color='#" << std::hex << which_color << "'>" << name << "</span>";
			name = sout.str();
		}
		
		list_data["cat_name"]["label"] = name;
		cats_listbox.add_row(list_data);
		
		tgrid* grid = cats_listbox.get_row_grid(cats_listbox.get_item_count() - 1);
		ttoggle_button& status = find_widget<ttoggle_button>(grid, "cat_status", false);
		status.set_value(visible);
		status.set_callback_state_change(boost::bind(&tlabel_settings::toggle_category, this, _1, category));
		
		if(category.substr(0,5) == "side:") {
			tlabel& label = find_widget<tlabel>(grid, "cat_name", false);
			label.set_use_markup(true);
		}
	}
}

bool tlabel_settings::execute(display_context& dc, CVideo& video) {
	tlabel_settings window(dc);
	if(!window.show(video)) return false;
	std::vector<std::string> hidden_categories;
	typedef std::map<std::string,bool>::value_type value_type;
	BOOST_FOREACH(value_type lbl, window.all_labels) {
		if(lbl.second == false) {
			hidden_categories.push_back(lbl.first);
		}
	}
	dc.hidden_label_categories_ref().swap(hidden_categories);
	return true;
}

void tlabel_settings::toggle_category(twidget& box, std::string category) {
	all_labels[category] = static_cast<ttoggle_button&>(box).get_value();
}
}