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

#pragma once

#include "editor/map/map_context.hpp"
#include "editor/map/map_fragment.hpp"
#include "filter_context.hpp"
#include "preferences/preferences.hpp"

class map_generator;
class game_config_view;

namespace editor
{

class context_manager : public filter_context
{
public:
	context_manager(editor_display& gui, const game_config_view& game_config, const std::string& addon_id);
	~context_manager();

	bool is_active_transitions_hotkey(const std::string& item);

	std::size_t modified_maps(std::string& modified);

	void set_update_transitions_mode(int mode)
	{
		auto_update_transitions_ = mode;
		prefs::get().set_editor_auto_update_transitions(mode);
	}

	bool toggle_update_transitions();

	bool clipboard_empty()
	{
		return clipboard_.empty();
	}

	map_fragment& get_clipboard()
	{
		return clipboard_;
	}

	/** Fill the selection with the foreground terrain */
	void fill_selection();

	/** Index into the map_contexts_ array */
	int current_context_index()
	{
		return current_context_index_;
	}

	std::size_t open_maps(void)
	{
		return map_contexts_.size();
	}

	/**
	 * Perform an action on the current map_context, then refresh the display.
	 */
	void perform_refresh(const editor_action& action, bool drag_part = false);


	/** Save all open map_contexts to memory */
	void save_contexts();

	/** Save all maps, show save dialogs for unsaved ones */
	void save_all_maps();

	/** Save the map, open dialog if not named yet. */
	void save_map(bool show_confirmation = true);

	editor_display& gui()
	{
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
	void expand_open_maps_menu(std::vector<config>& items, int i);

	/** Menu expanding for most recent loaded list */
	void expand_load_mru_menu(std::vector<config>& items, int i);

	/** Menu expanding for the map's player sides */
	void expand_sides_menu(std::vector<config>& items, int i);

	/** Menu expanding for the map's defined areas */
	void expand_areas_menu(std::vector<config>& items, int i);

	/** Menu expanding for the map's defined areas */
	void expand_time_menu(std::vector<config>& items, int i);

	/** Menu expanding for the map's defined areas */
	void expand_local_time_menu(std::vector<config>& items, int i);

	/** Display a load map dialog and process user input. */
	void load_map_dialog(bool force_same_context = false);

	/** Open the specified entry from the recent files list. */
	void load_mru_item(unsigned index, bool force_same_context = false);

	/** Display a scenario edit dialog and process user input. */
	void edit_scenario_dialog();

	/** Display a side edit dialog and process user input. */
	void edit_side_dialog(const team& t);

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

	std::size_t size()
	{
		return map_contexts_.size();
	}

	/** Get the current map context object */
	map_context& get_map_context()
	{
		return *map_contexts_[current_context_index_];
	}

	void set_addon_id(const std::string& id)
	{
		current_addon_ = id;
		for(auto& map : map_contexts_) {
			map->set_addon_id(id);
		}
	}

	/** Set the default dir (where the filebrowser is pointing at when there is no map file opened) */
	void set_default_dir(const std::string& str)
	{
		default_dir_ = str;
	}

	void edit_pbl();
	void change_addon_id();

	/** Inherited from @ref filter_context. */
	virtual const display_context& get_disp_context() const override
	{
		return get_map_context();
	}

	/** Inherited from @ref filter_context. */
	virtual const tod_manager& get_tod_man() const override
	{
		return *get_map_context().get_time_manager();
	}

	/** Inherited from @ref filter_context. */
	virtual const game_data* get_game_data() const override
	{
		return nullptr; // No game_data in the editor.
	}

	/** Inherited from @ref filter_context. */
	virtual game_lua_kernel* get_lua_kernel() const override
	{
		return nullptr; // No Lua kernel in the editor.
	}

	// TODO: Make this private with an accessor or something
	class location_palette* locs_;
private:
	/** init available random map generators */
	void init_map_generators(const game_config_view& game_config);

	/**
	 * Shows an are-you-sure dialog if the map was modified.
	 * @return true if the user confirmed or the map was not modified, false otherwise
	 */
	bool confirm_discard();

	/** Get the current map context object - const version */
	const map_context& get_map_context() const
	{
		return *map_contexts_[current_context_index_];
	}

	/**
	 * Add a map context. The controller assumes ownership.
	 * @return the index of the added map context in the map_contexts_ array
	 */
	template<typename... T>
	int add_map_context(const T&... args);

	int add_map_context_of(std::unique_ptr<map_context>&& mc);

	/**
	 * Replace the current map context and refresh accordingly
	 */
	template<typename... T>
	void replace_map_context(const T&... args);

	void replace_map_context_with(std::unique_ptr<map_context>&& mc);

	/**
	 * Creates a default map context object, used to ensure there is always at least one.
	 * When we have saved contexts, reopen them instead.
	 */
	void create_default_context();

	void create_blank_context();

	/** Performs the necessary housekeeping necessary when switching contexts. */
	void refresh_on_context_change();

public:
	/**
	 * Refresh the display after an action has been performed.
	 * The map context contains details of what needs to be refreshed.
	 */
	void refresh_after_action(bool drag_part = false);

	/** Closes the active map context. Switches to a valid context afterward or creates a dummy one. */
	void close_current_context();

	/** Switches the context to the one under the specified index. */
	void switch_context(const int index, const bool force = false);

private:
	/**
	 * Save the map under a given filename. Displays an error message on failure.
	 * @return true on success
	 */
	bool write_map(bool display_confirmation = true);
	bool write_scenario(bool display_confirmation = true);

	/**
	 * Create a new map.
	 */
	void new_map(int width, int height, const t_translation::terrain_code & fill, bool new_context);

	/**
	 * Create a new scenario.
	 */
	void new_scenario(int width, int height, const t_translation::terrain_code & fill, bool new_context);

	/**
	 * Check if a map is already open.
	 * @return index of the map context containing the given filename,
	 *         or map_contexts_.size() if not found.
	 */
	std::size_t check_open_map(const std::string& fn) const;

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

	const game_config_view& game_config_;

	/** Default directory for map load/save as dialogs */
	std::string default_dir_;

	/** The currently selected add-on */
	std::string current_addon_;

	/** Available random map generators */
	std::vector<std::unique_ptr<map_generator>> map_generators_;
	map_generator* last_map_generator_;

	int current_context_index_;

	/** Flag to rebuild terrain on every terrain change */
	int auto_update_transitions_;

	/** The currently opened map context object */
	std::vector<std::unique_ptr<map_context>> map_contexts_;

	/** Clipboard map_fragment -- used for copy-paste. */
	map_fragment clipboard_;
};

}
