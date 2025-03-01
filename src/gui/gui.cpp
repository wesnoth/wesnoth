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
class theme_parser
{
public:
	theme_parser()
	try
		: validator_(filesystem::get_wml_location("schema/gui.cfg").value())
		, defines_(game_config::config_cache::instance().get_preproc_map())
	{
		guis.clear(); // Reset in case we're re-initializing the UI subsystem
	} catch(const std::bad_optional_access&) {
		FAIL("Setting: gui schema file not found.");
	} catch(const abstract_validator::error& e) {
		FAIL("Setting: could not read gui schema file: " + e.message);
	}

	/**
	 * Parses any GUI2 theme definitions at the file path specified.
	 *
	 * @param full_path   The file containing one or more [gui] tags.
	 * @param is_core     If true, look for the default theme.
	 */
	void parse(const std::string& full_path, bool is_core)
	{
		// NOTE: don't put this in the for loop itself, since that creates an iterator range
		// to a temporary. Despite what one would expect, its lifetime is not extended. >_<
		const config definitions = get_definitions(full_path);

		for(const config& def : definitions.child_range("gui")) {
			const bool is_default = def["id"] == "default";
			if(is_default && !is_core) {
				ERR_GUI_P << "GUI Theme ID 'default' is reserved for core themes.";
				continue;
			}

			const auto iter = register_theme(def);
			if(iter && is_default) {
				assert(is_core);
				default_gui = *iter;
				current_gui = default_gui;
			}
		}
	}

private:
	/** Reads and validates the given cfg file. */
	config get_definitions(const std::string& full_path)
	try {
		config cfg;
		filesystem::scoped_istream stream = preprocess_file(full_path, &defines_);
		::read(cfg, *stream, &validator_);
		return cfg;

	} catch(const config::error& e) {
		ERR_GUI_P << e.what();
		ERR_GUI_P << "Setting: could not read gui file: " << full_path;
		return {};
	};

	/**
	 * Adds a theme definition object to the global registry.
	 *
	 * @param cfg         A valid gui_definition config.
	 *
	 * @returns           An optional iterator to the newly-constructed definition.
	 *                    If errors occurred while parsing the config or a theme with
	 *                    the given ID already exists, returns nullopt.
	 */
	auto register_theme(const config& def) const -> utils::optional<gui_theme_map_t::iterator>
	try {
		auto [iter, is_unique] = guis.try_emplace(def["id"], def);
		if(is_unique) return iter;

		ERR_GUI_P << "GUI Theme ID '" << def["id"] << "' already exists.";
		return utils::nullopt;

	} catch(const wml_exception& e) {
		ERR_GUI_P << "Non-functional UI theme: " << def["id"];
		ERR_GUI_P << e.user_message;
		return utils::nullopt;
	}

	/** GUI2 schema validator. */
	schema_validation::schema_validator validator_;

	/** Common macro context shared by all themes (@todo: document this fact). */
	preproc_map defines_;
};

} // namespace

void init()
{
	LOG_GUI_G << "Initializing UI subststem.";

	// Save current screen size.
	settings::update_screen_size_variables();

	//
	// Parse GUI definitions from mainline
	//

	theme_parser parser;
	parser.parse(filesystem::get_wml_location("gui/_main.cfg").value(), true);

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
			parser.parse(gui_file, false);
		}
	}
}

void switch_theme(const std::string& theme_id)
{
	if(theme_id.empty() || theme_id == "default") {
		current_gui = default_gui;
	} else {
		current_gui = std::find_if(guis.begin(), guis.end(),
			[&](const auto& theme) { return theme.first == theme_id; });

		if(current_gui == guis.end()) {
			ERR_GUI_P << "Missing [gui] definition for '" << theme_id << "'";
			current_gui = default_gui;
		}
	}

	current_gui->second.activate();
}

} // namespace gui2
