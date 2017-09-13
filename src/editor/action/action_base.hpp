/*
   Copyright (C) 2008 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
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
 * Base class for editor actions
 *
 * An action is constructed in response to a user command, then executed on
 * the map. An undo action is returned by default via pointer, caller-owned).
 * It is possible to call an action without creating the undo action.
 * Actions report failure via exceptions.
 * Code that only deals with actions polymorphically should only need to
 * include this header file.
 */

#pragma once

#include "editor/editor_common.hpp"

namespace editor
{
/**
 * Base class for all editor actions. An editor_action object
 * encapsulates the action to be performed on a map, in separation
 * from the user interface or display, and provides a way of reversing
 * its affects by creating an "undo" action.  All the actions can be
 * processed via this base class' virtual functions.
 */
class editor_action
{
public:
	editor_action();
	virtual ~editor_action();

	/**
	 * Action cloning
	 */
	virtual editor_action* clone() const = 0;

	/**
	 * Perform the action, returning an undo action that,
	 * when performed, shall reverse any effects of this
	 * action.  The undo action object is owned by the
	 * caller. Default behavior is to create a whole-map
	 * undo, call the perform_without_undo function and
	 * return the undo object.
	 */
	virtual editor_action* perform(map_context&) const;

	/**
	 * Perform the action without creating an undo action.
	 */
	virtual void perform_without_undo(map_context&) const = 0;

	/**
	 * @return the actual number of actions contained within
	 */
	virtual int action_count() const;

	/**
	 * @return a short name of this action type, defaults to unknown
	 */
	virtual const char* get_name() const
	{
		return "unknown";
	}

	/**
	 * A textual description of the action. For use
	 * e.g. in the undo menu, to have a "Undo: Fill with
	 * Grassland" item rather than just "Undo". Should be
	 * overridden by derived Actions, defaults to a debug
	 * message.
	 */
	virtual std::string get_description() const;

	/**
	 * Debugging aid. Return an unique identifier of this Action.
	 */
	int get_id() const
	{
		return id_;
	}

	/**
	 * Debugging aid. Return number of existing instances of Actions.
	 */
	static int get_instance_count()
	{
		return instance_count_;
	}

private:
	static int next_id_;
	static int instance_count_;

	const int id_;
};

// TODO: add messages etc
struct editor_action_exception : public editor_exception
{
	editor_action_exception(const std::string& msg)
		: editor_exception(msg)
	{
	}
};

} // end namespace editor
