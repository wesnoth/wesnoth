/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/gui_definition.hpp"

#include "config.hpp"
#include "formatter.hpp"
#include "gui/core/log.hpp"
#include "gui/core/static_registry.hpp"
#include "gui/widgets/settings.hpp"
#include "serialization/chrono.hpp"
#include "wml_exception.hpp"

namespace gui2
{
gui_theme_map_t guis;
gui_theme_map_t::iterator current_gui = guis.end();
gui_theme_map_t::iterator default_gui = guis.end();

gui_definition::gui_definition(const config& cfg)
	: widget_types()
	, window_types()
	, id_(cfg["id"])
	, description_(cfg["description"].t_str())
	, popup_show_delay_(0)
	, popup_show_time_(0)
	, help_show_time_(0)
	, double_click_time_(0)
	, repeat_button_repeat_time_(0)
	, sound_button_click_()
	, sound_toggle_button_click_()
	, sound_toggle_panel_click_()
	, sound_slider_adjust_()
	, has_helptip_message_()
	, tips_(tip_of_the_day::load(cfg))
{
	VALIDATE(!id_.empty(), missing_mandatory_wml_key("gui", "id"));
	VALIDATE(!description_.empty(), missing_mandatory_wml_key("gui", "description"));

	DBG_GUI_P << "Parsing gui " << id_;

	//
	// Widget parsing
	//

	/** Parse widget definitions of each registered type. */
	for(const auto& [type_id, widget_parser] : registered_widget_types()) {
		auto& def_map = widget_types[type_id];

		const std::string key =	widget_parser.key
			? widget_parser.key
			: type_id + "_definition";

		bool found_default_def = false;

		for(const config& definition : cfg.child_range(key)) {
			// Run the static parser to get a definition ptr.
			styled_widget_definition_ptr def_ptr = widget_parser.parser(definition);

			const std::string& def_id = def_ptr->id;

			if(def_map.find(def_id) != def_map.end()) {
				ERR_GUI_P << "Skipping duplicate definition '" << def_id << "' for '" << type_id << "'";
				continue;
			}

			def_map.emplace(def_id, std::move(def_ptr));

			if(def_id == "default") {
				found_default_def = true;
			}
		}

		// Only the default GUI needs to ensure each widget has a default definition.
		// Non-default ones can just fall back to the default definition in the default GUI.
		if(id_ == "default") {
			VALIDATE(found_default_def, "No default definition found for widget '" + type_id + "'");
		}
	}

	//
	// Window parsing
	//

	/** Parse each window. */
	for(auto& w : cfg.child_range("window")) {
		window_types.emplace(w["id"], builder_window(w));
	}

	if(id_ == "default") {
		// The default gui needs to define all window types since we're the
		// fallback in case another gui doesn't define the window type.
		for(const auto& window_type : registered_window_types()) {
			const std::string error_msg(
				"Window not defined in WML: '" + window_type + "'."
				"Perhaps a mismatch between data and source versions. Try --data-dir <trunk-dir>");

			VALIDATE(window_types.find(window_type) != window_types.end(), error_msg);
		}
	}

	/***** settings *****/

	/**
	 * @todo Regarding sounds:
	 * Need to evaluate but probably we want the widget definition be able to:
	 * - Override the default (and clear it). This will allow toggle buttons in a
	 *   listbox to sound like a toggle panel.
	 * - Override the default and above per instance of the widget, some buttons
	 *   can give a different sound.
	 */
	const config& settings = cfg.mandatory_child("settings");

	using namespace std::chrono_literals;
	popup_show_delay_ = chrono::parse_duration(settings["popup_show_delay"], 0ms);
	popup_show_time_ = chrono::parse_duration(settings["popup_show_time"], 0ms);
	help_show_time_ = chrono::parse_duration(settings["help_show_time"], 0ms);
	double_click_time_ = chrono::parse_duration(settings["double_click_time"], 0ms);

	repeat_button_repeat_time_ = chrono::parse_duration(settings["repeat_button_repeat_time"], 0ms);

	VALIDATE(!settings["double_click_time"].blank(), missing_mandatory_wml_key("settings", "double_click_time"));

	sound_button_click_ = settings["sound_button_click"].str();
	sound_toggle_button_click_ = settings["sound_toggle_button_click"].str();
	sound_toggle_panel_click_ = settings["sound_toggle_panel_click"].str();
	sound_slider_adjust_ = settings["sound_slider_adjust"].str();

	has_helptip_message_ = settings["has_helptip_message"].t_str();

	VALIDATE(!has_helptip_message_.empty(), missing_mandatory_wml_key("settings", "has_helptip_message"));
}

void gui_definition::activate() const
{
	settings::popup_show_delay = popup_show_delay_;
	settings::popup_show_time = popup_show_time_;
	settings::help_show_time = help_show_time_;
	settings::double_click_time = double_click_time_;
	settings::repeat_button_repeat_time = repeat_button_repeat_time_;
	settings::sound_button_click = sound_button_click_;
	settings::sound_toggle_button_click = sound_toggle_button_click_;
	settings::sound_toggle_panel_click = sound_toggle_panel_click_;
	settings::sound_slider_adjust = sound_slider_adjust_;
	settings::has_helptip_message = has_helptip_message_;
	settings::tips = tips_;
}

namespace
{
template<typename TList, typename TConv>
const typename TList::value_type& get_best_resolution(const TList& list, const TConv& get_size)
{
	using resolution_t = const typename TList::value_type;

	resolution_t* best_resolution = nullptr;
	int best_resolution_score = std::numeric_limits<int>::min();

	const int screen_w = settings::screen_width;
	const int screen_h = settings::screen_height;

	for(const auto& res : list) {
		point size = get_size(res);

		int w = size.x ? size.x : 1;
		int h = size.y ? size.y : 1;
		int score = 0;

		if(w <= screen_w && h <= screen_h) {
			score = w * h;
		} else {
			// Negative score, only used in case none of the given resolution fits on the screen
			// (workaround for a bug where the windows size can become < 800x600).
			score = std::min(screen_w - w, 0) + std::min(screen_h - h, 0);
		}

		if(score >= best_resolution_score) {
			best_resolution = &res;
			best_resolution_score = score;
		}
	}

	assert(best_resolution != nullptr);
	return *best_resolution;
}

} // namespace

resolution_definition_ptr get_control(const std::string& control_type, const std::string& definition)
{
	const auto& current_types = current_gui->second.widget_types;
	const auto& default_types = default_gui->second.widget_types;

	const auto widget_definitions = current_types.find(control_type);

	gui_definition::widget_definition_map_t::const_iterator control;

	if(widget_definitions == current_types.end()) {
		goto fallback;
	}

	control = widget_definitions->second.find(definition);

	if(control == widget_definitions->second.end()) {
	fallback:
		bool found_fallback = false;

		if(current_gui != default_gui) {
			auto default_widget_definitions = default_types.find(control_type);

			VALIDATE(widget_definitions != current_types.end(),
				formatter() << "Type '" << control_type << "' is unknown.");

			control = default_widget_definitions->second.find(definition);
			found_fallback = control != default_widget_definitions->second.end();
		}

		if(!found_fallback) {
			if(definition != "default") {
				LOG_GUI_G << "Control: type '" << control_type << "' definition '" << definition
						  << "' not found, falling back to 'default'.";
				return get_control(control_type, "default");
			}

			FAIL(formatter() << "default definition not found for styled_widget " << control_type);
		}
	}

	const auto& resolutions = (*control->second).resolutions;

	VALIDATE(!resolutions.empty(),
		formatter() << "Control: type '" << control_type << "' definition '" << definition << "' has no resolutions.\n");

	return get_best_resolution(resolutions, [&](const resolution_definition_ptr& ptr) {
		return point(
			static_cast<int>(ptr->window_width),
			static_cast<int>(ptr->window_height)
		);
	});
}

const builder_window::window_resolution& get_window_builder(const std::string& type)
{
	settings::update_screen_size_variables();

	const auto& current_windows = current_gui->second.window_types;
	const auto& default_windows = default_gui->second.window_types;

	auto iter = current_windows.find(type);

	if(iter == current_windows.end()) {
		// Current GUI is the default one and no window type was found. Throw.
		if(current_gui == default_gui) {
			throw window_builder_invalid_id();
		}

		// Else, try again to find the window, this time in the default GUI.
		iter = default_windows.find(type);

		if(iter == default_windows.end()) {
			throw window_builder_invalid_id();
		}
	}

	const auto& resolutions = iter->second.resolutions;

	VALIDATE(!resolutions.empty(), formatter() << "Window '" << type << "' has no resolutions.\n");

	return get_best_resolution(resolutions, [&](const builder_window::window_resolution& res) {
		return point(
			static_cast<int>(res.window_width),
			static_cast<int>(res.window_height)
		);
	});
}

bool add_single_widget_definition(const std::string& widget_type, const std::string& definition_id, const config& cfg)
{
	auto& def_map = current_gui->second.widget_types[widget_type];
	auto parser = registered_widget_types().find(widget_type);

	if(parser == registered_widget_types().end()) {
		throw std::invalid_argument("widget '" + widget_type + "' doesn't exist");
	}

	if(def_map.find(definition_id) != def_map.end()) {
		return false;
	}

	def_map.emplace(definition_id, parser->second.parser(cfg));
	return true;
}

void remove_single_widget_definition(const std::string& widget_type, const std::string& definition_id)
{
	auto& definition_map = current_gui->second.widget_types[widget_type];

	auto it = definition_map.find(definition_id);
	if(it != definition_map.end()) {
		definition_map.erase(it);
	}
}

} // namespace gui2
