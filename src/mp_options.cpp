/* $Id: mp_options.cpp 55420 2012-09-26 14:23:00Z lipk $ */
/*
   Copyright (C) 2012 by Boldizsár Lipka <lipka.boldizsar@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mp_options.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/window_builder.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

static lg::log_domain log_mp_create_options("mp/create/options");
#define DBG_MP LOG_STREAM(debug, log_mp_create_options)

namespace
{

// helper function
config& add_column(config& parent, const std::string& halign = "left",
					  float grow_factor = 0)
{
	config& result = parent.add_child("column");
	result["horizontal_alignment"] = halign;
	result["grow_factor"] = grow_factor;
	result["border"] = "all";
	result["border_size"] = 5;

	return result;
}

}

namespace mp
{

namespace options
{

config to_event(const config& cfg)
{
	config result;

	if (!cfg) {
		return result;
	}

	result["name"] = "prestart";

	BOOST_FOREACH (const config& c, cfg.child_range("option")) {
		config ev;
		ev["name"] = c["id"];
		ev["value"] = c["value"];
		result.add_child("set_variable", ev);
	}

	return result;
}

void manager::init_info(const config& cfg, const std::string& key)
{
	BOOST_FOREACH (const config& comp, cfg.child_range(key)) {
		config entry;
		entry["id"] = comp["id"];
		entry["name"] = comp["name"];

		if (comp.has_child("options") && comp["allow_new_game"].to_bool(true)) {
			const config& options = comp.child("options");

			BOOST_FOREACH (const config::any_child& c,
								options.all_children_range()) {
				entry.add_child(c.key, c.cfg);
			}

		}

		// We need to store components even if they don't have any options in
		// order to have set_xxx_by_index work properly
		options_info_.add_child(key, entry);
	}
}

manager::manager(const config& gamecfg, CVideo& video, const config& values)
		: options_info_()
		, values_(values)
		, video_(video)
		, era_()
		, scenario_()
		, modifications_()
{
	DBG_MP << "Initializing the options manager" << std::endl;
	init_info(gamecfg, "modification");
	init_info(gamecfg, "era");
	init_info(gamecfg, "multiplayer");

	BOOST_FOREACH (const config::any_child& i,
				   options_info_.all_children_range())
	{
		BOOST_FOREACH (const config::any_child& j, i.cfg.all_children_range())
		{
			if (is_valid_option(j.key, j.cfg)) {
				config& value = get_value_cfg(j.cfg["id"]);
				value["value"] = get_stored_value(j.cfg["id"]);
			}
		}
	}
}


void manager::set_values(const config& c)
{
	values_ = c;
}

void manager::set_era(const std::string& era)
{
	era_ = era;
}

void manager::set_era_by_index(int index)
{
	era_ = options_info_.child("era", index)["id"].str();
}

void manager::set_scenario(const std::string& scenario)
{
	scenario_ = scenario;
}

void manager::set_scenario_by_index(int index)
{
	scenario_ = options_info_.child("multiplayer", index - 1)["id"].str();
}

void manager::set_modifications(const std::vector<std::string>& modifications)
{
	modifications_ = modifications;
}

void manager::insert_element(elem_type type, const config& data, int pos)
{
	switch (type)
	{
	case SCENARIO:
		options_info_.add_child_at("multiplayer", data, pos);
		break;
	case ERA:
		options_info_.add_child_at("era", data, pos);
		break;
	case MODIFICATION:
		options_info_.add_child_at("modification", data, pos);
		break;
	}
}

void manager::show_dialog()
{
	DBG_MP << "Building the options dialog" << std::endl;
	// Constructing the dialog
	config dialog_cfg;

	dialog_cfg.add_child("resolution");
	dialog_cfg["definition"] = "default";
	dialog_cfg["automatic_placement"] = true;
	dialog_cfg["vertical_placement"] = "center";
	dialog_cfg["horizontal_placement"] = "center";
	dialog_cfg.add_child("helptip")["id"] = "tooltip_large";
	dialog_cfg.add_child("tooltip")["id"] = "tooltip_large";

	config& grid = dialog_cfg.add_child("grid");

	// Adding widgets for available options
	add_widgets(options_info_.find_child("era", "id", era_), grid);
	add_widgets(options_info_.find_child("multiplayer", "id", scenario_), grid);

	for (unsigned i = 0; i<modifications_.size(); i++) {
		add_widgets(
			options_info_.find_child("modification", "id", modifications_[i]),
			grid);
	}

	if (grid.ordered_begin() == grid.ordered_end()) {
		// No widgets were actually added, we've got nothing to show
		gui2::show_transient_message(video_, "", _(
				"None of the selected modifications, era or scenario provide " \
				"configuration options."));

		return;
	}

	// Dialog buttons
	config& row = grid.add_child("row");
	row["grow_factor"] = 0;

	config& column = add_column(row, "right", 1);
	config& widget_grid = column.add_child("grid");
	config& widget_row = widget_grid.add_child("row");
	widget_row["grow_factor"] = 0;

	config& defaults_column = add_column(widget_row);
	config& ok_column = add_column(widget_row, "right");
	config& cancel_column = add_column(widget_row);

	config& defaults_button = defaults_column.add_child("button");
	defaults_button["definition"] = "default";
	defaults_button["label"] = _("Restore defaults");
	defaults_button["id"] = "restore_defaults";

	config& ok_button = ok_column.add_child("button");
	ok_button["definition"] = "default";
	ok_button["label"] = _("OK");
	ok_button["id"] = "ok";

	config& cancel_button = cancel_column.add_child("button");
	cancel_button["definition"] = "default";
	cancel_button["label"] = _("Cancel");
	cancel_button["id"] = "cancel";

	// Building the window
	gui2::twindow_builder::tresolution resolution(dialog_cfg);
	gui2::twindow* window = gui2::build(video_, &resolution);

	__tmp_set_checkbox_defaults(window);

	gui2::tbutton* button = gui2::find_widget<gui2::tbutton>
									(window, "restore_defaults", false, true);

	// Callbacks (well, one callback)
	button->connect_click_handler(boost::bind(restore_defaults, this, window));

	// Show window
	DBG_MP << "Showing the dialog" << std::endl;
	if (window->show() == gui2::twindow::CANCEL) {
		DBG_MP << "User cancelled changes" << std::endl;
		delete window;

		return;
	}

	// Saving the results
	DBG_MP << "User accepted changes, saving values" << std::endl;
	extract_values("era", era_, window);
	extract_values("multiplayer", scenario_, window);
	for (unsigned i = 0; i<modifications_.size(); i++) {
		extract_values("modification", modifications_[i], window);
	}

	delete window;
}

void manager::add_widgets(const config& data, config& grid) const
{
	if (!data.has_child("entry") &&
		!data.has_child("slider") &&
		!data.has_child("checkbox"))
	{
		//Don't display the title if there're no options at all
		return;
	}

	{
		// The title for this section
		config& row = grid.add_child("row");
		row["grow_factor"] = 0;
		config& column = add_column(row, "left", 1);
		config& caption = column.add_child("label");
		caption["definition"] = "title";
		caption["label"] = data["name"];
	}

	// Adding the widgets
	BOOST_FOREACH (const config::any_child& c, data.all_children_range()) {
		if (!is_valid_option(c.key, c.cfg))
		{
			continue;
		}

		config& row = grid.add_child("row");
		row["grow_factor"] = 0;
		config& column = add_column(row, "left", 1);

		if (c.key == "entry") {
			add_entry(c.cfg, column);
		} else if (c.key == "slider") {
			add_slider(c.cfg, column);
		} else if (c.key == "checkbox") {
			add_checkbox(c.cfg, column);
		}
	}
}

void manager::add_entry(const config& data, config& column) const
{
	config& grid = column.add_child("grid");
	config& row = grid.add_child("row");
	row["grow_factor"] = 0;

	config& label_column = add_column(row);
	config& entry_column = add_column(row);

	config& label = label_column.add_child("label");
	label["definition"] = "default";
	label["label"] = data["description"];

	config& entry = entry_column.add_child("text_box");
	entry["id"] = data["id"];
	entry["definition"] = "default";
	entry["label"] = get_stored_value(data["id"]);
}

void manager::add_slider(const config& data, config& column) const
{
	config& grid = column.add_child("grid");
	config& row = grid.add_child("row");
	row["grow_factor"] = 0;

	config& label_column = add_column(row);
	config& slider_column = add_column(row);

	config& label = label_column.add_child("label");
	label["definition"] = "default";
	label["label"] = data["description"];

	config& slider = slider_column.add_child("slider");
	slider["id"] = data["id"];
	slider["definition"] = "default";
	slider["minimum_value"] = data["min_value"];
	slider["maximum_value"] = data["max_value"];
	slider["step_size"] = data["step"].to_int() ? data["step"].to_int() : 1;
	slider["value"] = get_stored_value(data["id"]);
}

void manager::add_checkbox(const config& data, config& column) const
{
	config& grid = column.add_child("grid");
	config& row = grid.add_child("row");
	row["grow_factor"] = 0;

	config& box_column = add_column(row);

	config& checkbox = box_column.add_child("toggle_button");
	checkbox["id"] = data["id"];
	checkbox["definition"] = "default";
	checkbox["label"] = data["description"];
}

config& manager::get_value_cfg(const std::string& id)
{
	{
		const config& value_cfg = get_value_cfg_or_empty(id);
		if (!value_cfg.empty()) {
			return const_cast<config&>(value_cfg);
		}
	}

	config::any_child info = get_option_parent(id);
	config* parent_cfg;
	if (!values_.find_child(info.key, "id", info.cfg["id"])) {
		parent_cfg = &values_.add_child(info.key);
		(*parent_cfg)["id"] = info.cfg["id"];
	} else {
		parent_cfg = &values_.find_child(info.key, "id", info.cfg["id"]);
	}

	config& value_cfg = parent_cfg->add_child("option");
	value_cfg["id"] = id;

	return value_cfg;
}

const config& manager::get_value_cfg_or_empty(const std::string& id) const
{
	static const config empty;

	BOOST_FOREACH (const config::any_child& i, values_.all_children_range()) {
		BOOST_FOREACH (const config& j, i.cfg.child_range("option")) {
			if (j["id"] == id) {
				return j;
			}
		}
	}

	return empty;
}

config::any_child manager::get_option_parent(const std::string& id) const
{
	static const config empty;
	static const std::string empty_key = "";
	static config::any_child not_found(&empty_key, &empty);

	BOOST_FOREACH (const config::any_child& i,
				   options_info_.all_children_range()) {
		BOOST_FOREACH (const config::any_child& j, i.cfg.all_children_range()) {
			if (j.cfg["id"] == id) {
				return i;
			}
		}
	}

	return not_found;
}

const config& manager::get_option_info_cfg(const std::string& id) const
{
	static const config empty;

	BOOST_FOREACH (const config::any_child& i,
				   options_info_.all_children_range()) {
		BOOST_FOREACH (const config::any_child& j, i.cfg.all_children_range()) {
			if (j.cfg["id"] == id) {
				return j.cfg;
			}
		}
	}

	return empty;
}


config::attribute_value manager::get_stored_value(const std::string& id) const
{
	const config& valcfg = get_value_cfg_or_empty(id);

	if (!valcfg["value"].empty()) {
		// There's a saved value for this option
		return valcfg["value"];
	}

	// Fall back to the option's default
	return get_default_value(id);
}

config::attribute_value manager::get_default_value(const std::string& id) const
{
	const config& optinfo = get_option_info_cfg(id);

	return optinfo["default"];
}

int manager::get_slider_value(const std::string& id, gui2::twindow* win) const
{
	gui2::tslider* widget =
						gui2::find_widget<gui2::tslider>(win, id, false, true);

	return widget->get_value();
}

bool manager::get_checkbox_value
					(const std::string& id, gui2::twindow* win) const
{
	gui2::ttoggle_button* widget =
				gui2::find_widget<gui2::ttoggle_button>(win, id, false, true);

	return widget->get_value();
}

std::string manager::get_entry_value(const std::string& id,
									 gui2::twindow* window) const
{
	gui2::ttext_box* widget =
				gui2::find_widget<gui2::ttext_box>(window, id, false, true);

	return widget->text();
}

void manager::set_slider_value(int val, const std::string& id,
							   gui2::twindow* win) const
{
	gui2::tslider* widget =
						gui2::find_widget<gui2::tslider>(win, id, false, true);

	widget->set_value(val);
}

void manager::set_checkbox_value(bool val, const std::string& id,
								 gui2::twindow* win) const
{
	gui2::ttoggle_button* widget =
				gui2::find_widget<gui2::ttoggle_button>(win, id, false, true);

	widget->set_value(val);
}

void manager::set_entry_value(const std::string& val, const std::string& id,
							  gui2::twindow* win) const
{
	gui2::ttext_box* widget =
				gui2::find_widget<gui2::ttext_box>(win, id, false, true);

	widget->set_value(val);
}

void manager::extract_values(const std::string& key, const std::string& id,
						   gui2::twindow* window)
{
	BOOST_FOREACH (const config::any_child& c,
				   options_info_.find_child(key, "id", id).all_children_range())
	{
		if (!is_valid_option(c.key, c.cfg)) {
			continue;
		}

		config& out = get_value_cfg(c.cfg["id"].str());

		if (c.key == "entry") {
			out["value"] = get_entry_value(c.cfg["id"], window);
		} else if (c.key == "slider") {
			out["value"] = get_slider_value(c.cfg["id"], window);
		} else if (c.key == "checkbox") {
			out["value"] = get_checkbox_value(c.cfg["id"], window);
		}
	}
}

bool manager::is_valid_option(const std::string& key, const config& option)
{
	return (key == "slider" || key == "entry" || key == "checkbox") &&
		   (!option["id"].empty());
}

void manager::restore_defaults(manager* m, gui2::twindow* w)
{
	const config& era = m->options_info_.find_child("era", "id", m->era_);
	restore_defaults_for_component(era, m, w);

	const config& scen = m->options_info_.find_child("multiplayer", "id",
													 m->scenario_);
	restore_defaults_for_component(scen, m, w);

	BOOST_FOREACH (const std::string& id, m->modifications_) {
		const config& mod = m->options_info_.find_child("modification", "id",
														id);
		restore_defaults_for_component(mod, m, w);
	}
}

void manager::restore_defaults_for_component(const config& c, manager* m,
											 gui2::twindow* w)
{
	BOOST_FOREACH (const config::any_child& i, c.all_children_range()) {
		if (!is_valid_option(i.key, i.cfg)) {
			continue;
		}

		const std::string id = i.cfg["id"].str();

		if (i.key == "entry") {
			m->set_entry_value(m->get_default_value(id).str(), id, w);
		} else if (i.key == "checkbox") {
			m->set_checkbox_value(m->get_default_value(id).to_bool(), id, w);
		} else if (i.key == "slider") {
			m->set_slider_value(m->get_default_value(id).to_int(), id, w);
		}
	}
}

void manager::__tmp_set_checkbox_defaults(gui2::twindow* window) const
{
	BOOST_FOREACH (const config::any_child& i,
				   options_info_.all_children_range())
	{
		BOOST_FOREACH (const config& j, i.cfg.child_range("checkbox"))
		{
			if (!is_valid_option("checkbox", j)) {
				continue;
			}

			gui2::ttoggle_button* button;
			button = gui2::find_widget<gui2::ttoggle_button>
										(window, j["id"].str(), false, false);

			if (button) {
				button->set_value(get_stored_value(j["id"]).to_bool());
			}
		}
	}
}

}	// namespace options

}	// namespace mp

