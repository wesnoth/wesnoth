/* $Id$ */
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

SDL_Rect create_rect(const tpoint& origin, const tpoint& size) 
{ 
	return ::create_rect(origin.x, origin.y, size.x, size.y); 
}

std::ostream &operator<<(std::ostream &stream, const tpoint& point)
{
	stream << point.x << ',' << point.y;
	return stream;
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

tcontrol::tcontrol(/*const int x, const int y, const int w, const int h*/) :
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

} // namespace gui2
