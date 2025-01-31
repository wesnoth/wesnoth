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

class config;
class game_board;

#include "display.hpp"
#include "display_chat_manager.hpp"
#include "pathfind/pathfind.hpp"


// This needs to be separate from display.h because of the static
// singleton member, which will otherwise trigger link failure
// when building the editor.

class game_display : public display
{
public:
	game_display(game_board& board,
			std::weak_ptr<wb::manager> wb,
			reports & reports_object,
			const std::string& theme_id,
			const config& level);

	~game_display();

	game_display(const game_display&) = delete;
	game_display& operator=(const game_display&) = delete;

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

	virtual const std::set<std::string>& observers() const override { return chat_man_->observers(); }
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
	virtual void select_hex(map_location hex) override;

	/**
	 * Function to highlight a location.
	 *
	 * If a unit is in the location, it will be displayed in the sidebar.
	 * Selection is used when a unit has been clicked on, while highlighting is
	 * used when a location has been moused over.
	 */
	virtual void highlight_hex(map_location hex) override;

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
	 * Used by Show Enemy Moves.  If @a goal is not @c null_location, highlight
	 * enemy units that can reach @a goal.
	 */
	void highlight_another_reach(const pathfind::paths &paths_list,
			const map_location& goal = map_location::null_location());
	/**
	 * Return the locations of units that can reach @a goal (@see highlight_another_reach()).
	 */
	const std::set<map_location>& units_that_can_reach_goal() const { return units_that_can_reach_goal_; }

	/** Reset highlighting of paths. */
	bool unhighlight_reach();

	/**
	 * Sets the route along which footsteps are drawn to show movement of a
	 * unit. If nullptr, no route is displayed. @a route does not have to remain
	 * valid after being set.
	 */
	void set_route(const pathfind::marked_route *route);
	/**
	 * Gets the route along which footsteps are drawn to show movement of a
	 * unit. If no route is currently being shown, the array get_route().steps
	 * will be empty.
	 */
	const pathfind::marked_route& get_route() { return route_; }

	/** Function to float a label above a tile */
	void float_label(const map_location& loc, const std::string& text, const color_t& color);

	/** Draws the movement info (turns available) for a given location. */
	void draw_movement_info(const map_location& loc);

	/** Function to invalidate that unit status displayed on the sidebar. */
	void invalidate_unit() { invalidateGameStatus_ = true; }

	/** Same as invalidate_unit() if moving the displayed unit. */
	void invalidate_unit_after_move(const map_location& src, const map_location& dst);

	virtual const time_of_day& get_time_of_day(const map_location& loc) const override;

	virtual bool has_time_area() const override;

	/**
	 * TLD update() override. Replaces old pre_draw(). Be sure to call
	 * the base class method as well.
	 *
	 * game_display does specific things related e.g. to unit rendering
	 * and calls the whiteboard pre-draw method here.
	 */
	virtual void update() override;

	/**
	 * TLD layout() override. Replaces old refresh_reports(). Be sure to
	 * call the base class method as well.
	 *
	 * This updates some reports, like clock, that need to be refreshed
	 * every frame.
	 */
	virtual void layout() override;

	/**
	 * TLD render() override. Replaces old post_draw(). Be sure to call
	 * the base class method as well.
	 *
	 * This calls the whiteboard's post-draw method after rendering.
	 */
	virtual void render() override;

protected:
	virtual void draw_invalidated() override;

	virtual void draw_hex(const map_location& loc) override;

	/** Inherited from display. */
	virtual overlay_map& get_overlays() override;

	std::set<map_location> units_that_can_reach_goal_;

	std::vector<texture> get_reachmap_images(const map_location& loc) const;

public:
	/** Set the attack direction indicator. */
	void set_attack_indicator(const map_location& src, const map_location& dst);
	void clear_attack_indicator();
	// TODO: compare reports::context::mhb()->current_unit_attacks_from()
	const map_location& get_attack_indicator_src() { return attack_indicator_src_; }

	/** Function to get attack direction suffix. */
	std::string attack_indicator_direction() const {
		return map_location::write_direction(
			attack_indicator_src_.get_relative_dir(attack_indicator_dst_));
	}

	virtual const map_location &displayed_unit_hex() const override { return displayedUnitHex_; }

	display_chat_manager & get_chat_manager() { return *chat_man_; }

	void begin_game();

	virtual bool in_game() const override { return in_game_; }

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

	/** Sets whether the screen (map visuals) needs to be rebuilt. This is typically after the map has been changed by wml. */
	void needs_rebuild(bool b);

	/** Rebuilds the screen if needs_rebuild(true) was previously called, and resets the flag. */
	bool maybe_rebuild();

private:
	overlay_map overlay_map_;

	// Locations of the attack direction indicator's parts
	map_location attack_indicator_src_;
	map_location attack_indicator_dst_;

	pathfind::marked_route route_;

	void invalidate_route();

	map_location displayedUnitHex_;

	bool first_turn_, in_game_;

	const std::unique_ptr<display_chat_manager> chat_man_;

	game_mode mode_;

	bool needs_rebuild_;

};
