/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

#include "gamestatus.hpp"
#include "key.hpp"
#include "map.hpp"
#include "pathfind.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "video.hpp"

#include "SDL.h"

#include <map>
#include <set>
#include <string>

class display
{
public:
	typedef std::map<gamemap::location,unit> unit_map;
	typedef short Pixel;

	display(unit_map& units, CVideo& video,
	        const gamemap& map, const gamestatus& status,
			const std::vector<team>& t);
	~display();

	Pixel rgb(int r, int g, int b) const;

	void scroll(double xmov, double ymov);
	void zoom(double amount);
	void default_zoom();

	enum SCROLL_TYPE { SCROLL, WARP };
	void scroll_to_tile(int x, int y, SCROLL_TYPE scroll_type=SCROLL);
	void scroll_to_tiles(int x1, int y1, int x2, int y2,
	                     SCROLL_TYPE scroll_type=SCROLL);

	void redraw_everything();
	void draw(bool update=true,bool force=false);

	int x() const;
	int mapx() const;
	int y() const;
	SDL_Rect screen_area() const;

	void select_hex(gamemap::location hex);
	void highlight_hex(gamemap::location hex);
	gamemap::location hex_clicked_on(int x, int y);
	gamemap::location minimap_location_on(int x, int y);

	//paths_list must remain valid until it is set again
	void set_paths(const paths* paths_list);

	//route does not have to remain valid after being set
	void set_route(const paths::route* route);

	double get_location_x(const gamemap::location& loc) const;
	double get_location_y(const gamemap::location& loc) const;

	void move_unit(const std::vector<gamemap::location>& path, unit& u);
	bool unit_attack(const gamemap::location& a, const gamemap::location& b,
	                 int damage, const attack_type& attack);
	void draw_tile(int x, int y, SDL_Surface* unit_image=NULL,
	               double alpha=1.0, short blend_to=0);

	CVideo& video() { return screen_; }

	enum IMAGE_TYPE { UNSCALED, SCALED, GREYED, BRIGHTENED };
	SDL_Surface* getImage(const std::string& filename,IMAGE_TYPE type=SCALED);

	//blits a surface with black as alpha
	void blit_surface(int x, int y, SDL_Surface* surface);

	void invalidate_all();
	void invalidate_game_status();
	void invalidate_unit();

	void recalculate_minimap();

	void add_overlay(const gamemap::location& loc, const std::string& image);
	void remove_overlay(const gamemap::location& loc);

	void draw_unit_details(int x, int y, const gamemap::location& loc,
	        const unit& u, SDL_Rect& description_rect, SDL_Rect& profile_rect);

	void update_display();

	void draw_terrain_palette(int x, int y, gamemap::TERRAIN selected);
	gamemap::TERRAIN get_terrain_on(int palx, int paly, int x, int y);

	void set_team(size_t team);

	void set_advancing_unit(const gamemap::location& loc, double amount);

	void lock_updates(bool value);
	bool update_locked() const;

	bool turbo() const;
	void set_turbo(bool turbo);

	void set_grid(bool grid);

	static void debug_highlight(const gamemap::location& loc, double amount);
	static void clear_debug_highlights();

	bool shrouded(int x, int y) const;

private:
	display(const display&);
	void operator=(const display&);

	void move_unit_between(const gamemap::location& a,
					       const gamemap::location& b,
						   const unit& u);

	void draw_unit(int x, int y, SDL_Surface* image,
	               bool reverse, bool upside_down=false,
	               double alpha=1.0, short blendto=0);

	void unit_die(const gamemap::location& loc, SDL_Surface* image=NULL);

	bool unit_attack_ranged(const gamemap::location& a,
	                        const gamemap::location& b,
	                        int damage, const attack_type& attack);

	void draw_sidebar();
	SDL_Rect get_minimap_location(int x, int y, int w, int h);
	void draw_minimap(int x, int y, int w, int h);
	void draw_game_status(int x, int y);

	SDL_Rect gameStatusRect_;
	SDL_Rect unitDescriptionRect_;
	SDL_Rect unitProfileRect_;

	void bounds_check_position();

	std::vector<SDL_Surface*> getAdjacentTerrain(int x, int y, IMAGE_TYPE type);
	SDL_Surface* getTerrain(gamemap::TERRAIN, IMAGE_TYPE type,
	                        int x, int y, const std::string& dir="");

	enum TINT { GREY_IMAGE, BRIGHTEN_IMAGE };
	SDL_Surface* getImageTinted(const std::string& filename, TINT tint);
	SDL_Surface* getMinimap(int w, int h);

	void clearImageCache();

	CVideo& screen_;
	mutable CKey keys_;
	double xpos_, ypos_, zoom_;
	const gamemap& map_;

	gamemap::location selectedHex_;
	gamemap::location mouseoverHex_;

	unit_map& units_;

	std::map<std::string,SDL_Surface*> images_, scaledImages_,
	                                   greyedImages_, brightenedImages_;

	//function which finds the start and end rows on the energy bar image
	//where white pixels are substituted for the colour of the energy
	const std::pair<int,int>& calculate_energy_bar();
	std::pair<int,int> energy_bar_count_;

	SDL_Surface* minimap_;
	bool minimapDecorationsDrawn_;

	const paths* pathsList_;
	paths::route route_;

	const gamestatus& status_;

	bool team_valid() const;

	const std::vector<team>& teams_;

	int lastDraw_;
	int drawSkips_;

	void invalidate(const gamemap::location& loc);
	void invalidate_route();

	std::set<gamemap::location> invalidated_;
	bool invalidateAll_;
	bool invalidateUnit_;
	bool invalidateGameStatus_;

	std::multimap<gamemap::location,std::string> overlays_;

	bool sideBarBgDrawn_;
	int lastTimeOfDay_;

	size_t currentTeam_;

	//used to store a unit that is not drawn, because it's currently
	//being moved or otherwise changed
	gamemap::location hiddenUnit_;

	//used to store any unit that is currently being hit
	gamemap::location hitUnit_;

	//used to store any unit that is dying
	gamemap::location deadUnit_;
	double deadAmount_;

	//used to store any unit that is advancing
	gamemap::location advancingUnit_;
	double advancingAmount_;

	int updatesLocked_;

	bool turbo_, grid_;
	double sidebarScaling_;

	//for debug mode
	static std::map<gamemap::location,double> debugHighlights_;
};

struct update_locker
{
	update_locker(display& d, bool lock=true) : disp(d), unlock(lock) {
		if(lock) {
			disp.lock_updates(true);
		}
	}

	~update_locker() {
		unlock_update();
	}

	void unlock_update() {
		if(unlock) {
			disp.lock_updates(false);
			unlock = false;
		}
	}

private:
	display& disp;
	bool unlock;
};

#endif
