/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_ITERATOR_WALKER_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_ITERATOR_WALKER_HPP_INCLUDED

namespace gui2
{

class widget;

namespace iteration
{

/** The walker abstract base class. */
class walker_base
{
public:
	virtual ~walker_base()
	{
	}

	/** The level to walk at. */
	enum level {
		/** Visit the widget itself. */
		self,
		/** Visit its nested grid. */
		internal,
		/** Visit the children of its nested grid. */
		child
	};

	/**
	 * The state of the walker.
	 *
	 * The enum is used to return the state of @ref next.
	 */
	enum state_t {
		/**
		 * When calling next the following it has the following results.
		 *
		 * @pre at_end == false
		 *
		 * @post the next widget became the current one.
		 * @post at_end == false
		 */
		valid

		/**
		 * When calling next the following it has the following results.
		 *
		 * @pre at_end == false
		 *
		 * @post there is no longer a current widget.
		 * @post at_end == true
		 */
		,
		invalid

		/**
		 * When calling next the following it has the following results.
		 *
		 * @pre at_end == true
		 *
		 * @post at_end == true
		 */
		,
		fail
	};

	/**
	 * Make the next widget the current one.
	 *
	 * @param level               Determines on which level the next one should
	 *                            be selected.
	 *
	 * @returns                   The status of the operation.
	 */
	virtual state_t next(const level level) = 0;

	/**
	 * Returns whether the current widget is valid.
	 *
	 * @param level               Determines on which level the test should be
	 *                            executed.
	 *
	 *
	 * @returns                   Whether the current widget is valid.
	 */
	virtual bool at_end(const level level) const = 0;

	/**
	 * Returns a pointer to the current widget.
	 *
	 * @pre                       The following assertion holds:
	 *                            @code at_end(level) == false @endcode
	 *
	 * @param level               Determines from which level should the
	 *                            current widget be returned.
	 *
	 * @returns                   Pointer to the current widget.
	 */
	virtual gui2::widget* get(const level level) = 0;
};

} // namespace iteration

} // namespace gui2

#endif
