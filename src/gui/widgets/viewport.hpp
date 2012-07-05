/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_VIEWPORT_HPP_INCLUDED
#define GUI_WIDGETS_VIEWPORT_HPP_INCLUDED

#include "gui/auxiliary/window_builder.hpp"
#include "gui/widgets/widget.hpp"

namespace gui2 {

namespace implementation {
struct tbuilder_viewport;
} // namespace implementation

class tgrid;

class tviewport
	: public twidget
{
	friend struct tviewport_implementation;
public:

	/** @deprecated use the second overload. */
	explicit tviewport(twidget& widget);

private:

	tviewport(
			  const implementation::tbuilder_viewport& builder
			, const tbuilder_widget::treplacements& replacements);

public:

	static tviewport* build(
			  const implementation::tbuilder_viewport& builder
			, const tbuilder_widget::treplacements& replacements);

	~tviewport();

	/** Inherited from twidget. */
	void place(const tpoint& origin, const tpoint& size);

	/** Inherited from twidget. */
	void layout_init(const bool full_initialization);

	/** Inherited from twidget. */
	void impl_draw_children(surface& frame_buffer, int x_offset, int y_offset);

	/** Inherited from twidget. */
	void child_populate_dirty_list(twindow& caller,
			const std::vector<twidget*>& call_stack);

	/** Inherited from twidget. */
	void request_reduce_width(const unsigned maximum_width);

	/** Inherited from twidget. */
	twidget* find_at(const tpoint& coordinate, const bool must_be_active);

	/** Inherited from twidget. */
	const twidget* find_at(
			  const tpoint& coordinate
			, const bool must_be_active) const;

	/** Inherited from twidget. */
	twidget* find(const std::string& id, const bool must_be_active);

	/** Inherited from twidget. */
	const twidget* find(const std::string& id, const bool must_be_active) const;

private:
	/** Inherited from twidget. */
	tpoint calculate_best_size() const;

public:
	/** Inherited from twidget. */
	bool disable_click_dismiss() const;

	/** Inherited from twidget. */
	virtual iterator::twalker_* create_walker();

private:

	twidget& widget_;

	bool owns_widget_;

};

} // namespace gui2

#endif
