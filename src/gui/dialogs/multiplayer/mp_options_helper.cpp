/*
   Copyright (C) 2008 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/multiplayer/mp_options_helper.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

tmp_options_helper::tmp_options_helper(twindow& window, ng::create_engine& create_engine)
	: options_tree_(find_widget<ttree_view>(&window, "custom_options", false))
	, no_options_notice_(find_widget<tcontrol>(&window, "no_options_notice", false))
	, visible_options_()
	, options_data_()
	, create_engine_(create_engine)
{
	update_options_list();
}

void tmp_options_helper::update_options_list()
{
	visible_options_.clear();
	options_tree_.clear();

	display_custom_options(options_tree_,
		create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ? "campaign" : "multiplayer",
		create_engine_.current_level().data());

	display_custom_options(options_tree_, "era", create_engine_.curent_era_cfg());

	std::set<std::string> activemods(create_engine_.active_mods().begin(), create_engine_.active_mods().end());
	for(const auto& mod : create_engine_.get_const_extras_by_type(ng::create_engine::MP_EXTRA::MOD)) {
		if(activemods.find(mod->id) != activemods.end()) {
			display_custom_options(options_tree_, "modification", *mod->cfg);
		}
	}

	// No custom options, display a message
	no_options_notice_.set_visible(options_tree_.empty() ? twindow::tvisible::visible : twindow::tvisible::invisible);
}

template<typename T>
void tmp_options_helper::update_options_data_map(T* widget, const option_source& source)
{
	options_data_[source][widget->id()] = widget->get_value();
}

template<>
void tmp_options_helper::update_options_data_map(ttoggle_button* widget, const option_source& source)
{
	options_data_[source][widget->id()] = widget->get_value_bool();
}

void tmp_options_helper::reset_options_data(const option_source& source, bool& handled, bool& halt)
{
	options_data_[source].clear();
	update_options_list();

	handled = true;
	halt = true;
}

void tmp_options_helper::display_custom_options(ttree_view& tree, std::string&& type, const config& cfg)
{
	// Needed since some compilers don't like passing just {}
	static const std::map<std::string, string_map> empty_map;

	visible_options_.push_back({type, cfg["id"]});
	auto& data_map = options_data_[visible_options_.back()];

	auto set_default_data_value = [&](const std::string& widget_id, const config& cfg) {
		if(data_map.find(widget_id) == data_map.end() || data_map[widget_id].empty()) {
			data_map[widget_id] = cfg["default"];
		}
	};

	for(const auto& options : cfg.child_range("options")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = cfg["name"];
		data.emplace("tree_view_node_label", item);

		ttree_view_node& option_node = tree.add_node("option_node", data);

		for(const auto& checkbox_option : options.child_range("checkbox")) {
			data.clear();

			item["label"] = checkbox_option["name"];
			item["tooltip"] = checkbox_option["description"];
			data.emplace("option_checkbox", item);

			ttree_view_node& node = option_node.add_child("option_checkbox_node", data);

			ttoggle_button* checkbox = dynamic_cast<ttoggle_button*>(node.find("option_checkbox", true));

			VALIDATE(checkbox, missing_widget("option_checkbox"));

			const std::string widget_id = checkbox_option["id"];

			set_default_data_value(widget_id, checkbox_option);

			checkbox->set_id(widget_id);
			checkbox->set_value(data_map[widget_id].to_bool());
			checkbox->set_callback_state_change(
				std::bind(&tmp_options_helper::update_options_data_map<ttoggle_button>, this, checkbox, visible_options_.back()));
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("checkbox")) {
			option_node.add_child("options_spacer_node", empty_map);
		}

		for(const auto& menu_button_option : options.child_range("combo")) {
			data.clear();
			item.clear();

			item["label"] = menu_button_option["name"];
			data.emplace("menu_button_label", item);

			item["tooltip"] = menu_button_option["description"];
			data.emplace("option_menu_button", item);

			std::vector<config> combo_items;
			std::vector<std::string> combo_values;

			config::const_child_itors items = menu_button_option.child_range("item");
			for(auto item : items) {
				// Comboboxes expect this key to be 'label' not 'name'
				item["label"] = item["name"];

				combo_items.push_back(item);
				combo_values.push_back(item["value"]);
			}

			if(combo_items.empty()) {
				continue;
			}

			ttree_view_node& node = option_node.add_child("option_menu_button_node", data);

			tmenu_button* menu_button = dynamic_cast<tmenu_button*>(node.find("option_menu_button", true));

			VALIDATE(menu_button, missing_widget("option_menu_button"));

			const std::string widget_id = menu_button_option["id"];

			set_default_data_value(widget_id, menu_button_option);

			menu_button->set_id(widget_id);
			menu_button->set_values(combo_items);

			config::attribute_value val = data_map[widget_id];
			auto iter = std::find_if(items.begin(), items.end(), [&val](const config& cfg) {
				return cfg["value"] == val;
			});

			if(iter != items.end()) {
				menu_button->set_selected(iter - items.begin());
			}

			menu_button->connect_click_handler(
				std::bind(&tmp_options_helper::update_options_data_map<tmenu_button>, this, menu_button, visible_options_.back()));
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("combo")) {
			option_node.add_child("options_spacer_node", empty_map);
		}

		for(const auto& slider_option : options.child_range("slider")) {
			data.clear();
			item.clear();

			item["label"] = slider_option["name"];
			data.emplace("slider_label", item);

			item["tooltip"] = slider_option["description"];
			data.emplace("option_slider", item);

			ttree_view_node& node = option_node.add_child("option_slider_node", data);

			tslider* slider = dynamic_cast<tslider*>(node.find("option_slider", true));

			VALIDATE(slider, missing_widget("option_slider"));

			const std::string widget_id = slider_option["id"];

			set_default_data_value(widget_id, slider_option);

			slider->set_id(widget_id);
			slider->set_maximum_value(slider_option["max"].to_int());
			slider->set_minimum_value(slider_option["min"].to_int());
			slider->set_step_size(slider_option["step"].to_int(1));
			slider->set_value(data_map[widget_id].to_int());

			connect_signal_notify_modified(*slider,
				std::bind(&tmp_options_helper::update_options_data_map<tslider>, this, slider, visible_options_.back()));
		}

		// Only add a spacer if there were an option of this type
		if(options.has_child("slider")) {
			option_node.add_child("options_spacer_node", empty_map);
		}

		for(const auto& text_entry_option : options.child_range("entry")) {
			data.clear();
			item.clear();

			item["label"] = text_entry_option["name"];
			data.emplace("text_entry_label", item);

			item["tooltip"] = text_entry_option["description"];
			data.emplace("option_text_entry", item);

			ttree_view_node& node = option_node.add_child("option_text_entry_node", data);

			ttext_box* textbox = dynamic_cast<ttext_box*>(node.find("option_text_entry", true));

			VALIDATE(textbox, missing_widget("option_text_entry"));

			const std::string widget_id = text_entry_option["id"];

			set_default_data_value(widget_id, text_entry_option);

			textbox->set_id(widget_id);
			textbox->set_value(data_map[widget_id].str());
			textbox->set_text_changed_callback(
				std::bind(&tmp_options_helper::update_options_data_map<ttext_box>, this, textbox, visible_options_.back()));
		}

		// Add the Defaults button at the end
		ttree_view_node& node = option_node.add_child("options_default_button", empty_map);

		connect_signal_mouse_left_click(find_widget<tbutton>(&node, "reset_option_values", false),
			std::bind(&tmp_options_helper::reset_options_data, this, visible_options_.back(),
				std::placeholders::_3, std::placeholders::_4));
	}
}

} // namespace gui2
