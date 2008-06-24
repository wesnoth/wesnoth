/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file window.hpp
//! This file contains the window object, this object is a top level container 
//! which has the event management as well.

#ifndef GUI_WIDGETS_WINDOW_HPP_INCLUDED
#define GUI_WIDGETS_WINDOW_HPP_INCLUDED

#include "gui/widgets/canvas.hpp"
#include "gui/widgets/event_handler.hpp"
#include "gui/widgets/helper.hpp" 
#include "gui/widgets/panel.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/tooltip.hpp"

#include "sdl_utils.hpp"
#include "video.hpp"

#include "tstring.hpp"
#include "config.hpp"
#include "variable.hpp"

#include "events.hpp"
#include "SDL.h"

#include <string>
#include <map>

namespace gui2{

class tdialog;

/**
 * base class of top level items, the only item 
 * which needs to store the final canvase to draw on
 */

// we kunnen de timer gebruiken om een http://www.libsdl.org/cgi/docwiki.cgi/SDL_5fAddTimer
// event aan ons te sturen, oftewel een movemove can dit genereren indien gewenst
//
// mogelijk dit ook gebruiken in de toekomst als aansturing van flip()
class twindow : public tpanel, public tevent_handler
{
public:
	twindow(CVideo& video, 
		const int x, const int y, const int w, const int h,
		const bool automatic_placement, 
		const unsigned horizontal_placement,
		const unsigned vertical_placement,
		const std::string& definition);

	// show the window
	// The flip function is the disp_.flip() if ommitted the video_flip() is used
	int show(const bool restore = true, void* flip_function = 0);

	// layout the window
	void layout(const SDL_Rect position);

	enum tstatus{ NEW, SHOWING, REQUEST_CLOSE, CLOSED };

	void close() { status_ = REQUEST_CLOSE; }

	void set_retval(const int retval, const bool close_window = true)
		{ retval_ = retval; if(close_window) close(); }

	/** Inherited from tevent_executor. */
	void key_press(tevent_handler& event_handler, bool& handled, 
		SDLKey key, SDLMod modifier, Uint16 unicode);

	//! Inherited from tevent_handler.
	twindow& get_window() { return *this; }
	const twindow& get_window() const { return *this; }

	void set_owner(tdialog* owner) { owner_ = owner; }

	/** Inherited from twidget. */
	tdialog* dialog() { return owner_; }

	/** Inherited from tevent_handler. */
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active) 
		{ return tpanel::find_widget(coordinate, must_be_active); }

	/** Inherited from tevent_handler. */
	const twidget* find_widget(const tpoint& coordinate, 
			const bool must_be_active) const
		{ return tpanel::find_widget(coordinate, must_be_active); }

	/** Inherited from tpanel. */
	twidget* find_widget(const std::string& id, const bool must_be_active) 
		{ return tpanel::find_widget(id, must_be_active); }
	
	/** Inherited from tpanel. */
	const twidget* find_widget(const std::string& id, 
			const bool must_be_active) const 
		{ return tpanel::find_widget(id, must_be_active); }
	
	tpoint client_position(const tpoint& screen_position) const
		{ return tpoint(screen_position.x - get_x(), screen_position.y - get_y()); }

	tpoint screen_position(const tpoint& client_position) const
		{ return tpoint(client_position.x + get_x(), client_position.y + get_y()); }


	void window_resize(tevent_handler&, 
		const unsigned new_width, const unsigned new_height);

	//! A window is always active atm so ignore the request.
	void set_active(const bool /*active*/) {}
	bool get_active() const { return true; }
	unsigned get_state() const { return 0; }
	bool needs_full_redraw() const { return false; /* FIXME IMPLEMENT */ }

	//! Inherited from tpanel.
	void draw(surface& surface, const bool force = false, 
			const bool invalidate_background = false);

	//! Gets the coordinates of the client area, for external use the height
	//! and the width are the most interesting things.
	//FIXME this can be removed it the panel defintion inherites from panel defintion
	SDL_Rect get_client_rect() const;

	/** 
	 * Updates the size of the window.
	 * 
	 * If the window has automatic placement set this function recacluates the
	 * window. To be used after creation and after modification or items which
	 * can have different sizes eg listboxes.
	 */
	void recalculate_size();

protected:
private:


	void flip();

	CVideo& video_;

	tstatus status_;

	// return value of the window, 0 default.
	int retval_;

	/** The dialog that owns the window. */
	tdialog* owner_;

	//! When set the form needs a full layout redraw cycle.
	bool need_layout_;

	//! Inherited from tevent_handler.
	void do_show_tooltip(const tpoint& location, const t_string& tooltip);
	void do_remove_tooltip() { tooltip_.set_visible(false); }
	void do_show_help_popup(const tpoint& location, const t_string& help_popup);
	void do_remove_help_popup() { help_popup_.set_visible(false); }

	//! Widget for the tooltip.
	ttooltip tooltip_;

	//! Widget for the help popup FIXME should be thelp_popup.
	ttooltip help_popup_;

	//! Inherited from tcontrol.
	const std::string& get_control_type() const 
		{ static const std::string type = "window"; return type; }


	/** Do we wish to place the widget automatically? */
	const bool automatic_placement_;

	/**
	 * Sets the horizontal placement.
	 *
	 * Only used if automatic_placement_ is true.
	 * The value should be a tgrid placement flag.
	 */
	const unsigned horizontal_placement_;

	/**
	 * Sets the vertical placement.
	 *
	 * Only used if automatic_placement_ is true.
	 * The value should be a tgrid placement flag.
	 */
	const unsigned vertical_placement_;

};

} // namespace gui2

#endif
