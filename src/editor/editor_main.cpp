/*
	Copyright (C) 2008 - 2023
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/controller/editor_controller.hpp"

#include "gettext.hpp"
#include "gui/dialogs/editor/choose_addon.hpp"
#include "gui/dialogs/message.hpp"
#include "filesystem.hpp"
#include "editor/action/action_base.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

lg::log_domain log_editor("editor");

namespace editor {

void initialize_addon(const std::string& addon_id)
{
	std::string addon_dir = filesystem::get_addons_dir() + "/" + addon_id;

	// create folders
	filesystem::create_directory_if_missing(addon_dir);
	filesystem::create_directory_if_missing(addon_dir + "/maps");
	filesystem::create_directory_if_missing(addon_dir + "/scenarios");
	filesystem::create_directory_if_missing(addon_dir + "/images");
	filesystem::create_directory_if_missing(addon_dir + "/images/units");
	filesystem::create_directory_if_missing(addon_dir + "/masks");
	filesystem::create_directory_if_missing(addon_dir + "/music");
	filesystem::create_directory_if_missing(addon_dir + "/sounds");
	filesystem::create_directory_if_missing(addon_dir + "/translations");
	filesystem::create_directory_if_missing(addon_dir + "/units");
	filesystem::create_directory_if_missing(addon_dir + "/utils");

	// create files
	// achievements
	{
		filesystem::scoped_ostream stream = filesystem::ostream_file(addon_dir + "/achievements.cfg");
		*stream << "";
	}

	// _server.pbl
	{
		filesystem::scoped_ostream stream = filesystem::ostream_file(addon_dir + "/_server.pbl");
		*stream << "";
	}

	// a basic _main.cfg
	{
		filesystem::scoped_ostream stream = filesystem::ostream_file(addon_dir + "/_main.cfg");
		*stream << "#textdomain wesnoth-" << addon_id << "\n"
				<< "[textdomain]" << "\n"
				<< "\tname=\"wesnoth-" << addon_id << "\"\n"
				<< "\tpath=\"data/add-ons/" << addon_id << "/translations\"\n"
				<< "[/textdomain]\n"
				<< "\n"
				<< "#ifdef MULTIPLAYER\n"
				<< "[binary_path]\n"
				<< "\tpath=data/add-ons/" << addon_id << "\n"
				<< "[/binary_path]\n"
				<< "\n"
				<< "{~add-ons/" << addon_id << "/scenarios}\n"
				<< "{~add-ons/" << addon_id << "/utils}\n"
				<< "\n"
				<< "[units]\n"
				<< "\t{~add-ons/" << addon_id << "/units}\n"
				<< "[/units]\n"
				<< "#endif\n";
	}
}

static EXIT_STATUS migrate_to_addon(const std::string& addon_id)
{
	std::string addon_dir = filesystem::get_addons_dir() + "/" + addon_id;
	if(!filesystem::file_exists(addon_dir)) {
		initialize_addon(addon_id);
	}

	// migrate any existing scenarios/maps to an add-on
	std::string scenarios_dir = filesystem::get_legacy_editor_dir() + "/scenarios";
	std::string maps_dir = filesystem::get_legacy_editor_dir() + "/maps";
	if(filesystem::dir_size(scenarios_dir) + filesystem::dir_size(maps_dir) > 0) {
		std::vector<std::string> maps;
		filesystem::get_files_in_dir(maps_dir, &maps);
		for(const auto& map : maps) {
			std::string map_base_name = filesystem::base_name(map, true);

			DBG_ED << "Moving map " << map << " to " << addon_dir + "/maps";
			std::string map_file = addon_dir + "/maps/" + map;
			filesystem::copy_file(maps_dir + "/" + map, map_file);

			// determine side count based on number of starting positions defined
			std::string map_contents = filesystem::read_file(addon_dir + "/maps/" + map);
			int sides = 1;
			while(map_contents.find(", "+std::to_string(sides)) != std::string::npos) {
				sides++;
			}
			// we found sides-1 starting positions
			sides--;
			// a scenario needs at least 1 side
			if(sides == 0) {
				sides = 1;
			}

			config cfg;
			config& multiplayer = cfg.add_child("multiplayer");
			multiplayer["id"] = addon_id+"-"+map_base_name;
			multiplayer["name"] = map_base_name;
			multiplayer["description"] = "";
			multiplayer["map_file"] = map;
			multiplayer["random_start_time"] = "no";
			multiplayer["victory_when_enemies_defeated"] = "yes";

			for(int i = 1; i <= sides; i++) {
				config& side = multiplayer.add_child("side");
				side["side"] = i;
				side["canrecruit"] = "yes";
				side["controller"] = "human";
				side["fog"] = "yes";
			}

			std::string cfg_file = addon_dir + "/scenarios/"+map_base_name+".cfg";

			filesystem::scoped_ostream stream = filesystem::ostream_file(cfg_file);
			*stream << cfg;

			// check that the files were moved before deleting it
			if(filesystem::file_size(map_file) > 0 && filesystem::file_size(cfg_file) > 0) {
				filesystem::delete_file(maps_dir + "/" + map);
			}
		}

		std::vector<std::string> scenarios;
		filesystem::get_files_in_dir(scenarios_dir, &scenarios);
		for(const auto& scenario : scenarios) {
			DBG_ED << "Moving scenario " << scenario << " to " << addon_dir + "/scenarios";

			config temp;
			try {
				// preprocessing needed to strip out comments added by the editor
				read(temp, *preprocess_file(scenarios_dir + "/" + scenario));
			} catch(const std::exception& e) {
				ERR_ED << "Skipping migration of scenario due to exception while parsing file: '" << scenarios_dir + "/" + scenario << "'\n" << e.what();
				continue;
			}

			config cfg;
			config& multiplayer = cfg.add_child("multiplayer");
			multiplayer.append_attributes(temp);
			std::string map_data = multiplayer["map_data"];
			std::string separate_map_file = addon_dir + "/maps/" + filesystem::base_name(scenario, true) + ".map";

			// check that there's embedded map data, since that's how the editor used to save scenarios
			if(!map_data.empty()) {
				// check if a .map file already exists as a separate standalone .map in the editor folders or if a .map file already exists in the add-on
				while(filesystem::file_exists(separate_map_file) || filesystem::file_exists(maps_dir + "/" + filesystem::base_name(separate_map_file, true) + ".map")) {
					separate_map_file = addon_dir + "/maps/" + filesystem::base_name(separate_map_file, true) + "1.map";
				}
				multiplayer["id"] = filesystem::base_name(separate_map_file, true);

				filesystem::write_file(separate_map_file, map_data);
				multiplayer.remove_attribute("map_data");
				multiplayer["map_file"] = filesystem::base_name(separate_map_file);
			} else {
				ERR_ED << "Skipping migration of " << scenario << " due to missing map_data attribute.";
				continue;
			}

			config& event = multiplayer.add_child("event");
			event["name"] = "start";
			event["id"] = "editor_event-start";

			// for all children that aren't [side] or [time], move them to an event
			// for [side]:
			//   keep all attributes in [side]
			//   also keep any [village]s in [side]
			//   move all other children to the start [event]
			//   if [unit], set the unit's side
			// for [time]:
			//   keep under [multiplayer]
			for(const config::any_child child : temp.all_children_range()) {
				if(child.key != "side" && child.key != "time") {
					config& c = event.add_child(child.key);
					c.append_attributes(child.cfg);
					c.append_children(child.cfg);
				} else if(child.key == "side") {
					config& c = multiplayer.add_child("side");
					c.append_attributes(child.cfg);
					for(const config::any_child side_child : child.cfg.all_children_range()) {
						if(side_child.key == "village") {
							config& c1 = c.add_child("village");
							c1.append_attributes(side_child.cfg);
						} else {
							config& c1 = event.add_child(side_child.key);
							c1.append_attributes(side_child.cfg);
							if(side_child.key == "unit") {
								c1["side"] = child.cfg["side"];
							}
						}
					}
				} else if(child.key == "time") {
					config& c = multiplayer.add_child("time");
					c.append_attributes(child.cfg);
				}
			}

			std::string cfg_file = addon_dir + "/scenarios/" + scenario;

			filesystem::scoped_ostream stream = filesystem::ostream_file(cfg_file);
			*stream << cfg;

			// check that the files were moved before deleting it
			// the cfg check is only for existance, since at least on linux the file exists but has not been written to when this check is reached
			if(filesystem::file_size(separate_map_file) > 0 && filesystem::file_exists(cfg_file)) {
				filesystem::delete_file(scenarios_dir + "/" + scenario);
			}
		}
	}

	return EXIT_STATUS::EXIT_NORMAL;
}

EXIT_STATUS start(const std::string& filename /* = "" */,
	bool take_screenshot /* = false */, const std::string& screenshot_filename /* = "map_screenshot.png" */)
{
	EXIT_STATUS e = EXIT_ERROR;
	try {
		const hotkey::scope_changer h{hotkey::scope_editor};

		std::string addon_id = "";
		while(true)
		{
			gui2::dialogs::editor_choose_addon choose(addon_id);
			if(choose.show()) {
				if(addon_id == "") {
					gui2::show_message(_("Error"), _("The add-on ID can't be empty."), gui2::dialogs::message::auto_close);
					continue;
				} else if(addon_id.find(" ") != std::string::npos) {
					gui2::show_message(_("Error"), _("Add-on IDs can't contain spaces."), gui2::dialogs::message::auto_close);
					continue;
				}
				break;
			} else {
				return EXIT_STATUS::EXIT_NORMAL;
			}
		}

		editor_controller editor(addon_id);

		// don't let people try to migrate their editor stuff into mainline folders
		if(!take_screenshot && addon_id != "mainline") {
			migrate_to_addon(addon_id);
		}

		if (!filename.empty() && filesystem::file_exists (filename)) {
			if (filesystem::is_directory(filename)) {
				editor.context_manager_->set_default_dir(filename);
				editor.context_manager_->load_map_dialog(true);
			} else {
				editor.context_manager_->load_map(filename, false);

				// HACK: this fixes an issue where the button overlays would be missing when
				// the loaded map appears. Since we're gonna drop this ridiculous GUI1 drawing
				// stuff in 1.15 I'm not going to waste time coming up with a better fix.
				//
				// Do note adding a redraw_everything call to context_manager::refresh_all also
				// fixes the issue, but I'm pretty sure thats just because editor_controller::
				// display_redraw_callback gets called, which then calls set_button_state.
				//
				// -- vultraz, 2018-02-24
				editor.set_button_state();
			}

			if (take_screenshot) {
				editor.do_screenshot(screenshot_filename);
				e = EXIT_NORMAL;
			}
		}

		if (!take_screenshot) {
			e = editor.main_loop();
		}
	} catch(const editor_exception& e) {
		ERR_ED << "Editor exception in editor::start: " << e.what();
		throw;
	}
	if (editor_action::get_instance_count() != 0) {
		ERR_ED << "Possibly leaked " << editor_action::get_instance_count() << " action objects";
	}

	return e;
}

} //end namespace editor
