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

#include "gui/widgets/multimenu_button.hpp"

#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"

#include "formula/string_utils.hpp"
#include <functional>
#include "gettext.hpp"
#include "wml_exception.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multimenu_button)

multimenu_button::multimenu_button(const implementation::builder_multimenu_button& builder)
	: options_button(builder, type())
	, max_shown_(1)
{
}


void multimenu_button::update_label()
{
	std::vector<t_string> selected;

	for(std::size_t i = 0; i < values_.size(); i++) {
		assert(values_[i].checkbox);
		if(!values_[i].checkbox.value()) {
			continue;
		}
		selected.push_back(values_[i].label);
	}

	if(selected.empty()) {
		set_label(_("multimenu^None Selected"));
	} else if(selected.size() == values_.size()) {
		set_label(_("multimenu^All Selected"));
	} else {
		if(selected.size() > max_shown_) {
			const unsigned excess = selected.size() - max_shown_;
			selected.resize(max_shown_ + 1);
			// TRANSLATORS: In a drop-down menu that's a list of toggle-boxes, this becomes part
			// of the text on the button when many of the boxes are selected. The text becomes
			// "x, y and 1 other", "x, y and 2 others", etc.
			selected.back() = VNGETTEXT("multimenu^$excess other", "$excess others", excess, {{"excess", std::to_string(excess)}});
		}
		set_label(utils::format_conjunct_list(_("multimenu^None Selected"), selected));
	}
}

void multimenu_button::select_option(const unsigned option, const bool selected)
{
	assert(option < values_.size());
	toggle_states_[option] = selected;
	update_config_from_toggle_states();
	update_label();
}

void multimenu_button::select_options(boost::dynamic_bitset<> states)
{
	assert(states.size() == values_.size());
	toggle_states_ = states;
	update_config_from_toggle_states();
	update_label();
}

menu_item& multimenu_button::add_row(config row, const int index)
{
	if(!row.has_attribute("checkbox")){
		row["checkbox"] = "false";
	}

	return options_button::add_row(row, index);
}


// }---------- DEFINITION ---------{

multimenu_button_definition::multimenu_button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing multimenu_button " << id;

	load_resolutions<resolution>(cfg);
}

multimenu_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in options_button.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_pressed", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_pressed")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_focused")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_multimenu_button::builder_multimenu_button(const config& cfg)
	: builder_styled_widget(cfg)
	, max_shown_(cfg["maximum_shown"].to_unsigned(1))
	, options_()
{
	for(auto option : cfg.child_range("option")) {
		if(!option.has_attribute("checkbox")) {
			LOG_GUI_G << "Missing checkbox attribute for multimenu_button menu_item, adding with value=false.";
			option["checkbox"] = "false";
		}
		options_.push_back(option);
	}
}

std::unique_ptr<widget> builder_multimenu_button::build() const
{
	auto widget = std::make_unique<multimenu_button>(*this);

	widget->set_max_shown(max_shown_);
	if(!options_.empty()) {
		widget->set_values(options_);
	}

	widget->update_label();

	DBG_GUI_G << "Window builder: placed multimenu_button '" << id
	          << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
