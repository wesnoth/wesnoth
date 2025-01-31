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

	config guis_cfg, addons_cfg;
	preproc_map preproc(game_config::config_cache::instance().get_preproc_map());

	//
	// Read and validate theme WML files from mainline
	//
	std::string current_file;
	const std::string schema_file = "schema/gui.cfg";
	try {
		schema_validation::schema_validator validator(filesystem::get_wml_location(schema_file).value());

		// Core theme files
		current_file = "gui/_main.cfg";
		filesystem::scoped_istream stream = preprocess_file(filesystem::get_wml_location(current_file).value(), &preproc);
		read(guis_cfg, *stream, &validator);

	} catch(const config::error& e) {
		ERR_GUI_P << e.what();
		ERR_GUI_P << "Setting: could not read gui file: " << current_file;
	} catch(const abstract_validator::error& e) {
		ERR_GUI_P << "Setting: could not read schema file: " << schema_file;
		ERR_GUI_P << e.message;
	}

	//
	// Read and validate theme WML files from addons
	//

	// Add the $user_campaign_dir/*/gui.cfg files to the addon gui config.
	std::vector<std::string> user_dirs;
	{
		const std::string user_campaign_dir = filesystem::get_addons_dir();
		std::vector<std::string> user_files;
		filesystem::get_files_in_dir(
			user_campaign_dir, &user_files, &user_dirs, filesystem::name_mode::ENTIRE_FILE_PATH);
	}

	for(const std::string& umc : user_dirs) {
		try {
			const std::string gui_file = umc + "/gui-theme.cfg";
			current_file = filesystem::get_short_wml_path(gui_file);
			if(filesystem::file_exists(gui_file)) {
				config addon_cfg;
				schema_validation::schema_validator validator(filesystem::get_wml_location(schema_file).value());
				read(addon_cfg, *preprocess_file(gui_file, &preproc), &validator);
				addons_cfg.append(addon_cfg);
			}
		} catch(const config::error& e) {
			ERR_GUI_P << e.what();
			ERR_GUI_P << "Setting: could not read gui file: " << current_file;
		} catch(const abstract_validator::error& e) {
			ERR_GUI_P << "Setting: could not read schema file: " << schema_file;
			ERR_GUI_P << e.message;
		}
	}

	//
	// Parse GUI definitions from mainline
	//
	for(const config& g : guis_cfg.child_range("gui")) {
		const std::string id = g["id"];

		auto [iter, is_unique] = guis.try_emplace(id, g);

		if (!is_unique) {
			ERR_GUI_P << "GUI Theme ID '" << id << "' already exists.";
		} else {
			if(id == "default") {
				default_gui = iter;
			}
		}
	}

	//
	// Parse GUI definitions from addons
	//
	for(const config& g : addons_cfg.child_range("gui")) {
		const std::string id = g["id"];

		try {
			auto [iter, is_unique] = guis.try_emplace(id, g);

			if (!is_unique) {
				ERR_GUI_P << "GUI Theme ID '" << id << "' already exists.";
			}
		} catch (const wml_exception& e) {
			ERR_GUI_P << "Non-functional theme: " << id;
			ERR_GUI_P << e.user_message;
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
