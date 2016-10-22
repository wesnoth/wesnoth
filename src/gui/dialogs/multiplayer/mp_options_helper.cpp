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

#include "config_assign.hpp"
#include "game_preferences.hpp"
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
	for(const auto& c : preferences::options().all_children_range()) {
		for(const auto& saved_option : c.cfg.child_range("option")) {
			const std::string& id = saved_option["id"];

			options_data_[{c.key, id}][id] = saved_option["value"];
		}
	}

	update_options_list();
}

void tmp_options_helper::update_options_list()
{
	visible_options_.clear();
	options_tree_.clear();

	display_custom_options(
		create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ? "campaign" : "multiplayer",
		create_engine_.current_level().data());

	display_custom_options("era", create_engine_.curent_era_cfg());

	for(const auto& mod : create_engine_.active_mods_data()) {
		display_custom_options("modification", *mod->cfg);
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

template<typename T>
std::pair<T*, config::attribute_value> tmp_options_helper::add_node_and_get_widget(
		ttree_view_node& option_node, const std::string& id, data_map& data, const config& cfg)
{
	ttree_view_node& node = option_node.add_child(id + "_node", data);

	T* widget = dynamic_cast<T*>(node.find(id, true));
	VALIDATE(widget, missing_widget(id));

	const std::string widget_id = cfg["id"];

	auto& data_map = options_data_[visible_options_.back()];
	if(data_map.find(widget_id) == data_map.end() || data_map[widget_id].empty()) {
		data_map[widget_id] = cfg["default"];
	}

	widget->set_id(widget_id);

	return {widget, data_map[widget_id]};
}

void tmp_options_helper::display_custom_options(std::string&& type, const config& cfg)
{
	// Needed since some compilers don't like passing just {}
	static const std::map<std::string, string_map> empty_map;

	// This ensures that any game, era, or mod with no options doesn't get an entry in the visible_options_
	// vector and prevents invalid options from different games, era, or mods being created when the options
	// config is created.
	if(!cfg.has_child("options")) {
		return;
	}

	visible_options_.push_back({type, cfg["id"]});

	for(const auto& options : cfg.child_range("options")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = cfg["name"];
		data.emplace("tree_view_node_label", item);

		ttree_view_node& option_node = options_tree_.add_node("option_node", data);

		for(const config::any_child opt : options.all_children_range()) {
			data.clear();

			if(opt.key == "checkbox") {
				const config& checkbox_option = opt.cfg;

				item["label"] = checkbox_option["name"];
				item["tooltip"] = checkbox_option["description"];
				data.emplace("option_checkbox", item);

				ttoggle_button* checkbox;
				config::attribute_value val;

				std::tie(checkbox, val) = add_node_and_get_widget<ttoggle_button>(option_node, "option_checkbox", data, checkbox_option);

				checkbox->set_value(val.to_bool());
				checkbox->set_callback_state_change(
					std::bind(&tmp_options_helper::update_options_data_map<ttoggle_button>, this, checkbox, visible_options_.back()));
			} else if(opt.key == "spacer") {
				option_node.add_child("options_spacer_node", empty_map);
			} else if(opt.key == "choice" || opt.key == "combo") {
				if(opt.key == "combo") {
					lg::wml_error() << "[options][combo] is deprecated; use [choice] instead\n";
				}

				const config& menu_button_option = opt.cfg;

				data.clear();
				item.clear();

				item["label"] = menu_button_option["name"];
				data.emplace("menu_button_label", item);

				item["tooltip"] = menu_button_option["description"];
				data.emplace("option_menu_button", item);

				std::vector<config> combo_items;
				std::vector<std::string> combo_values;

				config::const_child_itors items = menu_button_option.child_range("item");
				for(auto i : items) {
					// Comboboxes expect this key to be 'label' not 'name'
					i["label"] = i["name"];

					combo_items.push_back(i);
					combo_values.push_back(i["value"]);
				}

				if(combo_items.empty()) {
					continue;
				}

				tmenu_button* menu_button;
				config::attribute_value val;

				std::tie(menu_button, val) = add_node_and_get_widget<tmenu_button>(option_node, "option_menu_button", data, menu_button_option);

				auto iter = std::find_if(items.begin(), items.end(), [&val](const config& cfg) {
					return cfg["value"] == val;
				});

				if(iter != items.end()) {
					menu_button->set_selected(iter - items.begin());
				}

				menu_button->set_values(combo_items);
				menu_button->connect_click_handler(
					std::bind(&tmp_options_helper::update_options_data_map<tmenu_button>, this, menu_button, visible_options_.back()));
			} else if(opt.key == "slider") {
				const config& slider_option = opt.cfg;

				data.clear();
				item.clear();

				item["label"] = slider_option["name"];
				data.emplace("slider_label", item);

				item["tooltip"] = slider_option["description"];
				data.emplace("option_slider", item);

				tslider* slider;
				config::attribute_value val;

				std::tie(slider, val) = add_node_and_get_widget<tslider>(option_node, "option_slider", data, slider_option);

				slider->set_maximum_value(slider_option["max"].to_int());
				slider->set_minimum_value(slider_option["min"].to_int());
				slider->set_step_size(slider_option["step"].to_int(1));
				slider->set_value(val.to_int());

				connect_signal_notify_modified(*slider,
					std::bind(&tmp_options_helper::update_options_data_map<tslider>, this, slider, visible_options_.back()));
			} else if(opt.key == "entry") {
				const config& text_entry_option = opt.cfg;

				data.clear();
				item.clear();

				item["label"] = text_entry_option["name"];
				data.emplace("text_entry_label", item);

				item["tooltip"] = text_entry_option["description"];
				data.emplace("option_text_entry", item);

				ttext_box* textbox;
				config::attribute_value val;

				std::tie(textbox, val) = add_node_and_get_widget<ttext_box>(option_node, "option_text_entry", data, text_entry_option);

				textbox->set_value(val.str());
				textbox->set_text_changed_callback(
					std::bind(&tmp_options_helper::update_options_data_map<ttext_box>, this, textbox, visible_options_.back()));
			}
		}

		// Add the Defaults button at the end
		ttree_view_node& node = option_node.add_child("options_default_button", empty_map);

		connect_signal_mouse_left_click(find_widget<tbutton>(&node, "reset_option_values", false),
			std::bind(&tmp_options_helper::reset_options_data, this, visible_options_.back(),
				std::placeholders::_3, std::placeholders::_4));
	}
}

const config tmp_options_helper::get_options_config()
{
	config options;
	for(const auto& source : visible_options_) {
		config& mod = options.add_child(source.level_type);
		mod["id"] = source.id;
		for(const auto& option : options_data_[source]) {
			// TODO: change this to some key=value format as soon as we drop the old mp configure screen.
			mod.add_child("option", config_of("id", option.first)("value", option.second));
		}
	}

	return options;
}

} // namespace gui2
