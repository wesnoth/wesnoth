/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

class surface;

#include "cursor.hpp"
#include "floating_label.hpp"
#include "tooltips.hpp"
#include "video.hpp"
#include "widgets/button.hpp"

namespace gui
{

extern const int ButtonHPadding;
extern const int ButtonVPadding;
enum DIALOG_RESULT {
	DIALOG_BACK=-7,
	DIALOG_FORWARD=-6,
	CREATE_ITEM =-5,
	DELETE_ITEM=-4,
	ESCAPE_DIALOG=-3, //special return used by WML event dialogs
	CONTINUE_DIALOG=-2,
	CLOSE_DIALOG=-1
	/* results (0..N) reserved for standard button indices */
};

bool in_dialog();

struct dialog_manager : private cursor::setter, private font::floating_label_context {
	dialog_manager();
	~dialog_manager();

private:
	bool reset_to;
};

class dialog_frame :public video2::draw_layering {
public:
	struct dimension_measurements {
		dimension_measurements();
		SDL_Rect interior, exterior, title, button_row;
	};
	class style {
	public:
		style(const std::string& p, int br) : panel(p), blur_radius(br) {}
		std::string	panel;
		int	blur_radius;
	};

	//Static members
	static const int title_border_w, title_border_h;
	static const style default_style;
	static const style message_style;
	static const style preview_style;
	static const style titlescreen_style;

	dialog_frame(CVideo &video, const std::string& title="",
		const style& dialog_style=default_style,
		bool auto_restore=true, std::vector<button*>* buttons=nullptr,
		button* help_button=nullptr);
	~dialog_frame();

	dimension_measurements layout(int x, int y, int w, int h);
	dimension_measurements layout(const SDL_Rect& frame_area);
	void set_layout(dimension_measurements &new_dim) { dim_ = new_dim; }
	dimension_measurements get_layout() const { return dim_; }

	int top_padding() const;
	int bottom_padding() const;

	void draw();

	//called by draw
	void draw_border();
	void draw_background();

	//also called by layout with null param
	SDL_Rect draw_title(CVideo *video);

	void set_dirty(bool dirty = true);

	virtual void handle_event(const SDL_Event&);
	void handle_window_event(const SDL_Event& event);

private:
	void clear_background();

	std::string title_;
	CVideo &video_;
	const style& dialog_style_;
	std::vector<button*>* buttons_;
	button* help_button_;
	surface_restorer* restorer_;
	bool auto_restore_;
	dimension_measurements dim_;
	surface top_, bot_, left_, right_, top_left_, bot_left_, top_right_, bot_right_, bg_;
	bool have_border_;
	bool dirty_;
};

}
