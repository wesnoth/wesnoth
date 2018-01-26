/*
   Copyright (C) 2008 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "editor/map/editor_map.hpp"
#include "game_classification.hpp"
#include "map/label.hpp"
#include "mp_game_settings.hpp"
#include "sound_music_track.hpp"
#include "team.hpp"
#include "tod_manager.hpp"
#include "units/map.hpp"
#include "overlay.hpp"
#include "display_context.hpp"

namespace editor {

struct editor_team_info {
	editor_team_info(const team& t);

	int side;
	std::string id;
	std::string name;
	int gold;
	int income;
	int village_income;
	int village_support;
	bool fog;
	bool shroud;
	team::SHARE_VISION share_vision;
	team::CONTROLLER controller;
	bool no_leader;
	bool hidden;
};

/**
 * This class wraps around a map to provide a concise interface for the editor to work with.
 * The actual map object can change rapidly (be assigned to), the map context persists
 * data (like the undo stacks) in this case. The functionality is here, not in editor_controller
 * as e.g. the undo stack is part of the map, not the editor as a whole. This might allow many
 * maps to be open at the same time.
 */
class map_context : public display_context
{
public:
	map_context(const map_context&) = delete;
	map_context& operator=(const map_context&) = delete;

	/**
	 * Create a map context from an existing map. The filename is set to be
	 * empty, indicating a new map.
	 * Marked "explicit" to avoid automatic conversions.
	 */
	explicit map_context(const editor_map& map, bool pure_map, const config& schedule);

	/**
	 * Create map_context from a map file. If the map cannot be loaded, an
	 * exception will be thrown and the object will not be constructed. If the
	 * map file is a scenario, the map specified in its map_data key will be
	 * loaded, and the stored filename updated accordingly. Maps embedded
	 * inside scenarios do not change the filename, but set the "embedded" flag
	 * instead.
	 */
	map_context(const config& game_config, const std::string& filename);

	/**
	 * Map context destructor
	 */
	virtual ~map_context();

	/**
	 * Select the nth tod area.
	 * @param index of the tod area to select.
	 */
	bool select_area(int index);

	/**
	 * Map accessor
	 */
	editor_map& get_map() { return map_; }

	/**
	 * Map accessor - const version
	 */
	const editor_map& get_map() const { return map_; }

	/** Adds a new side to the map */
	void new_side();

	/** removes the last side from the scenario */
	void remove_side() {
		teams_.pop_back();
		++actions_since_save_;
	}

	void save_area(const std::set<map_location>& area) {
		tod_manager_->replace_area_locations(active_area_, area);
	}

	void new_area(const std::set<map_location>& area) {
		tod_manager_->add_time_area("", area, config());
		active_area_ = tod_manager_->get_area_ids().size() -1;
		++actions_since_save_;
	}

	void remove_area(int index);

	/** Get the team from the current map context object */
	std::vector<team>& get_teams() {
		return teams_;
	}

	map_labels& get_labels() {
		return labels_;
	}

	void replace_schedule(const std::vector<time_of_day>& schedule);

	// Import symbol from base class.
	using display_context::units;

	/**
	 * Const accessor names needed to implement "display_context" interface
	 */
	virtual const unit_map & units() const {
		return units_;
	}

	/** Local non-const overload of @ref units */
	unit_map& units()
	{
		return units_;
	}

	virtual const std::vector<team>& teams() const {
		return teams_;
	}
	virtual const gamemap & map() const {
		return map_;
	}
	virtual const std::vector<std::string>& hidden_label_categories() const {
		return lbl_categories_;
	}

	/**
	 * Replace the [time]s of the currently active area.
	 */
	void replace_local_schedule(const std::vector<time_of_day>& schedule);

	/**
	 * TODO
	 */
	void set_starting_time(int time);

	/**
	 * TODO
	 */
	void set_local_starting_time(int time) {
		tod_manager_->set_current_time(time, active_area_);
		++actions_since_save_;
	}

	tod_manager* get_time_manager() {
		return tod_manager_.get();
	}

	mp_game_settings & get_mp_settings() {
		return mp_settings_;
	}
	game_classification& get_classification() {
		return game_classification_;
	}

	/**
	 *
 	 * @return the index of the currently active area.
 	 */
	int get_active_area() const {
		return active_area_;
	}

	void set_active_area(int index) {
		active_area_ = index;
	}

	bool is_in_playlist(std::string track_id) {
		return music_tracks_.find(track_id) != music_tracks_.end();
	}

	void add_to_playlist(const sound::music_track& track) {

		if (music_tracks_.find(track.id()) == music_tracks_.end())
				music_tracks_.emplace(track.id(), track);
		else music_tracks_.erase(track.id());
	}

	/**
	 * Draw a terrain on a single location on the map.
	 * Sets the refresh flags accordingly.
	 */
	void draw_terrain(const t_translation::terrain_code & terrain, const map_location& loc,
		bool one_layer_only = false);

	/**
	 * Actual drawing function used by both overloaded variants of draw_terrain.
	 */
	void draw_terrain_actual(const t_translation::terrain_code & terrain, const map_location& loc,
		bool one_layer_only = false);

	/**
	 * Draw a terrain on a set of locations on the map.
	 * Sets the refresh flags accordingly.
	 */
	void draw_terrain(const t_translation::terrain_code & terrain, const std::set<map_location>& locs,
		bool one_layer_only = false);

	/**
	 * Getter for the reload flag. Reload is the highest level of required refreshing,
	 * set when the map size has changed or the map was reassigned.
	 */
	bool needs_reload() const { return needs_reload_; }

	/**
	 * Setter for the reload flag
	 */
	void set_needs_reload(bool value=true) { needs_reload_ = value; }

	/**
	 * Getter for the terrain rebuild flag. Set whenever any terrain has changed.
	 */
	bool needs_terrain_rebuild() const { return needs_terrain_rebuild_; }

	/**
	 * Setter for the terrain rebuild flag
	 */
	void set_needs_terrain_rebuild(bool value=true) { needs_terrain_rebuild_ = value; }

	/**
	 * TODO
	 */
	void set_scenario_setup(const std::string& id, const std::string& name, const std::string& description,
			int turns, int xp_mod, bool victory_defeated, bool random_time);

	/**
	 * TODO
	 */
	void set_side_setup(editor_team_info& info);

	/**
	 * Getter for the labels reset flag. Set when the labels need to be refreshed.
	 */
	bool needs_labels_reset() const { return needs_labels_reset_; }

	/**
	 * Setter for the labels reset flag
	 */
	void set_needs_labels_reset(bool value=true) { needs_labels_reset_ = value; }

	const std::set<map_location> changed_locations() const { return changed_locations_; }
	void clear_changed_locations();
	void add_changed_location(const map_location& loc);
	void add_changed_location(const std::set<map_location>& locs);
	void set_everything_changed();
	bool everything_changed() const;

	void set_labels(display& disp);

	void clear_starting_position_labels(display& disp);

	void set_starting_position_labels(display& disp);

	void reset_starting_position_labels(display& disp);

	const std::string& get_filename() const { return filename_; }

	void set_filename(const std::string& fn) { filename_ = fn; }

	const std::string& get_map_data_key() const { return map_data_key_; }

	const std::string& get_id() const { return scenario_id_; }
	const std::string& get_description() const { return scenario_description_; }
	const std::string& get_name() const { return scenario_name_; }

	const t_string get_default_context_name() const;

	int get_xp_mod() const { return xp_mod_; }

	bool random_start_time() const { return random_time_; }
	bool victory_defeated() const { return victory_defeated_; }

	bool is_embedded() const { return embedded_; }

	bool is_pure_map() const { return pure_map_; }

	void set_embedded(bool v) { embedded_ = v; }

	/**
	 * Saves the map under the current filename. Filename must be valid.
	 * May throw an exception on failure.
	 */
	bool save_map();

	/**
	 * Saves the scenario under the current filename. Filename must be valid.
	 * May throw an exception on failure.
	 */
	bool save_scenario();


	void load_scenario(const config& game_config);

	config to_config();

	void set_map(const editor_map& map);

	/**
	 * Performs an action (thus modifying the map). An appropriate undo action is added to
	 * the undo stack. The redo stack is cleared. Note that this may throw, use caution
	 * when calling this with a dereferenced pointer that you own (i.e. use a smart pointer).
	 */
	void perform_action(const editor_action& action);

	/**
	 * Performs a partial action, assumes that the top undo action has been modified to
	 * maintain coherent state of the undo stacks, and so a new undo action is not
	 * created.
	 */
	void perform_partial_action(const editor_action& action);

	/** @return whether the map was modified since the last save */
	bool modified() const;

	/** Clear the modified state */
	void clear_modified();

	/** Adds the map to the editor's recent files list. */
	void add_to_recent_files();

	/** @return true when undo can be performed, false otherwise */
	bool can_undo() const;

	/** @return true when redo can be performed, false otherwise */
	bool can_redo() const;

	/** @return a pointer to the last undo action or nullptr if the undo stack is empty */
	editor_action* last_undo_action();

	/** @return a pointer to the last redo action or nullptr if the undo stack is empty */
	editor_action* last_redo_action();

	/** const version of last_undo_action */
	const editor_action* last_undo_action() const;

	/** const version of last_redo_action */
	const editor_action* last_redo_action() const;

	/** Un-does the last action, and puts it in the redo stack for a possible redo */
	void undo();

	/** Re-does a previously undid action, and puts it back in the undo stack. */
	void redo();

	/**
	 * Un-does a single step from a undo action chain. The action is separated
	 * from the chain and it's undo (the redo) is added as a stand-alone action
	 * to the redo stack.
	 * Precondition: the last undo action has to actually be an action chain.
	 */
	void partial_undo();

	/**
	 * Clear the undo and redo stacks
	 */
	void clear_undo_redo();

protected:
	/**
	 * The actual filename of this map. An empty string indicates a new map.
	 */
	std::string filename_;

	/**
	 * When a scenario file is loaded, the referenced map is loaded instead.
	 * The verbatim form of the reference is kept here.
	 */
	std::string map_data_key_;

	/**
	 * Whether the map context refers to a map embedded in a scenario file.
	 * This distinction is important in order to avoid overwriting the scenario.
	 */
	bool embedded_;

	/**
	 * Whether the map context refers to a file containing only the pure map data.
	 */
	bool pure_map_;

	/**
	 * The map object of this map_context.
	 */
	editor_map map_;

	/**
	 * Checks if an action stack reached its capacity and removes the front element if so.
	 */
	void trim_stack(action_stack& stack);

	/**
	 * Perform an action at the back of one stack, and then move it to the back of the other stack.
	 * This is the implementation of both undo and redo which only differ in the direction.
	 */
	void perform_action_between_stacks(action_stack& from, action_stack& to);

	/**
	 * The undo stack. A double-ended queues due to the need to add items to one end,
	 * and remove from both when performing the undo or when trimming the size. This container owns
	 * all contents, i.e. no action in the stack shall be deleted, and unless otherwise noted the contents
	 * could be deleted at an time during normal operation of the stack. To work on an action, either
	 * remove it from the container or make a copy. Actions are inserted at the back of the container
	 * and disappear from the front when the capacity is exceeded.
	 * @todo Use boost's pointer-owning container?
	 */
	action_stack undo_stack_;

	/**
	 * The redo stack. @see undo_stack_
	 */
	action_stack redo_stack_;

	/**
	 * Action stack (i.e. undo and redo) maximum size
	 */
	static const size_t max_action_stack_size_;

	/**
	 * Number of actions performed since the map was saved. Zero means the map was not modified.
	 */
	int actions_since_save_;

	/**
	 * Cache of set starting position labels. Necessary for removing them.
	 */
	std::set<map_location> starting_position_label_locs_;

	/**
	 * Refresh flag indicating the map in this context should be completely reloaded by the display
	 */
	bool needs_reload_;

	/**
	 * Refresh flag indicating the terrain in the map has changed and requires a rebuild
	 */
	bool needs_terrain_rebuild_;

	/**
	 * Refresh flag indicating the labels in the map have changed
	 */
	bool needs_labels_reset_;

	std::set<map_location> changed_locations_;
	bool everything_changed_;

private:

	std::string scenario_id_, scenario_name_, scenario_description_;

	int xp_mod_;
	bool victory_defeated_, random_time_;

	int active_area_;

	map_labels labels_;
	unit_map units_;
	std::vector<team> teams_;
	std::vector<std::string> lbl_categories_;
	std::unique_ptr<tod_manager> tod_manager_;
	mp_game_settings mp_settings_;
	game_classification game_classification_;

	typedef std::map<std::string, sound::music_track> music_map;
	music_map music_tracks_;

	typedef std::multimap<map_location, overlay> overlay_map;
	overlay_map overlays_;

public:

	overlay_map& get_overlays() { return overlays_; }

};


} //end namespace editor
