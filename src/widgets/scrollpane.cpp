/*
   Copyright (C) 2004 - 2016 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include <algorithm>

#include "widgets/scrollpane.hpp"

namespace {
class widget_finder {
public:
	widget_finder(gui::widget* w) : w_(w) {}

	bool operator()(const std::pair<int, gui::scrollpane::scrollpane_widget>& p)
	{
		if(p.second.w == w_)
			return true;
		return false;
	}
private:
	gui::widget* w_;
};
}

namespace gui {

scrollpane::scrollpane(CVideo &video) : scrollarea(video), border_(5)
{
	content_pos_.x = 0;
	content_pos_.y = 0;
	update_content_size();
	set_scroll_rate(40);
}

void scrollpane::clear()
{
	content_.clear();
	update_content_size();
}

void scrollpane::set_location(SDL_Rect const& rect)
{
	scrollarea::set_location(rect);
	set_shown_size(client_area().h);
	update_widget_positions();
}

void scrollpane::hide(bool value)
{
	for(widget_map::iterator itor = content_.begin(); itor != content_.end(); ++itor) {
		itor->second.w->hide_override(value);
	}
}

void scrollpane::add_widget(widget* w, int x, int y, int z_order)
{
	if (w == NULL)
		return;

	widget_map::iterator itor = std::find_if(content_.begin(), content_.end(), widget_finder(w));
	if (itor != content_.end())
		return;

	scrollpane_widget spw(w, x, y, z_order);

	w->set_clip_rect(client_area());
	content_.insert(std::pair<int, scrollpane_widget>(z_order, spw));

	position_widget(spw);

	// Recalculates the whole content size
	update_content_size();
}

void scrollpane::remove_widget(widget* w)
{
	widget_map::iterator itor = std::find_if(content_.begin(), content_.end(), widget_finder(w));

	if (itor != content_.end())
		content_.erase(itor);

	update_content_size();
}

void scrollpane::set_inner_location(const SDL_Rect& /*rect*/)
{
	for(widget_map::iterator itor = content_.begin(); itor != content_.end(); ++itor) {
		itor->second.w->set_clip_rect(client_area());
	}
}

void scrollpane::draw()
{
	//draws the scrollpane background
}

void scrollpane::scroll(unsigned int pos)
{
	if (static_cast<int>(pos) == content_pos_.y)
		return;

	content_pos_.y = pos;
	update_widget_positions();
}

void scrollpane::update_widget_positions()
{
	widget_map::iterator itor;
	std::vector<bool> hidden(content_.size());
	int i = 0;
	for(itor = content_.begin(); itor != content_.end(); ++itor) {
		hidden[i++] = (itor->second.w->state_ == HIDDEN);
		itor->second.w->hide();
	}

	for(itor = content_.begin(); itor != content_.end(); ++itor) {
		position_widget(itor->second);
	}

	i = 0;
	for(itor = content_.begin(); itor != content_.end(); ++itor) {
		if (!hidden[i++])
			itor->second.w->hide(false);
	}

	set_dirty();
}

void scrollpane::position_widget(scrollpane_widget& spw)
{
	spw.w->set_location(spw.x + location().x + border_,
			spw.y + location().y - content_pos_.y + border_);
}

SDL_Rect scrollpane::client_area() const
{
	SDL_Rect res;

	res.x = location().x + border_;
	res.y = location().y + border_;
	res.w = location().w > 2 * border_ ? location().w - 2 * border_ : 0;
	res.h = location().h > 2 * border_ ? location().h - 2 * border_ : 0;

	return res;
}

void scrollpane::update_content_size()
{
	int maxx = 0;
	int maxy = 0;

	for(widget_map::iterator itor = content_.begin(); itor != content_.end(); ++itor) {
		if(itor->second.x + itor->second.w->width() > maxx) {
			maxx = itor->second.x + itor->second.w->width();
		}
		if(itor->second.y + itor->second.w->height() > maxy) {
			maxy = itor->second.y + itor->second.w->height();
		}
	}

	content_pos_.w = maxx;
	content_pos_.h = maxy;

	set_full_size(maxy);
	set_shown_size(client_area().h);

	set_dirty();
}

} // namespace gui

