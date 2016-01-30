/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONTEXT_MANAGER_HPP_INCLUDED
#define CONTEXT_MANAGER_HPP_INCLUDED

#include "editor/map/map_context.hpp"
#include "editor/editor_preferences.hpp"
#include "editor/map/map_fragment.hpp"
class map_generator;

namespace editor {

map_labels* get_current_labels();

class context_manager {

public:

	bool is_active_transitions_hotkey(const std::string& item);

	size_t modified_maps(std::string& modified);

	void set_update_trasitions_mode(int mode) {
		auto_update_transitions_ = mode;
		preferences::editor::set_auto_update_transitions(mode);
	}

	bool toggle_update_transitions() {
		auto_update_transitions_ = (auto_update_transitions_ + 1)
				% preferences::editor::TransitionUpdateMode::count;
		preferences::editor::set_auto_update_transitions(
				auto_update_transitions_);
		if (auto_update_transitions_
				!= preferences::editor::TransitionUpdateMode::on) {
			return true;
		}
		return false;
	}

	bool clipboard_empty() {
		return clipboard_.empty();
	}

	map_fragment& get_clipboard() {
		return clipboard_;
	}

	/** Fill the selection with the foreground terrain */
	void fill_selection();

	/** Index into the map_contexts_ array */
	int current_context_index() {
		return current_context_index_;
	}

public:
	context_manager(editor_display& gui, const config& game_config);
	~context_manager();

	size_t open_maps(void) {
		return map_contexts_.size();
	}

	/**
	 * Perform an action on the current map_context, then refresh the display.
	 */
	void perform_refresh(const editor_action& action, bool drag_part = false);

	/**
	 * Save all maps, open dialog if not named yet, except when using
	 * auto_save_windows which will name unnamed maps "windows_N".
	 * Also record all filenames for future reopening.
	 */
	void save_all_maps(bool auto_save_windows = false);

	/** Save the map, open dialog if not named yet. */
	void save_map();

	editor_display& gui() {
		return gui_;
	}

	/**
	 * Refresh everything, i.e. invalidate all hexes and redraw them. Does *not* reload the map.
	 */
	void refresh_all();

	/** Display an apply mask dialog and process user input. */
	void apply_mask_dialog();

	/** Display an apply mask dialog and process user input. */
	void create_mask_to_dialog();

	/** Display an dialog to querry a new id for an [time_area] */
	void rename_area_dialog();

	/** Menu expanding for open maps list */
	void expand_open_maps_menu(std::vector<std::string>& items);

	/** Menu expanding for most recent loaded list */
	void expand_load_mru_menu(std::vector<std::string>& items);

	/** Menu expanding for the map's player sides */
	void expand_sides_menu(std::vector<std::string>& items);

	/** Menu expanding for the map's defined areas */
	void expand_areas_menu(std::vector<std::string>& items);

	/** Menu expanding for the map's defined areas */
	void expand_time_menu(std::vector<std::string>& items);

	/** Menu expanding for the map's defined areas */
	void expand_local_time_menu(std::vector<std::string>& items);

	/** Display a load map dialog and process user input. */
	void load_map_dialog(bool force_same_context = false);

	/** Open the specified entry from the recent files list. */
	void load_mru_item(unsigned index, bool force_same_context = false);

	/** Display a scenario edit dialog and process user input. */
	void edit_scenario_dialog();

	/** TODO */
	void edit_side_dialog(int side);

	/** Display a new map dialog and process user input. */
	void new_map_dialog();

	/** Display a new map dialog and process user input. */
	void new_scenario_dialog();

	/** Display a save map as dialog and process user input. */
	void save_map_as_dialog();

	/** Display a save map as dialog and process user input. */
	void save_scenario_as_dialog();

	/** Display a generate random map dialog and process user input. */
	void generate_map_dialog();

	/** Display a load map dialog and process user input. */
	void resize_map_dialog();

	size_t size() {
		return map_contexts_.size();
	}

	/** Get the current map context object */
	map_context& get_map_context() {
		return *map_contexts_[current_context_index_];
	}
	/** Get the map from the current map context object - const version*/
	const editor_map& get_map() const {
		return get_map_context().get_map();
	}

	/** Set the default dir (where the filebrowser is pointing at when there is no map file opened) */
	void set_default_dir(const std::string& str);

private:

	/**
	 * Replace the current map context and refresh accordingly
	 */
	void replace_map_context(map_context* new_mc);
	/** init available random map generators */
	void init_map_generators(const config& game_config);

	/**
	 * Shows an are-you-sure dialog if the map was modified.
	 * @return true if the user confirmed or the map was not modified, false otherwise
	 */
	bool confirm_discard();

	/** Get the current map context object - const version */
	const map_context& get_map_context() const {
		return *map_contexts_[current_context_index_];
	}


	/**
	 * Add a map context. The controller assumes ownership.
	 * @return the index of the added map context in the map_contexts_ array
	 */
	int add_map_context(map_context* mc);

	/**
	 * Creates a default map context object, used to ensure there is always at least one.
	 * Except when we saved windows, in which case reopen them
	 */
	void create_default_context();

public:
	/**
	 * Refresh the display after an action has been performed.
	 * The map context contains details of what needs to be refreshed.
	 */
	void refresh_after_action(bool drag_part = false);

	/** Get the map from the current map context object */
	editor_map& get_map() {
		return get_map_context().get_map();
	}

	/** Closes the active map context. Switches to a valid context afterward or creates a dummy one. */
	void close_current_context();

	/** Switches the context to the one under the specified index. */
	void switch_context(const int index);

private:
	/**
	 * Save the map under a given filename.
	 * @return true on success
	 */
	bool save_map_as(const std::string& filename);
	//TODO
	bool save_scenario_as(const std::string& filename);

	/**
	 * Save the map under a given filename. Displays an error message on failure.
	 * @return true on success
	 */
	bool write_map(bool display_confirmation = false);
	bool write_scenario(bool display_confirmation = false);

	/**
	 * Create a new map.
	 */
	void new_map(int width, int height, const t_translation::t_terrain & fill,
			bool new_context);

	/**
	 * Create a new scenario.
	 */
	void new_scenario(int width, int height, const t_translation::t_terrain & fill,
			bool new_context);

	/**
	 * Check if a map is already open.
	 * @return index of the map context containing the given filename,
	 *         or map_contexts_.size() if not found.
	 */
	size_t check_open_map(const std::string& fn) const;

	/**
	 * Check if a map is already open. If yes, switch to it
	 * and return true, return false otherwise.
	 */
	bool check_switch_open_map(const std::string& fn);

	/**
	 * Displays the specified map name in the window titlebar
	 */
	void set_window_title();
public:
	/**
	 * Load a map given the filename
	 */
	void load_map(const std::string& filename, bool new_context);

	/**
	 * Revert the map by reloading it from disk
	 */
	void revert_map();

	/**
	 * Reload the map after it has significantly changed (when e.g. the dimensions changed).
	 * This is necessary to avoid issues with parts of the map being cached in the display class.
	 */
	void reload_map();

private:

	editor_display& gui_;

	const config& game_config_;

	/** Default directory for map load/save as dialogs */
	std::string default_dir_;

	/** Available random map generators */
	std::vector<map_generator*> map_generators_;
	map_generator* last_map_generator_;

	int current_context_index_;

	/** Flag to rebuild terrain on every terrain change */
	int auto_update_transitions_;


	/** The currently opened map context object */
	std::vector<map_context*> map_contexts_;

	/** Clipboard map_fragment -- used for copy-paste. */
	map_fragment clipboard_;
};

}

#endif
