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
        editor_action_whole_map(editor_map& m)
        : m_(m)
        {
        }
        editor_action_whole_map* perform(editor_map& m) const;
        void perform_without_undo(editor_map& m) const;
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
		editor_action_chain* perform(editor_map& m) const;
	    void perform_without_undo(editor_map& m) const;
    protected:
        std::vector<editor_action*> actions_;
};

//class editor_action_chain_whole_map : public editor_action_chain
//{
//	public:
//		explicit editor_action_chain_whole_map(std::vector<editor_action*> actions)
//		: editor_action_chain(actions)
//		{
//		}
//		editor_action_whole_map* perform(editor_map& m)
//		{
//			editor_action_whole_map* undo = new editor_action_whole_map(m);
//			perform_without_undo(m);
//			return undo;
//		}
//	
//};

//class editor_action_undo_wrapper : public editor_action
//{
//	public:
//		editor_action_undo_wrapper(editor_action* undo, editor_action* redo)
//		: undo_(undo)
//		, redo_(redo)
//		{
//		}
//		~editor_action_undo_wrapper();
//	protected:
//		
//};

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

//paste a region into the map.
class editor_action_paste : public editor_action_location
{
    public:
        editor_action_paste(const gamemap::location& loc, const map_fragment& paste)
        : editor_action_location(loc), paste_(paste)
        {
        }
        editor_action_paste* perform(editor_map& map) const;
        void perform_without_undo(editor_map& map) const;
    protected:
        map_fragment paste_;
};

//replace a hex at a given location with a given terrain
//since this is a lot simpler than a brush paint, it is separate at least for now
class editor_action_paint_hex : public editor_action_location_terrain
{
    public:
        editor_action_paint_hex(gamemap::location loc, t_translation::t_terrain t)
        : editor_action_location_terrain(loc, t)
        {
        }
        editor_action_paint_hex* perform(editor_map& m) const;            
        void perform_without_undo(editor_map& map) const;
};

class editor_action_paint_area : public editor_action
{
    public:
        editor_action_paint_area(std::set<gamemap::location> area, 
			t_translation::t_terrain t)
        : area_(area), t_(t)
        {
        }
        editor_action_paste* perform(editor_map& map) const;
        void perform_without_undo(editor_map& map) const;
    protected:
		std::set<gamemap::location> area_;
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
        editor_action_paint_area* perform(editor_map& map) const;
        void perform_without_undo(editor_map& map) const;
		void perform_actual(editor_map& map, const std::set<gamemap::location>& to_fill) const;
};

//resize map (streching / clipping behaviour?)
class editor_action_resize_map : public editor_action
{
	public:
		editor_action_resize_map(int to_x_size, int to_y_size)
		: to_x_size_(to_x_size), to_y_size_(to_y_size)
		{
		}
		editor_action_whole_map* perform(editor_map& map) const;
		void perform_without_undo(editor_map& map) const;
	protected:
		int to_x_size_;
		int to_y_size_;
};

//basic rotations. angle is multiplied by 60 degrees so 3 does a 180 turn
class editor_action_rotate_map : public editor_action
{
	public:
		editor_action_rotate_map(int angle)
		: angle_(angle)
		{
		}
		editor_action_rotate_map* perform(editor_map& map) const;
		void perform_without_undo(editor_map& map) const;
	protected:
		int angle_;
};

//mirror. angle is multiplied by 30 degrees
//e.g. 0 is mirror by x axis, 3 is by y axis.
class editor_action_mirror_map : public editor_action
{
	public:
		editor_action_mirror_map(int angle)
		: angle_(angle)
		{
		}
		editor_action_mirror_map* perform(editor_map& map) const;
		void perform_without_undo(editor_map& map) const;
	protected:
		int angle_;
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
		editor_action_paste* perform(editor_map& map) const;
		void perform_without_undo(editor_map& map) const;
	protected:
		gamemap::location loc2_;
};

} //end namespace editor2

#endif
