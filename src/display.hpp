/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 *
 * map_display and display: classes which take care of
 * displaying the map and game-data on the screen.
 *
 * The display is divided into two main sections:
 * - the game area, which displays the tiles of the game board, and units on them,
 * - and the side bar, which appears on the right hand side.
 * The side bar display is divided into three sections:
 * - the minimap, which is displayed at the top right
 * - the game status, which includes the day/night image,
 *   the turn number, information about the current side,
 *   and information about the hex currently moused over (highlighted)
 * - the unit status, which displays an image and stats
 *   for the current unit.
 */

#pragma once

class config;
class fake_unit_manager;
class terrain_builder;
class map_labels;
class arrow;
class reports;
class team;

namespace halo
{
class manager;
}

namespace wb
{
class manager;
}

#include "animated.hpp"
#include "display_context.hpp"
#include "drawing_queue.hpp"
#include "font/sdl_ttf.hpp"
#include "font/standard_colors.hpp"
#include "image.hpp" //only needed for enums (!)
#include "key.hpp"
#include "overlay.hpp"
#include "sdl/rect.hpp"
#include "theme.hpp"
#include "time_of_day.hpp"
#include "utils/functional.hpp"
#include "video.hpp"
#include "widgets/button.hpp"

#include <SDL_rect.h>
#include <boost/circular_buffer.hpp>

#include <chrono>
#include <cstdint>
#include <deque>
#include <list>
#include <map>
#include <memory>

class gamemap;

class display : public video2::draw_layering
{
public:
	display(const display_context* dc,
			std::weak_ptr<wb::manager> wb,
			reports& reports_object,
			const config& theme_cfg,
			const config& level,
			bool auto_join = true);

	virtual ~display();

	/**
	 * Returns the display object if a display object exists. Otherwise it returns nullptr.
	 * Rhe display object represents the game gui which handles ThemeWML and drawing the map.
	 * A display object only exists during a game or while the map editor is running.
	 */
	static display* get_singleton()
	{
		return singleton_;
	}

	bool show_everything() const
	{
		return !dont_show_all_ && !is_blindfolded();
	}

	const gamemap& get_map() const
	{
		return dc_->map();
	}

	const std::vector<team>& get_teams() const
	{
		return dc_->teams();
	}

	/** The playing team is the team whose turn it is. */
	size_t playing_team() const
	{
		return activeTeam_;
	}

	bool team_valid() const;

	/** The viewing team is the team currently viewing the game. */
	size_t viewing_team() const
	{
		return currentTeam_;
	}

	int viewing_side() const
	{
		return currentTeam_ + 1;
	}

	/**
	 * Sets the team controlled by the player using the computer.
	 * Data from this team will be displayed in the game status.
	 */
	void set_team(size_t team, bool observe = false);

	/**
	 * set_playing_team sets the team whose turn it currently is
	 */
	void set_playing_team(size_t team);

	/**
	 * Cancels all the exclusive draw requests.
	 */
	void clear_exclusive_draws()
	{
		exclusive_unit_draw_requests_.clear();
	}

	const unit_map& get_units() const
	{
		return dc_->units();
	}

	/**
	 * Allows a unit to request to be the only one drawn in its hex. Useful for situations where
	 * multiple units (one real, multiple temporary) can end up stacked, such as with the whiteboard.
	 * @param loc The location of the unit requesting exclusivity.
	 * @param unit The unit requesting exclusivity.
	 * @return false if there's already an exclusive draw request for this location.
	 */
	bool add_exclusive_draw(const map_location& loc, unit& unit);

	/**
	 * Cancels an exclusive draw request.
	 * @return The id of the unit whose exclusive draw request was canceled, or else
	 *         the empty string if there was no exclusive draw request for this location.
	 */
	std::string remove_exclusive_draw(const map_location& loc);

	/**
	 * Functions to add and remove overlays from locations.
	 *
	 * An overlay is an image that is displayed on top of the tile.
	 * One tile may have multiple overlays.
	 */
	void add_overlay(const map_location& loc,
			const std::string& image,
			const std::string& halo = "",
			const std::string& team_name = "",
			const std::string& item_id = "",
			bool visible_under_fog = true);

	/** remove_overlay will remove all overlays on a tile. */
	void remove_overlay(const map_location& loc);

	/** remove_single_overlay will remove a single overlay from a tile */
	void remove_single_overlay(const map_location& loc, const std::string& toDelete);

	/**
	 * Updates internals that cache map size. This should be called when the map
	 * size has changed.
	 */
	void reload_map();

	void change_display_context(const display_context* dc);

	const display_context& get_disp_context() const
	{
		return *dc_;
	}

	void reset_halo_manager();

	void reset_halo_manager(halo::manager& hm);

	halo::manager& get_halo_manager()
	{
		return *halo_man_;
	}

	/**
	 * Applies r,g,b coloring to the map.
	 *
	 * The color is usually taken from @ref get_time_of_day unless @a tod_override is given, in which
	 * case that color is used.
	 *
	 * @param tod_override             The ToD to apply to the map instead of that of the current ToD's.
	 */
	void update_tod(const time_of_day* tod_override = nullptr);

	/**
	 * Add r,g,b to the colors for all images displayed on the map.
	 *
	 * Used for special effects like flashes.
	 */
	void adjust_color_overlay(int r, int g, int b);

	/** Gets the underlying screen object. */
	CVideo& video()
	{
		return video_;
	}

	/** return the screen surface or the surface used for map_screenshot. */
	surface& get_screen_surface()
	{
		return map_screenshot_ ? map_screenshot_surf_ : video_.getSurface();
	}

	virtual bool in_game() const
	{
		return false;
	}
	virtual bool in_editor() const
	{
		return false;
	}

	/** Virtual functions shadowed in game_display. These are needed to generate reports easily, without dynamic
	 * casting. Hope to factor out eventually. */
	virtual const map_location& displayed_unit_hex() const
	{
		return map_location::null_location();
	}

	/** In this case give an obviously wrong answer to fail fast, since this could actually cause a big bug. */
	virtual int playing_side() const
	{
		return -100;
	}

	virtual const std::set<std::string>& observers() const
	{
		static const std::set<std::string> fake_obs = std::set<std::string>();
		return fake_obs;
	}

	/**
	 * mapx is the width of the portion of the display which shows the game area.
	 * Between mapx and x is the sidebar region.
	 */

	const SDL_Rect& minimap_area() const
	{
		return theme_.mini_map_location(video_.screen_area());
	}

	const SDL_Rect& palette_area() const
	{
		return theme_.palette_location(video_.screen_area());
	}

	const SDL_Rect& unit_image_area() const
	{
		return theme_.unit_image_location(video_.screen_area());
	}

	/** Returns the maximum area used for the map regardless to resolution and view size */
	const SDL_Rect& max_map_area() const;

	/** Returns the area used for the map */
	const SDL_Rect& map_area() const;

	/**
	 * Returns the available area for a map, this may differ from the above.
	 * This area will get the background area applied to it.
	 */
	const SDL_Rect& map_outside_area() const
	{
		return map_screenshot_ ? max_map_area() : theme_.main_map_location(video_.screen_area());
	}

	/** Check if the bbox of the hex at x,y has pixels outside the area rectangle. */
	static bool outside_area(const SDL_Rect& area, const int x, const int y);

	/**
	 * Function which returns the width of a hex in pixels,
	 * up to where the next hex starts.
	 * (i.e. not entirely from tip to tip -- use hex_size()
	 * to get the distance from tip to tip)
	 */
	static int hex_width()
	{
		return (zoom_ * 3) / 4;
	}

	/**
	 * Function which returns the size of a hex in pixels
	 * (from top tip to bottom tip or left edge to right edge).
	 */
	static int hex_size()
	{
		return zoom_;
	}

	/** Returns the current zoom factor. */
	double get_zoom_factor() const
	{
		return double(zoom_) / double(game_config::tile_size);
	}

	/**
	 * given x,y co-ordinates of an onscreen pixel, will return the
	 * location of the hex that this pixel corresponds to.
	 * Returns an invalid location if the mouse isn't over any valid location.
	 */
	const map_location hex_clicked_on(int x, int y) const;

	/**
	 * given x,y co-ordinates of a pixel on the map, will return the
	 * location of the hex that this pixel corresponds to.
	 * Returns an invalid location if the mouse isn't over any valid location.
	 */
	const map_location pixel_position_to_hex(int x, int y) const;

	/**
	 * given x,y co-ordinates of the mouse, will return the location of the
	 * hex in the minimap that the mouse is currently over, or an invalid
	 * location if the mouse isn't over the minimap.
	 */
	map_location minimap_location_on(int x, int y);

	const map_location& selected_hex() const
	{
		return selectedHex_;
	}

	const map_location& mouseover_hex() const
	{
		return mouseoverHex_;
	}

	virtual void select_hex(map_location hex);

	virtual void highlight_hex(map_location hex);

	/** Function to invalidate the game status displayed on the sidebar. */
	void invalidate_game_status()
	{
		invalidateGameStatus_ = true;
	}

	/** Functions to get the on-screen positions of hexes. */
	int get_location_x(const map_location& loc) const;
	int get_location_y(const map_location& loc) const;

	/**
	 * Wrapper to return the drawing origin for the specified location in screen coordinates.
	 * Combines @ref get_location_x and @ref get_location_y.
	 *
	 * @param loc                   The map location to look up.
	 */
	SDL_Point get_loc_drawing_origin(const map_location& loc) const;

	/**
	 * Rectangular area of hexes, allowing to decide how the top and bottom
	 * edges handles the vertical shift for each parity of the x coordinate
	 */
	struct rect_of_hexes
	{
		int left;
		int right;
		int top[2]; // for even and odd values of x, respectively
		int bottom[2];

		/**  very simple iterator to walk into the rect_of_hexes */
		struct iterator
		{
			iterator(const map_location& loc, const rect_of_hexes& rect)
				: loc_(loc)
				, rect_(rect)
			{
			}

			/** increment y first, then when reaching bottom, increment x */
			iterator& operator++();
			bool operator==(const iterator& that) const
			{
				return that.loc_ == loc_;
			}

			bool operator!=(const iterator& that) const
			{
				return that.loc_ != loc_;
			}

			const map_location& operator*() const
			{
				return loc_;
			}

			typedef std::forward_iterator_tag iterator_category;
			typedef map_location value_type;
			typedef int difference_type;
			typedef const map_location* pointer;
			typedef const map_location& reference;

		private:
			map_location loc_;
			const rect_of_hexes& rect_;
		};

		typedef iterator const_iterator;

		iterator begin() const;
		iterator end() const;
	};

	/** Return the rectangular area of hexes overlapped by r (r is in screen coordinates) */
	const rect_of_hexes hexes_under_rect(const SDL_Rect& r) const;

	/** Returns the rectangular area of visible hexes */
	const rect_of_hexes get_visible_hexes() const
	{
		return hexes_under_rect(map_area());
	}

	/** Returns true if location (x,y) is covered in shroud. */
	bool shrouded(const map_location& loc) const;

	/** Returns true if location (x,y) is covered in fog. */
	bool fogged(const map_location& loc) const;

	/**
	 * Determines whether a grid should be overlayed on the game board.
	 * (to more clearly show where hexes are)
	 */
	void set_grid(const bool grid)
	{
		grid_ = grid;
	}

	/** Getter for the x,y debug overlay on tiles */
	bool get_draw_coordinates() const
	{
		return draw_coordinates_;
	}

	/** Setter for the x,y debug overlay on tiles */
	void set_draw_coordinates(bool value)
	{
		draw_coordinates_ = value;
	}

	/** Getter for the terrain code debug overlay on tiles */
	bool get_draw_terrain_codes() const
	{
		return draw_terrain_codes_;
	}

	/** Setter for the terrain code debug overlay on tiles */
	void set_draw_terrain_codes(bool value)
	{
		draw_terrain_codes_ = value;
	}

	/** Getter for the number of bitmaps debug overlay on tiles */
	bool get_draw_num_of_bitmaps() const
	{
		return draw_num_of_bitmaps_;
	}

	/** Setter for the terrain code debug overlay on tiles */
	void set_draw_num_of_bitmaps(bool value)
	{
		draw_num_of_bitmaps_ = value;
	}

	/** Save a (map-)screenshot and return whether the operation succeeded. */
	bool screenshot(const std::string& filename, bool map_screenshot = false);

	/** Invalidates entire screen, including all tiles and sidebar. Calls redraw observers. */
	void redraw_everything();

	/** Adds a redraw observer, a function object to be called when redraw_everything is used */
	void add_redraw_observer(std::function<void(display&)> f)
	{
		redraw_observers_.push_back(f);
	}

	/** Clear the redraw observers */
	void clear_redraw_observers()
	{
		redraw_observers_.clear();
	}

	theme& get_theme()
	{
		return theme_;
	}

	void set_theme(config theme_cfg);

	/**
	 * Retrieves a pointer to a theme UI button.
	 *
	 * @note The returned pointer may either be nullptr, meaning the button
	 *       isn't defined by the current theme, or point to a valid
	 *       gui::button object. However, the objects retrieved will be
	 *       destroyed and recreated by draw() method calls. Do *NOT* store
	 *       these pointers for longer than strictly necessary to
	 *       accomplish a specific task before the next screen refresh.
	 */
	std::shared_ptr<gui::button> find_action_button(const std::string& id);
	std::shared_ptr<gui::button> find_menu_button(const std::string& id);

	static gui::button::TYPE string_to_button_type(std::string type);
	void create_buttons();

	void layout_buttons();

	void render_buttons();

	void refresh_report(const std::string& report_name, const config* new_cfg = nullptr);

	void draw_minimap_units();

	/**
	 * Function to invalidate animated terrains and units which may have changed.
	 */
	void invalidate_animations();

	void reset_standing_animations();

	/**
	 * mouseover_hex_overlay_ require a prerendered surface
	 * and is drawn underneath the mouse's location
	 */
	void set_mouseover_hex_overlay(const surface& image)
	{
		mouseover_hex_overlay_ = image;
	}

	void clear_mouseover_hex_overlay()
	{
		mouseover_hex_overlay_ = nullptr;
	}

	/** Toggle to continuously redraw the screen. */
	static void toggle_benchmark();

	/**
	 * Toggle to debug foreground terrain.
	 * Separate background and foreground layer
	 * to better spot any error there.
	 */
	static void toggle_debug_foreground();

	terrain_builder& get_builder()
	{
		return *builder_;
	}

	/** Rebuild all dynamic terrain. */
	void rebuild_all();

	const theme::action* action_pressed();
	const theme::menu* menu_pressed();

	/**
	 * Finds the menu which has a given item in it,
	 * and enables or disables it.
	 */
	void enable_menu(const std::string& item, bool enable);

	void set_diagnostic(const std::string& msg);

	/**
	 * Set/Get whether 'turbo' mode is on.
	 * When turbo mode is on, everything moves much faster.
	 */
	void set_turbo(const bool turbo)
	{
		turbo_ = turbo;
	}

	double turbo_speed() const;

	void set_turbo_speed(const double speed)
	{
		turbo_speed_ = speed;
	}

	/** control unit idle animations and their frequency */
	void set_idle_anim(bool ison)
	{
		idle_anim_ = ison;
	}

	bool idle_anim() const
	{
		return idle_anim_;
	}

	void set_idle_anim_rate(int rate);
	double idle_anim_rate() const
	{
		return idle_anim_rate_;
	}

	void bounds_check_position();
	void bounds_check_position(int& xpos, int& ypos) const;

	/**
	 * Scrolls the display by xmov,ymov pixels.
	 * Invalidation and redrawing will be scheduled.
	 * @return true if the map actually moved.
	 */
	bool scroll(int xmov, int ymov, bool force = false);

	/** Zooms the display in (true) or out (false). */
	bool set_zoom(bool increase);

	/** Sets the display zoom to the specified amount. */
	bool set_zoom(unsigned int amount, const bool validate_value_and_set_index = true);

	static bool zoom_at_max();
	static bool zoom_at_min();

	/** Sets the zoom amount to the default. */
	void set_default_zoom();

	bool view_locked() const
	{
		return view_locked_;
	}

	/** Sets whether the map view is locked (e.g. so the user can't scroll away) */
	void set_view_locked(bool value)
	{
		view_locked_ = value;
	}

	enum SCROLL_TYPE { SCROLL, WARP, ONSCREEN, ONSCREEN_WARP };

	/**
	 * Scroll such that location loc is on-screen.
	 * WARP jumps to loc; SCROLL uses scroll speed;
	 * ONSCREEN only scrolls if x,y is offscreen
	 * force : scroll even if preferences tell us not to,
	 * or the view is locked.
	 */
	void scroll_to_tile(
			const map_location& loc, SCROLL_TYPE scroll_type = ONSCREEN, bool check_fogged = true, bool force = true);

	/**
	 * Scroll such that location loc1 is on-screen.
	 * It will also try to make it such that loc2 is on-screen,
	 * but this is not guaranteed. For ONSCREEN scrolls add_spacing
	 * sets the desired minimum distance from the border in hexes.
	 */
	void scroll_to_tiles(map_location loc1,
			map_location loc2,
			SCROLL_TYPE scroll_type = ONSCREEN,
			bool check_fogged = true,
			double add_spacing = 0.0,
			bool force = true);

	/** Scroll to fit as many locations on-screen as possible, starting with the first. */
	void scroll_to_tiles(const std::vector<map_location>::const_iterator& begin,
			const std::vector<map_location>::const_iterator& end,
			SCROLL_TYPE scroll_type = ONSCREEN,
			bool check_fogged = true,
			bool only_if_possible = false,
			double add_spacing = 0.0,
			bool force = true);

	/** Scroll to fit as many locations on-screen as possible, starting with the first. */
	void scroll_to_tiles(const std::vector<map_location>& locs,
			SCROLL_TYPE scroll_type = ONSCREEN,
			bool check_fogged = true,
			bool only_if_possible = false,
			double add_spacing = 0.0,
			bool force = true)
	{
		scroll_to_tiles(locs.begin(), locs.end(), scroll_type, check_fogged, only_if_possible, add_spacing, force);
	}

	/** Expose the event, so observers can be notified about map scrolling. */
	events::generic_event& scroll_event() const
	{
		return scroll_event_;
	}

	events::generic_event& complete_redraw_event()
	{
		return complete_redraw_event_;
	}

	/** Check if a tile is fully visible on screen. */
	bool tile_fully_on_screen(const map_location& loc) const;

	/** Checks if location @a loc or one of the adjacent tiles is visible on screen. */
	bool tile_nearly_on_screen(const map_location& loc) const;

	map_labels& labels();
	const map_labels& labels() const;

	/** Holds options for calls to function 'announce' (@ref announce). */
	struct announce_options
	{
		/** Lifetime measured in frames. */
		int lifetime;

		/**
		 * An announcement according these options should replace the
		 * previous announce (typical of fast announcing) or not
		 * (typical of movement feedback).
		 */
		bool discard_previous;

		announce_options()
			: lifetime(100)
			, discard_previous(false)
		{
		}
	};

	/** Announce a message prominently. */
	void announce(const std::string& msg,
			const color_t& color = font::GOOD_COLOR,
			const announce_options& options = announce_options());

	/**
	 * Schedule the minimap for recalculation.
	 * Useful if any terrain in the map has changed.
	 */
	void recalculate_minimap()
	{
		minimap_ = nullptr;
		redrawMinimap_ = true;
	}

	/**
	 * Schedule the minimap to be redrawn.
	 * Useful if units have moved about on the map.
	 */
	void redraw_minimap()
	{
		redrawMinimap_ = true;
	}

	virtual const time_of_day& get_time_of_day(const map_location& loc = map_location::null_location()) const;

	virtual bool has_time_area() const
	{
		return false;
	}

	void blindfold(bool flag);

	bool is_blindfolded() const;

	void write(config& cfg) const;

	virtual void handle_event(const SDL_Event&);
	virtual void handle_window_event(const SDL_Event& event);

private:
	void read(const config& cfg);

public:
	/** Init the flag list and the team colors used by ~TC */
	void init_flags();

	/** Rebuild the flag list (not team colors) for a single side. */
	void reinit_flags_for_side(size_t side);

	void reset_reports(reports& reports_object)
	{
		reports_object_ = &reports_object;
	}

private:
	void init_flags_for_side_internal(size_t side, const std::string& side_color);

	int blindfold_ctr_;

protected:
	// TODO sort
	const display_context* dc_;

	std::unique_ptr<halo::manager> halo_man_;

	std::weak_ptr<wb::manager> wb_;

	// =====================================================================================
	// DRAWING CODE
	//=====================================================================================

	/**
	 * Main drawing function.
	 *
	 * This handles drawing everything in this class and is called each draw cycle.
	 * It also calls the various protected virtual functions declared below to allow specialized
	 * behavior in derived classes.
	 *
	 * Inherited from events::sdl_handler.
	 */
	virtual void draw() override final;

	/**
	 * Called at the beginning of each draw cycle.
	 * Derived classes can use this to add extra actions before any drawing takes place.
	 * No action here by default.
	 */
	virtual void pre_draw()
	{
	}

	/**
	 * Called at the end of each draw cycle.
	 * Derived classes can use this to add extra actions after all drawing takes place.
	 * No action here by default.
	 */
	virtual void post_draw()
	{
	}

	/**
	 * Called near the end of a draw operation, derived classes can use this
	 * to render a specific sidebar. Very similar to post_commit.
	 */
	virtual void draw_sidebar()
	{
	}

	/** Redraw all panels associated with the map display */
	void draw_all_panels();

	/** Draws the minimap. */
	void draw_minimap();

private:
	enum TERRAIN_TYPE { FOREGROUND, BACKGROUND };

	/** Draws the visible map hex terrains. Used by @ref draw_gamemap. */
	void draw_visible_hexes(const rect_of_hexes& visible_hexes, TERRAIN_TYPE layer);

	/** Draws the gamemap itself and its various components, such as units, items, fog/shroud, etc. */
	void draw_gamemap();

protected:
	/** Draws the map's hex cursor. No action here by default. */
	virtual void draw_hex_cursor(const map_location& /*loc*/)
	{
	}

	/** Draws various map overlays such as game reachmap. */
	virtual void draw_hex_overlays();

public:
	void draw_debugging_aids();

	/**
	 * Renders a texture directly to the screen (or current rendering target) scaled to the
	 * current zoom factor.
	 *
	 * @param tex              The texture to render.
	 * @param x_pos            The x coordinate to render at.
	 * @param y_pos            The y coordinate to render at.
	 * @param extra_args       Any additional arguments to pass to @ref CVideo::render_copy.
	 *                         This should not contain the texture or source/destination rects.
	 */
	template<typename... T>
	void render_scaled_to_zoom(const texture& tex, const int x_pos, const int y_pos, T&&... extra_args) const
	{
		if(tex.null()) {
			return;
		}

		texture::info info = tex.get_info();

		// Scale the coordinates to the appropriate zoom factor.
		const double zoom_factor = get_zoom_factor();

		SDL_Rect dst {
			x_pos,
			y_pos,
			static_cast<int>(info.w * zoom_factor),
			static_cast<int>(info.h * zoom_factor),
		};

		video_.render_copy(tex, nullptr, &dst, std::forward<T>(extra_args)...);
	}

	/**
	 * Renders a texture directly to the screen (or current rendering target) scaled to the
	 * current zoom factor.
	 *
	 * @param tex              The texture to render.
	 * @param loc              The map location to render at.
	 * @param extra_args       Any additional arguments to pass to @ref CVideo::render_copy.
	 *                         This should not contain the texture or source/destination rects.
	 */
	template<typename... T>
	void render_scaled_to_zoom(const texture& tex, const map_location& loc, T&&... extra_args) const
	{
		if(tex.null()) {
			return;
		}

		SDL_Point origin = get_loc_drawing_origin(loc);

		render_scaled_to_zoom(tex, origin.x, origin.y, std::forward<T>(extra_args)...);
	}

	map_location get_middle_location() const;
	/**
	 * Adds a floating label with the specified text at the given location.
	 *
	 * @param loc                   The map location to draw the label.
	 * @param text                  The label's text.
	 * @param font_size             The label's font size. Will be adjusted by the zoom factor.
	 * @param color                 The label's color.
	 * @param fl_label_id           The label's existing id. If not 0 the given label will be moved as
	 *                              the label with that id will be moved to @a loc.
	 * @param x_in_hex              The relative x location within the hex to draw the label.
	 * @param y_in_hex              The relative y location within the hex to draw the label.
	 *                              Note that (0.5, 0.5) indicates the center of the hex.
	 *
	 * @returns                     The new floating label's id.
	 */
	int draw_text_in_hex(const map_location& loc,
			const std::string& text,
			size_t font_size,
			color_t color,
			int fl_label_id = 0,
			double x_in_hex = 0.5,
			double y_in_hex = 0.5);

protected:
	/** @returns the image type to be used for the passed hex */
	virtual image::TYPE get_image_type(const map_location& loc);

	/**
	 * Get the clipping rectangle for drawing.
	 * Virtual since the editor might use a slightly different approach.
	 */
	virtual const SDL_Rect& get_clip_rect();

	/** Draw the appropriate fog or shroud transition images for a specific hex. */
	void draw_fog_shroud_transition_images(const map_location& loc, image::TYPE image_type);

	void draw_image_for_report(surface& img, SDL_Rect& rect);

	void scroll_to_xy(int screenxpos, int screenypos, SCROLL_TYPE scroll_type, bool force = true);

	static void fill_images_list(const std::string& prefix, std::vector<std::string>& images);

	const std::string& get_variant(const std::vector<std::string>& variants, const map_location& loc) const;

	using exclusive_unit_draw_requests_t = std::map<map_location, std::string>;

	/// map of hexes where only one unit should be drawn, the one identified by the associated id string
	exclusive_unit_draw_requests_t exclusive_unit_draw_requests_;

	CVideo& video_;
	size_t currentTeam_;
	bool dont_show_all_; // const team *viewpoint_;
	int xpos_, ypos_;
	// camera_controller camera_controller_;
	bool view_locked_;
	theme theme_;
	static unsigned int zoom_;
	int zoom_index_;
	static unsigned int last_zoom_;
	const std::unique_ptr<fake_unit_manager> fake_unit_man_;
	const std::unique_ptr<terrain_builder> builder_;
	surface minimap_;
	SDL_Rect minimap_location_;
	bool redrawMinimap_;
	bool grid_;
	int diagnostic_label_;
	double turbo_speed_;
	bool turbo_;
	bool invalidateGameStatus_;
	const std::unique_ptr<map_labels> map_labels_;
	reports* reports_object_;

	/** Event raised when the map is being scrolled */
	mutable events::generic_event scroll_event_;

	/**
	 * notify observers that the screen has been redrawn completely
	 * atm this is used for replay_controller to add replay controls to the standard theme
	 */
	events::generic_event complete_redraw_event_;

	boost::circular_buffer<unsigned> frametimes_; // in milliseconds
	unsigned int fps_counter_;
	std::chrono::seconds fps_start_;
	unsigned int fps_actual_;
	uint32_t last_frame_finished_ = 0u;

	// Not set by the initializer:
	std::map<std::string, SDL_Rect> reportRects_;
	std::map<std::string, surface> reportSurfaces_;
	std::map<std::string, config> reports_;
	std::vector<std::shared_ptr<gui::button>> menu_buttons_, action_buttons_;
	surface mouseover_hex_overlay_;
	// If we're transitioning from one time of day to the next,
	// then we will use these two masks on top of all hexes when we blit.
	surface tod_hex_mask1, tod_hex_mask2;
	std::vector<std::string> fog_images_;
	std::vector<std::string> shroud_images_;

	map_location selectedHex_;
	map_location mouseoverHex_;
	CKey keys_;

	/** Local cache for preferences::animate_map, since it is constantly queried. */
	bool animate_map_;

	/** Local version of preferences::animate_water, used to detect when it's changed. */
	bool animate_water_;

private:
	/** Animated flags for each team */
	std::vector<animated<image::locator>> flags_;

protected:
	// TODO sort
	size_t activeTeam_;

	drawing_queue drawing_queue_;

public:
	drawing_queue& get_drawing_queue()
	{
		return drawing_queue_;
	}

	/**
	 * Add an item to the drawing buffer. You need to update screen on affected area
	 *
	 * @param layer              The layer to draw on.
	 * @param loc                The hex the image belongs to, needed for the
	 *                           drawing order.
	 */

	void drawing_queue_add(const drawing_queue::layer,
			const map_location&,
			int,
			int,
			const surface&,
			const SDL_Rect& clip = SDL_Rect())
	{
		UNUSED(clip);
	}

	void drawing_queue_add(const drawing_queue::layer,
			const map_location&,
			int,
			int,
			const std::vector<surface>&,
			const SDL_Rect& clip = SDL_Rect())
	{
		UNUSED(clip);
	}

protected:

	/** Used to indicate to drawing functions that we are doing a map screenshot */
	bool map_screenshot_;

public: // operations for the arrow framework
	void add_arrow(arrow&);

	void remove_arrow(arrow&);

	/** Called by arrow objects when they change. You should not need to call this directly. */
	void update_arrow(arrow& a);

protected:
	// Tiles lit for showing where unit(s) can reach
	typedef std::map<map_location, unsigned int> reach_map;
	reach_map reach_map_;

	typedef std::multimap<map_location, overlay> overlay_map;

private:
	overlay_map* overlays_;

	/** Handle for the label which displays frames per second. */
	int fps_handle_;
	/** Count work done for the debug info displayed under fps */
	int drawn_hexes_;

	bool idle_anim_;
	double idle_anim_rate_;

	surface map_screenshot_surf_;

	std::vector<std::function<void(display&)>> redraw_observers_;

	/** Debug flag - overlay x,y coords on tiles */
	bool draw_coordinates_;

	/** Debug flag - overlay terrain codes on tiles */
	bool draw_terrain_codes_;

	/** Debug flag - overlay number of bitmaps on tiles */
	bool draw_num_of_bitmaps_;

	typedef std::list<arrow*> arrows_list_t;
	typedef std::map<map_location, arrows_list_t> arrows_map_t;
	/** Maps the list of arrows for each location */
	arrows_map_t arrows_map_;

	tod_color color_adjust_;

public:
	void replace_overlay_map(overlay_map* overlays)
	{
		overlays_ = overlays;
	}

protected:
	static display* singleton_;
};

struct blindfold
{
	blindfold(display& d, bool lock = true)
		: display_(d)
		, blind(lock)
	{
		if(blind) {
			display_.blindfold(true);
		}
	}

	~blindfold()
	{
		unblind();
	}

	void unblind()
	{
		if(blind) {
			display_.blindfold(false);
			blind = false;
		}
	}

private:
	display& display_;
	bool blind;
};
