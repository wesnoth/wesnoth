/*
	Copyright (C) 2010 - 2025
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

#include "gui/dialogs/editor/generator_settings.hpp"

#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/status_label_helper.hpp"
#include "gettext.hpp"
#include "formula/string_utils.hpp"

#include <functional>

namespace gui2::dialogs
{
namespace
{
constexpr int max_coastal = 5;
constexpr int extra_size_per_player = 2;
constexpr int min_size = 20;

std::string get_village_description(int count)
{
	return VGETTEXT("$count/1000 tiles", {{"count", std::to_string(count)}});
}

std::string get_landform_description(int count)
{
	return count == 0 ? _("Inland") : (count < max_coastal ? _("Coastal") : _("Island"));
}

} // namespace

REGISTER_DIALOG(generator_settings)

generator_settings::generator_settings(generator_data& data)
	: modal_dialog(window_id())
	, players_(register_integer("players", true, data.nplayers))
	, width_(register_integer("width",     true, data.width))
	, height_(register_integer("height",   true, data.height))
{
	register_integer("hills_num",    true, data.iterations);
	register_integer("hills_size",   true, data.hill_size);
	register_integer("villages",     true, data.nvillages);
	register_integer("castle_size",  true, data.castle_size);
	register_integer("landform",     true, data.island_size);

	register_bool("connect_castles", true, data.link_castles);
	register_bool("show_labels",     true, data.show_labels);
}

void generator_settings::pre_show()
{
	// We adjust the minimum values of the width and height sliders when the number of players changes.
	// This is done because the map generator needs more space to generate more castles for more players.
	connect_signal_notify_modified(*players_->get_widget(), std::bind(
		&generator_settings::adjust_minimum_size_by_players, this));

	gui2::bind_default_status_label(find_widget<slider>("players"));
	gui2::bind_default_status_label(find_widget<slider>("castle_size"));
	gui2::bind_default_status_label(find_widget<slider>("width"));
	gui2::bind_default_status_label(find_widget<slider>("height"));

	gui2::bind_status_label(find_widget<slider>("villages"),
		[](const slider& s) { return get_village_description(s.get_value()); });
	gui2::bind_status_label(find_widget<slider>("landform"),
		[](const slider& s) { return get_landform_description(s.get_value()); });

	// Update min size initially.
	adjust_minimum_size_by_players();
}

void generator_settings::adjust_minimum_size_by_players()
{
	const int extra_size = (players_->get_widget_value() - 2) * extra_size_per_player;

	const auto update_dimension_slider = [&](field_integer* field) {
		slider& w = dynamic_cast<slider&>(*field->get_widget());
		w.set_value_range(min_size + extra_size, w.get_maximum_value());
	};

	update_dimension_slider(width_);
	update_dimension_slider(height_);
}

} // namespace dialogs
