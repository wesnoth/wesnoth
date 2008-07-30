/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file action.hpp
//! Editor action classes

#ifndef EDITOR2_ACTION_HPP
#define EDITOR2_ACTION_HPP

#include "action_base.hpp"
#include "editor_map.hpp"
#include "map_fragment.hpp"

#include "../map.hpp"
#include "../terrain.hpp"


namespace editor2 {

/**
 * Replace contents of the entire map,
 * Useful as a fallback undo method when something else would be impractical
 */
class editor_action_whole_map : public editor_action
{
    public:
        editor_action_whole_map(const editor_map& m)
        : m_(m)
        {
        }
        void perform_without_undo(map_context& m) const;
    protected:
        editor_map m_;
};

/**
 * Container action wrapping several actions into one
 */
class editor_action_chain : public editor_action
{
	public:
		explicit editor_action_chain(std::vector<editor_action*> actions)
		: actions_(actions)
		{
		}
		~editor_action_chain();
		editor_action_chain* perform(map_context& m) const;
	    void perform_without_undo(map_context& m) const;
    protected:
        std::vector<editor_action*> actions_;
};

//common base classes for actions with common behaviour

//actions which act on a specified location (and possibly on other locations 
//that can be derived from the staring hex)
class editor_action_location : public editor_action
{
  public:
        editor_action_location(gamemap::location loc)
        : loc_(loc)
        {
        }
    protected:
        gamemap::location loc_;
};

//actions which in addition to acting on a hex, act with one terrain type.
//Mostly paint-related actions.
class editor_action_location_terrain : public editor_action_location
{
    public:
        editor_action_location_terrain(gamemap::location loc, 
			t_translation::t_terrain t)
        : editor_action_location(loc), t_(t)
        {
        }
    protected:
        t_translation::t_terrain t_;
};

class editor_action_area : public editor_action
{
    public:
        editor_action_area(const std::set<gamemap::location>& area)
        : area_(area)
        {
        }
		bool add_location(const gamemap::location& loc);
    protected:
		std::set<gamemap::location> area_;
};

//paste a region into the map.
class editor_action_paste : public editor_action_location
{
    public:
        editor_action_paste(const gamemap::location& loc, const map_fragment& paste)
        : editor_action_location(loc), paste_(paste)
        {
        }
        editor_action_paste* perform(map_context& mc) const;
        void perform_without_undo(map_context& mc) const;
    protected:
        map_fragment paste_;
};

//replace a hex at a given location with a given terrain
//since this is a lot simpler than a brush paint, it is separate at least for now
class editor_action_paint_hex : public editor_action_location_terrain
{
    public:
        editor_action_paint_hex(const gamemap::location& loc, t_translation::t_terrain t)
        : editor_action_location_terrain(loc, t)
        {
        }
        editor_action_paint_hex* perform(map_context& mc) const;            
        void perform_without_undo(map_context& mc) const;
};

class editor_action_paint_area : public editor_action_area
{
    public:
        editor_action_paint_area(const std::set<gamemap::location>& area, 
			t_translation::t_terrain t)
        : editor_action_area(area), t_(t)
        {
        }
        editor_action_paste* perform(map_context& mc) const;
        void perform_without_undo(map_context& mc) const;
    protected:
		t_translation::t_terrain t_;
};

//flood fill
class editor_action_fill : public editor_action_location_terrain
{
    public:
        editor_action_fill(gamemap::location loc, 
			t_translation::t_terrain t)
        : editor_action_location_terrain(loc, t)
        {
        }
        editor_action_paint_area* perform(map_context& mc) const;
        void perform_without_undo(map_context& mc) const;
};

class editor_action_select_xor : public editor_action_area
{
	public:
		editor_action_select_xor(const std::set<gamemap::location>& area)
		: editor_action_area(area)
		{
		}
		editor_action_select_xor* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

class editor_action_select : public editor_action_area
{
	public:
		editor_action_select(const std::set<gamemap::location>& area)
		: editor_action_area(area)
		{
		}
		editor_action_select_xor* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

class editor_action_deselect : public editor_action_area
{
	public:
		editor_action_deselect(const std::set<gamemap::location>& area)
		: editor_action_area(area)
		{
		}
		editor_action_select_xor* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

//resize map (streching / clipping behaviour?)
class editor_action_resize_map : public editor_action
{
	public:
		editor_action_resize_map(int x_size, int y_size, int x_offset, int y_offset)
		: x_size_(x_size), y_size_(y_size), x_offset_(x_offset), y_offset_(y_offset)
		{
		}
		void perform_without_undo(map_context& mc) const;
	protected:
		int x_size_;
		int y_size_;
		int x_offset_;
		int y_offset_;
};

//basic rotations. angle is multiplied by 90 degrees so 2 does a 180 turn
class editor_action_rotate_map : public editor_action
{
	public:
		editor_action_rotate_map(int angle)
		: angle_(angle)
		{
		}
		editor_action_rotate_map* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
	protected:
		int angle_;
};

class editor_action_flip_x : public editor_action
{
	public:
		editor_action_flip_x()
		{
		}
		void perform_without_undo(map_context& mc) const;
};

class editor_action_flip_y : public editor_action
{
	public:
		editor_action_flip_y()
		{
		}
		void perform_without_undo(map_context& mc) const;
		bool require_map_reload() { return true; }
};

//plot a route between two points
class editor_action_plot_route : public editor_action_location_terrain
{
	public:
		editor_action_plot_route(gamemap::location l1, 
			t_translation::t_terrain t, gamemap::location l2)
		: editor_action_location_terrain(l1, t)
		, loc2_(l2)
		{
		}
		editor_action_paste* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
	protected:
		gamemap::location loc2_;
};

} //end namespace editor2

#endif
