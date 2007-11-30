/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file display.hpp
//!

#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

class config;
class gamestatus;
class team;
class unit;
class unit_map;

#include "builder.hpp"
#include "generic_event.hpp"
#include "image.hpp"
#include "key.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "pathfind.hpp"
#include "reports.hpp"
#include "theme.hpp"
#include "video.hpp"
#include "widgets/button.hpp"

#include "SDL.h"

#include <map>
#include <set>
#include <string>

// map_display and display: classes which take care of
// displaying the map and game-data on the screen.
//
// The display is divided into two main sections:
// - the game area, which displays the tiles of the game board, and units on them,
// - and the side bar, which appears on the right hand side.
// The side bar display is divided into three sections:
// - the minimap, which is displayed at the top right
// - the game status, which includes the day/night image,
//   the turn number, information about the current side,
//   and information about the hex currently moused over (highlighted)
// - the unit status, which displays an image and stats
//   for the current unit.

class display
{
public:
	display(CVideo& video, const gamemap& map, const config& theme_cfg,
			const config& cfg, const config& level);
	virtual ~display();

	static Uint32 rgb(Uint8 red, Uint8 green, Uint8 blue)
		{ return 0xFF000000 | (red << 16) | (green << 8) | blue; }

	// Gets the underlying screen object.
	CVideo& video() { return screen_; }

	virtual bool in_game() const { return false; }

	// the dimensions of the display. x and y are width/height.
	// mapx is the width of the portion of the display which shows the game area.
	// Between mapx and x is the sidebar region.
	int w() const { return screen_.getx(); }	//!< width
	int h() const { return screen_.gety(); }	//!< height
	const SDL_Rect& minimap_area() const
		{ return theme_.mini_map_location(screen_area()); }
	const SDL_Rect& unit_image_area() const
		{ return theme_.unit_image_location(screen_area()); }

	SDL_Rect screen_area() const
		{ const SDL_Rect res = {0,0,w(),h()}; return res; }

	/**
	 * Returns the area used for the map
	 */
	const SDL_Rect& map_area() const;

	/**
	 * Returns the available area for a map, this may differ
	 * from the above. This area will get the background area
	 * applied to it.
	 */
	const SDL_Rect& map_outside_area() const
		{ return theme_.main_map_location(screen_area()); }

	//! Check if pixel x,y is outside specified area.
	bool outside_area(const SDL_Rect& area, const int x,const int y) const;

	//! Function which returns the width of a hex in pixels,
	//! up to where the next hex starts.
	//! (i.e. not entirely from tip to tip -- use hex_size()
	//! to get the distance from tip to tip)
	int hex_width() const { return (zoom_*3)/4; }

	//! Function which returns the size of a hex in pixels
	//! (from top tip to bottom tip or left edge to right edge).
	int hex_size() const { return zoom_; }

	//! Returns the current zoom factor.
	double get_zoom_factor() { return double(zoom_)/double(image::tile_size); }

	// given x,y co-ordinates of an onscreen pixel, will return the
	// location of the hex that this pixel corresponds to.
	// Returns an invalid location if the mouse isn't over any valid location.
	const gamemap::location hex_clicked_on(int x, int y,
		gamemap::location::DIRECTION* nearest_hex=NULL,
		gamemap::location::DIRECTION* second_nearest_hex=NULL) const;

	// given x,y co-ordinates of a pixel on the map, will return the
	// location of the hex that this pixel corresponds to.
	// Returns an invalid location if the mouse isn't over any valid location.
	const gamemap::location pixel_position_to_hex(int x, int y,
		gamemap::location::DIRECTION* nearest_hex=NULL,
		gamemap::location::DIRECTION* second_nearest_hex=NULL) const;

	// given x,y co-ordinates of the mouse, will return the location of the
	// hex in the minimap that the mouse is currently over, or an invalid
	// location if the mouse isn't over the minimap.
	gamemap::location minimap_location_on(int x, int y);

	const gamemap::location& selected_hex() { return selectedHex_; }
	const gamemap::location& mouseover_hex() { return mouseoverHex_; }

	virtual void select_hex(gamemap::location hex);
	virtual void highlight_hex(gamemap::location hex);

	//! Function to invalidate the game status displayed on the sidebar.
	void invalidate_game_status() { invalidateGameStatus_ = true; }

	void get_rect_hex_bounds(SDL_Rect rect, gamemap::location &topleft, gamemap::location &bottomright) const;

	//! Functions to get the on-screen positions of hexes.
	int get_location_x(const gamemap::location& loc) const;
	int get_location_y(const gamemap::location& loc) const;

	//! Returns true if location (x,y) is covered in shroud.
	bool shrouded(const gamemap::location& loc) const
		{return viewpoint_ && viewpoint_->shrouded(loc.x, loc.y);}
	//! Returns true if location (x,y) is covered in fog.
	bool fogged(const gamemap::location& loc) const
		{return viewpoint_ && viewpoint_->fogged(loc.x, loc.y);}

	//! Determines whether a grid should be overlayed on the game board.
	//! (to more clearly show where hexes are)
	void set_grid(const bool grid) { grid_ = grid; }

	//! Returns the locations of 2 hexes
	//! that bind the visible area of the map.
	void get_visible_hex_bounds(gamemap::location &topleft, gamemap::location &bottomright) const;

	//! Make a screenshot and save it in a default location.
	void screenshot();

	//! Invalidates entire screen, including all tiles and sidebar.
	void redraw_everything();

	theme& get_theme() { return theme_; }
	gui::button* find_button(const std::string& id);
	gui::button::TYPE string_to_button_type(std::string type);
	void create_buttons();
	void invalidate_theme() { panelsDrawn_ = false; }

	void refresh_report(reports::TYPE report_num, reports::report report,
		      bool brightened = false);

	// Will be overridden in the display subclass
	virtual void invalidate(const gamemap::location& loc) {invalidated_.insert(loc);};
	virtual void draw_minimap_units() {};

	const gamemap& get_map()const { return map_;}

	// The last action in drawing a tile is adding the overlays.
	// These overlays are drawn in the following order:
	// hex_overlay_			if the drawn location is in the map
	// selected_hex_overlay_	if the drawn location is selected
	// mouseover_hex_overlay_	if the drawn location is underneath the mouse
	//
	// These functions require a prerendered surface.
	// Since they are drawn at the top, they are not influenced by TOD, shroud etc.
	void set_hex_overlay(const gamemap::location& loc, surface image) { hex_overlay_[loc] = image; }
	void clear_hex_overlay(const gamemap::location& loc);

	void set_selected_hex_overlay(const surface& image) { selected_hex_overlay_ = image; }
	void clear_selected_hex_overlay() { selected_hex_overlay_ = NULL; }

	void set_mouseover_hex_overlay(const surface& image) { mouseover_hex_overlay_ = image; }
	void clear_mouseover_hex_overlay() { mouseover_hex_overlay_ = NULL; }

	//! Debug function to toggle the "sunset" mode.
	//! The map area become progressively darker,
	//! except where hexes are refreshed.
	//! delay is the number of frames between each darkening
	//! (0 to toggle).
	void sunset(const size_t delay = 0);

	//! Toogle to continuously redraw the screen.
	void toggle_benchmark();

	//! Draw text on a hex. (0.5, 0.5) is the center.
	//! The font size is adjusted to the zoom factor
	//! and divided by 2 for tiny-gui.
	void draw_text_in_hex(const gamemap::location& loc, const std::string& text,
		size_t font_size, SDL_Color color, double x_in_hex=0.5, double y_in_hex=0.5);

	void flip();

	//! Copy the backbuffer to the framebuffer.
	void update_display();

	//! Rebuild all dynamic terrain.
	void rebuild_all() { builder_.rebuild_all(); }

	//! Draw the image of a unit at a certain location.
	//! x,y: pixel location on screen to draw the unit
	//! image: the image of the unit
	//! reverse: if the unit should be flipped across the x axis
	//! greyscale: used when the unit is stoned
	//! alpha: the merging to use with the background
	//! blendto: blend to this colour using blend_ratio
	//! submerged: the amount of the unit out of 1.0 that is submerged
	//!            (presumably under water) and thus shouldn't be drawn
	void render_unit_image(int x, int y, surface image,
			bool hreverse=false, bool greyscale=false,
			fixed_t alpha=ftofxp(1.0), Uint32 blendto=0,
			double blend_ratio=0, double submerged=0.0,bool vreverse =false);

	const theme::menu* menu_pressed();

	//! Finds the menu which has a given item in it,
	//! and enables or disables it.
	void enable_menu(const std::string& item, bool enable);

	void set_diagnostic(const std::string& msg);

	// Delay routines: use these not SDL_Delay (for --nogui).
	void delay(unsigned int milliseconds) const;

	//! Set/Get whether 'turbo' mode is on.
	//! When turbo mode is on, everything moves much faster.
	void set_turbo(const bool turbo) { turbo_ = turbo; }

	double turbo_speed() const;

	void set_turbo_speed(const double speed) { turbo_speed_ = speed; }

	// control unit idle animations and their frequency
	void set_idle_anim(bool ison) { idle_anim_ = ison; }
	bool idle_anim() const { return idle_anim_; }
	void set_idle_anim_rate(int rate);
	double idle_anim_rate() const { return idle_anim_rate_; }

	//! Add a location to highlight.
	//! Note that this has nothing to do with selecting hexes,
	//! it is pure highlighting. These hexes will be highlighted
	//! slightly darker than the currently selected hex.
	void add_highlighted_loc(const gamemap::location &hex);

	void clear_highlighted_locs();

	void remove_highlighted_loc(const gamemap::location &hex);

	void bounds_check_position();
	void bounds_check_position(int& xpos, int& ypos);

	//! Function to invalidate all tiles.
	void invalidate_all();

	//! Scrolls the display by xmov,ymov pixels.
	//! Invalidation and redrawing will be scheduled.
	void scroll(int xmov, int ymov);

	//! Zooms the display by the specified amount.
	//! Negative values zoom out.
	//! Note the amount should be a multiple of four,
	//! otherwise the images might start to look odd
	//! (hex_width() gets rounding errors).
	void set_zoom(int amount);

	//! Sets the zoom amount to the default.
	void set_default_zoom();

	enum SCROLL_TYPE { SCROLL, WARP, ONSCREEN };

	//! Scroll such that location loc is on-screen.
	//! WARP jumps to loc; SCROLL uses scroll speed;
	//! ONSCREEN only scrolls if x,y is offscreen
	void scroll_to_tile(const gamemap::location& loc, SCROLL_TYPE scroll_type=ONSCREEN, bool check_fogged=true);

	//! Scroll such that location loc1 is on-screen.
	//! It will also try to make it such that loc2 is on-screen,
	//! but this is not guaranteed.
	void scroll_to_tiles(const gamemap::location& loc1, const gamemap::location& loc2,
	                     SCROLL_TYPE scroll_type=ONSCREEN, bool check_fogged=true);

	//! Expose the event, so observers can be notified about map scrolling.
	events::generic_event &scroll_event() const { return _scroll_event; }

	//! Draws invalidated items.
	//! If update is true, will also copy the display to the frame buffer.
	//! If force is true, will not skip frames, even if running behind.
	virtual void draw(bool update=true,bool force=false) = 0;

	map_labels& labels() { return map_labels_; }
	const map_labels& labels() const { return map_labels_; }

	//! Announce a message prominently.
	void announce(const std::string msg,
		       const SDL_Color& colour = font::GOOD_COLOUR);

	//! Schedule the minimap for recalculation.
	//! Useful if any terrain in the map has changed.
	void recalculate_minimap() {minimap_ = NULL; redrawMinimap_ = true; };

	//! Schedule the minimap to be redrawn.
	//! Useful if units have moved about on the map.
	void redraw_minimap() { redrawMinimap_ = true; }

	//! Set what will be shown for the report with type which_report.
	//! Note that this only works for some reports,
	//! i.e. reports that can not be deducted
	//! from the supplied arguments to generate_report,
	//! currently: SELECTED_TERRAIN, EDIT_LEFT_BUTTON_FUNCTION
	void set_report_content(const reports::TYPE which_report, const std::string &content);
	std::map<reports::TYPE, std::string> get_report_contents() {return report_;};

protected:

	/**
	 * Draws the border tile overlay.
	 * The routine determines by itself which border it is on
	 * and draws an overlay accordingly. The definition of the
	 * border is stored in the 'main_map_border' part of the theme.
	 *
	 * @param loc	the map location of the tile
	 * @param xpos	the on-screen pixels x coordinate of the tile
	 * @param ypos	the on-screen pixels y coordinate of the tile
	 */
	virtual void draw_border(const gamemap::location& loc,
		const int xpos, const int ypos);

	void draw_minimap();

	virtual void zoom_redraw_hook() {};

	enum ADJACENT_TERRAIN_TYPE { ADJACENT_BACKGROUND, ADJACENT_FOREGROUND, ADJACENT_FOGSHROUD };

	std::vector<surface> get_terrain_images(const gamemap::location &loc,
					const std::string timeid,
					image::TYPE type,
					ADJACENT_TERRAIN_TYPE terrain_type);

	std::vector<std::string> get_fog_shroud_graphics(const gamemap::location& loc);

	void draw_image_for_report(surface& img, SDL_Rect& rect);

	CVideo& screen_;
	const gamemap& map_;
	const viewpoint *viewpoint_;
	int xpos_, ypos_;
	theme theme_;
	int zoom_;
	int last_zoom_;
	terrain_builder builder_;
	surface minimap_;
	SDL_Rect minimap_location_;
	bool redrawMinimap_;
	bool redraw_background_;
	bool invalidateAll_;
	bool grid_;
	int diagnostic_label_;
	bool panelsDrawn_;
	double turbo_speed_;
	bool turbo_;
	bool invalidateGameStatus_;
	map_labels map_labels_;
	//! Event raised when the map is being scrolled
	mutable events::generic_event _scroll_event;
	//! Holds the tick count for when the next drawing event is scheduled.
	//! Drawing shouldn't occur before this time.
	int nextDraw_;

	// Not set by the initializer:
	SDL_Rect reportRects_[reports::NUM_REPORTS];
	surface reportSurfaces_[reports::NUM_REPORTS];
	reports::report reports_[reports::NUM_REPORTS];
	std::map<reports::TYPE, std::string> report_;
	std::vector<gui::button> buttons_;
	std::set<gamemap::location> invalidated_;
	std::map<gamemap::location, surface> hex_overlay_;
	surface selected_hex_overlay_;
	surface mouseover_hex_overlay_;
	gamemap::location selectedHex_;
	gamemap::location mouseoverHex_;
	std::set<gamemap::location> highlighted_locations_;
	CKey keys_;

	//! Composes and draws the terrains on a tile
	void tile_stack_append(surface surf);
	void tile_stack_append(const std::vector<surface>& surfaces);
	void tile_stack_render(int x, int y);
	void tile_stack_clear() {tile_stack_.clear();};

	//! Helper structure for rendering the terrains.
	struct tblit{
		tblit(const int x, const int y, const surface& surf) :
			x(x),
			y(y),
			surf()
			{}

		int x;                      //!< x screen coordinate to render at
		int y;                      //!< y screen coordinate to render at
		std::vector<surface> surf;  //!< surface(s) to render
	};

	//! The layers to render something on. This value should never be stored
	//! it's the internal drawing order and adding removing and reordering
	//! the layers should be save.
	//! If needed in WML use the name and map that to the enum value.
	enum tdrawing_layer{ 
		//LAYER_TERRAIN_BG,        //! Sample for terrain drawn behind a unit.
		//LAYER_UNIT,              //! Sample for the layer to draw a unit on.
		//LAYER_TERRAIN_FG,        //! Sample for terrain to draw in front of a unit.
		LAYER_LINGER_OVERLAY,      //! The overlay used for the linger mode.
		
		LAYER_LAST_LAYER           //! Don't draw to this layer it's a dummy
		                           //! to size the vector.
		};

	//! * Surfaces are rendered per level in a vector.
	//! * Per level the items are rendered per location these locations are
	//!   stored in the drawing order required for units.
	//! * every location has a vector with surfaces, each with its own screen
	//!   coordinate to render at.
	//! * every vector element has a vector with surfaces to render.
	typedef std::vector<std::map<int /*drawing_order*/, std::vector<tblit> > > tdrawing_buffer;
	tdrawing_buffer drawing_buffer_;

public:

	//! Add an item to the drawing buffer.
	//!
	//! @param layer              The layer to draw on.
	//! @param drawing_order      The order in which to draw, needed for units.
	//! @param blit               The structure to blit.
	void drawing_buffer_add(const tdrawing_layer layer, const int drawing_order, const tblit& blit)
		{ drawing_buffer_[layer][drawing_order].push_back(blit); }

protected:

	//! Draws the drawing_buffer_ and clears it.
	void drawing_buffer_commit();

	//! Clears the drawing buffer.
	void drawing_buffer_clear();

	//! redraw all panels associated with the map display
	void draw_all_panels();

	void invalidate_locations_in_rect(SDL_Rect r);

	//! Strict weak ordering to sort a STL-set of hexes
	//! for drawing using the z-order.
	//! (1000 are just to weight the y compare to x)
	struct ordered_draw : public std::binary_function<gamemap::location, gamemap::location, bool> {
		bool operator()(gamemap::location a, gamemap::location b) {
			return (a.y*2 + a.x%2) * 1024 + a.x < (b.y*2 + b.x%2) * 1024 + b.x;
		}
	};

	//! Invalidate controls and panels when changed
	//! after they have been drawn initially.
	//! Useful for dynamic theme modification.
	bool draw_init();
	void draw_wrap(bool update,bool force,bool changed);

private:
	//! Tile stack for terrain rendering.
	std::vector<surface> tile_stack_;
	//! Handle for the label which displays frames per second.
	int fps_handle_;

	bool idle_anim_;
	double idle_anim_rate_;
};

//! Simplified display class for the editor.
//! It only needs to draw terrain, no units, no fog, etc.
class editor_display : public display
{
public:
	editor_display(CVideo& video, const gamemap& map, const config& theme_cfg,
			const config& cfg, const config& level);

	//! draw() for the editor display.
	//! It only has to know about terrain.
	void draw(bool update=true,bool force=false);

	//! Rebuild the dynamic terrain at the given location.
	void rebuild_terrain(const gamemap::location &loc)
		{ builder_.rebuild_terrain(loc); }
};

#endif

