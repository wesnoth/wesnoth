/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file game_display.hpp */

#ifndef GAME_DISPLAY_H_INCLUDED
#define GAME_DISPLAY_H_INCLUDED

class config;
class tod_manager;
class team;
class unit;
class unit_map;

#include "animated.hpp"
#include "chat_events.hpp"
#include "display.hpp"
#include "image.hpp"
#include "pathfind/pathfind.hpp"

#include "SDL.h"

#include <map>
#include <set>
#include <string>
#include <deque>

// This needs to be separate from display.h because of the static
// singleton member, which will otherwise trigger link failure
// when building the editor.

class game_display : public display
{
public:
	game_display(unit_map& units, CVideo& video,
			const gamemap& map, const tod_manager& tod_manager,
			const std::vector<team>& t, const config& theme_cfg,
			const config& level);

	static game_display* create_dummy_display(CVideo& video);

	~game_display();
	static game_display* get_singleton() { return singleton_ ;}


	/**
	 * Update lighting settings.
	 *
	 * Should be called on every new turn.
	 */
	void new_turn();

	/**
	 * Add r,g,b to the colours for all images displayed on the map.
	 *
	 * Used for special effects like flashes.
	 */
	void adjust_colours(int r, int g, int b);

	/**
	 * Scrolls to the leader of a certain side.
	 *
	 * This will normally be the playing team.
	 */
	void scroll_to_leader(unit_map& units, int side, SCROLL_TYPE scroll_type = ONSCREEN,bool force = true);

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
	 * All other paths will be greyed out.
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
	 * unit. If NULL, no route is displayed. @a route does not have to remain
	 * valid after being set.
	 */
  void set_route(const pathfind::marked_route *route);

	/** Function to float a label above a tile */
	void float_label(const map_location& loc, const std::string& text,
	                 int red, int green, int blue);

	/**
	 * Function to return 2 half-hex footsteps images for the given location.
	 * Only loc is on the current route set by set_route.
	 */
	std::vector<surface> footsteps_images(const map_location& loc);

	/** Draws the movement info (turns available) for a given location. */
	void draw_movement_info(const map_location& loc);

	void draw_report(reports::TYPE report_num);

	/** Function to invalidate that unit status displayed on the sidebar. */
	void invalidate_unit() { invalidateUnit_ = true; }

	/** Same as invalidate_unit() if moving the displayed unit. */
	void invalidate_unit_after_move(const map_location& src, const map_location& dst);

protected:
	/**
	 * game_display pre_draw does specific things related e.g. to unit rendering
	 */
	void pre_draw();

	/**
	 * This function runs through invalidated_ and returns a vector of tiles
	 * containing units, sorted according to a custom ordering
	 */
	std::vector<map_location> get_invalidated_unit_locations();

	/**
	 * Hex brightening for game - take units into account
	 */
	image::TYPE get_image_type(const map_location& loc);

	/**
	 * Draws units on specified hexes
	 */
	void redraw_units(const std::vector<map_location>& invalidated_unit_locations);

	void draw_invalidated();

	void post_commit();

	void draw_hex(const map_location& loc);

	void update_time_of_day();

	/**
	 * Animated hex invalidation specific to gameplay
	 */
	void invalidate_animations();

	/**
	 * Extra game per-location invalidation (village ownership)
	 */
	void invalidate_animations_location(const map_location& loc);

	virtual void draw_minimap_units();

public:
	/** Temporarily place a unit on map (moving: can overlap others).
	 *  The temp unit is added at the end of the temporary unit deque,
	 *  and therefore gets drawn last, over other units and temp units.
	 *  Adding the same unit twice isn't allowed.
	 */
	void place_temporary_unit(unit *u);
	/** Removes any instances of this temporary unit from the temporary unit vector.
	 *  Returns the number of temp units deleted (normally 0 or 1).
	 */
	int remove_temporary_unit(unit *u);
	/** Returns a reference to the temp unit deque.
	 *  Use the place/remove methods instead of this whenever possible.
	 */
	std::deque<unit*>& get_temp_units() {return temp_units_;}

	/** Set the attack direction indicator. */
	void set_attack_indicator(const map_location& src, const map_location& dst);
	void clear_attack_indicator();

	/** Function to get attack direction suffix. */
	std::string attack_indicator_direction() const {
		return map_location::write_direction(
			attack_indicator_src_.get_relative_dir(attack_indicator_dst_));
	}

	/**
	 * Functions to add and remove overlays from locations.
	 *
	 * An overlay is an image that is displayed on top of the tile.
	 * One tile may have multiple overlays.
	 */
	void add_overlay(const map_location& loc, const std::string& image,
		const std::string& halo="", const std::string& team_name="",
		bool visible_under_fog = true);

	/** remove_overlay will remove all overlays on a tile. */
	void remove_overlay(const map_location& loc);

	/** remove_single_overlay will remove a single overlay from a tile */
	void remove_single_overlay(const map_location& loc, const std::string& toDelete);

	/** Function to serialize overlay data. */
	void write_overlays(config& cfg) const;

	/**
	 * Check the overlay_map for proper team-specific overlays to be
	 * displayed/hidden
	 */
	void parse_team_overlays();

	// Functions used in the editor:

	//void draw_terrain_palette(int x, int y, terrain_type::TERRAIN selected);
	t_translation::t_terrain get_terrain_on(int palx, int paly, int x, int y);

	void send_notification(const std::string& owner, const std::string& message);

	/**
	 * Sets the team controlled by the player using the computer.
	 *
	 * Data from this team will be displayed in the game status.
	 * set_playing_team sets the team whose turn it currently is
	 */
	void set_team(size_t team, bool observe=false);
	void set_playing_team(size_t team);
	size_t get_playing_team() const {return activeTeam_;}
	const std::vector<team>& get_teams() {return teams_;}

	unit_map& get_units() {return units_;}
	const unit_map& get_const_units() const {return units_;}

	/**
	 * Draws a cross on a tile to emphasize something there.
	 *
	 * It is used in debug mode, typically to show AI plans.
	 */
	static void debug_highlight(const map_location& loc, fixed_t amount);
	static void clear_debug_highlights() { debugHighlights_.clear(); }

	/** The viewing team is the team currently viewing the game. */
	size_t viewing_team() const { return currentTeam_; }
	int viewing_side() const { return currentTeam_ + 1; }

	/** The playing team is the team whose turn it is. */
	size_t playing_team() const { return activeTeam_; }

	bool team_valid() const { return currentTeam_ < teams_.size(); }
	std::string current_team_name() const;

	void add_observer(const std::string& name) { observers_.insert(name); }
	void remove_observer(const std::string& name) { observers_.erase(name); }
	const std::set<std::string>& observers() const { return observers_; }

	void add_chat_message(const time_t& time, const std::string& speaker,
		int side, const std::string& msg, events::chat_handler::MESSAGE_TYPE type, bool bell);
	void clear_chat_messages() { prune_chat_messages(true); }

	void begin_game();

	virtual bool in_game() const { return in_game_; }
	void draw_bar(const std::string& image, int xpos, int ypos,
		const map_location& loc, size_t height, double filled,
		const SDL_Color& col, fixed_t alpha);

	/**
	 * Sets the linger mode for the display.
	 * There have been some discussions on what to do with fog and shroud
	 * the extra variables make it easier to modify the behaviour. There
	 * might even be a split between victory and defeat.
	 *
	 * @todo if the current implementation is wanted we can change
	 * the stuff back to a boolean
	 */
	enum tgame_mode {
		RUNNING,         /**< no linger overlay, show fog and shroud. */
		LINGER_SP,       /**< linger overlay, show fog and shroud. */
		LINGER_MP };     /**< linger overlay, show fog and shroud. */

	void set_game_mode(const tgame_mode game_mode);

private:
	game_display(const game_display&);
	void operator=(const game_display&);

	void draw_sidebar();
	void draw_game_status();

	// This surface must be freed by the caller
	surface get_flag(const map_location& loc);

	unit_map& units_;

	std::deque<unit*> temp_units_;

	// Locations of the attack direction indicator's parts
	map_location attack_indicator_src_;
	map_location attack_indicator_dst_;

	/**
	 * Finds the start and end rows on the energy bar image.
	 *
	 * White pixels are substituted for the colour of the energy.
	 */
	const SDL_Rect& calculate_energy_bar(surface surf);
	std::map<surface,SDL_Rect> energy_bar_rects_;

	pathfind::marked_route route_;

	const tod_manager& tod_manager_;

	const std::vector<team>& teams_;

	const config& level_;

	void invalidate_route();

	bool invalidateUnit_;
	map_location displayedUnitHex_;

	struct overlay {
		overlay(const std::string& img, const std::string& halo_img,
		        int handle, const std::string& overlay_team_name, const bool fogged) : image(img), halo(halo_img),
				team_name(overlay_team_name), halo_handle(handle) , visible_in_fog(fogged){}
		std::string image;
		std::string halo;
		std::string team_name;
		int halo_handle;
		bool visible_in_fog;
	};

	typedef std::multimap<map_location,overlay> overlay_map;

	overlay_map overlays_;

	size_t currentTeam_, activeTeam_;

	double sidebarScaling_;

	bool first_turn_, in_game_;

	std::set<std::string> observers_;

	struct chat_message
	{
		chat_message(int speaker, int h) : speaker_handle(speaker), handle(h), created_at(SDL_GetTicks())
		{}

		int speaker_handle;
		int handle;
		Uint32 created_at;
	};

	void prune_chat_messages(bool remove_all=false);

	std::vector<chat_message> chat_messages_;

	// If we're transitioning from one time of day to the next,
	// then we will use these two masks on top of all hexes when we blit.
	surface tod_hex_mask1, tod_hex_mask2;

	// Tiles lit for showing where unit(s) can reach
	typedef std::map<map_location,unsigned int> reach_map;
	reach_map reach_map_;
	reach_map reach_map_old_;
	bool reach_map_changed_;
	void process_reachmap_changes();

	tgame_mode game_mode_;

	// For debug mode
	static std::map<map_location,fixed_t> debugHighlights_;

	/** Animated flags for each team */
	std::vector<animated<image::locator> > flags_;

	/**
	 * the tiles invalidated at last redraw,
	 * to simplify the cleaning up of tiles left by units
	 */
	static game_display * singleton_;
};

#endif
