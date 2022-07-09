/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "events.hpp"
#include "gui/core/top_level_drawable.hpp"
#include "sdl/rect.hpp"
#include "sdl/surface.hpp"

#include <string>

class CVideo;

namespace gui {

// TODO: making widgets TLDs is horrible. Please move everything to GUI2.
class widget : public events::sdl_handler, public gui2::top_level_drawable
{
public:
	const rect& location() const;
	virtual void set_location(const SDL_Rect& rect);
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

	/** Indicate that the widget should be redrawn. */
	void queue_redraw();
	/** Indicate that a specific region of the screen should be redrawn.
	  * This is in absolute drawing coordinates, and is not clipped. */
	void queue_redraw(const rect&);

	void set_dirty(bool dirty=true); // TODO: draw_manager - remove
	bool dirty() const; // TODO: draw_manager - remove
	const std::string& id() const;
	void set_id(const std::string& id);

	void set_help_string(const std::string& str);
	void set_tooltip_string(const std::string& str);

	virtual void process_help_string(int mousex, int mousey) override;
	virtual void process_tooltip_string(int mousex, int mousey) override;

protected:
	widget(CVideo& video, const bool auto_join=true);
	virtual ~widget();

	// During each relocation, this function should be called to register
	// the rectangles the widget needs to refresh automatically
	void bg_register(const SDL_Rect& rect); // TODO: draw_manager - remove
	void bg_restore() const; // TODO: draw_manager - remove
	void bg_restore(const SDL_Rect& rect) const; // TODO: draw_manager - remove
	void bg_update(); // TODO: draw_manager - remove
	void bg_cancel(); // TODO: draw_manager - remove

	CVideo& video() const { return *video_; }

public:
	/* draw_manager interface */

	/** Called by draw_manager to validate layout. */
	virtual void layout() override;
	/** Called by draw_manager when it believes a redraw is necessary. */
	virtual bool expose(const SDL_Rect &region) override;
	/** The current draw location of the display, on the screen. */
	virtual rect screen_location() override { return location(); }

public:
	virtual void draw() override; // TODO: draw_manager - private nonvirtual
protected:
	virtual void draw_contents() {}
	virtual void update_location(const SDL_Rect& rect);

	const SDL_Rect* clip_rect() const;

	// TODO: draw_manager - only things that need events should be handlers
	virtual void handle_event(const SDL_Event&) override {};
	virtual void handle_window_event(const SDL_Event&) override {};
	bool focus_;		// Should user input be ignored?

	bool mouse_locked() const;

	void aquire_mouse_lock();
	void free_mouse_lock();
private:
	void hide_override(bool value = true);

	CVideo* video_;
	std::vector<rect> restorer_;
	rect rect_;
	mutable bool needs_restore_; // Have we drawn ourselves, so that if moved, we need to restore the background?

	enum { UNINIT, HIDDEN, DIRTY, DRAWN } state_;
	bool hidden_override_;
	bool enabled_;
	bool clip_;
	SDL_Rect clip_rect_;

	std::string help_text_;
	std::string tooltip_text_;
	int help_string_;
	std::string id_;

	bool mouse_lock_local_;
	static bool mouse_lock_;

	friend class dialog;
};

}
