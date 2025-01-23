/*
	Copyright (C) 2008 - 2024
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

#include "editor/map/map_context.hpp"

#include "display.hpp"
#include "editor/action/action.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "map/label.hpp"
#include "preferences/preferences.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "game_config_view.hpp"

#include <boost/regex.hpp>

namespace editor
{
editor_team_info::editor_team_info(const team& t)
	: side(t.side())
	, id(t.team_name())
	, name(t.user_team_name())
	, recruit_list(utils::join(t.recruits(), ","))
	, gold(t.gold())
	, income(t.base_income())
	, village_income(t.village_gold())
	, village_support(t.village_support())
	, fog(t.uses_fog())
	, shroud(t.uses_shroud())
	, share_vision(t.share_vision())
	, controller(t.controller())
	, no_leader(t.no_leader())
	, hidden(t.hidden())
{
}

const std::size_t map_context::max_action_stack_size_ = 100;

namespace {
	static const int editor_team_default_gold = 100;
}

map_context::map_context(const editor_map& map, bool pure_map, const config& schedule, const std::string& addon_id)
	: filename_()
	, map_data_key_()
	, embedded_(false)
	, pure_map_(pure_map)
	, map_(map)
	, undo_stack_()
	, redo_stack_()
	, actions_since_save_(0)
	, starting_position_label_locs_()
	, needs_reload_(false)
	, needs_terrain_rebuild_(false)
	, needs_labels_reset_(false)
	, changed_locations_()
	, everything_changed_(false)
	, addon_id_(addon_id)
	, previous_cfg_()
	, scenario_id_()
	, scenario_name_()
	, scenario_description_()
	, xp_mod_()
	, victory_defeated_(true)
	, random_time_(false)
	, active_area_(-1)
	, labels_(nullptr)
	, units_()
	, teams_()
	, tod_manager_(new tod_manager(schedule))
	, mp_settings_()
	, game_classification_()
	, music_tracks_()
{
}

static std::string get_map_location(const std::string& file_contents, const std::string& attr)
{
	std::size_t attr_name_start = file_contents.find(attr);
	if(attr_name_start == std::string::npos) return "";

	std::size_t attr_value_start = file_contents.find("=", attr_name_start);
	std::size_t line_end = file_contents.find("\n", attr_name_start);
	if(line_end < attr_value_start) return "";

	attr_value_start++;
	std::string attr_value = file_contents.substr(attr_value_start, line_end - attr_value_start);
	std::string_view v2 = attr_value;
	utils::trim(v2);

	return std::string(v2);
}

map_context::map_context(const game_config_view& game_config, const std::string& filename, const std::string& addon_id)
	: filename_(filename)
	, map_data_key_()
	, embedded_(false)
	, pure_map_(false)
	, map_()
	, undo_stack_()
	, redo_stack_()
	, actions_since_save_(0)
	, starting_position_label_locs_()
	, needs_reload_(false)
	, needs_terrain_rebuild_(false)
	, needs_labels_reset_(false)
	, changed_locations_()
	, everything_changed_(false)
	, addon_id_(addon_id)
	, previous_cfg_()
	, scenario_id_()
	, scenario_name_()
	, scenario_description_()
	, xp_mod_()
	, victory_defeated_(true)
	, random_time_(false)
	, active_area_(-1)
	, labels_(nullptr)
	, units_()
	, teams_()
	, tod_manager_(new tod_manager(game_config.find_mandatory_child("editor_times", "id", "empty")))
	, mp_settings_()
	, game_classification_()
	, music_tracks_()
{
	/*
	 * Overview of situations possibly found in the file:
	 *
	 * embedded_ - the map data is directly in the scenario file
	 * pure_map_ - the map data is in its own separate file (map_file, map_data+macro inclusion) or this is a .map file
	 *
	 * an editor-generated file uses neither of these and is its own thing - it's not embedded (since the editor now saves using map_file) and it's not a pure map since there's also scenario data involved
	 *
	 * 0. Not a scenario or map file.
	 *    0.1 File not found
	 *    0.2 Map file empty
	 *    0.3 Not a .map or .cfg file
	 * 1. It's a .map file.
	 *    * embedded_ = false
	 *    * pure_map_ = true
	 * 2. A scenario embedding the map
	 *    * embedded_ = true
	 *    * pure_map_ = true
	 *    The scenario-test.cfg for example.
	 *    The map is written back to the file.
	 * 3. The map file is referenced by map_data={MACRO_ARGUEMENT}.
	 *    * embedded_ = false
	 *    * pure_map_ = true
	 * 4. The file contains an editor generated scenario file.
	 *    * embedded_ = false
	 *    * pure_map_ = false
	 * 5. The file is using map_file.
	 *    5.1 The file doesn't contain a macro and so can be loaded by the editor as a scenario
	 *       * embedded_ = false
	 *       * pure_map_ = false
	 *    5.2 The file contains a macro and so can't be loaded by the editor as a scenario
	 *       * embedded_ = false
	 *       * pure_map_ = true
	 */

	log_scope2(log_editor, "Loading file " + filename);

	// 0.1 File not found
	if(!filesystem::file_exists(filename) || filesystem::is_directory(filename)) {
		throw editor_map_load_exception(filename, _("File not found"));
	}

	std::string file_string = filesystem::read_file(filename);

	// 0.2 Map file empty
	if(file_string.empty()) {
		std::string message = _("Empty file");
		throw editor_map_load_exception(filename, message);
	}

	// 0.3 Not a .map or .cfg file
	if(!filesystem::is_map(filename)
		&& !filesystem::is_mask(filename)
		&& !filesystem::is_cfg(filename))
	{
		std::string message = _("File does not have .map, .cfg, or .mask extension");
		throw editor_map_load_exception(filename, message);
	}

	// 1.0 Pure map data
	if(filesystem::is_map(filename)
		|| filesystem::is_mask(filename)) {
		LOG_ED << "Loading map or mask file";
		map_ = editor_map::from_string(file_string); // throws on error
		pure_map_ = true;

		add_to_recent_files();
	} else {
		// 4.0 old-style editor generated scenario which lacks a top-level tag
		if(file_string.find("[multiplayer]") == std::string::npos &&
			file_string.find("[scenario]") == std::string::npos &&
			file_string.find("[test]") == std::string::npos) {
			LOG_ED << "Loading generated scenario file";
			try {
				load_scenario();
			} catch(const std::exception& e) {
				throw editor_map_load_exception("load_scenario: old-style scenario", e.what());
			}
			add_to_recent_files();
		} else {
			std::string map_data_loc = get_map_location(file_string, "map_data");
			std::string map_file_loc = get_map_location(file_string, "map_file");

			if(!map_data_loc.empty()) {
				if(map_data_loc.find("\"{") == std::string::npos) {
					// 2.0 Embedded pure map
					LOG_ED << "Loading embedded map file";
					embedded_ = true;
					pure_map_ = true;
					std::size_t start = file_string.find(map_data_loc)+1;
					std::size_t length = file_string.find("\"", start)-start;
					std::string map_data = file_string.substr(start, length);
					map_ = editor_map::from_string(map_data);
					add_to_recent_files();
				} else {
					// 3.0 Macro referenced pure map
					const std::string& macro_argument = map_data_loc.substr(2, map_data_loc.size()-4);
					LOG_ED << "Map looks like a scenario, trying {" << macro_argument << "}";

					auto new_filename = filesystem::get_wml_location(macro_argument, filesystem::directory_name(filesystem::get_short_wml_path(filename_)));

					if(!new_filename) {
						std::string message = _("The map file looks like a scenario, but the map_data value does not point to an existing file")
											+ std::string("\n") + macro_argument;
						throw editor_map_load_exception(filename, message);
					}

					LOG_ED << "New filename is: " << new_filename.value();

					filename_ = new_filename.value();
					file_string = filesystem::read_file(filename_);
					map_ = editor_map::from_string(file_string);
					pure_map_ = true;

					add_to_recent_files();
				}
			} else if(!map_file_loc.empty()) {
				// 5.0 The file is using map_file.
				try {
					// 5.1 The file can be loaded by the editor as a scenario
					if(file_string.find("<<") != std::string::npos) {
						throw editor_map_load_exception(filename, _("Found the characters ‘<<’ indicating inline lua is present — aborting"));
					}
					load_scenario();
				} catch(const std::exception&) {
					// 5.2 The file can't be loaded by the editor as a scenario, so try to just load the map
					gui2::show_message(_("Error"), _("Failed to load the scenario, attempting to load only the map."), gui2::dialogs::message::auto_close);

					// NOTE: this means that loading the map file from a scenario where the maps are in nested directories under maps/ will not work
					//       this is done to address mainline scenarios referencing their maps as "multiplayer/maps/<map_file>.map"
					//       otherwise this results in the "multiplayer/maps/" part getting duplicated in the path and then not being found
					std::string new_filename = filesystem::get_current_editor_dir(addon_id_) + "/maps/" + filesystem::base_name(map_file_loc);
					if(!filesystem::file_exists(new_filename)) {
						std::string message = _("The map file looks like a scenario, but the map_file value does not point to an existing file")
											+ std::string("\n") + new_filename;
						throw editor_map_load_exception(filename, message);
					}

					LOG_ED << "New filename is: " << new_filename;

					filename_ = new_filename;
					file_string = filesystem::read_file(filename_);
					map_ = editor_map::from_string(file_string);
					pure_map_ = true;
				}

				add_to_recent_files();
			} else {
				throw editor_map_load_exception(filename, _("Unable to parse file to find map data"));
			}
		}
	}
}

map_context::~map_context()
{
	undo_stack_.clear();
	redo_stack_.clear();
}

void map_context::new_side()
{
	teams_.emplace_back();

	config cfg;
	cfg["side"] = teams_.size(); // side is 1-indexed, so we can just use size()
	cfg["hidden"] = false;
	cfg["gold"] = editor_team_default_gold;

	teams_.back().build(cfg, map());

	++actions_since_save_;
}

void map_context::set_side_setup(editor_team_info& info)
{
	assert(teams_.size() >= static_cast<unsigned int>(info.side));

	team& t = teams_[info.side - 1];
	t.change_team(info.id, info.name);
	t.set_recruits(utils::split_set(info.recruit_list, ','));
	t.have_leader(!info.no_leader);
	t.change_controller(info.controller);
	t.set_gold(info.gold);
	t.set_base_income(info.income);
	t.set_hidden(info.hidden);
	t.set_fog(info.fog);
	t.set_shroud(info.shroud);
	t.set_share_vision(info.share_vision);
	t.set_village_gold(info.village_income);
	t.set_village_support(info.village_support);

	++actions_since_save_;
}

void map_context::set_scenario_setup(const std::string& id,
		const std::string& name,
		const std::string& description,
		int turns,
		int xp_mod,
		bool victory_defeated,
		bool random_time)
{
	scenario_id_ = id;
	scenario_name_ = name;
	scenario_description_ = description;
	random_time_ = random_time;
	victory_defeated_ = victory_defeated;
	tod_manager_->set_number_of_turns(turns);
	xp_mod_ = xp_mod;
	++actions_since_save_;
}

void map_context::set_starting_time(int time)
{
	tod_manager_->set_current_time(time);
	if(!pure_map_) {
		++actions_since_save_;
	}
}

void map_context::remove_area(int index)
{
	tod_manager_->remove_time_area(index);
	active_area_--;
	++actions_since_save_;
}

void map_context::replace_schedule(const std::vector<time_of_day>& schedule)
{
	tod_manager_->replace_schedule(schedule);
	if(!pure_map_) {
		++actions_since_save_;
	}
}

void map_context::replace_local_schedule(const std::vector<time_of_day>& schedule)
{
	tod_manager_->replace_local_schedule(schedule, active_area_);
	if(!pure_map_) {
		++actions_since_save_;
	}
}

config map_context::convert_scenario(const config& old_scenario)
{
	config cfg;
	config& multiplayer = cfg.add_child("multiplayer");
	multiplayer.append_attributes(old_scenario);
	std::string map_data = multiplayer["map_data"];
	std::string separate_map_file = filesystem::get_current_editor_dir(addon_id_) + "/maps/" + filesystem::base_name(filename_, true) + filesystem::map_extension;

	// check that there's embedded map data, since that's how the editor used to save scenarios
	if(!map_data.empty()) {
		// check if a .map file already exists as a separate standalone .map in the editor folders or if a .map file already exists in the add-on
		if(filesystem::file_exists(separate_map_file)) {
			separate_map_file = filesystem::get_current_editor_dir(addon_id_) + "/maps/" + filesystem::get_next_filename(filesystem::base_name(filename_, true), filesystem::map_extension);
		}
		multiplayer["id"] = filesystem::base_name(separate_map_file, true);

		filesystem::write_file(separate_map_file, map_data);
		multiplayer.remove_attribute("map_data");
		multiplayer["map_file"] = filesystem::base_name(separate_map_file);
	} else {
		ERR_ED << "Cannot convert " << filename_ << " due to missing map_data attribute.";
		throw editor_map_load_exception("load_scenario: no embedded map_data attribute found in old-style scenario", filename_);
	}

	config& event = multiplayer.add_child("event");
	event["name"] = "prestart";
	event["id"] = "editor_event-prestart";

	// for all children that aren't [side] or [time], move them to an event
	// for [side]:
	//   keep all attributes in [side]
	//   also keep any [village]s in [side]
	//   move all other children to the start [event]
	//   if [unit], set the unit's side
	// for [time]:
	//   keep under [multiplayer]
	for(const auto [child_key, child_cfg]: old_scenario.all_children_view()) {
		if(child_key != "side" && child_key != "time") {
			config& c = event.add_child(child_key);
			c.append_attributes(child_cfg);
			c.append_children(child_cfg);
		} else if(child_key == "side") {
			config& c = multiplayer.add_child("side");
			c.append_attributes(child_cfg);
			for(const auto [side_key, side_cfg] : child_cfg.all_children_view()) {
				if(side_key == "village") {
					config& c1 = c.add_child("village");
					c1.append_attributes(side_cfg);
				} else {
					config& c1 = event.add_child(side_key);
					c1.append_attributes(side_cfg);
					if(side_key == "unit") {
						c1["side"] = child_cfg["side"];
					}
				}
			}
		} else if(child_key == "time") {
			config& c = multiplayer.add_child("time");
			c.append_attributes(child_cfg);
		}
	}

	return cfg;
}

void map_context::load_scenario()
{
	config scen;
	read(scen, *(preprocess_file(filename_)));

	config scenario;
	if(scen.has_child("scenario")) {
		scenario = scen.mandatory_child("scenario");
	} else if(scen.has_child("multiplayer")) {
		scenario = scen.mandatory_child("multiplayer");
	} else if(scen.has_child("test")) {
		scenario = scen.mandatory_child("test");
	} else {
		ERR_ED << "Found no [scenario], [multiplayer], or [test] tag in " << filename_ << ", assuming old-style editor scenario and defaulting to [multiplayer]";
		scen = convert_scenario(scen);
		scenario = scen.mandatory_child("multiplayer");
	}

	scenario_id_ = scenario["id"].str();
	scenario_name_ = scenario["name"].str();
	scenario_description_ = scenario["description"].str();

	if(const config::attribute_value* experience_modifier = scenario.get("experience_modifier")) {
		xp_mod_ = experience_modifier->to_int();
	}
	victory_defeated_ = scenario["victory_when_enemies_defeated"].to_bool(true);
	random_time_ = scenario["random_start_time"].to_bool(false);

	if(!scenario["map_data"].str().empty()) {
		map_ = editor_map::from_string(scenario["map_data"]); // throws on error
	} else if(!scenario["map_file"].str().empty()) {
		map_ = editor_map::from_string(filesystem::read_file(filesystem::get_current_editor_dir(addon_id_) + "/maps/" + filesystem::base_name(scenario["map_file"]))); // throws on error
	} else {
		throw editor_map_load_exception("load_scenario: no map_file or map_data attribute found", filename_);
	}

	for(config& side : scenario.child_range("side")) {
		teams_.emplace_back();
		teams_.back().build(side, map_);
		if(!side["recruit"].str().empty()) {
			teams_.back().set_recruits(utils::split_set(side["recruit"].str(), ','));
		}
	}

	tod_manager_.reset(new tod_manager(scenario));

	auto event = scenario.find_child("event", "id", "editor_event-start");
	if(!event) {
		event = scenario.find_child("event", "id", "editor_event-prestart");
	}
	if(event) {
		config& evt = event.value();

		labels_.read(evt);

		for(const config& time_area : evt.child_range("time_area")) {
			tod_manager_->add_time_area(map_, time_area);
		}

		for(const config& item : evt.child_range("item")) {
			const map_location loc(item);
			overlays_[loc].push_back(overlay(item));
		}

		for(const config& music : evt.child_range("music")) {
			music_tracks_.emplace(music["name"], sound::music_track(music));
		}

		for(config& a_unit : evt.child_range("unit")) {
			units_.insert(unit::create(a_unit, true));
		}
	}

	previous_cfg_ = scen;
}

bool map_context::select_area(int index)
{
	return map_.set_selection(tod_manager_->get_area_by_index(index));
}

void map_context::draw_terrain(const t_translation::terrain_code& terrain, const map_location& loc, bool one_layer_only)
{
	t_translation::terrain_code full_terrain = one_layer_only
		? terrain
		: map_.get_terrain_info(terrain).terrain_with_default_base();

	draw_terrain_actual(full_terrain, loc, one_layer_only);
}

void map_context::draw_terrain_actual(
		const t_translation::terrain_code& terrain, const map_location& loc, bool one_layer_only)
{
	if(!map_.on_board_with_border(loc)) {
		// requests for painting off the map are ignored in set_terrain anyway,
		// but ideally we should not have any
		LOG_ED << "Attempted to draw terrain off the map (" << loc << ")";
		return;
	}

	t_translation::terrain_code old_terrain = map_.get_terrain(loc);

	if(terrain != old_terrain) {
		if(terrain.base == t_translation::NO_LAYER) {
			map_.set_terrain(loc, terrain, terrain_type_data::OVERLAY);
		} else if(one_layer_only) {
			map_.set_terrain(loc, terrain, terrain_type_data::BASE);
		} else {
			map_.set_terrain(loc, terrain);
		}

		add_changed_location(loc);
	}
}

void map_context::draw_terrain(
		const t_translation::terrain_code& terrain, const std::set<map_location>& locs, bool one_layer_only)
{
	t_translation::terrain_code full_terrain = one_layer_only
		? terrain
		: map_.get_terrain_info(terrain).terrain_with_default_base();

	for(const map_location& loc : locs) {
		draw_terrain_actual(full_terrain, loc, one_layer_only);
	}
}

void map_context::clear_changed_locations()
{
	everything_changed_ = false;
	changed_locations_.clear();
}

void map_context::add_changed_location(const map_location& loc)
{
	if(!everything_changed()) {
		changed_locations_.insert(loc);
	}
}

void map_context::add_changed_location(const std::set<map_location>& locs)
{
	if(!everything_changed()) {
		changed_locations_.insert(locs.begin(), locs.end());
	}
}

void map_context::set_everything_changed()
{
	everything_changed_ = true;
}

bool map_context::everything_changed() const
{
	return everything_changed_;
}

void map_context::clear_starting_position_labels(display& disp)
{
	disp.labels().clear_all();
	starting_position_label_locs_.clear();
}

void map_context::set_starting_position_labels(display& disp)
{
	std::set<map_location> new_label_locs = map_.set_starting_position_labels(disp);
	starting_position_label_locs_.insert(new_label_locs.begin(), new_label_locs.end());
}

void map_context::reset_starting_position_labels(display& disp)
{
	clear_starting_position_labels(disp);
	set_starting_position_labels(disp);
	set_needs_labels_reset(false);
}

config map_context::to_config()
{
	config scen;

	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	// the state of the previous scenario cfg
	// if it exists, alter specific parts of it (sides, times, and editor events) rather than replacing it entirely
	if(previous_cfg_) {
		scen = *previous_cfg_;
	}

	// if this has [multiplayer], use [multiplayer]
	// else if this has [scenario], use [scenario]
	// else if this has [test], use [test]
	// else if none, add a [multiplayer]
	config& scenario = scen.has_child("multiplayer")
		? scen.mandatory_child("multiplayer")
		: scen.has_child("scenario")
			? scen.mandatory_child("scenario")
			: scen.has_child("test")
				? scen.mandatory_child("test")
				: scen.add_child("multiplayer");

	scenario.remove_children("side");
	scenario.remove_children("event", [](const config& cfg) {
		return cfg["id"].str() == "editor_event-start" || cfg["id"].str() == "editor_event-prestart";
	});

	scenario["id"] = scenario_id_;
	scenario["name"] = t_string(scenario_name_, current_textdomain);
	scenario["description"] = t_string(scenario_description_, current_textdomain);

	if(xp_mod_) {
		scenario["experience_modifier"] = *xp_mod_;
	}
	if(victory_defeated_) {
		scenario["victory_when_enemies_defeated"] = *victory_defeated_;
	}
	scenario["random_start_time"] = random_time_;

	// write out the map data
	scenario["map_file"] = scenario_id_ + filesystem::map_extension;
	filesystem::write_file(filesystem::get_current_editor_dir(addon_id_) + "/maps/" + scenario_id_ + filesystem::map_extension, map_.write());

	// find or add the editor's start event
	config& event = scenario.add_child("event");
	event["name"] = "prestart";
	event["id"] = "editor_event-prestart";
	event["priority"] = 1000;

	// write out all the scenario data below

	// [time]s and [time_area]s
	// put the [time_area]s into the event to keep as much editor-specific stuff separated in its own event as possible
	config times = tod_manager_->to_config(current_textdomain);
	times.remove_attribute("turn_at");
	times.remove_attribute("it_is_a_new_turn");
	if(scenario["turns"].to_int() == -1) {
		times.remove_attribute("turns");
	} else {
		scenario["turns"] = times["turns"];
	}

	for(const config& time : times.child_range("time")) {
		config& t = scenario.add_child("time");
		t.append(time);
	}
	for(const config& time_area : times.child_range("time_area")) {
		config& t = event.add_child("time_area");
		t.append(time_area);
	}

	// [label]s
	labels_.write(event);

	// [item]s
	for(const auto& overlay_pair : overlays_) {
		for(const overlay& o : overlay_pair.second) {
			config& item = event.add_child("item");

			// Write x,y location
			overlay_pair.first.write(item);

			// These should always have a value
			item["image"] = o.image;
			item["visible_in_fog"] = o.visible_in_fog;

			// Optional keys
			item["id"].write_if_not_empty(o.id);
			item["name"].write_if_not_empty(t_string(o.name, current_textdomain));
			item["team_name"].write_if_not_empty(o.team_name);
			item["halo"].write_if_not_empty(o.halo);
			if(o.submerge) {
				item["submerge"] = o.submerge;
			}
		}
	}

	// [music]s
	for(const music_map::value_type& track : music_tracks_) {
		track.second.write(event, true);
	}

	// [unit]s
	config traits;
	preproc_map traits_map;
	read(traits, *(preprocess_file(game_config::path + "/data/core/macros/traits.cfg", &traits_map)));

	for(const auto& unit : units_) {
		config& u = event.add_child("unit");

		unit.get_location().write(u);

		u["side"] = unit.side();
		u["type"] = unit.type_id();
		u["name"].write_if_not_empty(t_string(unit.name(), current_textdomain));
		u["facing"] = map_location::write_direction(unit.facing());

		if(!boost::regex_match(unit.id(), boost::regex(".*-[0-9]+"))) {
			u["id"] = unit.id();
		}

		if(unit.can_recruit()) {
			u["canrecruit"] = unit.can_recruit();
		}

		if(unit.unrenamable()) {
			u["unrenamable"] = unit.unrenamable();
		}

		config& mods = u.add_child("modifications");
		if(unit.loyal()) {
			config trait_loyal;
			read(trait_loyal, preprocess_string("{TRAIT_LOYAL}", &traits_map, "wesnoth-help"));
			mods.append(trait_loyal);
		}
		//TODO this entire block could also be replaced by unit.write(u, true)
		//however, the resultant config is massive and contains many attributes we don't need.
		//need to find a middle ground here.
	}

	// [side]s
	for(const auto& team : teams_) {
		config& side = scenario.add_child("side");

		side["side"] = scenario.child_count("side");
		side["hidden"] = team.hidden();

		side["controller"] = side_controller::get_string(team.controller());
		side["no_leader"] = team.no_leader();

		side["team_name"] = team.team_name();
		side["user_team_name"].write_if_not_empty(t_string(team.user_team_name(), current_textdomain));
		if(team.recruits().size() > 0) {
			side["recruit"] = utils::join(team.recruits(), ",");
			side["faction"] = "Custom";
		}

		side["fog"] = team.uses_fog();
		side["shroud"] = team.uses_shroud();
		side["share_vision"] = team_shared_vision::get_string(team.share_vision());

		side["gold"] = team.gold();
		side["income"] = team.base_income();

		for(const map_location& village : team.villages()) {
			village.write(side.add_child("village"));
		}
	}

	previous_cfg_ = scen;
	return scen;
}

void map_context::save_schedule(const std::string& schedule_id, const std::string& schedule_name)
{
	// Textdomain
	std::string current_textdomain = "wesnoth-"+addon_id_;

	// Path to schedule.cfg
	std::string schedule_path = filesystem::get_current_editor_dir(addon_id_) + "/utils/schedule.cfg";

	// Create schedule config
	config schedule;
	try {
		if (filesystem::file_exists(schedule_path)) {
			/* If exists, read the schedule.cfg
			 * and insert [editor_times] block at correct place */
			preproc_map editor_map;
			editor_map["EDITOR"] = preproc_define("true");
			read(schedule, *(preprocess_file(schedule_path, &editor_map)));
		}
	} catch(const filesystem::io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = VGETTEXT("Could not save time schedule: $msg", symbols);
		throw editor_map_save_exception(msg);
	}

	config& editor_times = schedule.add_child("editor_times");

	editor_times["id"] = schedule_id;
	editor_times["name"] = t_string(schedule_name, current_textdomain);
	config times = tod_manager_->to_config(current_textdomain);
	for(const config& time : times.child_range("time")) {
		config& t = editor_times.add_child("time");
		t.append(time);
	}

	// Write to file
	try {
		std::stringstream wml_stream;

		wml_stream
			<< "#textdomain " << current_textdomain << "\n"
			<< "#\n"
			<< "# This file was generated using the scenario editor.\n"
			<< "#\n"
			<< "#ifdef EDITOR\n";

		{
			config_writer out(wml_stream, false);
			out.write(schedule);
		}

		wml_stream << "#endif";

		if(!wml_stream.str().empty()) {
			filesystem::write_file(schedule_path, wml_stream.str());
			gui2::show_transient_message("", _("Time schedule saved."));
		}

	} catch(const filesystem::io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = VGETTEXT("Could not save time schedule: $msg", symbols);
		throw editor_map_save_exception(msg);
	}
}

void map_context::save_scenario()
{
	assert(!is_embedded());

	if(scenario_id_.empty()) {
		scenario_id_ = filesystem::base_name(filename_, true);
	}

	if(scenario_name_.empty()) {
		scenario_name_ = scenario_id_;
	}

	try {
		std::stringstream wml_stream;
		wml_stream
			<< "# This file was generated using the scenario editor.\n"
			<< "#\n"
			<< "# If you edit this file by hand, then do not use macros.\n"
			<< "# The editor doesn't support macros, and so using them will result in only being able to edit the map.\n"
			<< "# Additionally, the contents of all [side] and [time] tags as well as any events that have an id starting with 'editor_event-' are replaced entirely.\n"
			<< "# Any manual changes made to those will be lost.\n"
			<< "\n";
		{
			config_writer out(wml_stream, false);
			out.write(to_config());
		}

		if(!wml_stream.str().empty()) {
			filesystem::write_file(get_filename(), wml_stream.str());
		}

		clear_modified();
	} catch(const filesystem::io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = VGETTEXT("Could not save the scenario: $msg", symbols);

		throw editor_map_save_exception(msg);
	}

	// After saving the map as a scenario, it's no longer a pure map.
	pure_map_ = false;
}

void map_context::save_map()
{
	std::string map_data = map_.write();

	try {
		if(!is_embedded()) {
			filesystem::write_file(get_filename(), map_data);
		} else {
			std::string map_string = filesystem::read_file(get_filename());

			boost::regex rexpression_map_data(R"((.*map_data\s*=\s*")(.+?)(".*))");
			boost::smatch matched_map_data;

			if(boost::regex_search(map_string, matched_map_data, rexpression_map_data,
					   boost::regex_constants::match_not_dot_null)) {
				std::stringstream ss;
				ss << matched_map_data[1];
				ss << map_data;
				ss << matched_map_data[3];

				filesystem::write_file(get_filename(), ss.str());
			} else {
				throw editor_map_save_exception(_("Could not save into scenario"));
			}
		}

		add_to_recent_files();

		clear_modified();
	} catch(const filesystem::io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = VGETTEXT("Could not save the map: $msg", symbols);

		throw editor_map_save_exception(msg);
	}
}

void map_context::set_map(const editor_map& map)
{
	if(map_.h() != map.h() || map_.w() != map.w()) {
		set_needs_reload();
	} else {
		set_needs_terrain_rebuild();
	}

	map_ = map;
}

void map_context::perform_action(const editor_action& action)
{
	LOG_ED << "Performing action " << action.get_id() << ": " << action.get_name() << ", actions count is "
		   << action.get_instance_count();
	auto undo = action.perform(*this);
	if(actions_since_save_ < 0) {
		// set to a value that will make it impossible to get to zero, as at this point
		// it is no longer possible to get back the original map state using undo/redo
		actions_since_save_ = 1 + undo_stack_.size();
	}

	++actions_since_save_;

	undo_stack_.emplace_back(std::move(undo));

	trim_stack(undo_stack_);

	redo_stack_.clear();
}

void map_context::perform_partial_action(const editor_action& action)
{
	LOG_ED << "Performing (partial) action " << action.get_id() << ": " << action.get_name() << ", actions count is "
		   << action.get_instance_count();
	if(!can_undo()) {
		throw editor_logic_exception("Empty undo stack in perform_partial_action()");
	}

	editor_action_chain* undo_chain = dynamic_cast<editor_action_chain*>(last_undo_action());
	if(undo_chain == nullptr) {
		throw editor_logic_exception("Last undo action not a chain in perform_partial_action()");
	}

	auto undo = action.perform(*this);

	// actions_since_save_ += action.action_count();
	undo_chain->prepend_action(std::move(undo));

	redo_stack_.clear();
}

bool map_context::modified() const
{
	return actions_since_save_ != 0;
}

void map_context::clear_modified()
{
	actions_since_save_ = 0;
}

void map_context::add_to_recent_files()
{
	prefs::get().add_recent_files_entry(get_filename());
}

bool map_context::can_undo() const
{
	return !undo_stack_.empty();
}

bool map_context::can_redo() const
{
	return !redo_stack_.empty();
}

editor_action* map_context::last_undo_action()
{
	return undo_stack_.empty() ? nullptr : undo_stack_.back().get();
}

editor_action* map_context::last_redo_action()
{
	return redo_stack_.empty() ? nullptr : redo_stack_.back().get();
}

const editor_action* map_context::last_undo_action() const
{
	return undo_stack_.empty() ? nullptr : undo_stack_.back().get();
}

const editor_action* map_context::last_redo_action() const
{
	return redo_stack_.empty() ? nullptr : redo_stack_.back().get();
}

void map_context::undo()
{
	LOG_ED << "undo() beg, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size();

	if(can_undo()) {
		perform_action_between_stacks(undo_stack_, redo_stack_);
		actions_since_save_--;
	} else {
		WRN_ED << "undo() called with an empty undo stack";
	}

	LOG_ED << "undo() end, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size();
}

void map_context::redo()
{
	LOG_ED << "redo() beg, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size();

	if(can_redo()) {
		perform_action_between_stacks(redo_stack_, undo_stack_);
		++actions_since_save_;
	} else {
		WRN_ED << "redo() called with an empty redo stack";
	}

	LOG_ED << "redo() end, undo stack is " << undo_stack_.size() << ", redo stack " << redo_stack_.size();
}

void map_context::partial_undo()
{
	// callers should check for these conditions
	if(!can_undo()) {
		throw editor_logic_exception("Empty undo stack in partial_undo()");
	}

	editor_action_chain* undo_chain = dynamic_cast<editor_action_chain*>(last_undo_action());
	if(undo_chain == nullptr) {
		throw editor_logic_exception("Last undo action not a chain in partial undo");
	}

	// a partial undo performs the first action form the current action's action_chain that would be normally performed
	// i.e. the *first* one.
	const auto first_action_in_chain = undo_chain->pop_first_action();
	if(undo_chain->empty()) {
		actions_since_save_--;
		undo_stack_.pop_back();
	}

	redo_stack_.emplace_back(first_action_in_chain->perform(*this));
	// actions_since_save_ -= last_redo_action()->action_count();
}

void map_context::clear_undo_redo()
{
	undo_stack_.clear();
	redo_stack_.clear();
}

void map_context::trim_stack(action_stack& stack)
{
	if(stack.size() > max_action_stack_size_) {
		stack.pop_front();
	}
}

void map_context::perform_action_between_stacks(action_stack& from, action_stack& to)
{
	assert(!from.empty());

	std::unique_ptr<editor_action> action;
	action.swap(from.back());

	from.pop_back();

	auto reverse_action = action->perform(*this);
	to.emplace_back(std::move(reverse_action));

	trim_stack(to);
}

const t_string map_context::get_default_context_name() const
{
	return is_pure_map() ? _("New Map") : _("New Scenario");
}

} // end namespace editor
