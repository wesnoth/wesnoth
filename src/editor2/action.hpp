/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file action.hpp
 * Editor action classes. Some important points:
 * - This is a polymorphic hierarchy of classes, so actions are usually passed around
 *   as editor_action pointers
 * - The pointers can, in general, be null. Always check for null before doing anything.
 *   The helper functions perform_ that take a pointer do that.
 * - The perform() functions can throw when an error occurs. Use smart pointers if you
 *   need to ensure the pointer is deleted.
 */

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
 * Base class for actions that:
 * 1) operate on an area
 * 2) can be used as undo for a click-drag operation
 * 3) can be extended so one undo action undos several actual drag actions
 */
class editor_action_extendable : public editor_action
{
	public:
		editor_action_extendable()
		{
		}
		/**
		 * The crux of the extendable contract. This member function must be
		 * implemented so that the undo behaviour is consistent, exactly the
		 * same as would be with separate undo actions for every part of
		 * the drag.
		 */
		virtual void extend(const editor_map& map, const std::set<map_location>& locs) = 0;
};

/**
 * Container action wrapping several actions into one.
 * The actions are performed in the order they are added,
 * i.e. in the usual iteration order through the container.
 */
class editor_action_chain : public editor_action
{
	public:
		/**
		 * Create an empty action chain
		 */
		editor_action_chain() :
			actions_()
		{
		}

		/**
		 * Create an action chain from a deque of action pointers.
		 * Note: the action chain assumes ownership of the pointers.
		 */
		explicit editor_action_chain(std::deque<editor_action*> actions)
		: actions_(actions)
		{
		}

		/**
		 * Create an action chain by wrapping around a single action pointer.
		 * Note: the action chain assumes ownership of the pointer.
		 */
		explicit editor_action_chain(editor_action* action)
		: actions_(1, action)
		{
		}

		/**
		 * The destructor deletes all the owned action pointers
		 */
		~editor_action_chain();

		/**
		 * Go through the chain and add up all the action counts
		 */
		int action_count() const;

		/**
		 * Add an action at the end of the chain
		 */
		void append_action(editor_action* a);

		/**
		 * Add an action at the beginning of the chain
		 */
		void prepend_action(editor_action* a);

		/**
		 * @return true when there are no actions in the chain. Empty
		 * action chains should usually be discarded as to not keep
		 * "empty" actions around.
		 */
		bool empty() const;

		/**
		 * Remove the last added action and return it, transfering
		 * ownership to the caller
		 */
		editor_action* pop_last_action();

		/**
		 * Remove the first added action and return it, transfering
		 * ownership to the caller
		 */
		editor_action* pop_first_action();

		/**
		 * Perform all the actions in order and create a undo action chain
		 */
		editor_action_chain* perform(map_context& m) const;

		/**
		 * Perform all the actions in order
		 */
		void perform_without_undo(map_context& m) const;

	protected:
		/**
		 * The action pointers owned by this action chain
		 */
		std::deque<editor_action*> actions_;
};


/**
 * Base class for actions which act on a specified location (and possibly on other locations
 * that can be derived from the staring hex)
 */
class editor_action_location : public editor_action
{
	public:
		editor_action_location(map_location loc)
		: loc_(loc)
		{
		}
	protected:
		map_location loc_;
};

/** Base class for actions which in addition to acting on a hex,
 * act with one terrain type, i.e. paint-related actions.
 */
class editor_action_location_terrain : public editor_action_location
{
	public:
		editor_action_location_terrain(map_location loc,
			t_translation::t_terrain t)
		: editor_action_location(loc), t_(t)
		{
		}
	protected:
		t_translation::t_terrain t_;
};

/**
 * Base class for area-affecting actions
 */
class editor_action_area : public editor_action_extendable
{
	public:
		editor_action_area(const std::set<map_location>& area)
		: area_(area)
		{
		}
		bool add_location(const map_location& loc);
		void add_locations(const std::set<map_location>& locs);
		void extend(const editor_map& map, const std::set<map_location>& locs);
	protected:
		std::set<map_location> area_;
};

/**
 * Paste a map fragment into the map. No offset is used.
 */
class editor_action_paste : public editor_action_extendable
{
	public:
		editor_action_paste(const map_fragment& paste, const map_location& offset = map_location(0,0))
		: offset_(offset), paste_(paste)
		{
		}
		editor_action_paste* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
		void extend(const editor_map& map, const std::set<map_location>& locs);
	protected:
		map_location offset_;
		map_fragment paste_;
};

/**
 * Draw -- replace a hex at a given location with a given terrain. Since this is
  * a lot simpler than a brush paint, it is separate at least for now
 * (it is somewhat redundant)
 */
class editor_action_paint_hex : public editor_action_location_terrain
{
	public:
		editor_action_paint_hex(const map_location& loc, t_translation::t_terrain t)
		: editor_action_location_terrain(loc, t)
		{
		}
		editor_action_paint_hex* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

/**
 * Paint the same terrain on a number of locations on the map.
 */
class editor_action_paint_area : public editor_action_area
{
	public:
		editor_action_paint_area(const std::set<map_location>& area,
			t_translation::t_terrain t, bool one_layer=false)
		: editor_action_area(area), t_(t), one_layer_(one_layer)
		{
		}
		editor_action_paste* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
	protected:
		t_translation::t_terrain t_;
		bool one_layer_;
};

/**
 * Flood fill. Somewhat redundand with paint_area.
 */
class editor_action_fill : public editor_action_location_terrain
{
	public:
		editor_action_fill(map_location loc,
			t_translation::t_terrain t, bool one_layer=false)
		: editor_action_location_terrain(loc, t), one_layer_(one_layer)
		{
		}
		editor_action_paint_area* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
	protected:
		bool one_layer_;
};

/**
 * Set starting position action
 */
class editor_action_starting_position : public editor_action_location
{
	public:
		editor_action_starting_position(map_location loc, int player)
		: editor_action_location(loc), player_(player)
		{
		}
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
	protected:
		int player_;
};

/**
 * Select the given locations
 */
class editor_action_select : public editor_action_area
{
	public:
		editor_action_select(const std::set<map_location>& area)
		: editor_action_area(area)
		{
		}
		void extend(const editor_map& map, const std::set<map_location>& locs);
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

/**
 * Deselect the given locations
 */
class editor_action_deselect : public editor_action_area
{
	public:
		editor_action_deselect(const std::set<map_location>& area)
		: editor_action_area(area)
		{
		}
		void extend(const editor_map& map, const std::set<map_location>& locs);
		editor_action* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

/**
 * Select the entire map
 */
class editor_action_select_all : public editor_action
{
	public:
		editor_action_select_all()
		{
		}
		editor_action_deselect* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

/**
 * Clear selection
 */
class editor_action_select_none : public editor_action
{
	public:
		editor_action_select_none()
		{
		}
		editor_action_select* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

/**
 * Invert the selection
 */
class editor_action_select_inverse : public editor_action
{
	public:
		editor_action_select_inverse()
		{
		}
		editor_action_select_inverse* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};

/**
 * Resize the map. The offsets specify, indirectly, the direction of expanding/shrinking,
 * and fill=NONE enables copying of edge terrain instead of filling.
 */
class editor_action_resize_map : public editor_action
{
	public:
		editor_action_resize_map(int x_size, int y_size, int x_offset, int y_offset,
			t_translation::t_terrain fill = t_translation::NONE_TERRAIN)
		: x_size_(x_size), y_size_(y_size), x_offset_(x_offset), y_offset_(y_offset), fill_(fill)
		{
		}
		void perform_without_undo(map_context& mc) const;
	protected:
		int x_size_;
		int y_size_;
		int x_offset_;
		int y_offset_;
		t_translation::t_terrain fill_;
};

/**
 * Basic rotations. angle is multiplied by 90 degrees so 2 does a 180 turn
 * @todo implement
 */
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

class editor_action_apply_mask : public editor_action
{
	public:
		editor_action_apply_mask(const gamemap& mask)
		: mask_(mask)
		{
		}
		void perform_without_undo(map_context& mc) const;
	private:
		gamemap mask_;
};

class editor_action_create_mask : public editor_action
{
	public:
		editor_action_create_mask(const gamemap& target)
		: target_(target)
		{
		}
		void perform_without_undo(map_context& mc) const;
	private:
		gamemap target_;
};

//plot a route between two points
class editor_action_plot_route : public editor_action_location_terrain
{
	public:
		editor_action_plot_route(map_location l1,
			t_translation::t_terrain t, map_location l2)
		: editor_action_location_terrain(l1, t)
		, loc2_(l2)
		{
		}
		editor_action_paste* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
	protected:
		map_location loc2_;
};

/**
 * Randomize terrain in an area
 */
class editor_action_shuffle_area : public editor_action_area
{
	public:
		editor_action_shuffle_area(const std::set<map_location>& area)
		: editor_action_area(area)
		{
		}
		editor_action_paste* perform(map_context& mc) const;
		void perform_without_undo(map_context& mc) const;
};


} //end namespace editor2

#endif
