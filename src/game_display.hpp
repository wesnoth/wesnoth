/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#ifndef GAME_DISPLAY_H_INCLUDED
#define GAME_DISPLAY_H_INCLUDED

class config;
class tod_manager;
class team;
class unit;
class unit_map;
class game_board;

#include "animated.hpp"
#include "chat_events.hpp"
#include "display.hpp"
#include "pathfind/pathfind.hpp"
#include "unit.hpp" // TODO: Refactor so that this is not necessary

#include <deque>

// This needs to be separate from display.h because of the static
// singleton member, which will otherwise trigger link failure
// when building the editor.

class game_display : public display
{
public:
	game_display(game_board& board, CVideo& video,
			boost::weak_ptr<wb::manager> wb,
			const tod_manager& tod_manager,
			const config& theme_cfg,
			const config& level);

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

	/** Function to invalidate that unit status displayed on the sidebar. */
	void invalidate_unit() { invalidateGameStatus_ = true; }

	/** Same as invalidate_unit() if moving the displayed unit. */
	void invalidate_unit_after_move(const map_location& src, const map_location& dst);

	const time_of_day& get_time_of_day(const map_location& loc) const;

	bool has_time_area() const;

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

	/**
	 * the list of units we need to look at, game_display adds fake units
	 */
	virtual std::vector<const unit*> get_unit_list_for_invalidation();


public:
	/** A temporary unit that can be placed on the map.
		Temporary units can overlap units.
		Adding the same unit twice isn't allowed.
		The fake_unit owns its underlying unit and when
		it goes out of scope it removes itself from the fake_units list.
		The intent is to provide exception safety when the code
		creating the temp unit is unexpectedly forced out of scope.
	 */
	class fake_unit : public unit {
	public:
		explicit fake_unit(unit const & u) : unit(u), my_display_(NULL) {}
		fake_unit(fake_unit const & u) : unit(u), my_display_(NULL) {}
		fake_unit(const unit_type& t, int side, unit_race::GENDER gender = unit_race::NUM_GENDERS)
			: unit(t, side, false, gender)
			, my_display_(NULL)
		{}
		/// Assignment operator, taking a fake_unit.
		/// If already in the queue, @a this will be moved to the end of the
		/// queue (drawn last). The queue (if any) of the parameter is ignored.
		fake_unit & operator=(fake_unit const & u)
		{ return operator=(static_cast<unit const &>(u)); }
		/// Assignment operator, taking a unit.
		virtual fake_unit & operator=(unit const & u);
		/// Removes @a this from the fake_units_ list if necessary.
		~fake_unit();

		/// Place @a this on @a display's fake_units_ dequeue.
		void place_on_game_display(game_display * d);
		/// Removes @a this from whatever fake_units_ list it is on (if any).
		int remove_from_game_display();

	private :
		game_display * my_display_;
	};

	//Anticipate making place_temporary_unit and remove_temporary_unit private to force exception safety
	friend class game_display::fake_unit;

private:
	/** Temporarily place a unit on map (moving: can overlap others).
	 *  The temp unit is added at the end of the temporary unit dequeue,
	 *  and therefore gets drawn last, over other units and temp units.
	 *  Adding the same unit twice isn't allowed.
	 */
	void place_temporary_unit(unit *u);

	/** Removes any instances of this temporary unit from the temporary unit vector.
	 *  Returns the number of temp units deleted (0 or 1, any other number indicates an error).
	 */
	int remove_temporary_unit(unit *u);
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
	t_translation::t_terrain get_terrain_on(int palx, int paly, int x, int y);

	void send_notification(const std::string& owner, const std::string& message);

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

	void add_observer(const std::string& name) { observers_.insert(name); }
	void remove_observer(const std::string& name) { observers_.erase(name); }
	const std::set<std::string>& observers() const { return observers_; }

	void add_chat_message(const time_t& time, const std::string& speaker,
		int side, const std::string& msg, events::chat_handler::MESSAGE_TYPE type, bool bell);
	void clear_chat_messages() { prune_chat_messages(true); }

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
	enum tgame_mode {
		RUNNING,         /**< no linger overlay, show fog and shroud. */
		LINGER_SP,       /**< linger overlay, show fog and shroud. */
		LINGER_MP };     /**< linger overlay, show fog and shroud. */

	void set_game_mode(const tgame_mode game_mode);

private:
	game_display(const game_display&);
	void operator=(const game_display&);

	void draw_sidebar();

	overlay_map overlay_map_;

	/// collection of units destined to be drawn but not put into the unit map
	std::deque<unit*> fake_units_;

	// Locations of the attack direction indicator's parts
	map_location attack_indicator_src_;
	map_location attack_indicator_dst_;



	pathfind::marked_route route_;

	const tod_manager& tod_manager_;

	const config& level_;

	void invalidate_route();

	map_location displayedUnitHex_;

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



	tgame_mode game_mode_;

	// For debug mode
	static std::map<map_location, int> debugHighlights_;



};

#endif
