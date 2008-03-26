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

#include "gui/widgets/text_box.hpp"

#include "log.hpp"

#define DBG_G LOG_STREAM(debug, gui)
#define LOG_G LOG_STREAM(info, gui)
#define WRN_G LOG_STREAM(warn, gui)
#define ERR_G LOG_STREAM(err, gui)

#define DBG_G_D LOG_STREAM(debug, gui_draw)
#define LOG_G_D LOG_STREAM(info, gui_draw)
#define WRN_G_D LOG_STREAM(warn, gui_draw)
#define ERR_G_D LOG_STREAM(err, gui_draw)

#define DBG_G_E LOG_STREAM(debug, gui_event)
#define LOG_G_E LOG_STREAM(info, gui_event)
#define WRN_G_E LOG_STREAM(warn, gui_event)
#define ERR_G_E LOG_STREAM(err, gui_event)

#define DBG_G_P LOG_STREAM(debug, gui_parse)
#define LOG_G_P LOG_STREAM(info, gui_parse)
#define WRN_G_P LOG_STREAM(warn, gui_parse)
#define ERR_G_P LOG_STREAM(err, gui_parse)


namespace gui2 {

void ttext_box::set_width(const unsigned width)
{ 
	// resize canvasses
	canvas_.set_width(width);

	// inherited
	tcontrol::set_width(width);
}

void ttext_box::set_height(const unsigned height) 
{ 
	// resize canvasses
	canvas_.set_height(height);

	// inherited
	tcontrol::set_height(height);
}

void ttext_box::set_label(const t_string& label)
{

	// set label in canvases
	canvas_.set_variable("text", variant(label.str()));

	// inherited
	tcontrol::set_label(label);
}

void ttext_box::mouse_move(tevent_handler&)
{
	DBG_G_E << "Text_box: mouse move.\n"; 

	// if in select mode select text and move cursor
}

void ttext_box::mouse_hover(tevent_handler&)
{
	DBG_G_E << "Text_box: mouse hover.\n"; 
}

void ttext_box::mouse_left_button_down(tevent_handler& event) 
{ 
	DBG_G_E << "Text_box: left mouse button down.\n"; 

	//FIXME place cursor
	//set select  mode

	event.keyboard_capture(this);
	event.mouse_capture();
}

void ttext_box::mouse_left_button_up(tevent_handler&) 
{ 
	// reset select  mode
	DBG_G_E << "Text_box: left mouse button up.\n";
}

void ttext_box::mouse_left_button_double_click(tevent_handler&) 
{ 
	DBG_G_E << "Text_box: left mouse button double click.\n";

	sel_start_ = 0;
	sel_len_ = label().size();

}

void ttext_box::key_press(tevent_handler& event, bool& handled, SDLKey key, SDLMod modifier, Uint16 unicode)
{
	DBG_G_E << "Text_box: key press.\n";
	

	if(unicode >= 32 && unicode != 127) {
		// FIXME this is rather inefficent!!!
		std::string text = label();
		text.insert(text.begin() + sel_start_++, unicode);
		set_label(text);
	}


}

void ttext_box::draw(surface& canvas)
{
	SDL_Rect rect = get_rect();

	DBG_G_D << "Text box: drawing enabled state.\n";
	canvas_.draw(true);
	SDL_BlitSurface(canvas_.surf(), 0, canvas, &rect);

	set_dirty(false);
}

tpoint ttext_box::get_best_size() const
{
	if(definition_ == std::vector<ttext_box_definition::tresolution>::const_iterator()) {
		return tpoint(get_text_box(definition())->default_width, get_text_box(definition())->default_height); 
	} else {
		return tpoint(definition_->default_width, definition_->default_height); 
	}
}

void ttext_box::set_best_size(const tpoint& origin)
{
	resolve_definition();

	set_x(origin.x);
	set_y(origin.y);
	set_width(definition_->default_width);
	set_height(definition_->default_height);
}

void ttext_box::resolve_definition()
{
	if(definition_ == std::vector<ttext_box_definition::tresolution>::const_iterator()) {
		definition_ = get_text_box(definition());

		canvas_= definition_->enabled.canvas;

		// FIXME we need some extra routines since a lot of code will
		// be duplicated here otherwise.
		canvas_.set_variable("text", variant(label()));
	}
}

} //namespace gui2


