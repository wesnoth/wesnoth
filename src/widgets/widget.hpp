/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef WIDGET_HPP_INCLUDED
#define WIDGET_HPP_INCLUDED

#include "events.hpp"
#include "sdl/surface.hpp"

#include <string>

class CVideo;

namespace gui {

class widget : public events::sdl_handler
{
public:
	SDL_Rect const &location() const;
	virtual void set_location(SDL_Rect const &rect);
	void set_location(int x, int y);
	void set_width(int w);
	void set_height(int h);
	void set_measurements(int w, int h);

	int width() const;
	int height() const;

	//focus() may gain the focus if the currently focused handler doesn't require this event
	bool focus(const SDL_Event* event);
	void set_focus(bool focus);

	virtual void hide(bool value = true);
	bool hidden() const;
	virtual void enable(bool new_val = true);
	bool enabled() const;

	void set_clip_rect(const SDL_Rect& rect);

	//Function to set the widget to draw in 'volatile' mode.
	//When in 'volatile' mode, instead of using the normal
	//save-background-redraw-when-dirty procedure, redrawing is done
	//every frame, and then after every frame the area under the widget
	//is restored to the state it was in before the frame. This is useful
	//for drawing widgets with alpha components in volatile settings where
	//the background may change at any time.
	//(e.g. for putting widgets on top of the game map)
	void set_volatile(bool val=true);

	void set_dirty(bool dirty=true);
	bool dirty() const;
	const std::string& id() const;
	void set_id(const std::string& id);

	void set_help_string(const std::string& str);
	void set_tooltip_string(const std::string& str);

	virtual void process_help_string(int mousex, int mousey);
	virtual void process_tooltip_string(int mousex, int mousey);

protected:
	widget(widget const &o);
	widget(CVideo& video, const bool auto_join=true);
	virtual ~widget();

	// During each relocation, this function should be called to register
	// the rectangles the widget needs to refresh automatically
	void bg_register(SDL_Rect const &rect);
	void bg_restore() const;
	void bg_restore(SDL_Rect const &rect) const;
	void bg_update();
	void bg_cancel();

	CVideo& video() const { return *video_; }

	virtual void draw();
	virtual void draw_contents() {}
	virtual void update_location(SDL_Rect const &rect);

	const SDL_Rect* clip_rect() const;
	virtual sdl_handler_vector member_handlers() { return sdl_handler::handler_members(); }

	virtual void handle_event(SDL_Event const &);
	virtual void handle_window_event(SDL_Event const &event);
	bool focus_;		// Should user input be ignored?

	bool mouse_locked() const;

	void aquire_mouse_lock();
	void free_mouse_lock();
private:
	void volatile_draw();
	void volatile_undraw();

	void hide_override(bool value = true);

	CVideo* video_;
	std::vector< surface_restorer > restorer_;
	SDL_Rect rect_;
	mutable bool needs_restore_; // Have we drawn ourselves, so that if moved, we need to restore the background?

	enum { UNINIT, HIDDEN, DIRTY, DRAWN } state_;
	bool hidden_override_;
	bool enabled_;
	bool clip_;
	SDL_Rect clip_rect_;

	bool volatile_;

	std::string help_text_;
	std::string tooltip_text_;
	int help_string_;
	std::string id_;

	bool mouse_lock_local_;
	static bool mouse_lock_;

	friend class dialog;
};

}

#endif
