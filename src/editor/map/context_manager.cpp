/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "config_assign.hpp"
#include "display.hpp"
#include "editor/map/context_manager.hpp"
#include "editor/map/map_context.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "generators/map_create.hpp"
#include "generators/map_generator.hpp"
#include "gettext.hpp"

#include "editor/action/action.hpp"
#include "editor/controller/editor_controller.hpp"
#include "preferences/editor.hpp"

#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/editor/generate_map.hpp"
#include "gui/dialogs/editor/new_map.hpp"
#include "gui/dialogs/editor/resize_map.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"

#include "gui/dialogs/editor/edit_scenario.hpp"
#include "gui/dialogs/editor/edit_side.hpp"

#include "terrain/translation.hpp"

#include <memory>

namespace editor {

static std::vector<std::string> saved_windows_;

static const std::string get_menu_marker(const bool changed)
{
	std::ostringstream ss;
	ss << "[<span ";

	if(changed) {
		ss << "color='#f00' ";
	}

	ss << "size='large'>" << font::unicode_bullet << "</span>]";
	return ss.str();
}

/**
 * Utility class to properly refresh the display when the map context object is replaced
 * without duplicating code.
 */
class map_context_refresher
{
public:
	map_context_refresher(context_manager& ec)
		: context_manager_(ec)
		, refreshed_(false)
	{
	}

	~map_context_refresher()
	{
		if(!refreshed_) refresh();
	}

	void refresh()
	{
		context_manager_.gui().change_display_context(&context_manager_.get_map_context());

		// TODO register the tod_manager with the gui?
		resources::tod_manager = context_manager_.get_map_context().get_time_manager();
		resources::units = &context_manager_.get_map_context().get_units();
		resources::filter_con = &context_manager_.gui();

		context_manager_.gui().replace_overlay_map(&context_manager_.get_map_context().get_overlays());

		resources::classification = &context_manager_.get_map_context().get_classification();

		context_manager_.gui().init_flags();

		context_manager_.reload_map();

		// Enable the labels of the current context;
		context_manager_.get_labels().enable(true);

		refreshed_ = true;
	}

private:
	context_manager& context_manager_;
	bool refreshed_;
};


context_manager::context_manager(editor_display& gui, const config& game_config)
	: gui_(gui)
	, game_config_(game_config)
	, default_dir_(preferences::editor::default_dir())
	, map_generators_()
	, last_map_generator_(nullptr)
	, current_context_index_(0)
	, auto_update_transitions_(preferences::editor::auto_update_transitions())
	, map_contexts_()
	, clipboard_()
{
	if(default_dir_.empty()) {
		default_dir_ = filesystem::get_dir(filesystem::get_user_data_dir() + "/editor");
	}

	create_default_context();
	init_map_generators(game_config);
}

context_manager::~context_manager()
{
	for(map_generator* m : map_generators_) {
		delete m;
	}

	// Restore default window title
	CVideo::get_singleton().set_window_title(game_config::get_default_title_string());
}

void context_manager::refresh_all()
{
	gui_.rebuild_all();
	get_map_context().set_needs_terrain_rebuild(false);
	gui_.create_buttons();
	gui_.invalidate_all();
	gui_.draw(false);
	get_map_context().clear_changed_locations();
	gui_.recalculate_minimap();
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
		case preferences::editor::TransitionUpdateMode::on:
			return (item == "editor-auto-update-transitions");
		case preferences::editor::TransitionUpdateMode::partial:
			return (item == "editor-partial-update-transitions");
		case preferences::editor::TransitionUpdateMode::off:
			return (item == "editor-no-update-transitions");
	}

	return true; //should not be reached
}

bool context_manager::toggle_update_transitions()
{
	auto_update_transitions_ = (auto_update_transitions_ + 1) % preferences::editor::TransitionUpdateMode::count;
	preferences::editor::set_auto_update_transitions(auto_update_transitions_);

	if(auto_update_transitions_ != preferences::editor::TransitionUpdateMode::on) {
		return true;
	}

	return false;
}

size_t context_manager::modified_maps(std::string& message)
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
	std::string fn = filesystem::directory_name(get_map_context().get_filename());
	if(fn.empty()) {
		fn = default_dir_;
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Load Map"))
	   .set_path(fn);

	if(dlg.show(gui_.video())) {
		load_map(dlg.path(), !force_same_context);
	}
}

void context_manager::load_mru_item(unsigned int index, bool force_same_context /* = false */)
{
	const std::vector<std::string>& mru = preferences::editor::recent_files();
	if(mru.empty() || index >= mru.size()) {
		return;
	}

	load_map(mru[index], !force_same_context);
}

void context_manager::edit_side_dialog(int side_index)
{
	team& t = get_map_context().get_teams()[side_index];

	editor_team_info team_info(t);

	if(gui2::dialogs::editor_edit_side::execute(team_info, gui_.video())) {
		get_map_context().set_side_setup(team_info);
	}
}

void context_manager::edit_scenario_dialog()
{
	map_context& context = get_map_context();

	// TODO
	//std::string fn = filesystem::directory_name(context.get_filename());

	std::string id          = context.get_id();
	std::string name        = context.get_name();
	std::string description = context.get_description();

	int turns  = context.get_time_manager()->number_of_turns();
	int xp_mod = context.get_xp_mod();

	bool victory = context.victory_defeated();
	bool random  = context.random_start_time();

	const bool ok = gui2::dialogs::editor_edit_scenario::execute(
		id, name, description, turns, xp_mod, victory, random, gui_.video()
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
	int w = get_map().w();
	int h = get_map().h();

	if(gui2::dialogs::editor_new_map::execute(_("New Map"), w, h, gui_.video())) {
		const t_translation::terrain_code& fill = get_selected_bg_terrain();
		new_map(w, h, fill, true);
	}
}

void context_manager::new_scenario_dialog()
{
	int w = get_map().w();
	int h = get_map().h();

	if(gui2::dialogs::editor_new_map::execute(_("New Scenario"), w, h, gui_.video())) {
		const t_translation::terrain_code& fill = get_selected_bg_terrain();
		new_scenario(w, h, fill, true);
	}
}

void context_manager::expand_open_maps_menu(std::vector<config>& items, int i)
{
	auto pos = items.erase(items.begin() + i);
	std::vector<config> contexts;

	for(size_t mci = 0; mci < map_contexts_.size(); ++mci) {
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
			ss << "<i>" << filename << "</i>";
		} else {
			ss << filename;
		}

		if(mc.is_embedded()) {
			ss << " (E)";
		}

		const std::string label = ss.str();
		const std::string details = get_menu_marker(changed);

		contexts.emplace_back(config_of("label", label)("details", details));
	}

	items.insert(pos, contexts.begin(), contexts.end());
}

void context_manager::expand_load_mru_menu(std::vector<config>& items, int i)
{
	std::vector<std::string> mru = preferences::editor::recent_files();

	auto pos = items.erase(items.begin() + i);

	if(mru.empty()) {
		items.insert(pos, config_of("label", _("No Recent Files")));
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
		return config_of("label", str);
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

	for(size_t mci = 0; mci < area_ids.size(); ++mci) {
		const std::string& area = area_ids[mci];

		std::stringstream ss;
		ss << "[" << mci + 1 << "] ";\

		if(area.empty()) {
			ss << "<i>" << _("Unnamed Area") << "</i>";
		} else {
			ss << area;
		}

		const bool changed =
			mci == static_cast<size_t>(get_map_context().get_active_area())
			&& tod->get_area_by_index(mci) != get_map_context().get_map().selection();

		const std::string label = ss.str();
		const std::string details = get_menu_marker(changed);

		area_entries.emplace_back(config_of("label", label)("details", details));
	}

	items.insert(pos, area_entries.begin(), area_entries.end());
}

void context_manager::expand_sides_menu(std::vector<config>& items, int i)
{
	auto pos = items.erase(items.begin() + i);
	std::vector<config> contexts;

	for(size_t mci = 0; mci < get_map_context().get_teams().size(); ++mci) {

		const team& t = get_map_context().get_teams()[mci];
		const std::string& teamname = t.user_team_name();
		std::stringstream label;
		label << "[" << mci+1 << "] ";

		if(teamname.empty()) {
			label << "<i>" << _("New Side") << "</i>";
		} else {
			label << teamname;
		}

		contexts.emplace_back(config_of("label", label.str()));
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
		times.emplace_back(config_of
			("details", time.name) // Use 'details' field here since the image will take the first column
			("image", time.image)
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
		times.emplace_back(config_of
			("details", time.name) // Use 'details' field here since the image will take the first column
			("image", time.image)
		);
	}

	items.insert(pos, times.begin(), times.end());
}

void context_manager::apply_mask_dialog()
{
	std::string fn = get_map_context().get_filename();
	if(fn.empty()) {
		fn = default_dir_;
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Apply Mask"))
	   .set_path(fn);

	if(dlg.show(gui_.video())) {
		try {
			map_context mask(game_config_, dlg.path());
			editor_action_apply_mask a(mask.get_map());
			perform_refresh(a);
		} catch (editor_map_load_exception& e) {
			gui2::show_transient_message(gui_.video(), _("Error loading mask"), e.what());
			return;
		} catch (editor_action_exception& e) {
			gui2::show_error_message(gui_.video(), e.what());
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

	if(gui2::dialogs::edit_text::execute(N_("Rename Area"), N_("Identifier:"), name, gui_.video())) {
		get_map_context().get_time_manager()->set_area_id(active_area, name);
	}
}

void context_manager::create_mask_to_dialog()
{
	std::string fn = get_map_context().get_filename();
	if(fn.empty()) {
		fn = default_dir_;
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Choose Target Map"))
	   .set_path(fn);

	if(dlg.show(gui_.video())) {
		try {
			map_context map(game_config_, dlg.path());
			editor_action_create_mask a(map.get_map());
			perform_refresh(a);
		} catch (editor_map_load_exception& e) {
			gui2::show_transient_message(gui_.video(), _("Error loading map"), e.what());
			return;
		} catch (editor_action_exception& e) {
			gui2::show_error_message(gui_.video(), e.what());
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
		if((auto_update_transitions_ == preferences::editor::TransitionUpdateMode::on)
		|| ((auto_update_transitions_ == preferences::editor::TransitionUpdateMode::partial)
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
	int w = get_map().w();
	int h = get_map().h();

	gui2::dialogs::editor_resize_map::EXPAND_DIRECTION dir = gui2::dialogs::editor_resize_map::EXPAND_DIRECTION();
	bool copy = false;

	if(!gui2::dialogs::editor_resize_map::execute(w, h, dir, copy, gui_.video())) {
		return;
	}

	if(w != get_map().w() || h != get_map().h()) {
		t_translation::terrain_code fill = get_selected_bg_terrain();
		if(copy) {
			fill = t_translation::NONE_TERRAIN;
		}

		int x_offset = get_map().w() - w;
		int y_offset = get_map().h() - h;

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
				WRN_ED << "Unknown resize expand direction" << std::endl;
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
	std::string input_name = get_map_context().get_filename();
	if(input_name.empty()) {
		input_name = filesystem::get_dir(default_dir_ + "/maps");
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Save Map As"))
	   .set_save_mode(true)
	   .set_path(input_name)
	   .set_extension(".map");

	if(!dlg.show(gui_.video())) {
		return;
	}

	save_map_as(dlg.path());
}

void context_manager::save_scenario_as_dialog()
{
	std::string input_name = get_map_context().get_filename();
	if(input_name.empty()) {
		input_name = filesystem::get_dir(default_dir_ + "/scenarios");
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Save Scenario As"))
	   .set_save_mode(true)
	   .set_path(input_name)
	   .set_extension(".cfg");

	if(!dlg.show(gui_.video())) {
		return;
	}

	save_scenario_as(dlg.path());
}

void context_manager::init_map_generators(const config& game_config)
{
	for(const config& i : game_config.child_range("multiplayer")) {
		if(i["map_generation"].empty() && i["scenario_generation"].empty()) {
			continue;
		}

		const config& generator_cfg = i.child("generator");
		if(!generator_cfg) {
			ERR_ED << "Scenario \"" << i["name"] << "\" with id " << i["id"]
					<< " has map_generation= but no [generator] tag" << std::endl;
		} else {
			map_generator* m = create_map_generator(i["map_generation"], generator_cfg);
			map_generators_.push_back(m);
		}
	}
}

void context_manager::generate_map_dialog()
{
	if(map_generators_.empty()) {
		gui2::show_error_message(gui_.video(), _("No random map generators found."));
		return;
	}

	gui2::dialogs::editor_generate_map dialog(map_generators_);
	dialog.select_map_generator(last_map_generator_);
	dialog.show(gui_.video());

	if(dialog.get_retval() == gui2::window::OK) {
		std::string map_string;
		map_generator* const map_generator = dialog.get_selected_map_generator();
		try {
			map_string = map_generator->create_map(dialog.get_seed());
		} catch (mapgen_exception& e) {
			gui2::show_transient_message(gui_.video(), _("Map creation failed."), e.what());
			return;
		}

		if(map_string.empty()) {
			gui2::show_transient_message(gui_.video(), "", _("Map creation failed."));
		} else {
			editor_map new_map(game_config_, map_string);
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
		const int res = gui2::show_message(gui_.video(), _("Unsaved Changes"),
			_("Do you want to discard all changes made to the map since the last save?"), gui2::dialogs::message::yes_no_buttons);
		return gui2::window::CANCEL != res;
	}

	return true;
}

void context_manager::fill_selection()
{
	perform_refresh(editor_action_paint_area(get_map().selection(), get_selected_bg_terrain()));
}

void context_manager::save_all_maps(bool auto_save_windows)
{
	int current = current_context_index_;
	saved_windows_.clear();
	for(size_t i = 0; i < map_contexts_.size(); ++i) {
		switch_context(i);
		std::string name = get_map_context().get_filename();
		if(auto_save_windows) {
			if(name.empty() || filesystem::is_directory(name)) {
				std::ostringstream s;
				s << default_dir_ << "/" << "window_" << i;
				name = s.str();
				get_map_context().set_filename(name);
			}
		}
		saved_windows_.push_back(name);
		save_map();
	}

	switch_context(current);
}

void context_manager::save_map()
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
			write_map();
		} else {
			write_scenario();
		}
	}
}

bool context_manager::save_scenario_as(const std::string& filename)
{
	size_t is_open = check_open_map(filename);
	if(is_open < map_contexts_.size() && is_open != static_cast<unsigned>(current_context_index_)) {
		gui2::show_transient_message(gui_.video(), _("This scenario is already open."), filename);
		return false;
	}

	std::string old_filename = get_map_context().get_filename();
	bool embedded = get_map_context().is_embedded();

	get_map_context().set_filename(filename);
	get_map_context().set_embedded(false);

	if(!write_scenario(true)) {
		get_map_context().set_filename(old_filename);
		get_map_context().set_embedded(embedded);
		return false;
	}

	return true;
}

bool context_manager::save_map_as(const std::string& filename)
{
	size_t is_open = check_open_map(filename);
	if(is_open < map_contexts_.size() && is_open != static_cast<unsigned>(current_context_index_)) {
		gui2::show_transient_message(gui_.video(), _("This map is already open."), filename);
		return false;
	}

	std::string old_filename = get_map_context().get_filename();
	bool embedded = get_map_context().is_embedded();

	get_map_context().set_filename(filename);
	get_map_context().set_embedded(false);

	if(!write_map(true)) {
		get_map_context().set_filename(old_filename);
		get_map_context().set_embedded(embedded);
		return false;
	}

	return true;
}

bool context_manager::write_scenario(bool display_confirmation)
{
	try {
		get_map_context().save_scenario();
		if(display_confirmation) {
			gui2::show_transient_message(gui_.video(), "", _("Scenario saved."));
		}
	} catch (editor_map_save_exception& e) {
		gui2::show_transient_message(gui_.video(), "", e.what());
		return false;
	}

	return true;
}

bool context_manager::write_map(bool display_confirmation)
{
	try {
		get_map_context().save_map();
		if(display_confirmation) {
			gui2::show_transient_message(gui_.video(), "", _("Map saved."));
		}
	} catch (editor_map_save_exception& e) {
		gui2::show_transient_message(gui_.video(), "", e.what());
		return false;
	}

	return true;
}

size_t context_manager::check_open_map(const std::string& fn) const
{
	size_t i = 0;
	while(i < map_contexts_.size() && map_contexts_[i]->get_filename() != fn) {
		++i;
	}

	return i;
}

bool context_manager::check_switch_open_map(const std::string& fn)
{
	size_t i = check_open_map(fn);
	if(i < map_contexts_.size()) {
		gui2::show_transient_message(gui_.video(), _("This map is already open."), fn);
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

	LOG_ED << "Load map: " << filename << (new_context ? " (new)" : " (same)") << "\n";
	try {
		{
			context_ptr mc(new map_context(game_config_, filename));
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
			gui2::show_transient_message(gui_.video(), _("Map loaded from scenario"), msg);
		} else {
			if(get_map_context().get_filename() != filename) {
				if(get_map_context().get_map_data_key().empty()) {
					ERR_ED << "Internal error, map context filename changed: "
						<< filename << " -> " << get_map_context().get_filename()
						<< " with no apparent scenario load\n";
				} else {
					utils::string_map symbols;
					symbols["old"] = filename;
					const std::string& msg = _("Loaded referenced map file:\n$new");
					symbols["new"] = get_map_context().get_filename();
					symbols["map_data"] = get_map_context().get_map_data_key();
					gui2::show_transient_message(gui_.video(), _("Map loaded from scenario"),
						//TODO: msg is already translated does vgettext make sense?
						vgettext(msg.c_str(), symbols));
				}
			}
		}
	} catch (editor_map_load_exception& e) {
		gui2::show_transient_message(gui_.video(), _("Error loading map"), e.what());
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
		ERR_ED << "Empty filename in map revert" << std::endl;
		return;
	}

	load_map(filename, false);
}

void context_manager::new_map(int width, int height, const t_translation::terrain_code& fill, bool new_context)
{
	const config& default_schedule = game_config_.find_child("editor_times", "id", "default");
	editor_map m(game_config_, width, height, fill);

	if(new_context) {
		int new_id = add_map_context(m, true, default_schedule);
		switch_context(new_id);
	} else {
		replace_map_context(m, true, default_schedule);
	}
}

void context_manager::new_scenario(int width, int height, const t_translation::terrain_code& fill, bool new_context)
{
	const config& default_schedule = game_config_.find_child("editor_times", "id", "default");
	editor_map m(game_config_, width, height, fill);

	if(new_context) {
		int new_id = add_map_context(m, false, default_schedule);
		switch_context(new_id);
	} else {
		replace_map_context(m, false, default_schedule);
	}

	// Give the new scenario an initial side.
	get_map_context().new_side();
	gui_.init_flags();
}

//
// Context manipulation
//

template<typename... T>
int context_manager::add_map_context(const T&... args)
{
	map_contexts_.emplace_back(new map_context(args...));
	return map_contexts_.size() - 1;
}

int context_manager::add_map_context_of(context_ptr&& mc)
{
	map_contexts_.emplace_back(std::move(mc));
	return map_contexts_.size() - 1;
}

template<typename... T>
void context_manager::replace_map_context(const T&... args)
{
	context_ptr new_mc(new map_context(args...));
	replace_map_context_with(std::move(new_mc));
}

void context_manager::replace_map_context_with(context_ptr&& mc)
{
	map_context_refresher mcr(*this);
	map_contexts_[current_context_index_].swap(mc);

	set_window_title();
}

void context_manager::create_default_context()
{
	if(saved_windows_.empty()) {
		t_translation::terrain_code default_terrain =
			t_translation::read_terrain_code(game_config::default_terrain);

		const config& default_schedule = game_config_.find_child("editor_times", "id", "default");
		add_map_context(editor_map(game_config_, 44, 33, default_terrain), true, default_schedule);
	} else {
		for(const std::string& filename : saved_windows_) {
			add_map_context(game_config_, filename);
		}

		saved_windows_.clear();
	}
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

	map_context_refresher(*this);
	set_window_title();
}

void context_manager::switch_context(const int index, const bool force)
{
	if(index < 0 || static_cast<size_t>(index) >= map_contexts_.size()) {
		WRN_ED << "Invalid index in switch map context: " << index << std::endl;
		return;
	}

	if(index == current_context_index_ && !force) {
		return;
	}

	// Disable the labels of the current context before switching.
	// The refresher handles enabling the new ones.
	get_labels().enable(false);

	map_context_refresher mcr(*this);
	current_context_index_ = index;

	set_window_title();
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
	CVideo::get_singleton().set_window_title(wm_title_string);
}

} //Namespace editor
