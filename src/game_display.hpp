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

/** @file */

#pragma once

class config;
class tod_manager;
class team;
class unit_map;
class game_board;

#include "animated.hpp"
#include "chat_events.hpp"
#include "display.hpp"
#include "display_chat_manager.hpp"
#include "pathfind/pathfind.hpp"

#include <deque>

// This needs to be separate from display.h because of the static
// singleton member, which will otherwise trigger link failure
// when building the editor.

class game_display : public display
{
public:
	game_display(game_board& board, CVideo& video,
			std::weak_ptr<wb::manager> wb,
			reports & reports_object,
			const tod_manager& tod_manager,
			const config& theme_cfg,
			const config& level,
			bool dummy=false);

	static game_display* create_dummy_display(CVideo& video);

	~game_display();
	static game_display* get_singleton()
	{
		return static_cast<game_display*>(singleton_);
	}

	/**
	 * Update lighting settings.
	 *
	 * Should be called on every new turn.
	 */
	void new_turn();

	const std::set<std::string>& observers() const { return chat_man_->observers(); }
	/**
	 * Scrolls to the leader of a certain side.
	 *
	 * This will normally be the playing team.
	 */
	void scroll_to_leader(int side, SCROLL_TYPE scroll_type = ONSCREEN,bool force = true);

	/**
	 * Function to display a location as selected.
	 *
	 * If a unit is in the location, and there is no unit in the currently
	 * highlighted hex, the unit will be displayed in the sidebar.
	 */
	virtual void select_hex(map_location hex);

	/**
	 * Function to highlight a location.
	 *
	 * If a unit is in the location, it will be displayed in the sidebar.
	 * Selection is used when a unit has been clicked on, while highlighting is
	 * used when a location has been moused over.
	 */
	virtual void highlight_hex(map_location hex);

	/**
	 * Change the unit to be displayed in the sidebar.
	 *
	 * This is used when selecting or highlighting is not wanted.
	 */
	void display_unit_hex(map_location hex);

	/**
	 * Sets the paths that are currently displayed as available
	 * for the unit to move along.
	 * All other paths will be grayed out.
	 */
	void highlight_reach(const pathfind::paths &paths_list);

	/**
	 * Add more paths to highlight.  Print numbers where they overlap.
	 * Used only by Show Enemy Moves.
	 */
	void highlight_another_reach(const pathfind::paths &paths_list);

	/** Reset highlighting of paths. */
	void unhighlight_reach();

	/**
	 * Sets the route along which footsteps are drawn to show movement of a
	 * unit. If nullptr, no route is displayed. @a route does not have to remain
	 * valid after being set.
	 */
	void set_route(const pathfind::marked_route *route);

	/** Function to float a label above a tile */
	void float_label(const map_location& loc, const std::string& text, const color_t& color);

	/** Draws the movement info (turns available) for a given location. */
	void draw_movement_info(const map_location& loc);

	/** Function to invalidate that unit status displayed on the sidebar. */
	void invalidate_unit() { invalidateGameStatus_ = true; }

	/** Same as invalidate_unit() if moving the displayed unit. */
	void invalidate_unit_after_move(const map_location& src, const map_location& dst);

	const time_of_day& get_time_of_day(const map_location& loc) const;

	bool has_time_area() const;

	const tod_manager & get_tod_man() const { return *tod_manager_; } /**< Allows this class to properly implement filter context, used for animations */

protected:
	/**
	 * game_display pre_draw does specific things related e.g. to unit rendering
	 * and calls the whiteboard pre-draw method.
	 */
	void pre_draw();
	/**
	 * Calls the whiteboard's post-draw method.
	 */
	void post_draw();

	void draw_invalidated();

	void post_commit();

	void draw_hex(const map_location& loc);


public:



	/** Set the attack direction indicator. */
	void set_attack_indicator(const map_location& src, const map_location& dst);
	void clear_attack_indicator();

	/** Function to get attack direction suffix. */
	std::string attack_indicator_direction() const {
		return map_location::write_direction(
			attack_indicator_src_.get_relative_dir(attack_indicator_dst_));
	}

	// Functions used in the editor:

	//void draw_terrain_palette(int x, int y, terrain_type::TERRAIN selected);
	t_translation::terrain_code get_terrain_on(int palx, int paly, int x, int y);

	/**
	 * Sets the team controlled by the player using the computer.
	 *
	 * Data from this team will be displayed in the game status.
	 * set_playing_team sets the team whose turn it currently is
	 */
	void set_playing_team(size_t team);


	const map_location &displayed_unit_hex() const { return displayedUnitHex_; }


	/**
	 * annotate hex with number, useful for debugging or UI prototype
	 */
	static int& debug_highlight(const map_location& loc);
	static void clear_debug_highlights() { debugHighlights_.clear(); }


	/** The playing team is the team whose turn it is. */
	int playing_side() const { return activeTeam_ + 1; }


	std::string current_team_name() const;

	display_chat_manager & get_chat_manager() { return *chat_man_; }

	void begin_game();

	virtual bool in_game() const { return in_game_; }


	/**
	 * Sets the linger mode for the display.
	 * There have been some discussions on what to do with fog and shroud
	 * the extra variables make it easier to modify the behavior. There
	 * might even be a split between victory and defeat.
	 *
	 * @todo if the current implementation is wanted we can change
	 * the stuff back to a boolean
	 */
	enum game_mode {
		RUNNING,         /**< no linger overlay, show fog and shroud. */
		LINGER };     /**< linger overlay, show fog and shroud. */

	void set_game_mode(const game_mode mode);

	/// Sets whether the screen (map visuals) needs to be rebuilt. This is typically after the map has been changed by wml.
	void needs_rebuild(bool b);

	/// Rebuilds the screen if needs_rebuild(true) was previously called, and resets the flag.
	bool maybe_rebuild();
	void reset_tod_manager(const tod_manager& tod_manager)
	{
		tod_manager_ = &tod_manager;
	}
private:
	game_display(const game_display&);
	void operator=(const game_display&);

	void draw_sidebar();

	overlay_map overlay_map_;

	// Locations of the attack direction indicator's parts
	map_location attack_indicator_src_;
	map_location attack_indicator_dst_;



	pathfind::marked_route route_;

	const tod_manager* tod_manager_;

	void invalidate_route();

	map_location displayedUnitHex_;

	double sidebarScaling_;

	bool first_turn_, in_game_;

	const std::unique_ptr<display_chat_manager> chat_man_;

	game_mode mode_;

	// For debug mode
	static std::map<map_location, int> debugHighlights_;

	bool needs_rebuild_;

};
