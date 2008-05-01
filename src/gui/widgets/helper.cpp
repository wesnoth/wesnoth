/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/helper.hpp"
#include "gui/widgets/settings.hpp"
#include "serialization/string_utils.hpp"

#include "log.hpp"

#include "SDL_ttf.h"

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

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
tpoint& tpoint::operator+=(const tpoint& point)
{ 
	x += point.x; 
	y += point.y;
	return *this;
}

std::ostream &operator<<(std::ostream &stream, const tpoint& point)
{
	stream << point.x << ',' << point.y;
	return stream;
}


int decode_font_style(const std::string& style)
{
	if(style == "bold") {
		return TTF_STYLE_BOLD;
	} else if(style == "italic") {
		return TTF_STYLE_ITALIC;
	} else if(style == "underline") {
		return TTF_STYLE_UNDERLINE;
	} else if(style.empty() || style == "normal") {
		return TTF_STYLE_NORMAL;
	}

	ERR_G << "Unknown style '" << style << "' using 'normal' instead.\n";

	return TTF_STYLE_NORMAL;
}

Uint32 decode_colour(const std::string& colour)
{
	std::vector<std::string> fields = utils::split(colour);

	// make sure we have four fields
	while(fields.size() < 4) fields.push_back("0");

	Uint32 result = 0;
	for(int i = 0; i < 4; ++i) {
		// shift the previous value before adding, since it's a nop on the
		// first run there's no need for an if.
		result = result << 8;
		result |= lexical_cast_default<int>(fields[i]);
	}

	return result;
}

} // namespace gui2

