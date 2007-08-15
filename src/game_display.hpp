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
#ifndef GAME_DISPLAY_H_INCLUDED
#define GAME_DISPLAY_H_INCLUDED

class config;
class gamestatus;
class team;
class unit;
class unit_map;

#include "display.hpp"
#include "image.hpp"

#include "SDL.h"

#include <map>
#include <set>
#include <string>

// This needs to be separate from display.h because of the static
// singleton member, which will otherwise trigger link failure 
// when building the editor.

class game_display : public display
{
public:
	game_display(unit_map& units, CVideo& video,
			const gamemap& map, const gamestatus& status,
			const std::vector<team>& t, const config& theme_cfg,
			const config& cfg, const config& level);
	~game_display();
	static game_display* get_singleton() { return singleton_ ;}

	//new_turn should be called on every new turn, to update
	//lighting settings.
	void new_turn();

	//this will add r,g,b to the colours for all images displayed on
	//the map. Used for special effects like flashes.
	void adjust_colours(int r, int g, int b);

	//scrolls to the leader of a certain side. This will normally
	//be the playing team.
	void scroll_to_leader(unit_map& units, int side);

	// draw for the game display has to know about units 
	void draw(bool update=true,bool force=false);

	//function to display a location as selected. If a unit is in
	//the location, and there is no unit in the currently
	//highlighted hex, the unit will be displayed in the sidebar.
	virtual void select_hex(gamemap::location hex);

	//function to highlight a location. If a unit is in the
	//location, it will be displayed in the sidebar. Selection is
	//used when a unit has been clicked on, while highlighting is
	//used when a location has been moused over
	virtual void highlight_hex(gamemap::location hex);

	//sets the paths that are currently displayed as available for the unit
	//to move along.  All other paths will be greyed out.
	void highlight_reach(const paths &paths_list);

	//add more paths to highlight.  Print numbers where they overlap.
	//Used only by Show Enemy Moves.
	void highlight_another_reach(const paths &paths_list);

	//reset highlighting of paths.
	void unhighlight_reach();

	//sets the route along which footsteps are drawn to show movement of a
	//unit. If NULL, no route is displayed.
	//route does not have to remain valid after being set
	void set_route(const paths::route* route);

	//function to remove a footstep from a specific location
	void remove_footstep(const gamemap::location& loc);

	//function to float a label above a tile
	void float_label(const gamemap::location& loc, const std::string& text,
	                 int red, int green, int blue);

public:
	//function to return 2 half-hex footsteps images for the given location.
	//only loc is on the current route set by set_route.
	std::vector<surface> footsteps_images(const gamemap::location& loc);

	//draws the movement info (turns available) for a given location
	void draw_movement_info(const gamemap::location& loc);

	//function to invalidate a specific tile for redrawing
	void invalidate(const gamemap::location& loc);

	const gamestatus &get_game_status() { return status_; }
	void draw_report(reports::TYPE report_num);

	//function to invalidate that unit status displayed on the sidebar.
	void invalidate_unit() { invalidateUnit_ = true; }

private:
	//function to invalidate animated terrains which may have changed.
	void invalidate_animations();

	virtual void draw_minimap_units(int x, int y, int w, int h);

public:
	//temporarily place a unit on map (moving: can overlap others)
	void place_temporary_unit(unit &u, const gamemap::location& loc);
	void remove_temporary_unit();

	// set the attack direction indicator
	void set_attack_indicator(const gamemap::location& src, const gamemap::location& dst);
	void clear_attack_indicator();
	//function to get attack direction suffix
	const std::string attack_indicator_direction() const
	{ return gamemap::location::write_direction(
		attack_indicator_src_.get_relative_dir(attack_indicator_dst_)); }

	//functions to add and remove overlays from locations. An overlay is an
	//image that is displayed on top of the tile. One tile may have multiple
	//overlays. remove_overlay will remove all overlays on a tile.
	void add_overlay(const gamemap::location& loc, const std::string& image, const std::string& halo="");
	void remove_overlay(const gamemap::location& loc);

	//function to serialize overlay data
	void write_overlays(config& cfg) const;


	//functions used in the editor.
	//void draw_terrain_palette(int x, int y, terrain_type::TERRAIN selected);
	t_translation::t_letter get_terrain_on(int palx, int paly, int x, int y);

	//set_team sets the team controlled by the player using the computer,
	//and it is this team whose data is displayed in the game status.
	//set_playing_team sets the team whose turn it currently is
	void set_team(size_t team, bool observe=false);
	void set_playing_team(size_t team);
	const std::vector<team>& get_teams() {return teams_;};

	unit_map& get_units() {return units_;};
	const unit_map& get_const_units() const {return units_;};

	//a debug highlight draws a cross on a tile to emphasize
	//something there.  it is used in debug mode, typically to
	//show AI plans.
	static void debug_highlight(const gamemap::location& loc, fixed_t amount);
	static void clear_debug_highlights() { debugHighlights_.clear(); }

	//the viewing team is the team currently viewing the game. The
	//playing team is the team whose turn it is
	size_t viewing_team() const { return currentTeam_; }
	size_t playing_team() const { return activeTeam_; }

	bool team_valid() const { return currentTeam_ < teams_.size(); }
	const std::string current_team_name() const;

	void add_observer(const std::string& name) { observers_.insert(name); }
	void remove_observer(const std::string& name) { observers_.erase(name); }
	const std::set<std::string>& observers() const { return observers_; }

	enum MESSAGE_TYPE { MESSAGE_PUBLIC, MESSAGE_PRIVATE };
	void add_chat_message(const std::string& speaker, int side, const std::string& msg, MESSAGE_TYPE type, bool bell);
	void clear_chat_messages() { prune_chat_messages(true); }

	void begin_game();

	virtual bool in_game() const { return in_game_; }
	void draw_bar(const std::string& image, int xpos, int ypos, size_t height, double filled, const SDL_Color& col, fixed_t alpha);

private:
	game_display(const game_display&);
	void operator=(const game_display&);

	void zoom_redraw_hook() {energy_bar_rects_.clear();}

	void draw_sidebar();
	void draw_game_status();

	//this surface must be freed by the caller
	surface get_flag(const gamemap::location& loc);

	unit_map& units_;

	unit *temp_unit_;
	gamemap::location temp_unit_loc_;

	//locations of the attack direction indicator's parts
	gamemap::location attack_indicator_src_;
	gamemap::location attack_indicator_dst_;

	//function which finds the start and end rows on the energy bar image
	//where white pixels are substituted for the colour of the energy
	const SDL_Rect& calculate_energy_bar(surface surf);
	std::map<surface,SDL_Rect> energy_bar_rects_;

	paths::route route_;

	const gamestatus& status_;

	const std::vector<team>& teams_;

	const config& level_;

	void invalidate_route();

	bool invalidateUnit_;

	struct overlay {
		overlay(const std::string& img, const std::string& halo_img,
		        int handle) : image(img), halo(halo_img),
				halo_handle(handle) {}
		std::string image;
		std::string halo;
		int halo_handle;
	};

	typedef std::multimap<gamemap::location,overlay> overlay_map;

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

	//if we're transitioning from one time of day to the next,
	//then we will use these two masks on top of all hexes when we blit
	surface tod_hex_mask1, tod_hex_mask2;

	//tiles lit for showing where unit(s) can reach
	typedef std::map<gamemap::location,unsigned int> reach_map;
	reach_map reach_map_;
	reach_map reach_map_old_;
	bool reach_map_changed_;
	void process_reachmap_changes();

	//for debug mode
	static std::map<gamemap::location,fixed_t> debugHighlights_;

	//animated flags for each team
	//
	std::vector<animated<image::locator> > flags_;

	static game_display * singleton_;
};

#endif
