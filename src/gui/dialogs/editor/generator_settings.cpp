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

#include "formatter.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/status_label_helper.hpp"
#include "gettext.hpp"

#include "utils/functional.hpp"

namespace gui2
{

static int max_coastal = 5;
static int extra_size_per_player = 2;
static int min_size = 20;

REGISTER_DIALOG(generator_settings)

static const auto landform_label = [](tslider& s)->std::string {
	return s.get_value() == 0 ? _("Inland") : (s.get_value() < max_coastal ? _("Coastal") : _("Island"));
};

tgenerator_settings::tgenerator_settings(generator_data& data)
	: players_(register_integer("players", true, data.nplayers))
	, width_(register_integer("width",     true, data.width))
	, height_(register_integer("height",   true, data.height))
	, update_width_label_()
	, update_height_label_()
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
	connect_signal_notify_modified(*players_->widget(), std::bind(
		&tgenerator_settings::adjust_minimum_size_by_players, this, std::ref(window)));

	gui2::bind_status_label<tslider>(window, "players");

	update_width_label_  = gui2::bind_status_label<tslider>(window, "width");
	update_height_label_ = gui2::bind_status_label<tslider>(window, "height");

	gui2::bind_status_label<tslider>(window, "villages", [](tslider& s)->std::string { return formatter() << s.get_value() << _("/1000 tiles"); });
	gui2::bind_status_label<tslider>(window, "castle_size");
	gui2::bind_status_label<tslider>(window, "landform", [](tslider& s)->std::string { return landform_label(s); });
}

void tgenerator_settings::adjust_minimum_size_by_players(twindow& window)
{
	const int extra_size = (players_->get_widget_value(window) - 2) * extra_size_per_player;

	const auto update_dimension_slider = [&](tfield_integer* field) {
		tslider& w = dynamic_cast<tslider&>(*field->widget());
		w.set_minimum_value(min_size + extra_size);
	};

	update_dimension_slider(width_);
	update_dimension_slider(height_);

	update_width_label_();
	update_height_label_();
}

} // end namespace gui2
