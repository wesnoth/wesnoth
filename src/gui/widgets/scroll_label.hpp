/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_WIDGETS_SCROLL_LABEL_HPP_INCLUDED
#define GUI_WIDGETS_SCROLL_LABEL_HPP_INCLUDED

#include "gui/widgets/vertical_scrollbar_container.hpp"

namespace gui2 {

class tlabel;
class tspacer;

/** 
 * Label showing a text. 
 *
 * This version shows a scrollbar if the text gets too long and has some
 * scrolling features. In general this widget is slower as the normal label so
 * the normal label should be preferred.
 */
class tscroll_label : public tvertical_scrollbar_container_
{
	friend class tbuilder_scroll_label;
public:
	
	tscroll_label();

	~tscroll_label();

	/** Inherited from tcontrol. */
	void set_label(const t_string& label);

	/** Inherited from tcontainer_. */
	void set_self_active(const bool active) 
		{ state_ = active ? ENABLED : DISABLED; }

	/***** ***** ***** setters / getters for members ***** ****** *****/

	bool get_active() const { return state_ != DISABLED; }

	unsigned get_state() const { return state_; }

private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate { ENABLED, DISABLED, COUNT };

//  It's not needed for now so keep it disabled, no definition exists yet.
//	void set_state(const tstate state);

	/** 
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	tstate state_;

	/**
	 * Helper label.
	 *
	 * The text is rendered on a separate label widget. After rendering this
	 * widget the part which can be shown is shown on the dummy spacer. This
	 * dummy spacer is put in the container so it widget has a size.
	 */
	tlabel* label_;

	/** 
	 * Returns the label widget.
	 *
	 * This always returns the label, regardless of the mode.
	 */
	tlabel* find_label(const bool must_exist = true);
	const tlabel* find_label(const bool must_exist = true) const;

	/**
	 * Returns the spacer which is the replacement for the label
	 */
	tspacer* find_spacer(const bool must_exist = true);
	const tspacer* find_spacer(const bool must_exist = true) const;

	void finalize();

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/** Inherited from tvertical_scrollbar_container_. */
	tpoint content_calculate_best_size() const;

	/** Inherited from tvertical_scrollbar_container_. */
	bool content_can_wrap() const { return true; }

	/** Inherited from tvertical_scrollbar_container_. */
	void content_layout_wrap(const unsigned maximum_width);

	/** Inherited from tvertical_scrollbar_container_. */
	void content_use_vertical_scrollbar(const unsigned maximum_height);

	/** Inherited from tvertical_scrollbar_container_. */
	void content_set_size(const SDL_Rect& rect);

	/***** ***** ***** inherited ****** *****/

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const 
		{ static const std::string type = "scroll_label"; return type; }
#ifndef NEW_DRAW
	/** Inherited from tvertical_scrollbar_container_. */
	void draw_content(surface& surface,  const bool force = false,
	        const bool invalidate_background = false);
#else
//FIXME we can't draw yet.	
#endif
	/** Inherited from tvertical_scrollbar_container_. */
	twidget* content_find_widget(
		const tpoint& coordinate, const bool must_be_active);

	/** Inherited from tvertical_scrollbar_container_. */
	const twidget* content_find_widget(const tpoint& coordinate, 
			const bool must_be_active) const;
};

} // namespace gui2

#endif

