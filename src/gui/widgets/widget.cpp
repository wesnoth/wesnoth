/* $id: boilerplate-header.cpp 20001 2007-08-31 19:09:40z soliton $ */
/*
   copyright (c) 2007 - 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/widget.hpp"

#include "filesystem.hpp"
#include "gui/widgets/settings.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

#include <cassert>

#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

namespace gui2{

namespace {
	static bool initialized_ = false;


}

bool init() {
	if(initialized_) {
		return true;
	}
	
	load_settings();

	initialized_ = true;

	return initialized_;
}

#if 0
tcontainer::~tcontainer()
{
	for(std::multimap<std::string, twidget *>::iterator itor = 
			children_.begin(); itor !=  children_.end(); ++itor) {
    
		delete itor->second;
	}
}
twidget* tcontainer::child(const std::string& id)
{
	std::multimap<std::string, twidget *>::iterator itor = children_.find(id);
	if(itor == children_.end()) {
		return 0;
	} else {
		return itor->second;
	}

}

void tcontainer::add_child(twidget *child) 
{ 
	if(child) {
		children_.insert(std::make_pair(child->id(), child)); 
	}
}

bool tcontainer::remove_child(const std::string& id) 
{
	std::multimap<std::string, twidget *>::iterator itor = children_.find(id);
	if(itor == children_.end()) {
		return false;
	} else {
		delete itor->second;
		children_.erase(itor);
		return true;
	}
}
#endif

tsizer::tsizer(const unsigned rows, const unsigned cols, 
		const unsigned default_flags, const unsigned default_border_size) :
	rows_(rows),
	cols_(cols),
	default_flags_(default_flags),
	default_border_size_(default_border_size),
	children_(rows * cols)
{
}

tsizer::~tsizer()
{
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->widget) {
			delete itor->widget;
		}
	}
}

void tsizer::add_child(twidget* widget, const unsigned row, 
		const unsigned col, const unsigned flags, const unsigned border_size) 
{
	assert(row < rows_ && col < cols_);

	tchild& cell = child(row, col);

	// clear old child if any
	if(cell.widget) {
		// free a child when overwriting it
		LOG_GUI << "Child '" << cell.id << "' at cell '" 
			<< row << "," << col << "' will be overwritten and is disposed\n";
		delete cell.widget;
	}

	// copy data
	cell.flags = flags;
	cell.border_size = border_size;
	cell.widget = widget;
	if(cell.widget) {
		// make sure the new child is valid before deferring
		cell.id = cell.widget->id();
//		cell.widget->parent() = this; FIXME enable
	} else {
		cell.id = "";
	}
}

void tsizer::set_rows(const unsigned rows)
{
	if(rows == rows_) {
		return;
	}

	if(!children_.empty()) {
		WRN_GUI << "Resizing a non-empty container may give unexpected problems\n";
	}

	rows_ = rows;
	children_.resize(rows_ * cols_);
}

void tsizer::set_cols(const unsigned cols)
{
	if(cols == cols_) {
		return;
	}

	if(!children_.empty()) {
		WRN_GUI << "Resizing a non-empty container may give unexpected problems\n";
	}

	cols_ = cols;
	children_.resize(cols_ * cols_);
}

void tsizer::remove_child(const unsigned row, const unsigned col)
{
	assert(row < rows_ && col < cols_);

	tchild& cell = child(row, col);

	cell.id = "";
	cell.widget = 0;
}

void tsizer::removed_child(const std::string& id, const bool find_all)
{

	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->id == id) {
			itor->id = "";
			itor->widget = 0;

			if(!find_all) {
				break;
			}
		}
	}
}

void tsizer::layout()
{

	DBG_GUI << __FUNCTION__ << '\n';

	std::vector<unsigned> best_col_width;
	std::vector<unsigned> best_row_height;
	
	// First get the best sizes for all items.
	for(unsigned row = 0; row < rows_; ++row) {
		for(unsigned col = 0; col < cols_; ++col) {

		}
	}

	// Does the best fit if so set the best, else
	// we need to determine the minimum and optimize.

	// FIXME improve
	tpoint start(10, 10);
	for(std::vector<tchild>::iterator itor = children_.begin();
			itor != children_.end(); ++itor) {

		if(itor->widget) {
			tpoint size = itor->widget->get_best_size();

		 	itor->widget->set_x(start.x);
		 	itor->widget->set_y(start.y);

		 	itor->widget->set_width(size.x);
		 	itor->widget->set_height(size.y);

			start.y += 30;
		}
	}
}


tcontrol::tcontrol(/*const int x, const int y, const int w, const int h*/) :
	twidget("") ,
/*	x_(x),
	y_(y),
	w_(w),
	h_(h),
	dirty_(true),
*/	visible_(true),
	label_(),
	tooltip_(),
	help_message_(),
	canvas_(0)
{
}

void tbutton::set_width(const int width)
{ 
	// resize canvasses
	canvas_up_.set_width(width);
	canvas_up_mouse_over_.set_width(width);
	canvas_down_.set_width(width);

	// inherited
	tcontrol::set_width(width);
}

void tbutton::set_height(const int height) 
{ 
	// resize canvasses
	canvas_up_.set_height(height);
	canvas_up_mouse_over_.set_height(height);
	canvas_down_.set_height(height);

	// inherited
	tcontrol::set_height(height);
}

void tbutton::draw(surface& canvas)
{
	// create a dummy config with some objects to draw

	DBG_GUI << "Drawing button\n";
#if 0	
	std::string dummy = 
		"[line]\n"
		"    x1, y1 = 0, 0\n"
		"    x2, y2 = -1, 0\n"
		"    colour = 255, 255, 255, 255\n"
		"    thickness = 1\n"
		"[/line]\n"

		"[line]\n"
		"    x1, y1 = -1, 0\n"
		"    x2, y2 = -1, -1\n"
		"    colour = 255, 255, 255, 255\n"
		"    thickness = 1\n"
		"[/line]\n"

		"[line]\n"
		"    x1, y1 = -1, -1\n"
		"    x2, y2 = 0, -1\n"
		"    colour = 0, 0, 0, 255\n"
		"    thickness = 1\n"
		"[/line]\n"

		"[line]\n"
		"    x1, y1 = 0, -1\n"
		"    x2, y2 = 0, 0\n"
		"    colour = 0, 0, 0, 255\n"
		"    thickness = 1\n"
		"[/line]\n";

	config cfg; 
	read(cfg, dummy);
#endif
//	const config* cfg = button_enabled();

//	canvas_up_.draw(*cfg);

	canvas_up_.draw();

	// now blit the cached image on the screen
	SDL_Rect rect = get_rect();
	SDL_BlitSurface(canvas_up_.surf(), 0, canvas, &rect);
}

}
