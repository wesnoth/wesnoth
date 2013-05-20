/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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

#include "context_manager.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "filechooser.hpp"
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "map_create.hpp"
#include "map_context.hpp"

#include "editor/action/action.hpp"
#include "editor/editor_controller.hpp"
#include "editor/editor_preferences.hpp"

#include "gui/dialogs/editor_generate_map.hpp"
#include "gui/dialogs/editor_new_map.hpp"
#include "gui/dialogs/editor_resize_map.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"

#include <boost/foreach.hpp>

#include "terrain_translation.hpp"

#include "wml_separators.hpp"

namespace {
static std::vector<std::string> saved_windows_;
}

namespace editor {

map_labels* current_labels;

map_labels* get_current_labels() {
	return current_labels;
}


/**
 * Utility class to properly refresh the display when the map context object is replaced
 * without duplicating code.
 */
class map_context_refresher
{
public:
	map_context_refresher(context_manager& ec, const map_context& other_mc)
	: context_manager_(ec), size_changed_(!ec.get_map().same_size_as(other_mc.get_map())), refreshed_(false)
	{
	}
	~map_context_refresher() {
		if (!refreshed_) refresh();
	}
	void refresh() {
		context_manager_.gui().change_map(&context_manager_.get_map());
		resources::game_map = &context_manager_.get_map();

		context_manager_.gui().change_units(&context_manager_.get_map_context().get_units());
		resources::units = &context_manager_.get_map_context().get_units();

		// TODO register the tod_manager with the gui?
		resources::tod_manager = context_manager_.get_map_context().get_time_manager();
		context_manager_.gui().change_teams(&context_manager_.get_map_context().get_teams());

		resources::teams = &context_manager_.get_map_context().get_teams();

		resources::state_of_game = &context_manager_.get_map_context().get_game_state();
		context_manager_.gui().init_flags();

		context_manager_.reload_map();

		current_labels->enable(false);
		current_labels = &context_manager_.get_map_context().get_labels();
		current_labels->enable(true);

		refreshed_ = true;
	}
private:
	context_manager& context_manager_;
	bool size_changed_;
	bool refreshed_;
};

bool context_manager::is_active_transitions_hotkey(const std::string& item) {
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

size_t context_manager::modified_maps(std::string& message) {
	std::vector<std::string> modified;
	BOOST_FOREACH(map_context* mc, map_contexts_) {
		if (mc->modified()) {
			if (!mc->get_filename().empty()) {
				modified.push_back(mc->get_filename());
			} else {
				modified.push_back(_("(New Map)"));
			}
		}
	}
	BOOST_FOREACH(std::string& str, modified) {
		message += "\n" + str;
	}
	return modified.size();
}

context_manager::context_manager(editor_display& gui, const config& game_config)
	: gui_(gui)
	, game_config_(game_config)
	, default_dir_(preferences::editor::default_dir())
	, map_generators_()
	, current_context_index_(0)
	, auto_update_transitions_(preferences::editor::auto_update_transitions())
	, map_contexts_()
    , clipboard_()
{
	if (default_dir_.empty()) {
		default_dir_ = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps");
	}
	create_default_context();
	init_map_generators(game_config);
}

context_manager::~context_manager()
{
	BOOST_FOREACH(map_generator* m, map_generators_) {
		delete m;
	}
	BOOST_FOREACH(map_context* mc, map_contexts_) {
		delete mc;
	}
}

void context_manager::refresh_all()
{
	gui_.rebuild_all();
	get_map_context().set_needs_terrain_rebuild(false);
	gui_.redraw_everything();
	get_map_context().clear_changed_locations();
	gui_.recalculate_minimap();
}

void context_manager::set_default_dir(const std::string& str)
{
	default_dir_ = str;
}

void context_manager::load_map_dialog(bool force_same_context /* = false */)
{
	if (!preferences::editor::use_mdi() && !confirm_discard()) {
		return;
	}

	std::string fn = directory_name(get_map_context().get_filename());
	if (fn.empty()) {
		fn = default_dir_;
	}
	int res = dialogs::show_file_chooser_dialog(gui_, fn, _("Choose a Map to Open"));
	if (res == 0) {
		load_map(fn, force_same_context
				? false
				: preferences::editor::use_mdi());
	}
}

void context_manager::new_map_dialog()
{
	if (!preferences::editor::use_mdi() && !confirm_discard()) {
		return;
	}

	int w = get_map().w();
	int h = get_map().h();
	if(gui2::teditor_new_map::execute(w, h, gui_.video())) {
		const t_translation::t_terrain fill = get_selected_bg_terrain();
		new_map(w, h, fill, preferences::editor::use_mdi());
	}
}

void context_manager::expand_open_maps_menu(std::vector<std::string>& items)
{
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "editor-switch-map") {
			items.erase(items.begin() + i);
			std::vector<std::string> contexts;
			for (size_t mci = 0; mci < map_contexts_.size(); ++mci) {
				std::string filename = map_contexts_[mci]->get_filename();
				if (filename.empty()) {
					filename = _("(New Map)");
				}
				std::string label = "[" + lexical_cast<std::string>(mci) + "] "
					+ filename;
				if (map_contexts_[mci]->is_embedded()) {
					label += " (E)";
				}
				contexts.push_back(label);
			}
			items.insert(items.begin() + i, contexts.begin(), contexts.end());
			break;
		}
	}
}

void context_manager::expand_areas_menu(std::vector<std::string>& items)
{
	if (!get_map_context().get_time_manager())
		return;
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "editor-switch-area") {
			items.erase(items.begin() + i);
			std::vector<std::string> area_entries;

			std::vector<std::string> area_ids =
					get_map_context().get_time_manager()->get_area_ids();

			for (size_t mci = 0; mci < area_ids.size(); ++mci) {

				const std::string& area = area_ids[mci];
				std::stringstream label;
				label << "[" << mci+1 << "] ";
				label << (area.empty() ? _("(New Area)") : area);
				area_entries.push_back(label.str());
			}

			items.insert(items.begin() + i,
					area_entries.begin(), area_entries.end());
			break;
		}
	}
}

void context_manager::expand_sides_menu(std::vector<std::string>& items)
{
       for (unsigned int i = 0; i < items.size(); ++i) {
               if (items[i] == "editor-switch-side") {
                       items.erase(items.begin() + i);
                       std::vector<std::string> contexts;

                       for (size_t mci = 0; mci < get_map_context().get_teams().size(); ++mci) {

                    	   	   const team& t = get_map_context().get_teams()[mci];
                               std::stringstream label;
                               label << "[" << mci+1 << "] ";
                               label << (t.name().empty() ? _("(New Side)") : t.name());
                               contexts.push_back(label.str());
                       }

                       items.insert(items.begin() + i, contexts.begin(), contexts.end());
                       break;
               }
       }
}

void context_manager::expand_time_menu(std::vector<std::string>& items)
{
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "editor-switch-time") {
			items.erase(items.begin() + i);
			std::vector<std::string> contexts;

			tod_manager* tod_m = get_map_context().get_time_manager();

			BOOST_FOREACH(const time_of_day& time, tod_m->times()) {

				//for (size_t mci = 0; mci < tod_m->times().size(); ++mci) {
				//const time_of_day& time = tod_m->times()[mci];

				std::stringstream label;
				//  label << "[" << mci+1 << "] ";
				if (!time.image.empty())
					label << IMAGE_PREFIX << time.image << IMG_TEXT_SEPARATOR;
					//<< COLUMN_SEPARATOR;
				label << time.name;
				//label << (time.name.empty() ? _("(New Side)") : time.name);
				contexts.push_back(label.str());
			}

			items.insert(items.begin() + i, contexts.begin(), contexts.end());
			break;
		}
	}
}

void context_manager::apply_mask_dialog()
{
	std::string fn = get_map_context().get_filename();
	if (fn.empty()) {
		fn = default_dir_;
	}
	int res = dialogs::show_file_chooser_dialog(gui_, fn, _("Choose a Mask to Apply"));
	if (res == 0) {
		try {
			map_context mask(game_config_, fn, gui_);
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

void context_manager::create_mask_to_dialog()
{
	std::string fn = get_map_context().get_filename();
	if (fn.empty()) {
		fn = default_dir_;
	}
	int res = dialogs::show_file_chooser_dialog(gui_, fn, _("Choose Target Map"));
	if (res == 0) {
		try {
			map_context map(game_config_, fn, gui_);
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
	if (get_map_context().needs_reload()) {
		reload_map();
		return;
	} else {
		const std::set<map_location>& changed_locs =
				get_map_context().changed_locations();

		if (get_map_context().needs_terrain_rebuild()) {
			if ((auto_update_transitions_ == preferences::editor::TransitionUpdateMode::on)
			|| ((auto_update_transitions_ == preferences::editor::TransitionUpdateMode::partial)
			&& (!drag_part || get_map_context().everything_changed()))) {
				gui_.rebuild_all();
				get_map_context().set_needs_terrain_rebuild(false);
				gui_.invalidate_all();
			} else {
				BOOST_FOREACH(const map_location& loc, changed_locs) {
					gui_.rebuild_terrain(loc);
				}
				gui_.invalidate(changed_locs);
			}
		} else {
			if (get_map_context().everything_changed()) {
				gui_.invalidate_all();
			} else {
				gui_.invalidate(changed_locs);
			}
		}
		if (get_map_context().needs_labels_reset()) {
			get_map_context().reset_starting_position_labels(gui_);
		}
	}
	get_map_context().clear_changed_locations();
	gui_.recalculate_minimap();
}

void context_manager::resize_map_dialog()
{
	int w = get_map().w();
	int h = get_map().h();
	gui2::teditor_resize_map::EXPAND_DIRECTION dir;
	bool copy = false;
	if(gui2::teditor_resize_map::execute(w, h, dir, copy, gui_.video())) {

		if (w != get_map().w() || h != get_map().h()) {
	        t_translation::t_terrain fill = get_selected_bg_terrain();
			if (copy) {
				fill = t_translation::NONE_TERRAIN;
			}
			int x_offset = get_map().w() - w;
			int y_offset = get_map().h() - h;
			switch (dir) {
				case gui2::teditor_resize_map::EXPAND_BOTTOM_RIGHT:
				case gui2::teditor_resize_map::EXPAND_BOTTOM:
				case gui2::teditor_resize_map::EXPAND_BOTTOM_LEFT:
					y_offset = 0;
					break;
				case gui2::teditor_resize_map::EXPAND_RIGHT:
				case gui2::teditor_resize_map::EXPAND_CENTER:
				case gui2::teditor_resize_map::EXPAND_LEFT:
					y_offset /= 2;
					break;
				case gui2::teditor_resize_map::EXPAND_TOP_RIGHT:
				case gui2::teditor_resize_map::EXPAND_TOP:
				case gui2::teditor_resize_map::EXPAND_TOP_LEFT:
					break;
				default:
					y_offset = 0;
					WRN_ED << "Unknown resize expand direction\n";
					break;
			}
			switch (dir) {
				case gui2::teditor_resize_map::EXPAND_BOTTOM_RIGHT:
				case gui2::teditor_resize_map::EXPAND_RIGHT:
				case gui2::teditor_resize_map::EXPAND_TOP_RIGHT:
					x_offset = 0;
					break;
				case gui2::teditor_resize_map::EXPAND_BOTTOM:
				case gui2::teditor_resize_map::EXPAND_CENTER:
				case gui2::teditor_resize_map::EXPAND_TOP:
					x_offset /= 2;
					break;
				case gui2::teditor_resize_map::EXPAND_BOTTOM_LEFT:
				case gui2::teditor_resize_map::EXPAND_LEFT:
				case gui2::teditor_resize_map::EXPAND_TOP_LEFT:
					break;
				default:
					x_offset = 0;
					break;
			}
			editor_action_resize_map a(w, h, x_offset, y_offset, fill);
			perform_refresh(a);
		}
	}
}

void context_manager::save_map_as_dialog()
{
	std::string input_name = get_map_context().get_filename();
	if (input_name.empty()) {
		input_name = default_dir_;
	}
	const std::string old_input_name = input_name;

	int res = 0;
	int overwrite_res = 1;
	do {
		input_name = old_input_name;
		res = dialogs::show_file_chooser_dialog_save(gui_, input_name, _("Save the Map As"));
		if (res == 0) {
			if (file_exists(input_name)) {
				const int res = gui2::show_message(gui_.video(), "",
						_("The file already exists. Do you want to overwrite it?"), gui2::tmessage::yes_no_buttons);
				overwrite_res = gui2::twindow::CANCEL == res ? 1 : 0;
			} else {
				overwrite_res = 0;
			}
		} else {
			return; //cancel pressed
		}
	} while (overwrite_res != 0);

	save_map_as(input_name);
}

void context_manager::init_map_generators(const config& game_config)
{
	BOOST_FOREACH(const config &i, game_config.child_range("multiplayer"))
	{
		if (i["map_generation"] == "default") {
			const config &generator_cfg = i.child("generator");
			if (!generator_cfg) {
				ERR_ED << "Scenario \"" << i["name"] << "\" with id " << i["id"]
					<< " has map_generation=default but no [generator] tag";
			} else {
				map_generator* m = create_map_generator("", generator_cfg);
				map_generators_.push_back(m);
			}
		}
	}
}

void context_manager::generate_map_dialog()
{
	if (map_generators_.empty()) {
		gui2::show_error_message(gui_.video(),
				_("No random map generators found."));
		return;
	}
	gui2::teditor_generate_map dialog;
	dialog.set_map_generators(map_generators_);
	dialog.set_gui(&gui_);
	dialog.show(gui_.video());
	if (dialog.get_retval() == gui2::twindow::OK) {
		std::string map_string;
		try {
			map_string = dialog.get_selected_map_generator()
				->create_map(std::vector<std::string>());
		} catch (mapgen_exception& e) {
			gui2::show_transient_message(gui_.video(), _("Map creation failed."), e.what());
			return;
		}
		if (map_string.empty()) {
			gui2::show_transient_message(gui_.video(), "", _("Map creation failed."));
		} else {
			editor_map new_map(game_config_, map_string);
			editor_action_whole_map a(new_map);
			perform_refresh(a);
		}
	}
}

bool context_manager::confirm_discard()
{
	if (get_map_context().modified()) {
		const int res = gui2::show_message(gui_.video(), _("Unsaved Changes"),
				_("Do you want to discard all changes you made to the map since the last save?"), gui2::tmessage::yes_no_buttons);
		return gui2::twindow::CANCEL != res;
	} else {
		return true;
	}
}

int context_manager::add_map_context(map_context* mc)
{
	map_contexts_.push_back(mc);
	return map_contexts_.size() - 1;
}

void context_manager::create_default_context()
{
	if(saved_windows_.empty()) {

		t_translation::t_terrain default_terrain =
				t_translation::read_terrain_code(game_config::default_terrain);

		map_context* mc = new map_context(editor_map(game_config_, 44, 33, default_terrain), gui_);
		add_map_context(mc);
	} else {
		BOOST_FOREACH(const std::string& filename, saved_windows_) {
			map_context* mc = new map_context(game_config_, filename, gui_);
			add_map_context(mc);
		}
		saved_windows_.clear();
	}
}

void context_manager::fill_selection()
{
	perform_refresh(editor_action_paint_area(get_map().selection(),
			get_selected_bg_terrain()));
}

void context_manager::close_current_context()
{
	if (!confirm_discard()) return;
	map_context* current = map_contexts_[current_context_index_];
	if (map_contexts_.size() == 1) {
		create_default_context();
		map_contexts_.erase(map_contexts_.begin());
	} else if (current_context_index_ == static_cast<int>(map_contexts_.size()) - 1) {
		map_contexts_.pop_back();
		current_context_index_--;
	} else {
		map_contexts_.erase(map_contexts_.begin() + current_context_index_);
	}
	map_context_refresher(*this, *current);
	delete current;
}

void context_manager::save_all_maps(bool auto_save_windows)
{
	int current = current_context_index_;
	saved_windows_.clear();
	for(size_t i = 0; i < map_contexts_.size(); ++i) {
		switch_context(i);
		std::string name = get_map_context().get_filename();
		if(auto_save_windows) {
			if(name.empty() || is_directory(name)) {
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
	if (name.empty() || is_directory(name)) {
		save_map_as_dialog();
	} else {
		write_map();
	}
}

bool context_manager::save_map_as(const std::string& filename)
{
	size_t is_open = check_open_map(filename);
	if (is_open < map_contexts_.size()
			&& is_open != static_cast<unsigned>(current_context_index_)) {

		gui2::show_transient_message(gui_.video(), _("This map is already open."), filename);
		return false;
	}
	std::string old_filename = get_map_context().get_filename();
	bool embedded = get_map_context().is_embedded();
	get_map_context().set_filename(filename);
	get_map_context().set_embedded(false);
	if (!write_map(true)) {
		get_map_context().set_filename(old_filename);
		get_map_context().set_embedded(embedded);
		return false;
	} else {
		return true;
	}
}

bool context_manager::write_map(bool display_confirmation)
{
	try {
		get_map_context().save();
		if (display_confirmation) {
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
	while (i < map_contexts_.size() && map_contexts_[i]->get_filename() != fn) ++i;
	return i;
}

bool context_manager::check_switch_open_map(const std::string& fn)
{
	size_t i = check_open_map(fn);
	if (i < map_contexts_.size()) {
		gui2::show_transient_message(gui_.video(), _("This map is already open."), fn);
		switch_context(i);
		return true;
	}
	return false;
}

void context_manager::load_map(const std::string& filename, bool new_context)
{
	if (new_context && check_switch_open_map(filename)) return;
	LOG_ED << "Load map: " << filename << (new_context ? " (new)" : " (same)") << "\n";
	try {
		util::unique_ptr<map_context> mc(new map_context(game_config_, filename, gui_));
		if (mc->get_filename() != filename) {
			if (new_context && check_switch_open_map(mc->get_filename())) return;
		}
		if (new_context) {
			int new_id = add_map_context(mc.release());
			switch_context(new_id);
		} else {
			replace_map_context(mc.release());
		}
		if (get_map_context().is_embedded()) {
			const char* msg = _("Loaded embedded map data");
			gui2::show_transient_message(gui_.video(), _("Map loaded from scenario"), msg);
		} else {
			if (get_map_context().get_filename() != filename) {
				if (get_map_context().get_map_data_key().empty()) {
					ERR_ED << "Internal error, map context filename changed: "
						<< filename << " -> " << get_map_context().get_filename()
						<< " with no apparent scenario load\n";
				} else {
					utils::string_map symbols;
					symbols["old"] = filename;
					const char* msg = _("Loaded referenced map file:\n"
						"$new");
					symbols["new"] = get_map_context().get_filename();
					symbols["map_data"] = get_map_context().get_map_data_key();
					gui2::show_transient_message(gui_.video(), _("Map loaded from scenario"),
						vgettext(msg, symbols));
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
	if (!confirm_discard()) return;
	std::string filename = get_map_context().get_filename();
	if (filename.empty()) {
		ERR_ED << "Empty filename in map revert\n";
		return;
	}
	load_map(filename, false);
}

void context_manager::new_map(int width, int height, const t_translation::t_terrain & fill, bool new_context)
{
	editor_map m(game_config_, width, height, fill);
	if (new_context) {
		int new_id = add_map_context(new map_context(m, gui_));
		switch_context(new_id);
	} else {
		replace_map_context(new map_context(m, gui_));
	}
}

void context_manager::reload_map()
{
	gui_.reload_map();
	get_map_context().set_needs_reload(false);
	get_map_context().reset_starting_position_labels(gui_);
	refresh_all();
}

void context_manager::switch_context(const int index)
{
	if (index < 0 || static_cast<size_t>(index) >= map_contexts_.size()) {
		WRN_ED << "Invalid index in switch map context: " << index << "\n";
		return;
	}
	map_context_refresher mcr(*this, *map_contexts_[index]);
	current_labels = &get_map_context().get_labels();
	current_context_index_ = index;
}

void context_manager::replace_map_context(map_context* new_mc)
{
	boost::scoped_ptr<map_context> del(map_contexts_[current_context_index_]);
	map_context_refresher mcr(*this, *new_mc);
	map_contexts_[current_context_index_] = new_mc;
}


} //Namespace editor
