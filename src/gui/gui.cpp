/*
	Copyright (C) 2008 - 2025
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
namespace
{
config read_and_validate(const std::string& path)
try {
	preproc_map defines;
#ifdef __ANDROID__
	defines.try_emplace("ANDROID");
#endif
	schema_validation::schema_validator validator{filesystem::get_wml_location("schema/gui.cfg").value()};
	return io::read(*preprocess_file(path, defines), &validator);

} catch(const utils::bad_optional_access&) {
	FAIL("GUI2: schema/gui.cfg not found.");

} catch(const abstract_validator::error& e) {
	FAIL("GUI2: could not read schema file: " + e.message);

} catch(const config::error& e) {
	ERR_GUI_P << "Could not read gui file: " << path;
	ERR_GUI_P << e.what();
	return {};
}

/**
 * Adds a theme definition object to the global registry.
 *
 * @param def         A valid gui_definition config.
 *
 * @returns           An optional iterator to the newly-constructed definition.
 *                    If gui_definition throws wml_exception or a theme with the
 *                    given ID already exists, returns nullopt.
 */
auto register_theme(const config& def) -> utils::optional<gui_theme_map_t::iterator>
try {
	auto [iter, is_unique] = guis.try_emplace(def["id"], def);
	if(is_unique) return iter;

	ERR_GUI_P << "UI Theme '" << def["id"] << "' already exists.";
	return utils::nullopt;

} catch(const wml_exception& e) {
	ERR_GUI_P << "Invalid UI theme: " << def["id"];
	ERR_GUI_P << e.user_message;
	return utils::nullopt;
}

/**
 * Parses any GUI2 theme definitions at the file path specified.
 *
 * @param full_path   Path to file containing one or more [gui] tags.
 * @param is_core     If true, look for the default theme here.
 */
void parse(const std::string& full_path, bool is_core)
{
	config cfg = read_and_validate(full_path);
	for(const config& def : cfg.child_range("gui")) {
		const bool is_default = def["id"] == "default";

		if(is_default && !is_core) {
			ERR_GUI_P << "UI theme id 'default' is reserved for core themes.";
			continue;
		}

		const auto iter = register_theme(def);
		if(!iter) continue;

		if(is_default && is_core) {
			default_gui = *iter;
			current_gui = default_gui;
		}
	}
}

} // namespace

void init()
{
	LOG_GUI_G << "Initializing UI subststem.";

	// Reset the registry in case we're re-initializing
	guis.clear();

	// Save current screen size.
	settings::update_screen_size_variables();

	//
	// Parse GUI definitions from mainline
	//

	try {
		parse(filesystem::get_wml_location("gui/_main.cfg").value(), true);
	} catch(const utils::bad_optional_access&) {
		FAIL("GUI2: gui/_main.cfg not found.");
	}

	// The default GUI must be in mainline
	VALIDATE(default_gui != guis.end(), _("No default gui defined."));

	//
	// Parse GUI definitions from addons
	//

	std::vector<std::string> addon_dirs;
	const std::string umc_dir = filesystem::get_addons_dir();
	filesystem::get_files_in_dir(umc_dir, nullptr, &addon_dirs, filesystem::name_mode::ENTIRE_FILE_PATH);

	// Search for all $user_campaign_dir/*/gui-theme.cfg files
	for(const std::string& umc : addon_dirs) {
		const std::string gui_file = umc + "/gui-theme.cfg";

		if(filesystem::file_exists(gui_file)) {
			parse(gui_file, false);
		}
	}
}

void switch_theme(const std::string& theme_id)
{
	if(theme_id.empty() || theme_id == "default") {
		current_gui = default_gui;
	} else {
		current_gui = utils::ranges::find(guis, theme_id,
			[&](const auto& theme) { return theme.first; });

		if(current_gui == guis.end()) {
			ERR_GUI_P << "Missing [gui] definition for '" << theme_id << "'";
			current_gui = default_gui;
		}
	}

	current_gui->second.activate();
}

} // namespace gui2
