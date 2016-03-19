/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SLIDER_HPP_INCLUDED
#define SLIDER_HPP_INCLUDED

#include "SDL.h"

#include "sdl/utils.hpp"

#include "widget.hpp"

#include <vector>

namespace gui {

class slider : public widget
{
public:
	slider(CVideo &video, const std::string& image = "buttons/sliders/slider", bool black = false);

	void set_min(int value);
	void set_max(int value);
	void set_value(int value);
	void set_increment(int increment);

	int value() const;
	int max_value() const;
	int min_value() const { return min_; }

	bool value_change();

	virtual void enable(bool new_val=true);

	//VC++ doesn't like a 'using scrollarea::set_location' directive here, so we declare
	//an inline forwarding function instead
	void set_location(int x, int y) { widget::set_location(x,y); }
	virtual void set_location(SDL_Rect const &rect);

protected:
	bool requires_event_focus(const SDL_Event *event=NULL) const;
	virtual void handle_event(const SDL_Event& event);
	virtual void draw_contents();
	virtual bool allow_key_events() { return true; }

private:
	void mouse_motion(const SDL_MouseMotionEvent& event);
	void mouse_down(const SDL_MouseButtonEvent& event);
	void mouse_wheel(const SDL_MouseWheelEvent& event);
	void set_slider_position(int x);
	SDL_Rect slider_area() const;
	surface image_, pressedImage_, activeImage_, disabledImage_;
	SDL_Color line_color_;

	int min_;
	int max_;
	int value_;
	int increment_;

	bool value_change_;

	enum STATE { UNINIT, NORMAL, ACTIVE, CLICKED, DRAGGED };
	STATE state_;
};


template<typename T>
class list_slider : public slider
{
	public:
		list_slider(CVideo &video);
		list_slider(CVideo &video, const std::vector<T> &items);
		void set_items(const std::vector<T> &items);
		bool select_item(const T& item); //use select_item() instead of set_value()
		const T& item_selected() const; //use item_selected() instead of value()
	private:
		std::vector<T> items_;
};

// This is a different style of slider, which doesn't implement key left/right responses
class zoom_slider : public slider
{
public:
	zoom_slider(CVideo &video, const std::string& image = "buttons/sliders/slider", bool black = false);
	virtual bool allow_key_events() { return false; }
};

}

#endif
