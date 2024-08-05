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

#include "gui/gui.hpp"

#include "config_cache.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "gui/core/log.hpp"
#include "gui/core/gui_definition.hpp"
#include "gui/widgets/settings.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/schema_validator.hpp"
#include "wml_exception.hpp"

namespace gui2
{

void init()
{
	LOG_GUI_G << "Initializing UI subststem.";

	// Save current screen size.
	settings::update_screen_size_variables();

	//
	// Read and validate the WML files.
	//
	config guis_cfg;
	try {
		schema_validation::schema_validator validator(filesystem::get_wml_location("schema/gui.cfg").value());

		preproc_map preproc(game_config::config_cache::instance().get_preproc_map());
		filesystem::scoped_istream stream = preprocess_file(filesystem::get_wml_location("gui/_main.cfg").value(), &preproc);
		read(guis_cfg, *stream, &validator);
	} catch(const config::error& e) {
		ERR_GUI_P << e.what();
		ERR_GUI_P << "Setting: could not read file 'data/gui/_main.cfg'.";
	} catch(const abstract_validator::error& e) {
		ERR_GUI_P << "Setting: could not read file 'data/schema/gui.cfg'.";
		ERR_GUI_P << e.message;
	}

	//
	// Parse GUI definitions.
	//
	for(const config& g : guis_cfg.child_range("gui")) {
		const std::string id = g["id"];

		auto iter = guis.emplace(id, gui_definition(g)).first;

		if(id == "default") {
			default_gui = iter;
		}
	}

	VALIDATE(default_gui != guis.end(), _("No default gui defined."));
}

void switch_theme(const std::string& current_theme)
{
	if (current_theme.empty() || current_theme == "default") {
		current_gui = default_gui;
	} else {
		gui_theme_map_t::iterator gui_itor = guis.begin();
		for (const auto& gui : guis) {
			if (gui.first == current_theme) {
				current_gui = gui_itor;
			}

			if (gui_itor != guis.end()) {
				gui_itor++;
			}
		}

		if(current_gui == guis.end()) {
			ERR_GUI_P << "Missing [gui] definition for '" << current_theme << "'";
			current_gui = default_gui;
		}
	}

	current_gui->second.activate();
}

} // namespace gui2
