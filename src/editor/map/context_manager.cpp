/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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

#include "resources.hpp"
#include "team.hpp"

#include "addon/validation.hpp"

#include "editor/map/context_manager.hpp"
#include "editor/map/map_context.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "generators/map_create.hpp"
#include "generators/map_generator.hpp"
#include "gettext.hpp"
#include "video.hpp"

#include "editor/action/action.hpp"
#include "editor/controller/editor_controller.hpp"
#include "preferences/preferences.hpp"

#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/prompt.hpp"
#include "gui/dialogs/editor/generate_map.hpp"
#include "gui/dialogs/editor/new_map.hpp"
#include "gui/dialogs/editor/resize_map.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/retval.hpp"

#include "gui/dialogs/editor/edit_scenario.hpp"
#include "gui/dialogs/editor/edit_side.hpp"
#include "gui/dialogs/editor/edit_pbl.hpp"
#include "game_config_view.hpp"

#include "serialization/markup.hpp"
#include "terrain/translation.hpp"

#include <memory>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace {

std::vector<std::unique_ptr<editor::map_context>> saved_contexts_;
int last_context_ = 0;

const std::string get_menu_marker(const bool changed)
{
	if (changed) {
		return "[" + markup::span_color("#f00", font::unicode_bullet) + "]";
	} else {
		return font::unicode_bullet;
	}
}

}

namespace editor {

context_manager::context_manager(editor_display& gui, const game_config_view& game_config, const std::string& addon_id)
	: locs_(nullptr)
	, gui_(gui)
	, game_config_(game_config)
	, default_dir_(filesystem::get_dir(filesystem::get_legacy_editor_dir()))
	, current_addon_(addon_id)
	, map_generators_()
	, last_map_generator_(nullptr)
	, current_context_index_(0)
	, auto_update_transitions_(prefs::get().editor_auto_update_transitions())
	, map_contexts_()
	, clipboard_()
{
	resources::filter_con = this;
	create_default_context();
	init_map_generators(game_config);
}

context_manager::~context_manager()
{
	// Restore default window title
	video::set_window_title(game_config::get_default_title_string());

	resources::filter_con = nullptr;
}

void context_manager::refresh_on_context_change()
{
	gui().change_display_context(&get_map_context());

	// TODO register the tod_manager with the gui?
	resources::tod_manager = get_map_context().get_time_manager();
	resources::classification = &get_map_context().get_classification();

	// Reset side when switching to an existing scenario
	if(!get_map_context().teams().empty()) {
		gui().set_viewing_team_index(0, true);
		gui().set_playing_team_index(0);
	}
	gui().init_flags();

	reload_map();

	// Enable the labels of the current context;
	get_map_context().get_labels().enable(true);

	set_window_title();
}

void context_manager::refresh_all()
{
	gui_.rebuild_all();
	get_map_context().set_needs_terrain_rebuild(false);
	gui_.create_buttons();
	gui_.invalidate_all();
	get_map_context().clear_changed_locations();
	gui_.recalculate_minimap();
	if(locs_) {
		for(const auto& loc : get_map_context().map().special_locations().left) {
			locs_->add_item(loc.first);
		}
		if(!get_map_context().is_pure_map()) {
			// If the scenario has more than 9 teams, add locations for them
			// (First 9 teams are always in the list)
			size_t n_teams = get_map_context().teams().size();
			for(size_t i = 10; i <= n_teams; i++) {
				locs_->add_item(std::to_string(i));
			}
		}
	}
}

void context_manager::reload_map()
{
	gui_.reload_map();
	get_map_context().set_needs_reload(false);
	get_map_context().reset_starting_position_labels(gui_);
	refresh_all();
}

bool context_manager::is_active_transitions_hotkey(const std::string& item)
{
	switch (auto_update_transitions_) {
		case pref_constants::TRANSITION_UPDATE_ON:
			return (item == "editor-auto-update-transitions");
		case pref_constants::TRANSITION_UPDATE_PARTIAL:
			return (item == "editor-partial-update-transitions");
		case pref_constants::TRANSITION_UPDATE_OFF:
			return (item == "editor-no-update-transitions");
	}

	return true; //should not be reached
}

bool context_manager::toggle_update_transitions()
{
	auto_update_transitions_ = (auto_update_transitions_ + 1) % pref_constants::TRANSITION_UPDATE_COUNT;
	prefs::get().set_editor_auto_update_transitions(auto_update_transitions_);

	if(auto_update_transitions_ != pref_constants::TRANSITION_UPDATE_ON) {
		return true;
	}

	return false;
}

std::size_t context_manager::modified_maps(std::string& message)
{
	std::vector<std::string> modified;
	for(auto& mc : map_contexts_) {
		if(mc->modified()) {
			if(!mc->get_name().empty()) {
				modified.push_back(mc->get_name());
			} else if(!mc->get_filename().empty()) {
				modified.push_back(mc->get_filename());
			} else {
				modified.push_back(mc->get_default_context_name());
			}
		}
	}

	for(std::string& str : modified) {
		message += "\n" + font::unicode_bullet + " " + str;
	}

	return modified.size();
}

void context_manager::load_map_dialog(bool force_same_context /* = false */)
{
	std::string fn = get_map_context().get_filename();
	if(fn.empty()) {
		if (editor_controller::current_addon_id_.empty()) {
			fn = filesystem::get_legacy_editor_dir() + "/maps";
		} else {
			fn = filesystem::get_current_editor_dir(editor_controller::current_addon_id_) + "/maps";
		}
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Load Map"))
	   .set_path(fn);

	if(dlg.show()) {
		load_map(dlg.path(), !force_same_context);
	}
}

void context_manager::load_mru_item(unsigned index, bool force_same_context /* = false */)
{
	const std::vector<std::string>& mru = prefs::get().recent_files();
	if(mru.empty() || index >= mru.size()) {
		return;
	}

	load_map(mru[index], !force_same_context);
}

void context_manager::edit_side_dialog(const team& t)
{
	editor_team_info team_info(t);

	if(gui2::dialogs::editor_edit_side::execute(team_info)) {
		get_map_context().set_side_setup(team_info);
	}
}

void context_manager::edit_pbl()
{
	if(!current_addon_.empty()) {
		std::string pbl = filesystem::get_current_editor_dir(current_addon_) + "/_server.pbl";
		gui2::dialogs::editor_edit_pbl::execute(pbl, current_addon_);
	}
}

void context_manager::change_addon_id()
{
	std::string new_addon_id = current_addon_;
	gui2::dialogs::prompt::execute(new_addon_id);

	std::string old_dir = filesystem::get_current_editor_dir(current_addon_);
	std::string new_dir = filesystem::get_current_editor_dir(new_addon_id);
	if(addon_filename_legal(new_addon_id) && filesystem::rename_dir(old_dir, new_dir)) {
		std::string main_cfg = new_dir + "/_main.cfg";
		std::string main = filesystem::read_file(main_cfg);

		// update paths
		boost::replace_all(main, "/"+current_addon_, "/"+new_addon_id);
		// update textdomain
		boost::replace_all(main, "wesnoth-"+current_addon_, "wesnoth-"+new_addon_id);
		filesystem::write_file(main_cfg, main);

		current_addon_ = new_addon_id;

		for(std::unique_ptr<map_context>& context : map_contexts_) {
			context->set_addon_id(current_addon_);
		}
	}
}

void context_manager::edit_scenario_dialog()
{
	map_context& context = get_map_context();

	std::string id          = context.get_id();
	std::string name        = context.get_name();
	std::string description = context.get_description();

	int turns  = context.get_time_manager()->number_of_turns();
	int xp_mod = context.get_xp_mod() ? *context.get_xp_mod() : 70;

	bool victory = context.victory_defeated();
	bool random  = context.random_start_time();

	const bool ok = gui2::dialogs::editor_edit_scenario::execute(
		id, name, description, turns, xp_mod, victory, random
	);

	if(!ok) {
		return;
	}

	context.set_scenario_setup(id, name, description, turns, xp_mod, victory, random);

	if(!name.empty()) {
		set_window_title();
	}
}

void context_manager::new_map_dialog()
{
	const editor_map& map = get_map_context().map();

	int w = map.w();
	int h = map.h();

	if(gui2::dialogs::editor_new_map::execute(_("New Map"), w, h)) {
		const t_translation::terrain_code& fill = get_selected_bg_terrain();
		new_map(w, h, fill, true);
	}
}

void context_manager::new_scenario_dialog()
{
	const editor_map& map = get_map_context().map();

	int w = map.w();
	int h = map.h();

	if(gui2::dialogs::editor_new_map::execute(_("New Scenario"), w, h)) {
		const t_translation::terrain_code& fill = get_selected_bg_terrain();
		new_scenario(w, h, fill, true);
	}
}

void context_manager::expand_open_maps_menu(std::vector<config>& items, int i)
{
	auto pos = items.erase(items.begin() + i);
	std::vector<config> contexts;

	for(std::size_t mci = 0; mci < map_contexts_.size(); ++mci) {
		map_context& mc = *map_contexts_[mci];

		std::string filename;
		if(mc.is_pure_map()) {
			filename = filesystem::base_name(mc.get_filename());
		} else {
			filename = mc.get_name();
		}

		if(filename.empty()) {
			filename = mc.get_default_context_name();
		}

		std::ostringstream ss;
		ss << "[" << mci + 1 << "] ";

		const bool changed = mc.modified();

		if(changed) {
			ss << markup::italic(filename);
		} else {
			ss << filename;
		}

		if(mc.is_embedded()) {
			ss << " (E)";
		}

		const std::string label = ss.str();
		const std::string details = get_menu_marker(changed);

		contexts.emplace_back("label", label, "details", details);
	}

	items.insert(pos, contexts.begin(), contexts.end());
}

void context_manager::expand_load_mru_menu(std::vector<config>& items, int i)
{
	std::vector<std::string> mru = prefs::get().recent_files();

	auto pos = items.erase(items.begin() + i);

	if(mru.empty()) {
		items.insert(pos, config {"label", _("No Recent Files")});
		return;
	}

	for(std::string& path : mru) {
		// TODO: add proper leading ellipsization instead, since otherwise
		// it'll be impossible to tell apart files with identical names and
		// different parent paths.
		path = filesystem::base_name(path);
	}

	std::vector<config> temp;
	std::transform(mru.begin(), mru.end(), std::back_inserter(temp), [](const std::string& str) {
		return config {"label", str};
	});

	items.insert(pos, temp.begin(), temp.end());
}

void context_manager::expand_areas_menu(std::vector<config>& items, int i)
{
	tod_manager* tod = get_map_context().get_time_manager();
	if(!tod) {
		return;
	}

	auto pos = items.erase(items.begin() + i);
	std::vector<config> area_entries;

	std::vector<std::string> area_ids = tod->get_area_ids();

	for(std::size_t mci = 0; mci < area_ids.size(); ++mci) {
		const std::string& area = area_ids[mci];

		std::stringstream ss;
		ss << "[" << mci + 1 << "] ";\

		if(area.empty()) {
			ss << markup::italic(_("Unnamed Area"));
		} else {
			ss << area;
		}

		const bool changed =
			mci == static_cast<std::size_t>(get_map_context().get_active_area())
			&& tod->get_area_by_index(mci) != get_map_context().map().selection();

		const std::string label = ss.str();
		const std::string details = get_menu_marker(changed);

		area_entries.emplace_back("label", label, "details", details);
	}

	items.insert(pos, area_entries.begin(), area_entries.end());
}

void context_manager::expand_sides_menu(std::vector<config>& items, int i)
{
	auto pos = items.erase(items.begin() + i);
	std::vector<config> contexts;

	for(std::size_t mci = 0; mci < get_map_context().teams().size(); ++mci) {

		const team& t = get_map_context().teams()[mci];
		const std::string& teamname = t.user_team_name();
		std::stringstream label;
		label << "[" << mci+1 << "] ";

		if(teamname.empty()) {
			label << markup::italic(_("New Side"));
		} else {
			label << teamname;
		}

		contexts.emplace_back("label", label.str());
	}

	items.insert(pos, contexts.begin(), contexts.end());
}

void context_manager::expand_time_menu(std::vector<config>& items, int i)
{
	auto pos = items.erase(items.begin() + i);
	std::vector<config> times;

	tod_manager* tod_m = get_map_context().get_time_manager();

	assert(tod_m != nullptr);

	for(const time_of_day& time : tod_m->times()) {
		times.emplace_back(
			"details", time.name, // Use 'details' field here since the image will take the first column
			"image", time.image
		);
	}

	items.insert(pos, times.begin(), times.end());
}

void context_manager::expand_local_time_menu(std::vector<config>& items, int i)
{
	auto pos = items.erase(items.begin() + i);
	std::vector<config> times;

	tod_manager* tod_m = get_map_context().get_time_manager();

	for(const time_of_day& time : tod_m->times(get_map_context().get_active_area())) {
		times.emplace_back(
			"details", time.name, // Use 'details' field here since the image will take the first column
			"image", time.image
		);
	}

	items.insert(pos, times.begin(), times.end());
}

void context_manager::apply_mask_dialog()
{
	std::string fn = get_map_context().get_filename();
	if(fn.empty()) {
		fn = filesystem::get_dir(filesystem::get_current_editor_dir(current_addon_) + "/masks");
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Apply Mask"))
	   .set_path(fn);

	if(dlg.show()) {
		try {
			map_context mask(game_config_, dlg.path(), current_addon_);
			editor_action_apply_mask a(mask.map());
			perform_refresh(a);
		} catch (const editor_map_load_exception& e) {
			gui2::show_transient_message(_("Error loading mask"), e.what());
			return;
		} catch (const editor_action_exception& e) {
			gui2::show_error_message(e.what());
			return;
		}
	}
}

void context_manager::perform_refresh(const editor_action& action, bool drag_part /* =false */)
{
	get_map_context().perform_action(action);
	refresh_after_action(drag_part);
}

void context_manager::rename_area_dialog()
{
	int active_area  = get_map_context().get_active_area();
	std::string name = get_map_context().get_time_manager()->get_area_ids()[active_area];

	if(gui2::dialogs::edit_text::execute(N_("Rename Area"), N_("Identifier:"), name)) {
		get_map_context().get_time_manager()->set_area_id(active_area, name);
	}
}

void context_manager::create_mask_to_dialog()
{
	std::string fn = get_map_context().get_filename();
	if(fn.empty()) {
		fn = filesystem::get_dir(filesystem::get_current_editor_dir(current_addon_) + "/masks");
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Choose Target Map"))
	   .set_path(fn);

	if(dlg.show()) {
		try {
			map_context map(game_config_, dlg.path(), current_addon_);
			editor_action_create_mask a(map.map());
			perform_refresh(a);
		} catch (const editor_map_load_exception& e) {
			gui2::show_transient_message(_("Error loading map"), e.what());
			return;
		} catch (const editor_action_exception& e) {
			gui2::show_error_message(e.what());
			return;
		}
	}
}

void context_manager::refresh_after_action(bool drag_part)
{
	if(get_map_context().needs_reload()) {
		reload_map();
		return;
	}

	const std::set<map_location>& changed_locs = get_map_context().changed_locations();

	if(get_map_context().needs_terrain_rebuild()) {
		if((auto_update_transitions_ == pref_constants::TRANSITION_UPDATE_ON)
		|| ((auto_update_transitions_ == pref_constants::TRANSITION_UPDATE_PARTIAL)
		&& (!drag_part || get_map_context().everything_changed())))
		{
			gui_.rebuild_all();
			get_map_context().set_needs_terrain_rebuild(false);
			gui_.invalidate_all();
		} else {
			for(const map_location& loc : changed_locs) {
				gui_.rebuild_terrain(loc);
			}
			gui_.invalidate(changed_locs);
		}
	} else {
		if(get_map_context().everything_changed()) {
			gui_.invalidate_all();
		} else {
			gui_.invalidate(changed_locs);
		}
	}

	if(get_map_context().needs_labels_reset()) {
		get_map_context().reset_starting_position_labels(gui_);
	}

	get_map_context().clear_changed_locations();
	gui_.recalculate_minimap();
}

void context_manager::resize_map_dialog()
{
	const editor_map& map = get_map_context().map();

	int w = map.w();
	int h = map.h();

	gui2::dialogs::editor_resize_map::EXPAND_DIRECTION dir = gui2::dialogs::editor_resize_map::EXPAND_DIRECTION();
	bool copy = false;

	if(!gui2::dialogs::editor_resize_map::execute(w, h, dir, copy)) {
		return;
	}

	if(w != map.w() || h != map.h()) {
		t_translation::terrain_code fill = get_selected_bg_terrain();
		if(copy) {
			fill = t_translation::NONE_TERRAIN;
		}

		int x_offset = map.w() - w;
		int y_offset = map.h() - h;

		switch (dir) {
			case gui2::dialogs::editor_resize_map::EXPAND_BOTTOM_RIGHT:
			case gui2::dialogs::editor_resize_map::EXPAND_BOTTOM:
			case gui2::dialogs::editor_resize_map::EXPAND_BOTTOM_LEFT:
				y_offset = 0;
				break;
			case gui2::dialogs::editor_resize_map::EXPAND_RIGHT:
			case gui2::dialogs::editor_resize_map::EXPAND_CENTER:
			case gui2::dialogs::editor_resize_map::EXPAND_LEFT:
				y_offset /= 2;
				break;
			case gui2::dialogs::editor_resize_map::EXPAND_TOP_RIGHT:
			case gui2::dialogs::editor_resize_map::EXPAND_TOP:
			case gui2::dialogs::editor_resize_map::EXPAND_TOP_LEFT:
				break;
			default:
				y_offset = 0;
				WRN_ED << "Unknown resize expand direction";
				break;
		}

		switch (dir) {
			case gui2::dialogs::editor_resize_map::EXPAND_BOTTOM_RIGHT:
			case gui2::dialogs::editor_resize_map::EXPAND_RIGHT:
			case gui2::dialogs::editor_resize_map::EXPAND_TOP_RIGHT:
				x_offset = 0;
				break;
			case gui2::dialogs::editor_resize_map::EXPAND_BOTTOM:
			case gui2::dialogs::editor_resize_map::EXPAND_CENTER:
			case gui2::dialogs::editor_resize_map::EXPAND_TOP:
				x_offset /= 2;
				break;
			case gui2::dialogs::editor_resize_map::EXPAND_BOTTOM_LEFT:
			case gui2::dialogs::editor_resize_map::EXPAND_LEFT:
			case gui2::dialogs::editor_resize_map::EXPAND_TOP_LEFT:
				break;
			default:
				x_offset = 0;
				break;
		}

		editor_action_resize_map a(w, h, x_offset, y_offset, fill);
		perform_refresh(a);
	}
}

void context_manager::save_map_as_dialog()
{
	bool first_pick = false;
	std::string input_name = get_map_context().get_filename();
	if(input_name.empty()) {
		first_pick = true;
		if (editor_controller::current_addon_id_.empty()) {
			input_name = filesystem::get_legacy_editor_dir() + "/maps";
		} else {
			input_name = filesystem::get_current_editor_dir(editor_controller::current_addon_id_) + "/maps";
		}
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Save Map As"))
	   .set_save_mode(true)
	   .set_path(input_name)
	   .set_extension(filesystem::map_extension)
	   .set_extension(filesystem::mask_extension);

	if(!dlg.show()) {
		return;
	}

	boost::filesystem::path save_path(dlg.path());

	// Show warning the first time user tries to save in a wrong folder
	std::string last_folder = save_path.parent_path().filename().string();
	if ((last_folder == "scenarios")
		&& first_pick
		&& (gui2::show_message(
				_("Error"),
				VGETTEXT("Do you really want to save $type1 in $type2 folder?", {{"type1", "map"}, {"type2", "scenarios"}}),
				gui2::dialogs::message::yes_no_buttons) != gui2::retval::OK))
	{
		return;
	}

	std::size_t is_open = check_open_map(save_path.string());
	if(is_open < map_contexts_.size() && is_open != static_cast<unsigned>(current_context_index_)) {
		gui2::show_transient_message(_("This map is already open."), save_path.string());
	}

	std::string old_filename = get_map_context().get_filename();

	get_map_context().set_filename(save_path.string());

	if(!write_map(true)) {
		get_map_context().set_filename(old_filename);
	}
}

void context_manager::save_scenario_as_dialog()
{
	bool first_pick = false;
	std::string input_name = get_map_context().get_filename();
	if(input_name.empty()) {
		first_pick = true;
		input_name = filesystem::get_current_editor_dir(editor_controller::current_addon_id_) + "/scenarios";
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Save Scenario As"))
	   .set_save_mode(true)
	   .set_path(input_name)
	   .set_extension(filesystem::wml_extension)
	   .add_extra_path(desktop::GAME_EDITOR_MAP_DIR);

	if(!dlg.show()) {
		return;
	}

	boost::filesystem::path save_path(dlg.path());

	// Show warning the first time user tries to save in a wrong folder
	std::string last_folder = save_path.parent_path().filename().string();
	if ((last_folder == "maps")
		&& first_pick
		&& (gui2::show_message(
				_("Error"),
				VGETTEXT("Do you really want to save $type1 in $type2 folder?", {{"type1", "scenario"}, {"type2", "maps"}}),
				gui2::dialogs::message::yes_no_buttons) != gui2::retval::OK))
	{
		return;
	}

	std::size_t is_open = check_open_map(save_path.string());
	if(is_open < map_contexts_.size() && is_open != static_cast<unsigned>(current_context_index_)) {
		gui2::show_transient_message(_("This scenario is already open."), save_path.string());
		return;
	}

	std::string old_filename = get_map_context().get_filename();

	get_map_context().set_filename(save_path.string());

	if(!write_scenario(true)) {
		get_map_context().set_filename(old_filename);
		return;
	}
}

void context_manager::init_map_generators(const game_config_view& game_config)
{
	for(const config& i : game_config.child_range("multiplayer")) {
		if(i["map_generation"].empty() && i["scenario_generation"].empty()) {
			continue;
		}

		// TODO: we should probably use `child` with a try/catch block once that function throws
		if(const auto generator_cfg = i.optional_child("generator")) {
			map_generators_.emplace_back(create_map_generator(i["map_generation"].empty() ? i["scenario_generation"] : i["map_generation"], generator_cfg.value()));
		} else {
			ERR_ED << "Scenario \"" << i["name"] << "\" with id " << i["id"]
					<< " has map_generation= but no [generator] tag";
		}
	}
}

void context_manager::generate_map_dialog()
{
	if(map_generators_.empty()) {
		gui2::show_error_message(_("No random map generators found."));
		return;
	}

	gui2::dialogs::editor_generate_map dialog(map_generators_);
	dialog.select_map_generator(last_map_generator_);

	if(dialog.show()) {
		std::string map_string;
		map_generator* const map_generator = dialog.get_selected_map_generator();
		try {
			map_string = map_generator->create_map(dialog.get_seed());
		} catch (const mapgen_exception& e) {
			gui2::show_transient_message(_("Map creation failed."), e.what());
			return;
		}

		if(map_string.empty()) {
			gui2::show_transient_message("", _("Map creation failed."));
		} else {
			editor_map new_map(map_string);
			editor_action_whole_map a(new_map);
			get_map_context().set_needs_labels_reset(); // Ensure Player Start labels are updated together with newly generated map
			perform_refresh(a);
		}

		last_map_generator_ = map_generator;
	}
}

bool context_manager::confirm_discard()
{
	if(get_map_context().modified()) {
		const int res = gui2::show_message(_("Unsaved Changes"),
			_("Do you want to discard all changes made to the map since the last save?"), gui2::dialogs::message::yes_no_buttons);
		return gui2::retval::CANCEL != res;
	}

	return true;
}

void context_manager::fill_selection()
{
	perform_refresh(editor_action_paint_area(get_map_context().map().selection(), get_selected_bg_terrain()));
}

void context_manager::save_all_maps()
{
	int current = current_context_index_;
	for(std::size_t i = 0; i < map_contexts_.size(); ++i) {
		switch_context(i);
		save_map();
	}
	switch_context(current);
}

void context_manager::save_contexts()
{
	saved_contexts_.swap(map_contexts_);
	std::swap(last_context_, current_context_index_);
	create_blank_context();
	switch_context(0, true);
}

void context_manager::save_map(bool show_confirmation)
{
	const std::string& name = get_map_context().get_filename();
	if(name.empty() || filesystem::is_directory(name)) {
		if(get_map_context().is_pure_map()) {
			save_map_as_dialog();
		} else {
			save_scenario_as_dialog();
		}
	} else {
		if(get_map_context().is_pure_map()) {
			write_map(show_confirmation);
		} else {
			write_scenario(show_confirmation);
		}
	}
}

bool context_manager::write_scenario(bool display_confirmation)
{
	try {
		get_map_context().save_scenario();
		if(display_confirmation) {
			gui_.set_status(_("Scenario saved."), true);
		}
	} catch (const editor_map_save_exception& e) {
		gui_.set_status(e.what(), false);
		return false;
	}

	return true;
}

bool context_manager::write_map(bool display_confirmation)
{
	try {
		get_map_context().save_map();
		if(display_confirmation) {
			gui_.set_status(_("Map saved"), true);
		}
	} catch (const editor_map_save_exception& e) {
		gui_.set_status(e.what(), false);
		return false;
	}

	return true;
}

std::size_t context_manager::check_open_map(const std::string& fn) const
{
	std::size_t i = 0;
	while(i < map_contexts_.size() && map_contexts_[i]->get_filename() != fn) {
		++i;
	}

	return i;
}

bool context_manager::check_switch_open_map(const std::string& fn)
{
	std::size_t i = check_open_map(fn);
	if(i < map_contexts_.size()) {
		gui2::show_transient_message(_("This map is already open."), fn);
		switch_context(i);
		return true;
	}

	return false;
}

void context_manager::load_map(const std::string& filename, bool new_context)
{
	if(new_context && check_switch_open_map(filename)) {
		return;
	}

	if(filesystem::is_cfg(filename)) {
		if(editor_controller::current_addon_id_.empty()) {
			// if no addon id has been set and the file being loaded is from an addon
			// then use the file path to determine the addon rather than showing a dialog
			if(auto addon_at_path = filesystem::get_addon_id_from_path(filename)) {
				editor_controller::current_addon_id_ = addon_at_path.value();
			} else {
				editor_controller::current_addon_id_ = editor::initialize_addon();
			}

			set_addon_id(editor_controller::current_addon_id_);
		}

		if(editor_controller::current_addon_id_.empty()) {
			return;
		}
	}

	LOG_ED << "Load map: " << filename << (new_context ? " (new)" : " (same)");
	try {
		{
			auto mc = std::make_unique<map_context>(game_config_, filename, current_addon_);
			if(mc->get_filename() != filename) {
				if(new_context && check_switch_open_map(mc->get_filename())) {
					return;
				}
			}

			if(new_context) {
				int new_id = add_map_context_of(std::move(mc));
				switch_context(new_id);
			} else {
				replace_map_context_with(std::move(mc));
			}
		}

		if(get_map_context().is_embedded()) {
			const std::string& msg = _("Loaded embedded map data");
			gui2::show_transient_message(_("Map loaded from scenario"), msg);
		} else {
			if(get_map_context().get_filename() != filename) {
				gui2::show_transient_message(_("Map loaded from scenario"), _("Loaded referenced map file:")+"\n"+get_map_context().get_filename());
			}
		}
	} catch(const editor_map_load_exception& e) {
		gui2::show_transient_message(_("Error loading map"), e.what());
		return;
	}
}

void context_manager::revert_map()
{
	if(!confirm_discard()) {
		return;
	}

	std::string filename = get_map_context().get_filename();
	if(filename.empty()) {
		ERR_ED << "Empty filename in map revert";
		return;
	}

	load_map(filename, false);
}

void context_manager::new_map(int width, int height, const t_translation::terrain_code& fill, bool new_context)
{
	const config& default_schedule = game_config_.find_mandatory_child("editor_times", "id", "empty");
	editor_map m(width, height, fill);

	if(new_context) {
		int new_id = add_map_context(m, true, default_schedule, current_addon_);
		switch_context(new_id);
	} else {
		replace_map_context(m, true, default_schedule, current_addon_);
	}
}

void context_manager::new_scenario(int width, int height, const t_translation::terrain_code& fill, bool new_context)
{
	auto default_schedule = game_config_.find_child("editor_times", "id", "empty");
	editor_map m(width, height, fill);

	if(new_context) {
		int new_id = add_map_context(m, false, *default_schedule, current_addon_);
		switch_context(new_id);
	} else {
		replace_map_context(m, false, *default_schedule, current_addon_);
	}

	// Give the new scenario an initial side.
	get_map_context().new_side();
	gui().set_viewing_team_index(0, true);
	gui().set_playing_team_index(0);
	gui_.init_flags();
}

//
// Context manipulation
//

template<typename... T>
int context_manager::add_map_context(const T&... args)
{
	map_contexts_.emplace_back(std::make_unique<map_context>(args...));
	return map_contexts_.size() - 1;
}

int context_manager::add_map_context_of(std::unique_ptr<map_context>&& mc)
{
	map_contexts_.emplace_back(std::move(mc));
	return map_contexts_.size() - 1;
}

template<typename... T>
void context_manager::replace_map_context(const T&... args)
{
	replace_map_context_with(std::move(std::make_unique<map_context>(args...)));
}

void context_manager::replace_map_context_with(std::unique_ptr<map_context>&& mc)
{
	map_contexts_[current_context_index_] = std::move(mc);
	refresh_on_context_change();
}

void context_manager::create_default_context()
{
	if(saved_contexts_.empty()) {
		create_blank_context();
		switch_context(0, true);
	} else {
		saved_contexts_.swap(map_contexts_);
		switch_context(last_context_, true);
		last_context_ = 0;
	}
}

void context_manager::create_blank_context()
{
	t_translation::terrain_code default_terrain =
			t_translation::read_terrain_code(game_config::default_terrain);

	const config& default_schedule = game_config_.find_mandatory_child("editor_times", "id", "empty");
	add_map_context(editor_map(44, 33, default_terrain), true, default_schedule, current_addon_);
}

void context_manager::close_current_context()
{
	if(!confirm_discard()) return;

	if(map_contexts_.size() == 1) {
		create_default_context();
		map_contexts_.erase(map_contexts_.begin());
	} else if(current_context_index_ == static_cast<int>(map_contexts_.size()) - 1) {
		map_contexts_.pop_back();
		current_context_index_--;
	} else {
		map_contexts_.erase(map_contexts_.begin() + current_context_index_);
	}

	refresh_on_context_change();
}

void context_manager::switch_context(const int index, const bool force)
{
	if(index < 0 || static_cast<std::size_t>(index) >= map_contexts_.size()) {
		WRN_ED << "Invalid index in switch map context: " << index;
		return;
	}

	if(index == current_context_index_ && !force) {
		return;
	}

	// Disable the labels of the current context before switching.
	// The refresher handles enabling the new ones.
	get_map_context().get_labels().enable(false);

	current_context_index_ = index;

	refresh_on_context_change();
}

void context_manager::set_window_title()
{
	std::string name = get_map_context().get_name();

	if(name.empty()) {
		name = filesystem::base_name(get_map_context().get_filename());
	}

	if(name.empty()){
		name = get_map_context().get_default_context_name();
	}

	const std::string& wm_title_string = name + " - " + game_config::get_default_title_string();
	video::set_window_title(wm_title_string);
}

} //Namespace editor
