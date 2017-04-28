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

#ifndef GUI_WIDGETS_AUXILIARY_ITERATOR_WALKER_WIDGET_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_ITERATOR_WALKER_WIDGET_HPP_INCLUDED

#include "gui/auxiliary/iterator/walker.hpp"

namespace gui2
{

namespace iteration
{

namespace walker
{

/** A walker for a @ref gui2::styled_widget. */
class widget : public walker_base
{
public:
	/**
	 * Constructor.
	 *
	 * @param widget              The styled_widget which the walker is attached to.
	 */
	explicit widget(gui2::widget& widget);

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual state_t next(const level level);

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual bool at_end(const level level) const;

	/** Inherited from @ref gui2::iteration::walker_base. */
	virtual gui2::widget* get(const level level);

private:
	/** The styled_widget which the walker is attached to. */
	gui2::widget* widget_;
};

} //  namespace walker

} // namespace iteration

} // namespace gui2

#endif
