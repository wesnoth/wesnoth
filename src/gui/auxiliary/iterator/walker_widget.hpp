/*
   Copyright (C) 2011 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_ITERATOR_WALKER_WIDGET_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_ITERATOR_WALKER_WIDGET_HPP_INCLUDED

#include "gui/auxiliary/iterator/walker.hpp"

namespace gui2
{

namespace iterator
{

namespace walker
{

/** A walker for a @ref gui2::tcontrol. */
class twidget : public twalker_
{
public:
	/**
	 * Constructor.
	 *
	 * @param widget              The control which the walker is attached to.
	 */
	explicit twidget(gui2::twidget& widget);

	/** Inherited from @ref gui2::iterator::twalker_. */
	virtual tstate next(const tlevel level);

	/** Inherited from @ref gui2::iterator::twalker_. */
	virtual bool at_end(const tlevel level) const;

	/** Inherited from @ref gui2::iterator::twalker_. */
	virtual gui2::twidget* get(const tlevel level);

private:
	/** The control which the walker is attached to. */
	gui2::twidget* widget_;
};

} //  namespace walker

} // namespace iterator

} // namespace gui2

#endif
