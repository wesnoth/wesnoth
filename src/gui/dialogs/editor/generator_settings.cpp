/*
   Copyright (C) 2010 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gettext.hpp"

#include "utils/functional.hpp"

namespace gui2
{

static int max_coastal = 5;
static int extra_size_per_player = 2;
static int min_size = 20;

REGISTER_DIALOG(generator_settings)

tgenerator_settings::tgenerator_settings(generator_data& data)
	: players_(register_integer("players", true, data.nplayers))
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

void tgenerator_settings::pre_show(twindow& window)
{
	// We adjust the minimum values of the width and height sliders when the number of players changes.
	// This is done because the map generator needs more space to generate more castles for more players.
	connect_signal_notify_modified(*players_->widget(), std::bind(&adjust_minimum_size_by_players, this, std::ref(window)));

	bind_status_label(window, "players");
	bind_status_label(window, "width");
	bind_status_label(window, "height");
	bind_status_label(window, "villages", _("/1000 tiles"));
	bind_status_label(window, "castle_size");
	bind_landform_status_label(window);
}

void tgenerator_settings::bind_status_label(twindow& window, const std::string& id, const std::string& suffix)
{
    tslider& slider = find_widget<tslider>(&window, id, false);
	tlabel& label = find_widget<tlabel>(&window, id + "_label", false);

	label.set_label(std::to_string(slider.get_value()) + suffix);

	connect_signal_notify_modified(slider, std::bind(&status_label_callback, this, std::ref(slider), std::ref(label), suffix));
}

void tgenerator_settings::status_label_callback(tslider& slider, tlabel& label, const std::string& suffix)
{
	label.set_label(std::to_string(slider.get_value()) + suffix);
}

// TODO: remove
void tgenerator_settings::bind_landform_status_label(twindow& window)
{
    tslider& slider = find_widget<tslider>(&window, "landform", false);
	tlabel& label = find_widget<tlabel>(&window, "landform_label", false);

	landform_status_label_callback(slider, label);

	connect_signal_notify_modified(slider, std::bind(&landform_status_label_callback, this, std::ref(slider), std::ref(label)));
}

// TODO: remove
void tgenerator_settings::landform_status_label_callback(tslider& slider, tlabel& label)
{
	label.set_label(slider.get_value() == 0 ? _("Inland") : (slider.get_value() < max_coastal ? _("Coastal") : _("Island")));
}

void tgenerator_settings::adjust_minimum_size_by_players(twindow& window)
{
	const int extra_size = (players_->get_widget_value(window) - 2) * extra_size_per_player;

	const auto update_dimension_slider = [&](tfield_integer* field) {
		tslider& w = dynamic_cast<tslider&>(*field->widget());
		w.set_minimum_value(min_size + extra_size);

		status_label_callback(w, find_widget<tlabel>(&window, w.id() + "_label", false));
	};

	update_dimension_slider(width_);
	update_dimension_slider(height_);
}

} // end namespace gui2
