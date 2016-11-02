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
	: create_engine_(create_engine)
	, options_tree_(find_widget<ttree_view>(&window, "custom_options", false))
	, no_options_notice_(find_widget<tcontrol>(&window, "no_options_notice", false))
	, last_game_pos_(-1)
	, last_era_pos_(-1)
	, last_mod_pos_(-1)
	, node_map_()
	, visible_options_()
	, options_data_()
{
	for(const auto& c : preferences::options().all_children_range()) {
		for(const auto& saved_option : c.cfg.child_range("option")) {
			options_data_[c.cfg["id"]][saved_option["id"]] = saved_option["value"];
		}
	}

	update_all_options();
}

void tmp_options_helper::update_all_options()
{
	visible_options_.clear();
	node_map_.clear();

	update_game_options();
	update_era_options();
	update_mod_options();
}

void tmp_options_helper::update_game_options()
{
	const std::string type = create_engine_.current_level_type() == ng::level::TYPE::CAMPAIGN ? "campaign" : "multiplayer";

	int pos = remove_nodes_for_type(type, last_game_pos_);
	pos = std::max(pos, last_game_pos_);

	display_custom_options(type, pos, create_engine_.current_level().data());

	update_status_label();
}

void tmp_options_helper::update_era_options()
{
	static const std::string type = "era";

	int pos = remove_nodes_for_type(type, last_era_pos_);
	pos = std::max(pos, last_era_pos_);

	display_custom_options(type, pos, create_engine_.curent_era_cfg());

	update_status_label();
}

void tmp_options_helper::update_mod_options()
{
	static const std::string type = "modification";

	int pos = remove_nodes_for_type(type, last_mod_pos_);
	pos = std::max(pos, last_mod_pos_);

	for(const auto& mod : create_engine_.active_mods_data()) {
		display_custom_options(type, pos, *mod->cfg);
	}

	update_status_label();
}

int tmp_options_helper::remove_nodes_for_type(const std::string& type, int& saved_pos)
{
	// Remove all visible options of the specified source type
	if(!visible_options_.empty()) {
		auto vo_iter = std::remove_if(visible_options_.begin(), visible_options_.end(), [&type](const option_source& source) {
			return source.level_type == type;
		});

		visible_options_.erase(vo_iter, visible_options_.end());
	}

	// Get the node vector for this specific source type
	node_vector* type_node_vector;

	auto node_map_iter = node_map_.end();
	std::tie(node_map_iter, std::ignore) = node_map_.emplace(type, node_vector());

	type_node_vector = &node_map_iter->second;

	// The position to insert a new node of this type. If no nodes exist yet, this default value is
	// accepted by ttree_view_node as meaning at-end.
	int position = -1;

	// Remove each node in reverse, so that in the end we have the position of the first node removed
	if(!type_node_vector->empty()) {
		for(auto i = type_node_vector->rbegin(); i != type_node_vector->rend(); i++) {
			saved_pos = position = options_tree_.remove_node(*i);
		}

		type_node_vector->clear();
	}

	return position;
}

void tmp_options_helper::update_status_label()
{
	// No custom options, display a message
	no_options_notice_.set_visible(options_tree_.empty() ? twindow::tvisible::visible : twindow::tvisible::invisible);
}

template<typename T>
void tmp_options_helper::update_options_data_map(T* widget, const option_source& source)
{
	options_data_[source.id][widget->id()] = widget->get_value();
}

template<>
void tmp_options_helper::update_options_data_map(ttoggle_button* widget, const option_source& source)
{
	options_data_[source.id][widget->id()] = widget->get_value_bool();
}

void tmp_options_helper::update_options_data_map_menu_button(tmenu_button* widget, const option_source& source, const config& cfg)
{
	options_data_[source.id][widget->id()] = cfg.child_range("item")[widget->get_value()]["value"].str();
}

void tmp_options_helper::reset_options_data(const option_source& source, bool& handled, bool& halt)
{
	options_data_[source.id].clear();

	if(source.level_type == "campaign" || source.level_type == "multiplayer") {
		update_game_options();
	} else if(source.level_type == "era") {
		update_era_options();
	} else if(source.level_type == "modification") {
		update_mod_options();
	}

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

	auto& option_config = options_data_[visible_options_.back().id];
	if(!option_config.has_attribute(widget_id) || option_config[widget_id].empty()) {
		option_config[widget_id] = cfg["default"];
	}

	widget->set_id(widget_id);
	widget->set_tooltip(cfg["description"]);

	return {widget, option_config[widget_id]};
}

void tmp_options_helper::display_custom_options(const std::string& type, int node_position, const config& cfg)
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

	// Get the node vector for this specific source type
	node_vector& type_node_vector = node_map_[type];

	for(const auto& options : cfg.child_range("options")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = cfg["name"];
		data.emplace("tree_view_node_label", item);

		ttree_view_node& option_node = options_tree_.add_node("option_node", data, node_position);
		type_node_vector.push_back(&option_node);

		for(const config::any_child opt : options.all_children_range()) {
			data.clear();
			item.clear();

			const config& option_cfg = opt.cfg;

			const auto add_name = [&](const std::string& id) {
				item["label"] = option_cfg["name"];
				data.emplace(id, item);
			};

			config::attribute_value val;

			if(opt.key == "checkbox") {
				add_name("option_checkbox");

				ttoggle_button* checkbox;
				std::tie(checkbox, val) = add_node_and_get_widget<ttoggle_button>(option_node, "option_checkbox", data, option_cfg);

				checkbox->set_value(val.to_bool());
				checkbox->set_callback_state_change(
					std::bind(&tmp_options_helper::update_options_data_map<ttoggle_button>, this, checkbox, visible_options_.back()));

			} else if(opt.key == "spacer") {
				option_node.add_child("options_spacer_node", empty_map);

			} else if(opt.key == "choice" || opt.key == "combo") {
				if(opt.key == "combo") {
					lg::wml_error() << "[options][combo] is deprecated; use [choice] instead\n";
				}

				if(!option_cfg.has_child("item")) {
					continue;
				}

				add_name("menu_button_label");

				std::vector<config> combo_items;
				std::vector<std::string> combo_values;

				for(auto i : option_cfg.child_range("item")) {
					// Comboboxes expect this key to be 'label' not 'name'
					i["label"] = i["name"];

					combo_items.push_back(i);
					combo_values.push_back(i["value"]);
				}

				tmenu_button* menu_button;
				std::tie(menu_button, val) = add_node_and_get_widget<tmenu_button>(option_node, "option_menu_button", data, option_cfg);

				// Needs to be called before set_selected
				menu_button->set_values(combo_items);

				auto iter = std::find(combo_values.begin(), combo_values.end(), val.str());

				if(iter != combo_values.end()) {
					menu_button->set_selected(iter - combo_values.begin());
				}

				menu_button->connect_click_handler(
					std::bind(&tmp_options_helper::update_options_data_map_menu_button, this, menu_button, visible_options_.back(), option_cfg));

			} else if(opt.key == "slider") {
				add_name("slider_label");

				tslider* slider;
				std::tie(slider, val) = add_node_and_get_widget<tslider>(option_node, "option_slider", data, option_cfg);

				slider->set_maximum_value(option_cfg["max"].to_int());
				slider->set_minimum_value(option_cfg["min"].to_int());
				slider->set_step_size(option_cfg["step"].to_int(1));
				slider->set_value(val.to_int());

				connect_signal_notify_modified(*slider,
					std::bind(&tmp_options_helper::update_options_data_map<tslider>, this, slider, visible_options_.back()));

			} else if(opt.key == "entry") {
				add_name("text_entry_label");

				ttext_box* textbox;
				std::tie(textbox, val) = add_node_and_get_widget<ttext_box>(option_node, "option_text_entry", data, option_cfg);

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

config tmp_options_helper::get_options_config()
{
	config options;
	for(const auto& source : visible_options_) {
		config& mod = options.add_child(source.level_type);
		mod["id"] = source.id;
#if 0
		// TODO: enable this as soon as we drop the old mp configure screen.
		mod.add_child("options", options_data_[source.id]);
#else
		for(const auto& option : options_data_[source.id].attribute_range()) {
			mod.add_child("option", config_of("id", option.first)("value", option.second));
		}
#endif
	}

	return options;
}

} // namespace gui2
